#ifndef _D3D9RENDERFACTORY_HPP
#define _D3D9RENDERFACTORY_HPP

#include <KlayGE/PreDeclare.hpp>
#include <KlayGE/RenderFactory.hpp>

#include <d3d9.h>
#include <d3dx9.h>

#pragma comment(lib, "KlayGE_RenderEngine_D3D9.lib")

namespace KlayGE
{
	class D3D9RenderFactory : public RenderFactory
	{
	public:
		const WString& Name() const;

		RenderEngine& RenderEngineInstance();

		TexturePtr MakeTexture(U32 width, U32 height, U16 mipMapsNum,
			PixelFormat format, Texture::TextureUsage usage = Texture::TU_Default);
		RenderTexturePtr MakeRenderTexture(U32 width, U32 height);

		FontPtr MakeFont(const WString& fontName, U32 fontHeight = 12, U32 flags = 0);

		RenderEffectPtr MakeRenderEffect(const String& srcData, UINT flags = 0);
	};
}

#endif			// _D3D9RENDERFACTORY_HPP