#ifndef _OGLTEXTURE_HPP
#define _OGLTEXTURE_HPP

#include <KlayGE/SharePtr.hpp>

#include <KlayGE/OpenGL/OGLRenderEngine.hpp>

#pragma comment(lib, "KlayGE_RenderEngine_OpenGL.lib")

namespace KlayGE
{
	class OGLTexture;
	class OGLRenderTexture;

	typedef SharePtr<OGLTexture>		D3D9TexturePtr;
	typedef SharePtr<OGLRenderTexture> D3D9RenderTexturePtr;

	class OGLRenderFactory;

	class OGLTexture : public Texture
	{
		friend class OGLRenderFactory;

	public:
		~OGLTexture();

		const WString& Name() const;

		void CustomAttribute(const String& name, void* pData);

		void CopyToTexture(Texture& target);
		void CopyMemoryToTexture(void* pData, PixelFormat pf,
			U32 width = 0, U32 height = 0, U32 pitch = 0, U32 xOffset = 0, U32 yOffset = 0);

		GLenum GLTexture() const
			{ return texture_; }

	private:
		OGLTexture(U32 width, U32 height, U16 mipMapsNum, PixelFormat format, TextureUsage usage = TU_Default);

		GLenum texture_;
	};
}

#endif			// _OGLTEXTURE_HPP
