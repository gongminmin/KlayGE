#include <KlayGE/KlayGE.hpp>
#include <KlayGE/OpenGL/OGLRenderEngine.hpp>
#include <KlayGE/OpenGL/OGLTexture.hpp>
//#include <KlayGE/OpenGL/OGLRenderTexture.hpp>
//#include <KlayGE/OpenGL/OGLRenderEffect.hpp>
//#include <KlayGE/OpenGL/OGLFont.hpp>

#include <KlayGE/OpenGL/OGLRenderFactory.hpp>

namespace KlayGE
{
	const WString& OGLRenderFactory::Name() const
	{
		static WString name(L"OpenGL Render Factory");
		return name;
	}

	RenderEngine& OGLRenderFactory::RenderEngineInstance()
	{
		static OGLRenderEngine RenderEngine;
		return RenderEngine;
	}

	TexturePtr OGLRenderFactory::MakeTexture(U32 width, U32 height,
		U16 mipMapsNum, PixelFormat format, Texture::TextureUsage usage)
	{
		return TexturePtr(new OGLTexture(width, height, mipMapsNum, format, usage));
	}

	RenderTexturePtr OGLRenderFactory::MakeRenderTexture(U32 width, U32 height)
	{
		return RenderTexturePtr();//new OGLRenderTexture(width, height));
	}

	FontPtr OGLRenderFactory::MakeFont(const WString& fontName, U32 fontHeight, U32 flags)
	{
		return FontPtr();//new OGLFont(fontName, fontHeight, flags));
	}

	RenderEffectPtr OGLRenderFactory::MakeRenderEffect(const String& srcData, UINT flags)
	{
		return RenderEffectPtr();//new OGLRenderEffect(srcData, flags));
	}
}
