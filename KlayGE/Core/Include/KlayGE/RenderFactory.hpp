#ifndef _RENDERFACTORY_HPP
#define _RENDERFACTORY_HPP

namespace KlayGE
{
	class RenderFactory
	{
	public:
		virtual ~RenderFactory()
			{ }

		virtual const WString& Name() const = 0;

		virtual RenderEngine& RenderEngineInstance() = 0;
		virtual TexturePtr MakeTexture(U32 width, U32 height, U16 mipMapsNum,
			PixelFormat format, Texture::TextureUsage usage = Texture::TU_Default) = 0;
		virtual RenderTexturePtr MakeRenderTexture(U32 width, U32 height) = 0;

		virtual FontPtr MakeFont(const WString& fontName, U32 fontHeight = 12, U32 flags = 0) = 0;

		virtual RenderEffectPtr MakeRenderEffect(const String& srcData, UINT flags = 0) = 0;
	};
}

#endif			// _RENDERFACTORY_HPP