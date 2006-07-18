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

#include <d3d9.h>

#include <KlayGE/D3D9/D3D9RenderEngine.hpp>
#include <KlayGE/D3D9/D3D9RenderView.hpp>
#include <KlayGE/D3D9/D3D9FrameBuffer.hpp>

namespace KlayGE
{
	D3D9FrameBuffer::D3D9FrameBuffer()
	{
		isDepthBuffered_ = false;

		left_ = 0;
		top_ = 0;

		viewport_.left	= left_;
		viewport_.top	= top_;
	}

	ID3D9SurfacePtr D3D9FrameBuffer::D3DRenderSurface(uint32_t n) const
	{
		if (n < clr_views_.size())
		{
			D3D9RenderView const & d3d_view(*checked_cast<D3D9RenderView const *>(clr_views_[n].get()));
			return d3d_view.D3DRenderSurface();
		}
		else
		{
			return ID3D9SurfacePtr();
		}
	}
	
	ID3D9SurfacePtr D3D9FrameBuffer::D3DRenderZBuffer() const
	{
		if (rs_view_)
		{
			D3D9RenderView const & d3d_view(*checked_cast<D3D9RenderView const *>(rs_view_.get()));
			return d3d_view.D3DRenderSurface();
		}
		else
		{
			return ID3D9SurfacePtr();
		}
	}

	void D3D9FrameBuffer::OnBind()
	{
		D3D9RenderEngine& re(*checked_cast<D3D9RenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance()));
		ID3D9DevicePtr d3dDevice = re.D3DDevice();

		for (uint32_t i = 0; i < re.DeviceCaps().max_simultaneous_rts; ++ i)
		{
			TIF(d3dDevice->SetRenderTarget(i, this->D3DRenderSurface(i).get()));
		}

		ID3D9SurfacePtr zBuffer = this->D3DRenderZBuffer();
		bool depth_enable;
		if (zBuffer)
		{
			depth_enable = true;
		}
		else
		{
			depth_enable = false;
		}
		re.SetRenderState(RenderEngine::RST_DepthEnable, depth_enable);
		TIF(d3dDevice->SetDepthStencilSurface(zBuffer.get()));
	}

	void D3D9FrameBuffer::DoOnLostDevice()
	{
	}
	
	void D3D9FrameBuffer::DoOnResetDevice()
	{
	}
}
