// D3D9RenderVertexStream.hpp
// KlayGE D3D9渲染到顶点流类 实现文件
// Ver 2.8.0
// 版权所有(C) 龚敏敏, 2005
// Homepage: http://klayge.sourceforge.net
//
// 2.8.0
// 初次建立 (2005.7.21)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KlayGE/ThrowErr.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/Util.hpp>

#define NOMINMAX
#include <d3d9.h>

#include <KlayGE/D3D9/D3D9RenderEngine.hpp>
#include <KlayGE/D3D9/D3D9VertexStream.hpp>
#include <KlayGE/D3D9/D3D9RenderVertexStream.hpp>

namespace KlayGE
{
	D3D9RenderVertexStream::D3D9RenderVertexStream()
	{
		left_ = 0;
		top_ = 0;

		viewport_.left		= left_;
		viewport_.top		= top_;
	}

	void D3D9RenderVertexStream::Attach(VertexStreamPtr vs)
	{
		assert(!vs->IsStatic());
		assert(vs->ElementsPerVertex() <= 4);
		assert(dynamic_cast<D3D9VertexStream*>(vs.get()) != NULL);

		RenderEngine const & render_eng = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
		assert(dynamic_cast<D3D9RenderEngine const *>(&render_eng) != NULL);

		d3d_device_ = static_cast<D3D9RenderEngine const &>(render_eng).D3DDevice();

		RenderDeviceCaps const & caps = render_eng.DeviceCaps();
		assert(vs->NumVertices() < caps.max_texture_width * caps.max_texture_height);

		int const buf_size = static_cast<int>(vs_->NumVertices());
		int const sqrt_buf_size = static_cast<int>(std::sqrt(static_cast<float>(buf_size)));

		vs_ = vs;
		width_ = buf_size;
		height_ = 1;
		while (width_ > sqrt_buf_size)
		{
			width_ /= 2;
			height_ *= 2;
		}
		assert(width_ * height_ == buf_size);

		isDepthBuffered_ = false;

		viewport_.width		= width_;
		viewport_.height	= height_;

		switch (vs_->Type())
		{
		case VST_Positions:
		case VST_Normals:
			fmt_ = D3DFMT_A32B32G32R32F;
			colorDepth_ = 128;
			break;

		case VST_Diffuses:
		case VST_Speculars:
			fmt_ = D3DFMT_A8R8G8B8;
			colorDepth_ = 32;
			break;
		
		case VST_TextureCoords0:
		case VST_TextureCoords1:
		case VST_TextureCoords2:
		case VST_TextureCoords3:
		case VST_TextureCoords4:
		case VST_TextureCoords5:
		case VST_TextureCoords6:
		case VST_TextureCoords7:
			fmt_ = D3DFMT_A32B32G32R32F;
			colorDepth_ = 128;
			break;

		default:
			assert(false);
			break;
		}

		this->CreateSurface(D3DPOOL_DEFAULT);
	}

	void D3D9RenderVertexStream::Detach()
	{
		this->CopyToVertexStream();

		vs_.reset();
		render_surf_.reset();
	}

	boost::shared_ptr<IDirect3DSurface9> D3D9RenderVertexStream::D3DRenderSurface() const
	{
		return render_surf_;
	}
	
	boost::shared_ptr<IDirect3DSurface9> D3D9RenderVertexStream::D3DRenderZBuffer() const
	{
		return boost::shared_ptr<IDirect3DSurface9>();
	}

	void D3D9RenderVertexStream::CustomAttribute(std::string const & name, void* pData)
	{
		if (("DDBACKBUFFER" == name) || ("DDFRONTBUFFER" == name))
		{
			IDirect3DSurface9** pSurf = reinterpret_cast<IDirect3DSurface9**>(pData);
			*pSurf = this->D3DRenderSurface().get();

			return;
		}

		if ("D3DZBUFFER" == name)
		{
			IDirect3DSurface9** pSurf = reinterpret_cast<IDirect3DSurface9**>(pData);
			*pSurf = NULL;

			return;
		}

		if ("HWND" == name)
		{
			HWND* pHwnd = reinterpret_cast<HWND*>(pData);
			*pHwnd = NULL;

			return;
		}

		assert(false);
	}

	void D3D9RenderVertexStream::DoOnLostDevice()
	{
		if (vs_)
		{
			IDirect3DSurface9Ptr sys_mem_surf = this->CreateSurface(D3DPOOL_SYSTEMMEM);

			TIF(d3d_device_->GetRenderTargetData(render_surf_.get(), sys_mem_surf.get()));
			render_surf_ = sys_mem_surf;
		}
	}
	
	void D3D9RenderVertexStream::DoOnResetDevice()
	{
		if (vs_)
		{
			RenderEngine const & render_eng = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
			assert(dynamic_cast<D3D9RenderEngine const *>(&render_eng) != NULL);

			d3d_device_ = static_cast<D3D9RenderEngine const &>(render_eng).D3DDevice();

			IDirect3DSurface9Ptr default_surf = this->CreateSurface(D3DPOOL_DEFAULT);

			TIF(D3DXLoadSurfaceFromSurface(render_surf_.get(), NULL, NULL,
				default_surf.get(), NULL, NULL, D3DX_FILTER_NONE, 0));
			render_surf_ = default_surf;
		}
	}

	D3D9RenderVertexStream::IDirect3DSurface9Ptr D3D9RenderVertexStream::CreateSurface(D3DPOOL pool)
	{
		IDirect3DSurface9* surface;
		TIF(d3d_device_->CreateOffscreenPlainSurface(width_, height_, fmt_, pool, &surface, NULL));
		return MakeCOMPtr(surface);
	}

	void D3D9RenderVertexStream::CopyToVertexStream()
	{
		D3DLOCKED_RECT locked_rect;
		TIF(render_surf_->LockRect(&locked_rect, NULL, D3DLOCK_NOSYSLOCK | D3DLOCK_READONLY));

		if (D3DFMT_A8R8G8B8 == fmt_)
		{
			std::vector<uint32_t> vertices(vs_->NumVertices());

			uint32_t* src = static_cast<uint32_t*>(locked_rect.pBits);
			std::vector<uint32_t>::iterator dst = vertices.begin();
			for (int i = 0; i < height_; ++ i)
			{
				std::copy(src, src + width_, dst);

				src += locked_rect.Pitch / sizeof(uint32_t);
				dst += width_;
			}

			vs_->Assign(&vertices[0], vs_->NumVertices());
		}
		else
		{
			assert(D3DFMT_A32B32G32R32F == fmt_);

			std::vector<float> vertices(vs_->NumVertices());

			float* src = static_cast<float*>(locked_rect.pBits);
			std::vector<float>::iterator dst = vertices.begin();
			for (int i = 0; i < height_; ++ i)
			{
				std::copy(src, src + width_ * 4, dst);

				src += locked_rect.Pitch / sizeof(float);
				dst += width_ * 4;
			}

			vs_->Assign(&vertices[0], vs_->NumVertices());
		}

		render_surf_->UnlockRect();
	}
}
