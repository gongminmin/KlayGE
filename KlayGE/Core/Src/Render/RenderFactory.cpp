// RenderFactory.cpp
// KlayGE 渲染工厂类 实现文件
// Ver 3.8.0
// 版权所有(C) 龚敏敏, 2003-2008
// Homepage: http://klayge.sourceforge.net
//
// 3.8.0
// 增加了MakeSamplerStateObject (2008.9.21)
//
// 2.8.0
// 增加了LoadEffect (2005.7.25)
//
// 修改记录
//////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KlayGE/Util.hpp>
#include <KlayGE/Math.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/Texture.hpp>
#include <KlayGE/RenderEffect.hpp>
#include <KlayGE/FrameBuffer.hpp>
#include <KlayGE/Query.hpp>
#include <KlayGE/ResLoader.hpp>
#include <KlayGE/Font.hpp>
#include <KlayGE/RenderStateObject.hpp>
#include <KlayGE/ShaderObject.hpp>
#include <KlayGE/RenderLayout.hpp>

#include <boost/foreach.hpp>
#include <boost/typeof/typeof.hpp>

#include <KlayGE/RenderFactory.hpp>

namespace KlayGE
{
	class NullRenderFactory : public RenderFactory
	{
		std::wstring const & Name() const
		{
			static std::wstring const name(L"Null Render Factory");
			return name;
		}

		TexturePtr MakeTexture1D(uint32_t /*width*/, uint32_t /*numMipMaps*/, uint32_t /*array_size*/,
			ElementFormat /*format*/, uint32_t /*sample_count*/, uint32_t /*sample_quality*/, uint32_t /*access_hint*/,
			ElementInitData* /*init_data*/)
		{
			return Texture::NullObject();
		}
		TexturePtr MakeTexture2D(uint32_t /*width*/, uint32_t /*height*/, uint32_t /*numMipMaps*/, uint32_t /*array_size*/,
			ElementFormat /*format*/, uint32_t /*sample_count*/, uint32_t /*sample_quality*/, uint32_t /*access_hint*/,
			ElementInitData* /*init_data*/)
		{
			return Texture::NullObject();
		}
		TexturePtr MakeTexture3D(uint32_t /*width*/, uint32_t /*height*/, uint32_t /*depth*/, uint32_t /*numMipMaps*/, uint32_t /*array_size*/,
			ElementFormat /*format*/, uint32_t /*sample_count*/, uint32_t /*sample_quality*/, uint32_t /*access_hint*/,
			ElementInitData* /*init_data*/)
		{
			return Texture::NullObject();
		}
		TexturePtr MakeTextureCube(uint32_t /*size*/, uint32_t /*numMipMaps*/, uint32_t /*array_size*/,
			ElementFormat /*format*/, uint32_t /*sample_count*/, uint32_t /*sample_quality*/, uint32_t /*access_hint*/,
			ElementInitData* /*init_data*/)
		{
			return Texture::NullObject();
		}
		FrameBufferPtr MakeFrameBuffer()
		{
			return FrameBuffer::NullObject();
		}

		RenderLayoutPtr MakeRenderLayout()
		{
			return RenderLayout::NullObject();
		}

		GraphicsBufferPtr MakeVertexBuffer(BufferUsage /*usage*/, uint32_t /*access_hint*/, ElementInitData* /*init_data*/, ElementFormat /*fmt*/)
		{
			return GraphicsBuffer::NullObject();
		}
		GraphicsBufferPtr MakeIndexBuffer(BufferUsage /*usage*/, uint32_t /*access_hint*/, ElementInitData* /*init_data*/, ElementFormat /*fmt*/)
		{
			return GraphicsBuffer::NullObject();
		}

		QueryPtr MakeOcclusionQuery()
		{
			return Query::NullObject();
		}

		QueryPtr MakeConditionalRender()
		{
			return Query::NullObject();
		}

		RenderViewPtr Make1DRenderView(Texture& /*texture*/, int /*array_index*/, int /*level*/)
		{
			return RenderView::NullObject();
		}

		RenderViewPtr Make2DRenderView(Texture& /*texture*/, int /*array_index*/, int /*level*/)
		{
			return RenderView::NullObject();
		}

		RenderViewPtr Make2DRenderView(Texture& /*texture*/, int /*array_index*/, Texture::CubeFaces /*face*/, int /*level*/)
		{
			return RenderView::NullObject();
		}

		RenderViewPtr Make2DRenderView(Texture& /*texture*/, int /*array_index*/, uint32_t /*slice*/, int /*level*/)
		{
			return RenderView::NullObject();
		}

		RenderViewPtr MakeCubeRenderView(Texture& /*texture*/, int /*array_index*/, int /*level*/)
		{
			return RenderView::NullObject();
		}

		RenderViewPtr Make3DRenderView(Texture& /*texture*/, int /*array_index*/, uint32_t /*first_slice*/, uint32_t /*num_slices*/, int /*level*/)
		{
			return RenderView::NullObject();
		}

		RenderViewPtr MakeGraphicsBufferRenderView(GraphicsBuffer& /*gbuffer*/, uint32_t /*width*/, uint32_t /*height*/, ElementFormat /*pf*/)
		{
			return RenderView::NullObject();
		}

		RenderViewPtr Make2DDepthStencilRenderView(uint32_t /*width*/, uint32_t /*height*/, ElementFormat /*pf*/,
			uint32_t /*sample_count*/, uint32_t /*sample_quality*/)
		{
			return RenderView::NullObject();
		}

		RenderViewPtr Make1DDepthStencilRenderView(Texture& /*texture*/, int /*array_index*/, int /*level*/)
		{
			return RenderView::NullObject();
		}

		RenderViewPtr Make2DDepthStencilRenderView(Texture& /*texture*/, int /*array_index*/, int /*level*/)
		{
			return RenderView::NullObject();
		}

		RenderViewPtr Make2DDepthStencilRenderView(Texture& /*texture*/, int /*array_index*/, Texture::CubeFaces /*face*/, int /*level*/)
		{
			return RenderView::NullObject();
		}

		RenderViewPtr Make2DDepthStencilRenderView(Texture& /*texture*/, int /*array_index*/, uint32_t slice, int level)
		{
			return RenderView::NullObject();
		}

		RenderViewPtr MakeCubeDepthStencilRenderView(Texture& /*texture*/, int /*array_index*/, int /*level*/)
		{
			return RenderView::NullObject();
		}

		RenderViewPtr Make3DDepthStencilRenderView(Texture& /*texture*/, int /*array_index*/, uint32_t /*first_slice*/, uint32_t /*num_slices*/, int /*level*/)
		{
			return RenderView::NullObject();
		}

		ShaderObjectPtr MakeShaderObject()
		{
			return ShaderObject::NullObject();
		}

		RenderEnginePtr DoMakeRenderEngine()
		{
			return RenderEngine::NullObject();
		}

		RasterizerStateObjectPtr DoMakeRasterizerStateObject(RasterizerStateDesc const & /*desc*/)
		{
			return RasterizerStateObject::NullObject();
		}

		DepthStencilStateObjectPtr DoMakeDepthStencilStateObject(DepthStencilStateDesc const & /*desc*/)
		{
			return DepthStencilStateObject::NullObject();
		}

		BlendStateObjectPtr DoMakeBlendStateObject(BlendStateDesc const & /*desc*/)
		{
			return BlendStateObject::NullObject();
		}

		SamplerStateObjectPtr DoMakeSamplerStateObject(SamplerStateDesc const & /*desc*/)
		{
			return SamplerStateObject::NullObject();
		}
	};


	RenderFactory::~RenderFactory()
	{
		BOOST_FOREACH(BOOST_TYPEOF(effect_pool_)::reference effect, effect_pool_)
		{
			effect.second.clear();
		}
		BOOST_FOREACH(BOOST_TYPEOF(font_pool_)::reference font, font_pool_)
		{
			font.second.first.reset();
			font.second.second.reset();
		}
		BOOST_FOREACH(BOOST_TYPEOF(rs_pool_)::reference rs, rs_pool_)
		{
			rs.second.reset();
		}
		BOOST_FOREACH(BOOST_TYPEOF(dss_pool_)::reference dss, dss_pool_)
		{
			dss.second.reset();
		}
		BOOST_FOREACH(BOOST_TYPEOF(bs_pool_)::reference bs, bs_pool_)
		{
			bs.second.reset();
		}
		BOOST_FOREACH(BOOST_TYPEOF(ss_pool_)::reference ss, ss_pool_)
		{
			ss.second.reset();
		}

		re_.reset();
	}

	RenderFactoryPtr RenderFactory::NullObject()
	{
		static RenderFactoryPtr obj = MakeSharedPtr<NullRenderFactory>();
		return obj;
	}

	RenderEngine& RenderFactory::RenderEngineInstance()
	{
		if (!re_)
		{
			re_ = this->DoMakeRenderEngine();
		}

		return *re_;
	}

	FontPtr RenderFactory::MakeFont(std::string const & fontName, uint32_t flags)
	{
		FontPtr ret;

		BOOST_AUTO(fiter, font_pool_.find(fontName));
		if (fiter == font_pool_.end())
		{
			RenderablePtr font_renderable = MakeSharedPtr<FontRenderable>(fontName);

			ret = MakeSharedPtr<Font>(font_renderable, flags);
			font_pool_[fontName] = std::make_pair(font_renderable, ret);
		}
		else
		{
			ret = fiter->second.second;
		}

		return ret;
	}

	RenderEffectPtr RenderFactory::LoadEffect(std::string const & effectName, std::pair<std::string, std::string>* macros)
	{
		RenderEffectPtr prototype;

		BOOST_AUTO(entry, std::make_pair(effectName, macros));
		BOOST_AUTO(iter, effect_pool_.find(entry));
		if (iter == effect_pool_.end())
		{
			prototype = MakeSharedPtr<RenderEffect>();
			prototype->Load(ResLoader::Instance().Load(effectName), macros);
			effect_pool_[entry].push_back(prototype);
		}
		else
		{
			BOOST_ASSERT(!iter->second.empty());
			prototype = iter->second[0];
		}

		RenderEffectPtr ret = prototype->Clone();
		ret->PrototypeEffect(prototype);
		effect_pool_[entry].push_back(ret);

		return ret;
	}

	RasterizerStateObjectPtr RenderFactory::MakeRasterizerStateObject(RasterizerStateDesc const & desc)
	{
		RasterizerStateObjectPtr ret;

		BOOST_AUTO(iter, rs_pool_.find(desc));
		if (iter == rs_pool_.end())
		{
			ret = this->DoMakeRasterizerStateObject(desc);
			rs_pool_.insert(std::make_pair(desc, ret));
		}
		else
		{
			ret = iter->second;
		}

		return ret;
	}

	DepthStencilStateObjectPtr RenderFactory::MakeDepthStencilStateObject(DepthStencilStateDesc const & desc)
	{
		DepthStencilStateObjectPtr ret;

		BOOST_AUTO(iter, dss_pool_.find(desc));
		if (iter == dss_pool_.end())
		{
			ret = this->DoMakeDepthStencilStateObject(desc);
			dss_pool_.insert(std::make_pair(desc, ret));
		}
		else
		{
			ret = iter->second;
		}

		return ret;
	}

	BlendStateObjectPtr RenderFactory::MakeBlendStateObject(BlendStateDesc const & desc)
	{
		BlendStateObjectPtr ret;

		BOOST_AUTO(iter, bs_pool_.find(desc));
		if (iter == bs_pool_.end())
		{
			ret = this->DoMakeBlendStateObject(desc);
			bs_pool_.insert(std::make_pair(desc, ret));
		}
		else
		{
			ret = iter->second;
		}

		return ret;
	}

	SamplerStateObjectPtr RenderFactory::MakeSamplerStateObject(SamplerStateDesc const & desc)
	{
		SamplerStateObjectPtr ret;

		BOOST_AUTO(iter, ss_pool_.find(desc));
		if (iter == ss_pool_.end())
		{
			ret = this->DoMakeSamplerStateObject(desc);
			ss_pool_.insert(std::make_pair(desc, ret));
		}
		else
		{
			ret = iter->second;
		}

		return ret;
	}
}
