#ifndef _RENDERFACTORY_HPP
#define _RENDERFACTORY_HPP

#include <KlayGE/Texture.hpp>
#include <KlayGE/RenderBuffer.hpp>

namespace KlayGE
{
	class RenderFactory
	{
	public:
		virtual ~RenderFactory()
			{ }

		virtual const std::wstring& Name() const = 0;

		virtual RenderEngine& RenderEngineInstance() = 0;
		virtual TexturePtr MakeTexture(U32 width, U32 height, U16 numMipMaps,
			PixelFormat format, Texture::TextureUsage usage = Texture::TU_Default) = 0;
		virtual RenderTexturePtr MakeRenderTexture(U32 width, U32 height) = 0;

		virtual FontPtr MakeFont(const std::wstring& fontName, U32 fontHeight = 12, U32 flags = 0) = 0;

		virtual RenderEffectPtr MakeRenderEffect(const std::string& srcData, UINT flags = 0) = 0;

		virtual VertexShaderPtr MakeVertexShader(const std::string& shaderName,
			const std::string& functionName, const std::string& profile) = 0;
		virtual PixelShaderPtr MakePixelShader(const std::string& shaderName,
			const std::string& functionName, const std::string& profile) = 0;

		// sizeElement表示流中每个元素的大小，比如Position流是size(float)
		// numElement表示一个顶点有几个元素表示，比如Position流是由(x, y, z)组成，所以为3
		virtual VertexStreamPtr MakeVertexStream(VertexStreamType type,
			U8 sizeElement, U8 numElement, bool staticStream = false) = 0;
		virtual IndexStreamPtr MakeIndexStream(bool staticStream = false) = 0;
	};

	template <typename RenderEngineType, typename TextureType, typename RenderTextureType,
		typename FontType, typename RenderEffectType, typename VertexShaderType, typename PixelShaderType>
	class ConcreteRenderFactory : public RenderFactory
	{
	public:
		ConcreteRenderFactory(const std::wstring& name)
				: name_(name)
			{ }
		virtual ~ConcreteRenderFactory()
			{ }

		const std::wstring& Name() const
			{ return name_; }

		RenderEngine& RenderEngineInstance()
		{
			static RenderEngineType renderEngine;
			return renderEngine;
		}

		TexturePtr MakeTexture(U32 width, U32 height, U16 numMipMaps,
				PixelFormat format, Texture::TextureUsage usage = Texture::TU_Default)
			{ return TexturePtr(new TextureType(width, height, numMipMaps, format, usage)); }

		RenderTexturePtr MakeRenderTexture(U32 width, U32 height)
			{ return RenderTexturePtr(new RenderTextureType(width, height)); }

		FontPtr MakeFont(const std::wstring& fontName, U32 fontHeight = 12, U32 flags = 0)
			{ return FontPtr(new FontType(fontName, fontHeight, flags)); }

		RenderEffectPtr MakeRenderEffect(const std::string& srcData, UINT flags = 0)
			{ return RenderEffectPtr(new RenderEffectType(srcData, flags)); }

		VertexShaderPtr MakeVertexShader(const std::string& shaderName, const std::string& functionName, const std::string& profile)
			{ return VertexShaderPtr(new VertexShaderType(shaderName, functionName, profile)); }
		PixelShaderPtr MakePixelShader(const std::string& shaderName, const std::string& functionName, const std::string& profile)
			{ return PixelShaderPtr(new PixelShaderType(shaderName, functionName, profile)); }

	private:
		const std::wstring name_;
	};
}

#endif			// _RENDERFACTORY_HPP
