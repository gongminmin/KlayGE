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
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/Texture.hpp>
#include <KlayGE/Math.hpp>
#include <KlayGE/RenderEffect.hpp>
#include <KlayGE/FrameBuffer.hpp>
#include <KlayGE/Query.hpp>
#include <KlayGE/ResLoader.hpp>
#include <KlayGE/Font.hpp>
#include <KlayGE/RenderStateObject.hpp>
#include <KlayGE/ShaderObject.hpp>
#include <KlayGE/RenderLayout.hpp>

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

		RenderEngine& RenderEngineInstance()
		{
			return *RenderEngine::NullObject();
		}

		TexturePtr MakeTexture1D(uint32_t /*width*/, uint16_t /*numMipMaps*/,
			ElementFormat /*format*/, uint32_t /*access_hint*/, ElementInitData* /*init_data*/)
		{
			return Texture::NullObject();
		}
		TexturePtr MakeTexture2D(uint32_t /*width*/, uint32_t /*height*/, uint16_t /*numMipMaps*/,
			ElementFormat /*format*/, uint32_t /*access_hint*/, ElementInitData* /*init_data*/)
		{
			return Texture::NullObject();
		}
		TexturePtr MakeTexture3D(uint32_t /*width*/, uint32_t /*height*/, uint32_t /*depth*/, uint16_t /*numMipMaps*/,
			ElementFormat /*format*/, uint32_t /*access_hint*/, ElementInitData* /*init_data*/)
		{
			return Texture::NullObject();
		}
		TexturePtr MakeTextureCube(uint32_t /*size*/, uint16_t /*numMipMaps*/,
			ElementFormat /*format*/, uint32_t /*access_hint*/, ElementInitData* /*init_data*/)
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

		GraphicsBufferPtr MakeVertexBuffer(BufferUsage /*usage*/, uint32_t /*access_hint*/, ElementInitData* /*init_data*/)
		{
			return GraphicsBuffer::NullObject();
		}
		GraphicsBufferPtr MakeIndexBuffer(BufferUsage /*usage*/, uint32_t /*access_hint*/, ElementInitData* /*init_data*/)
		{
			return GraphicsBuffer::NullObject();
		}

		QueryPtr MakeOcclusionQuery()
		{
			return Query::NullObject();
		}

		RenderViewPtr Make1DRenderView(Texture& /*texture*/, int /*level*/)
		{
			return RenderView::NullObject();
		}

		RenderViewPtr Make2DRenderView(Texture& /*texture*/, int /*level*/)
		{
			return RenderView::NullObject();
		}

		RenderViewPtr Make2DRenderView(Texture& /*texture*/, Texture::CubeFaces /*face*/, int /*level*/)
		{
			return RenderView::NullObject();
		}

		RenderViewPtr Make3DRenderView(Texture& /*texture*/, uint32_t /*slice*/, int /*level*/)
		{
			return RenderView::NullObject();
		}

		RenderViewPtr MakeGraphicsBufferRenderView(GraphicsBuffer& /*gbuffer*/, uint32_t /*width*/, uint32_t /*height*/, ElementFormat /*pf*/)
		{
			return RenderView::NullObject();
		}

		RenderViewPtr MakeDepthStencilRenderView(uint32_t /*width*/, uint32_t /*height*/, ElementFormat /*pf*/, uint32_t /*multi_sample*/)
		{
			return RenderView::NullObject();
		}

		ShaderObjectPtr MakeShaderObject()
		{
			return ShaderObject::NullObject();
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
	}

	RenderFactoryPtr RenderFactory::NullObject()
	{
		static RenderFactoryPtr obj(new NullRenderFactory);
		return obj;
	}

	FontPtr RenderFactory::MakeFont(std::string const & fontName, uint32_t fontHeight, uint32_t flags)
	{
		FontPtr ret;

		BOOST_AUTO(fiter, font_pool_.find(std::make_pair(fontName, fontHeight)));
		if (fiter == font_pool_.end())
		{
			ret.reset(new Font(fontName, fontHeight, flags));
			font_pool_.insert(std::make_pair(std::make_pair(fontName, fontHeight), ret));
		}
		else
		{
			ret = fiter->second;
		}

		return ret;
	}

	RenderEffectPtr RenderFactory::LoadEffect(std::string const & effectName)
	{
		RenderEffectPtr prototype;

		BOOST_AUTO(iter, effect_pool_.find(effectName));
		if (iter == effect_pool_.end())
		{
			prototype.reset(new RenderEffect);
			prototype->Load(ResLoader::Instance().Load(effectName));
			effect_pool_[effectName].push_back(prototype);
		}
		else
		{
			BOOST_ASSERT(!iter->second.empty());
			prototype = iter->second[0];
		}

		RenderEffectPtr ret = prototype->Clone();
		ret->PrototypeEffect(prototype);
		effect_pool_[effectName].push_back(ret);

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
