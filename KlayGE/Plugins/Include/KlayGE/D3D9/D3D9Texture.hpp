// D3D9Texture.hpp
// KlayGE D3D9纹理类 头文件
// Ver 3.0.0
// 版权所有(C) 龚敏敏, 2003-2005
// Homepage: http://klayge.sourceforge.net
//
// 3.0.0
// 去掉了ZBuffer (2005.10.12)
//
// 2.7.0
// 增加了AddressingMode, Filtering和Anisotropy (2005.6.27)
// 增加了MaxMipLevel和MipMapLodBias (2005.6.28)
//
// 2.4.0
// 改为派生自D3D9Resource (2005.3.3)
// 增加了D3DBaseTexture (2005.3.7)
// 增加了1D/2D/3D/cube的支持 (2005.3.8)
//
// 2.3.0
// 增加了CopyToMemory (2005.2.6)
// 增加了OnLostDevice和OnResetDevice (2005.2.23)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#ifndef _D3D9TEXTURE_HPP
#define _D3D9TEXTURE_HPP

#include <boost/smart_ptr.hpp>

#define NOMINMAX
#include <d3d9.h>
#include <d3dx9.h>
#include <dxerr9.h>

#include <KlayGE/D3D9/D3D9Typedefs.hpp>
#include <KlayGE/D3D9/D3D9Resource.hpp>
#include <KlayGE/D3D9/D3D9RenderView.hpp>

#ifdef KLAYGE_DEBUG
	#pragma comment(lib, "KlayGE_RenderEngine_D3D9_d.lib")
#else
	#pragma comment(lib, "KlayGE_RenderEngine_D3D9.lib")
#endif

namespace KlayGE
{
	class D3D9Texture : public Texture, public D3D9Resource
	{
	public:
		explicit D3D9Texture(TextureType type);
		virtual ~D3D9Texture();

		std::wstring const & Name() const;

		virtual uint32_t Width(int level) const;
		virtual uint32_t Height(int level) const;
		virtual uint32_t Depth(int level) const;

		virtual void CopyToMemory1D(int level, void* data);
		virtual void CopyToMemory2D(int level, void* data);
		virtual void CopyToMemory3D(int level, void* data);
		virtual void CopyToMemoryCube(CubeFaces face, int level, void* data);

		virtual void CopyMemoryToTexture1D(int level, void* data, PixelFormat pf,
			uint32_t dst_width, uint32_t dst_xOffset, uint32_t src_widtht);
		virtual void CopyMemoryToTexture2D(int level, void* data, PixelFormat pf,
			uint32_t dst_width, uint32_t dst_height, uint32_t dst_xOffset, uint32_t dst_yOffset,
			uint32_t src_width, uint32_t src_height);
		virtual void CopyMemoryToTexture3D(int level, void* data, PixelFormat pf,
			uint32_t dst_width, uint32_t dst_height, uint32_t dst_depth,
			uint32_t dst_xOffset, uint32_t dst_yOffset, uint32_t dst_zOffset,
			uint32_t src_width, uint32_t src_height, uint32_t src_depth);
		virtual void CopyMemoryToTextureCube(CubeFaces face, int level, void* data, PixelFormat pf,
			uint32_t dst_width, uint32_t dst_height, uint32_t dst_xOffset, uint32_t dst_yOffset,
			uint32_t src_width, uint32_t src_height);

		using Texture::Usage;
		void Usage(TextureUsage usage);

		ID3D9BaseTexturePtr D3DBaseTexture() const
			{ return d3dBaseTexture_; }

	protected:
		void CopySurfaceToMemory(boost::shared_ptr<IDirect3DSurface9> const & surface, void* data);

	protected:
		ID3D9DevicePtr			d3dDevice_;
		ID3D9BaseTexturePtr	d3dBaseTexture_;
	};

	typedef boost::shared_ptr<D3D9Texture> D3D9TexturePtr;


	class D3D9Texture1D : public D3D9Texture
	{
	public:
		D3D9Texture1D(uint32_t width, uint16_t numMipMaps, PixelFormat format);

		uint32_t Width(int level) const;

		void CopyToTexture(Texture& target);
		
		void CopyToMemory1D(int level, void* data);

		void CopyMemoryToTexture1D(int level, void* data, PixelFormat pf,
			uint32_t dst_width, uint32_t dst_xOffset, uint32_t src_widtht);

		void BuildMipSubLevels();

		D3D9RenderViewPtr CreateRenderView(int level);

		ID3D9TexturePtr D3DTexture1D() const
			{ return d3dTexture1D_; }

	private:
		void DoOnLostDevice();
		void DoOnResetDevice();

		void QueryBaseTexture();
		void UpdateParams();

		ID3D9TexturePtr CreateTexture1D(uint32_t usage, D3DPOOL pool);

	private:
		ID3D9TexturePtr			d3dTexture1D_;

		std::vector<uint32_t>	widths_;

		bool auto_gen_mipmaps_;
	};

	class D3D9Texture2D : public D3D9Texture
	{
	public:
		D3D9Texture2D(uint32_t width, uint32_t height, uint16_t numMipMaps, PixelFormat format);

		uint32_t Width(int level) const;
		uint32_t Height(int level) const;

		void CopyToTexture(Texture& target);
		
		void CopyToMemory2D(int level, void* data);

		void CopyMemoryToTexture2D(int level, void* data, PixelFormat pf,
			uint32_t dst_width, uint32_t dst_height, uint32_t dst_xOffset, uint32_t dst_yOffset,
			uint32_t src_width, uint32_t src_height);

		void BuildMipSubLevels();

		D3D9RenderViewPtr CreateRenderView(int level);

		ID3D9TexturePtr D3DTexture2D() const
			{ return d3dTexture2D_; }

	private:
		void DoOnLostDevice();
		void DoOnResetDevice();

		void QueryBaseTexture();
		void UpdateParams();

		ID3D9TexturePtr CreateTexture2D(uint32_t usage, D3DPOOL pool);

	private:
		ID3D9TexturePtr			d3dTexture2D_;

		std::vector<uint32_t>	widths_;
		std::vector<uint32_t>	heights_;

		bool auto_gen_mipmaps_;
	};

	class D3D9Texture3D : public D3D9Texture
	{
	public:
		D3D9Texture3D(uint32_t width, uint32_t height, uint32_t depth, uint16_t numMipMaps, PixelFormat format);

		uint32_t Width(int level) const;
		uint32_t Height(int level) const;
		uint32_t Depth(int level) const;

		void CopyToTexture(Texture& target);
		
		void CopyToMemory3D(int level, void* data);

		void CopyMemoryToTexture3D(int level, void* data, PixelFormat pf,
			uint32_t dst_width, uint32_t dst_height, uint32_t dst_depth,
			uint32_t dst_xOffset, uint32_t dst_yOffset, uint32_t dst_zOffset,
			uint32_t src_width, uint32_t src_height, uint32_t src_depth);

		void BuildMipSubLevels();

		D3D9RenderViewPtr CreateRenderView(int slice, int level);

		ID3D9VolumeTexturePtr D3DTexture3D() const
			{ return d3dTexture3D_; }

	private:
		void DoOnLostDevice();
		void DoOnResetDevice();

		void QueryBaseTexture();
		void UpdateParams();

		ID3D9VolumeTexturePtr CreateTexture3D(uint32_t usage, D3DPOOL pool);

	private:
		ID3D9VolumeTexturePtr	d3dTexture3D_;

		std::vector<uint32_t>	widths_;
		std::vector<uint32_t>	heights_;
		std::vector<uint32_t>	depths_;

		bool auto_gen_mipmaps_;
	};

	class D3D9TextureCube : public D3D9Texture
	{
	public:
		D3D9TextureCube(uint32_t size, uint16_t numMipMaps, PixelFormat format);

		uint32_t Width(int level) const;
		uint32_t Height(int level) const;

		void CopyToTexture(Texture& target);
		
		void CopyToMemoryCube(CubeFaces face, int level, void* data);

		void CopyMemoryToTextureCube(CubeFaces face, int level, void* data, PixelFormat pf,
			uint32_t dst_width, uint32_t dst_height, uint32_t dst_xOffset, uint32_t dst_yOffset,
			uint32_t src_width, uint32_t src_height);

		void BuildMipSubLevels();

		D3D9RenderViewPtr CreateRenderView(CubeFaces face, int level);

		ID3D9CubeTexturePtr D3DTextureCube() const
			{ return d3dTextureCube_; }

	private:
		void DoOnLostDevice();
		void DoOnResetDevice();

		void QueryBaseTexture();
		void UpdateParams();

		ID3D9CubeTexturePtr CreateTextureCube(uint32_t usage, D3DPOOL pool);

	private:
		ID3D9CubeTexturePtr		d3dTextureCube_;

		std::vector<uint32_t>	widths_;

		bool auto_gen_mipmaps_;
	};
}

#endif			// _D3D9TEXTURE_HPP
