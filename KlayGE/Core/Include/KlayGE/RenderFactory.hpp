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

	template <typename RenderEngineType, typename TextureType, typename RenderTextureType,
		typename FontType, typename RenderEffectType>
	class ConcreteRenderFactory : public RenderFactory
	{
	public:
		ConcreteRenderFactory(const WString& name)
				: name_(name)
			{ }

		const WString& Name() const
			{ return name_; }

		RenderEngine& RenderEngineInstance()
		{
			static RenderEngineType renderEngine;
			return renderEngine;
		}

		TexturePtr MakeTexture(U32 width, U32 height, U16 mipMapsNum,
				PixelFormat format, Texture::TextureUsage usage = Texture::TU_Default)
			{ return TexturePtr(new TextureType(width, height, mipMapsNum, format, usage)); }

		RenderTexturePtr MakeRenderTexture(U32 width, U32 height)
			{ return RenderTexturePtr(new RenderTextureType(width, height)); }

		FontPtr MakeFont(const WString& fontName, U32 fontHeight = 12, U32 flags = 0)
			{ return FontPtr(new FontType(fontName, fontHeight, flags)); }

		RenderEffectPtr MakeRenderEffect(const String& srcData, UINT flags = 0)
			{ return RenderEffectPtr(new RenderEffectType(srcData, flags)); }

	private:
		const WString name_;
	};
}

#endif			// _RENDERFACTORY_HPP
