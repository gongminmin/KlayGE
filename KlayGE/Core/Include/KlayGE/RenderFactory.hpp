// RenderFactory.hpp
// KlayGE 渲染工厂类 头文件
// Ver 2.8.0
// 版权所有(C) 龚敏敏, 2003-2005
// Homepage: http://klayge.sourceforge.net
//
// 2.8.0
// 增加了LoadEffect (2005.7.25)
//
// 2.4.0
// 增加了1D/2D/3D/cube的支持 (2005.3.8)
//
// 修改记录
//////////////////////////////////////////////////////////////////////////////////

#ifndef _RENDERFACTORY_HPP
#define _RENDERFACTORY_HPP

#include <KlayGE/PreDeclare.hpp>
#include <KlayGE/Texture.hpp>
#include <KlayGE/VertexBuffer.hpp>
#include <KlayGE/Font.hpp>

#include <string>
#include <map>
#include <boost/smart_ptr.hpp>
#include <boost/utility.hpp>

namespace KlayGE
{
	class RenderFactory
	{
	public:
		virtual ~RenderFactory();

		virtual std::wstring const & Name() const = 0;

		virtual RenderEngine& RenderEngineInstance() = 0;
		virtual TexturePtr MakeTexture1D(uint32_t width, uint16_t numMipMaps,
			PixelFormat format, Texture::TextureUsage usage = Texture::TU_Default) = 0;
		virtual TexturePtr MakeTexture2D(uint32_t width, uint32_t height, uint16_t numMipMaps,
			PixelFormat format, Texture::TextureUsage usage = Texture::TU_Default) = 0;
		virtual TexturePtr MakeTexture3D(uint32_t width, uint32_t height, uint32_t depth, uint16_t numMipMaps,
			PixelFormat format, Texture::TextureUsage usage = Texture::TU_Default) = 0;
		virtual TexturePtr MakeTextureCube(uint32_t size, uint16_t numMipMaps,
			PixelFormat format, Texture::TextureUsage usage = Texture::TU_Default) = 0;
		virtual RenderTexturePtr MakeRenderTexture() = 0;

		FontPtr MakeFont(std::string const & fontName, uint32_t fontHeight = 12, uint32_t flags = 0);

		RenderEffectPtr LoadEffect(std::string const & effectName);

		// sizeElement表示流中每个元素的大小，比如Position流是size(float)
		// numElement表示一个顶点有几个元素表示，比如Position流是由(x, y, z)组成，所以为3
		virtual VertexStreamPtr MakeVertexStream(VertexStreamType type,
			uint8_t sizeElement, uint8_t numElement, bool staticStream = false) = 0;
		virtual IndexStreamPtr MakeIndexStream(bool staticStream = false) = 0;

		virtual RenderVertexStreamPtr MakeRenderVertexStream(uint32_t width, uint32_t height) = 0;

	private:
		virtual RenderEffectPtr DoMakeRenderEffect(std::string const & effectData) = 0;

	protected:
		typedef std::map<std::string, boost::weak_ptr<RenderEffect> > effect_pool_type;
		effect_pool_type effect_pool_;
	};

	template <typename RenderEngineType, typename TextureType, typename RenderTextureType,
		typename RenderEffectType, typename RenderVertexStreamType>
	class ConcreteRenderFactory : boost::noncopyable, public RenderFactory
	{
	public:
		explicit ConcreteRenderFactory(std::wstring const & name)
				: name_(name)
			{ }
		virtual ~ConcreteRenderFactory()
			{ }

		std::wstring const & Name() const
			{ return name_; }

		RenderEngine& RenderEngineInstance()
		{
			static RenderEngineType renderEngine;
			return renderEngine;
		}

	private:
		std::wstring const name_;
	};
}

#endif			// _RENDERFACTORY_HPP
