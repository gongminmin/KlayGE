// OGLTexture.hpp
// KlayGE OpenGL纹理类 头文件
// Ver 2.3.0
// 版权所有(C) 龚敏敏, 2003-2005
// Homepage: http://klayge.sourceforge.net
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
#include <gl/gl.h>

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

		void CustomAttribute(std::string const & name, void* pData);

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

		GLenum GLTexture() const
			{ return texture_[0]; }
		GLenum GLTextureFace(int face) const
			{ return texture_[face]; }

	private:
		GLenum texture_[6];
	};

	typedef boost::shared_ptr<OGLTexture> OGLTexturePtr;
}

#endif			// _OGLTEXTURE_HPP
