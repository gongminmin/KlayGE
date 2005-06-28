// OGLTexture.hpp
// KlayGE OpenGL纹理类 头文件
// Ver 2.7.0
// 版权所有(C) 龚敏敏, 2003-2005
// Homepage: http://klayge.sourceforge.net
//
// 2.7.0
// 增加了TextureAddressingMode, extureFiltering和TextureAnisotropy (2005.6.27)
//
// 2.3.0
// 增加了CopyToMemory (2005.2.6)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#ifndef _OGLTEXTURE_HPP
#define _OGLTEXTURE_HPP

#include <KlayGE/Texture.hpp>

#define NOMINMAX
#include <windows.h>
#include <glloader/glloader.h>

#ifdef KLAYGE_DEBUG
	#pragma comment(lib, "KlayGE_RenderEngine_OpenGL_d.lib")
#else
	#pragma comment(lib, "KlayGE_RenderEngine_OpenGL.lib")
#endif

namespace KlayGE
{
	class OGLTexture : public Texture
	{
	public:
		OGLTexture(uint32_t width, uint16_t numMipMaps, PixelFormat format, TextureUsage usage);
		OGLTexture(uint32_t width, uint32_t height, uint16_t numMipMaps, PixelFormat format, TextureUsage usage);
		OGLTexture(uint32_t width, uint32_t height, uint32_t depth, uint16_t numMipMaps, PixelFormat format, TextureUsage usage);
		OGLTexture(uint32_t size, bool cube, uint16_t numMipMaps, PixelFormat format, TextureUsage usage);
		~OGLTexture();

		std::wstring const & Name() const;

		void CustomAttribute(std::string const & name, void* data);

		uint32_t Width(int level) const;
		uint32_t Height(int level) const;
		uint32_t Depth(int level) const;

		void CopyToTexture(Texture& target);
		
		void CopyToMemory1D(int level, void* data);
		void CopyToMemory2D(int level, void* data);
		void CopyToMemory3D(int level, void* data);
		void CopyToMemoryCube(CubeFaces face, int level, void* data);

		void CopyMemoryToTexture1D(int level, void* data, PixelFormat pf,
			uint32_t width, uint32_t xOffset);
		void CopyMemoryToTexture2D(int level, void* data, PixelFormat pf,
			uint32_t width, uint32_t height, uint32_t xOffset, uint32_t yOffset);
		void CopyMemoryToTexture3D(int level, void* data, PixelFormat pf,
			uint32_t width, uint32_t height, uint32_t depth,
			uint32_t xOffset, uint32_t yOffset, uint32_t zOffset);
		void CopyMemoryToTextureCube(CubeFaces face, int level, void* data, PixelFormat pf,
			uint32_t size, uint32_t xOffset);

		void BuildMipSubLevels();

		uint32_t MaxWidth() const;
		uint32_t MaxHeight() const;
		uint32_t MaxDepth() const;
		uint32_t MaxCubeSize() const;

		void TextureAddressingMode(TexAddressingType type, TexAddressingMode tam);
		void TextureFiltering(TexFilterType type, TexFilterOp op);
		void TextureAnisotropy(uint32_t maxAnisotropy);

		TexAddressingMode TextureAddressingMode(TexAddressingType type) const;
		TexFilterOp TextureFiltering(TexFilterType type) const;
		uint32_t TextureAnisotropy() const;

		GLenum GLTexture() const
			{ return texture_[0]; }
		GLenum GLTextureFace(int face) const
			{ return texture_[face]; }

		void GLBindTexture();
		GLenum GLType() const;

	private:
		void UpdateParams();

	private:
		GLenum texture_[6];

		std::vector<uint32_t> widths_;
		std::vector<uint32_t> heights_;
		std::vector<uint32_t> depths_;

		TexAddressingMode tex_addr_mode_u_, tex_addr_mode_v_, tex_addr_mode_w_;
		TexFilterOp tex_min_filter_, tex_mag_filter_, tex_mip_filter_;
		GLint tex_anisotropy_;
	};

	typedef boost::shared_ptr<OGLTexture> OGLTexturePtr;
}

#endif			// _OGLTEXTURE_HPP
