#ifndef _OGLTEXTURE_HPP
#define _OGLTEXTURE_HPP

#include <KlayGE/Texture.hpp>

#define NOMINMAX
#include <windows.h>
#include <gl/gl.h>

#pragma comment(lib, "KlayGE_RenderEngine_OpenGL.lib")

namespace KlayGE
{
	class OGLTexture : public Texture
	{
	public:
		OGLTexture(uint32 width, uint32 height, uint16 numMipMaps, PixelFormat format, TextureUsage usage = TU_Default);
		~OGLTexture();

		std::wstring const & Name() const;

		void CustomAttribute(std::string const & name, void* pData);

		void CopyToTexture(Texture& target);
		void CopyMemoryToTexture(void* pData, PixelFormat pf,
			uint32 width = 0, uint32 height = 0, uint32 xOffset = 0, uint32 yOffset = 0);

		GLenum GLTexture() const
			{ return texture_; }

	private:
		GLenum texture_;
	};

	typedef boost::shared_ptr<OGLTexture> OGLTexturePtr;
}

#endif			// _OGLTEXTURE_HPP
