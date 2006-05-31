// D3D9FrameBuffer.hpp
// KlayGE D3D9渲染纹理类 实现文件
// Ver 3.3.0
// 版权所有(C) 龚敏敏, 2003-2006
// Homepage: http://klayge.sourceforge.net
//
// 3.3.0
// 改为FrameBuffer (2006.5.30)
//
// 3.0.0
// 在D3D9FrameBuffer中建立DepthStencil Buffer (2005.10.12)
//
// 2.3.0
// 增加了OnLostDevice和OnResetDevice (2005.2.23)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KlayGE/Util.hpp>
#include <KlayGE/ThrowErr.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/Texture.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/FrameBuffer.hpp>

#define NOMINMAX
#include <d3d9.h>

#include <KlayGE/D3D9/D3D9RenderView.hpp>
#include <KlayGE/D3D9/D3D9FrameBuffer.hpp>

namespace KlayGE
{
	D3D9FrameBuffer::D3D9FrameBuffer()
	{
		left_ = 0;
		top_ = 0;

		viewport_.left	= left_;
		viewport_.top	= top_;
	}

	void D3D9FrameBuffer::Attach(uint32_t att, RenderViewPtr view)
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

			this->CreateDepthStencilBuffer();

			viewport_.width		= width_;
			viewport_.height	= height_;

			isDepthBuffered_ = depthStencilSurface_;

			if (isDepthBuffered_)
			{
				D3DSURFACE_DESC desc;
				depthStencilSurface_->GetDesc(&desc);
				switch (desc.Format)
				{
				case D3DFMT_D15S1:
					depthBits_ = 15;
					stencilBits_ = 1;
					break;

				case D3DFMT_D16:
					depthBits_ = 16;
					stencilBits_ = 0;
					break;

				case D3DFMT_D24X8:
					depthBits_ = 24;
					stencilBits_ = 0;
					break;

				case D3DFMT_D24S8:
					depthBits_ = 24;
					stencilBits_ = 8;
					break;

				case D3DFMT_D32:
					depthBits_ = 32;
					stencilBits_ = 0;
					break;

				default:
					depthBits_ = 0;
					stencilBits_ = 0;
					break;
				}
			}
			else
			{
				depthBits_ = 0;
				stencilBits_ = 0;
			}
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
	
	void D3D9FrameBuffer::Detach(uint32_t att)
	{
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

	boost::shared_ptr<IDirect3DSurface9> D3D9FrameBuffer::D3DRenderSurface(uint32_t n) const
	{
		if (n < clr_views_.size())
		{
			D3D9RenderView const & d3d_view(*checked_cast<D3D9RenderView const *>(clr_views_[n].get()));
			return d3d_view.D3DRenderSurface();
		}
		else
		{
			return boost::shared_ptr<IDirect3DSurface9>();
		}
	}
	
	boost::shared_ptr<IDirect3DSurface9> D3D9FrameBuffer::D3DRenderZBuffer() const
	{
		return depthStencilSurface_;
	}

	void D3D9FrameBuffer::CreateDepthStencilBuffer()
	{
		D3D9RenderEngine& renderEngine(*checked_cast<D3D9RenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance()));
		boost::shared_ptr<IDirect3DDevice9> d3dDevice = renderEngine.D3DDevice();

		IDirect3DSurface9* tempSurf = NULL;
		D3DSURFACE_DESC tempDesc;

		// Get the format of the depth stencil surface.
		d3dDevice->GetDepthStencilSurface(&tempSurf);
		if (tempSurf)
		{
			tempSurf->GetDesc(&tempDesc);
			tempSurf->Release();

			TIF(d3dDevice->CreateDepthStencilSurface(width_, height_, tempDesc.Format,
				tempDesc.MultiSampleType, 0, FALSE, &tempSurf, NULL));
			depthStencilSurface_ = MakeCOMPtr(tempSurf);
		}
	}

	void D3D9FrameBuffer::DoOnLostDevice()
	{
		depthStencilSurface_.reset();
	}
	
	void D3D9FrameBuffer::DoOnResetDevice()
	{
		this->CreateDepthStencilBuffer();
	}
}
