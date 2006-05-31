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
		glGenRenderbuffersEXT(1, &depth_rb_);
	}

	OGLFrameBuffer::~OGLFrameBuffer()
	{
		glDeleteFramebuffersEXT(1, &fbo_);
	}

	void OGLFrameBuffer::Attach(uint32_t att, RenderViewPtr view)
	{
		BOOST_ASSERT(glIsFramebufferEXT(fbo_));
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
		uint32_t min_clr_index = clr_id;
		for (uint32_t i = 0; i < clr_id; ++ i)
		{
			if (clr_views_[i])
			{
				min_clr_index = i;
			}
		}
		if (min_clr_index == clr_id)
		{
			width_ = clr_views_[clr_id]->Width();
			height_ = clr_views_[clr_id]->Height();
			colorDepth_ = clr_views_[clr_id]->Bpp();

			viewport_.width		= width_;
			viewport_.height	= height_;

			isDepthBuffered_ = true;

			glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, depth_rb_);
			glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT,
								GL_DEPTH_COMPONENT16, width_, height_);
			glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT,
								GL_DEPTH_ATTACHMENT_EXT,
								GL_RENDERBUFFER_EXT, depth_rb_);

			depthBits_ = 16;
			stencilBits_ = 0;
		}
		else
		{
			BOOST_ASSERT(clr_views_[min_clr_index]->Width() == view->Width());
			BOOST_ASSERT(clr_views_[min_clr_index]->Height() == view->Height());
			BOOST_ASSERT(clr_views_[min_clr_index]->Bpp() == view->Bpp());
		}

		clr_views_[clr_id]->OnAttached(*this, att);

		active_ = true;
	}

	void OGLFrameBuffer::Detach(uint32_t att)
	{
		BOOST_ASSERT(glIsFramebufferEXT(fbo_));
		BOOST_ASSERT(att >= ATT_Color0);

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
}
