#include <KlayGE/KlayGE.hpp>
#include <KlayGE/D3D9/D3D9RenderEngine.hpp>
#include <KlayGE/D3D9/D3D9Texture.hpp>
#include <KlayGE/D3D9/D3D9RenderTexture.hpp>
#include <KlayGE/D3D9/D3D9RenderEffect.hpp>
#include <KlayGE/D3D9/D3D9Font.hpp>

#include <KlayGE/D3D9/D3D9RenderFactory.hpp>

namespace KlayGE
{
	const WString& D3D9RenderFactory::Name() const
	{
		static WString name(L"Direct3D9 Render Factory");
		return name;
	}

	RenderEngine& D3D9RenderFactory::RenderEngineInstance()
	{
		static D3D9RenderEngine RenderEngine;
		return RenderEngine;
	}

	TexturePtr D3D9RenderFactory::MakeTexture(U32 width, U32 height,
		U16 mipMapsNum, PixelFormat format, Texture::TextureUsage usage)
	{
		return TexturePtr(new D3D9Texture(width, height, mipMapsNum, format, usage));
	}

	RenderTexturePtr D3D9RenderFactory::MakeRenderTexture(U32 width, U32 height)
	{
		return RenderTexturePtr(new D3D9RenderTexture(width, height));
	}

	FontPtr D3D9RenderFactory::MakeFont(const WString& fontName, U32 fontHeight, U32 flags)
	{
        return FontPtr(new D3D9Font(fontName, fontHeight, flags));
	}

	RenderEffectPtr D3D9RenderFactory::MakeRenderEffect(const String& srcData, UINT flags)
	{
		return RenderEffectPtr(new D3D9RenderEffect(srcData, flags));
	}
}
