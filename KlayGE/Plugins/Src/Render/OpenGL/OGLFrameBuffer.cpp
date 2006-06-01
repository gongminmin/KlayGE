// OGLFrameBuffer.cpp
// KlayGE OpenGL渲染到纹理类 实现文件
// Ver 3.3.0
// 版权所有(C) 龚敏敏, 2005-2006
// Homepage: http://klayge.sourceforge.net
//
// 3.3.0
// 改为FrameBuffer (2006.5.30)
//
// 2.8.0
// 初次建立 (2005.8.1)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KlayGE/ThrowErr.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/Texture.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/FrameBuffer.hpp>
#include <KlayGE/Util.hpp>

#include <glloader/glloader.h>

#include <KlayGE/OpenGL/OGLRenderView.hpp>
#include <KlayGE/OpenGL/OGLFrameBuffer.hpp>

namespace KlayGE
{
	OGLFrameBuffer::OGLFrameBuffer()
	{
		left_ = 0;
		top_ = 0;

		glGenFramebuffersEXT(1, &fbo_);
	}

	OGLFrameBuffer::~OGLFrameBuffer()
	{
		glDeleteFramebuffersEXT(1, &fbo_);
	}

	void OGLFrameBuffer::Attach(uint32_t att, RenderViewPtr view)
	{
		BOOST_ASSERT(glIsFramebufferEXT(fbo_));

		switch (att)
		{
		case ATT_Depth:
		case ATT_Stencil:
		case ATT_DepthStencil:
			{
				if (rs_view_)
				{
					this->Detach(att);
				}

				rs_view_ = view;

				isDepthBuffered_ = true;

				switch (view->Format())
				{
				case PF_D16:
					depthBits_ = 16;
					stencilBits_ = 0;
					break;

				case PF_D24X8:
					depthBits_ = 24;
					stencilBits_ = 0;
					break;

				case PF_D24S8:
					depthBits_ = 24;
					stencilBits_ = 8;
					break;

				default:
					depthBits_ = 0;
					stencilBits_ = 0;
					break;
				}
			}
			break;

		default:
			{
				BOOST_ASSERT(att >= ATT_Color0);

				RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
				if (att >= ATT_Color0 + re.DeviceCaps().max_simultaneous_rts)
				{
					THR(E_FAIL);
				}

				uint32_t clr_id = att - ATT_Color0;
				if ((clr_id < clr_views_.size()) && clr_views_[clr_id])
				{
					this->Detach(att);
				}

				if (clr_views_.size() < clr_id + 1)
				{
					clr_views_.resize(clr_id + 1);
				}

				clr_views_[clr_id] = view;
				size_t min_clr_index = clr_id;
				for (size_t i = 0; i < clr_id; ++ i)
				{
					if (clr_views_[i])
					{
						min_clr_index = i;
					}
				}
				if (min_clr_index == clr_id)
				{
					width_ = view->Width();
					height_ = view->Height();
					colorDepth_ = view->Bpp();

					viewport_.width		= width_;
					viewport_.height	= height_;
				}
				else
				{
					BOOST_ASSERT(clr_views_[min_clr_index]->Width() == view->Width());
					BOOST_ASSERT(clr_views_[min_clr_index]->Height() == view->Height());
					BOOST_ASSERT(clr_views_[min_clr_index]->Bpp() == view->Bpp());
				}
			}
			break;
		}

		view->OnAttached(*this, att);

		active_ = true;
	}

	void OGLFrameBuffer::Detach(uint32_t att)
	{
		BOOST_ASSERT(glIsFramebufferEXT(fbo_));

		switch (att)
		{
		case ATT_Depth:
		case ATT_Stencil:
		case ATT_DepthStencil:
			{
				rs_view_.reset();

				isDepthBuffered_ = false;

				depthBits_ = 0;
				stencilBits_ = 0;
			}
			break;

		default:
			{
				RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
				if (att >= ATT_Color0 + re.DeviceCaps().max_simultaneous_rts)
				{
					THR(E_FAIL);
				}

				uint32_t clr_id = att - ATT_Color0;

				BOOST_ASSERT(clr_id < clr_views_.size());

				clr_views_[clr_id]->OnDetached(*this, att);
				clr_views_[clr_id].reset();
			}
			break;
		}
	}
}
