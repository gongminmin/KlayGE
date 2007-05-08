// RenderFactory.cpp
// KlayGE 渲染工厂类 实现文件
// Ver 2.8.0
// 版权所有(C) 龚敏敏, 2003-2005
// Homepage: http://klayge.sourceforge.net
//
// 2.8.0
// 增加了LoadEffect (2005.7.25)
//
// 修改记录
//////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/Texture.hpp>
#include <KlayGE/RenderEffect.hpp>
#include <KlayGE/FrameBuffer.hpp>
#include <KlayGE/Query.hpp>
#include <KlayGE/ResLoader.hpp>
#include <KlayGE/Font.hpp>
#include <KlayGE/ShaderObject.hpp>

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
			ElementFormat /*format*/)
		{
			return Texture::NullObject();
		}
		TexturePtr MakeTexture2D(uint32_t /*width*/, uint32_t /*height*/, uint16_t /*numMipMaps*/,
			ElementFormat /*format*/)
		{
			return Texture::NullObject();
		}
		TexturePtr MakeTexture3D(uint32_t /*width*/, uint32_t /*height*/, uint32_t /*depth*/, uint16_t /*numMipMaps*/,
			ElementFormat /*format*/)
		{
			return Texture::NullObject();
		}
		TexturePtr MakeTextureCube(uint32_t /*size*/, uint16_t /*numMipMaps*/,
			ElementFormat /*format*/)
		{
			return Texture::NullObject();
		}
		FrameBufferPtr MakeFrameBuffer()
		{
			return FrameBuffer::NullObject();
		}

		RenderLayoutPtr MakeRenderLayout(RenderLayout::buffer_type /*type*/)
		{
			return RenderLayout::NullObject();
		}

		GraphicsBufferPtr MakeVertexBuffer(BufferUsage /*usage*/)
		{
			return GraphicsBuffer::NullObject();
		}
		GraphicsBufferPtr MakeIndexBuffer(BufferUsage /*usage*/)
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
		RenderEffectPtr ret;

		BOOST_AUTO(eiter, effect_pool_.find(effectName));
		if (eiter == effect_pool_.end())
		{
			ret.reset(new RenderEffect);
			ret->Load(ResLoader::Instance().Load(effectName));
			effect_pool_.insert(std::make_pair(effectName, ret));
		}
		else
		{
			ret = eiter->second;
		}

		return ret;
	}
}
