// D3D9FrameBuffer.hpp
// KlayGE D3D9渲染纹理类 头文件
// Ver 3.3.0
// 版权所有(C) 龚敏敏, 2003-2006
// Homepage: http://klayge.sourceforge.net
//
// 3.3.0
// 改为FrameBuffer (2006.5.30)
//
// 2.4.0
// 去掉了OnLostDevice和OnResetDevice，改由texture管理 (2005.3.3)
//
// 2.3.0
// 增加了OnLostDevice和OnResetDevice (2005.2.23)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#ifndef _D3D9FRAMEBUFFER_HPP
#define _D3D9FRAMEBUFFER_HPP

#include <KlayGE/FrameBuffer.hpp>
#include <KlayGE/D3D9/D3D9Resource.hpp>

#ifdef KLAYGE_DEBUG
	#pragma comment(lib, "KlayGE_RenderEngine_D3D9_d.lib")
#else
	#pragma comment(lib, "KlayGE_RenderEngine_D3D9.lib")
#endif

namespace KlayGE
{
	class D3D9FrameBuffer : public FrameBuffer, public D3D9Resource
	{
		typedef boost::shared_ptr<IDirect3DSurface9> IDirect3DSurface9Ptr;

	public:
		D3D9FrameBuffer();

		void AttachTexture2D(uint32_t n, TexturePtr texture2D);
		void AttachTextureCube(uint32_t n, TexturePtr textureCube, Texture::CubeFaces face);

		void AttachGraphicsBuffer(uint32_t n, GraphicsBufferPtr gb,
			uint32_t width, uint32_t height);

		void Detach(uint32_t n);

		boost::shared_ptr<IDirect3DSurface9> D3DRenderSurface(uint32_t n) const;
		boost::shared_ptr<IDirect3DSurface9> D3DRenderZBuffer() const;

	private:
		void DoOnLostDevice();
		void DoOnResetDevice();

		void CreateDepthStencilBuffer();

		IDirect3DSurface9Ptr CreateGBSurface(D3DPOOL pool);
		void CopyToGraphicsBuffer(uint32_t n);

		void UpdateParams(uint32_t n, TexturePtr texture);
		void UpdateParams(uint32_t n, GraphicsBufferPtr gb, uint32_t width, uint32_t height);

	private:
		std::vector<boost::shared_ptr<IDirect3DSurface9> > renderSurfaces_;
		boost::shared_ptr<IDirect3DSurface9> depthStencilSurface_;
	};

	typedef boost::shared_ptr<D3D9FrameBuffer> D3D9FrameBufferPtr;
}

#endif			// _D3D9RENDERTEXTURE_HPP
