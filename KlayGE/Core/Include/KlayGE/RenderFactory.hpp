// RenderFactory.hpp
// KlayGE 渲染工厂类 头文件
// Ver 3.0.0
// 版权所有(C) 龚敏敏, 2003-2005
// Homepage: http://klayge.sourceforge.net
//
// 3.0.0
// 增加了MakeVertexBuffer (2005.9.7)
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

#include <string>
#include <map>
#include <boost/tuple/tuple.hpp>
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
			PixelFormat format) = 0;
		virtual TexturePtr MakeTexture2D(uint32_t width, uint32_t height, uint16_t numMipMaps,
			PixelFormat format) = 0;
		virtual TexturePtr MakeTexture3D(uint32_t width, uint32_t height, uint32_t depth, uint16_t numMipMaps,
			PixelFormat format) = 0;
		virtual TexturePtr MakeTextureCube(uint32_t size, uint16_t numMipMaps,
			PixelFormat format) = 0;
		virtual RenderTexturePtr MakeRenderTexture() = 0;

		FontPtr MakeFont(std::string const & fontName, uint32_t fontHeight = 12, uint32_t flags = 0);

		RenderEffectPtr LoadEffect(std::string const & effectName);

		virtual VertexBufferPtr MakeVertexBuffer(VertexBuffer::BufferType type) = 0;

		template <typename tuple_type>
		VertexStreamPtr MakeVertexStream(tuple_type const & vertex_elems, bool staticStream = false)
		{
			return this->MakeVertexStream(Tuple2Vector(vertex_elems), staticStream);
		}

		virtual VertexStreamPtr MakeVertexStream(vertex_elements_type const & vertex_elems, bool staticStream = false) = 0;
		virtual IndexStreamPtr MakeIndexStream(bool staticStream = false) = 0;

		virtual RenderVertexStreamPtr MakeRenderVertexStream(uint32_t width, uint32_t height) = 0;

		virtual OcclusionQueryPtr MakeOcclusionQuery() = 0;

	private:
		virtual RenderEffectPtr DoMakeRenderEffect(std::string const & effectData) = 0;

	private:
		template <typename tuple_type>
		vertex_elements_type Tuple2Vector(tuple_type const & t)
		{
			vertex_elements_type ret;
			ret.push_back(boost::tuples::get<0>(t));

			vertex_elements_type tail(Tuple2Vector(t.get_tail()));
			ret.insert(ret.end(), tail.begin(), tail.end());

			return ret;
		}
		template <>
		vertex_elements_type Tuple2Vector<boost::tuples::null_type>(boost::tuples::null_type const & /*t*/)
		{
			return vertex_elements_type();
		}

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
