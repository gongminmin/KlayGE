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
			PixelFormat /*format*/)
		{
			return Texture::NullObject();
		}
		TexturePtr MakeTexture2D(uint32_t /*width*/, uint32_t /*height*/, uint16_t /*numMipMaps*/,
			PixelFormat /*format*/)
		{
			return Texture::NullObject();
		}
		TexturePtr MakeTexture3D(uint32_t /*width*/, uint32_t /*height*/, uint32_t /*depth*/, uint16_t /*numMipMaps*/,
			PixelFormat /*format*/)
		{
			return Texture::NullObject();
		}
		TexturePtr MakeTextureCube(uint32_t /*size*/, uint16_t /*numMipMaps*/,
			PixelFormat /*format*/)
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

		RenderViewPtr MakeGraphicsBufferRenderView(GraphicsBuffer& /*gbuffer*/, uint32_t /*width*/, uint32_t /*height*/)
		{
			return RenderView::NullObject();
		}

		RenderViewPtr MakeDepthStencilRenderView(uint32_t /*width*/, uint32_t /*height*/, PixelFormat /*pf*/, uint32_t /*multi_sample*/)
		{
			return RenderView::NullObject();
		}

	private:
		RenderEffectPtr DoMakeRenderEffect(std::string const & /*effectData*/)
		{
			return RenderEffect::NullObject();
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
		return FontPtr(new Font(fontName, fontHeight, flags));
	}

	RenderEffectPtr RenderFactory::LoadEffect(std::string const & effectName)
	{
		RenderEffectPtr ret;

		for (effect_pool_type::iterator iter = effect_pool_.begin(); iter != effect_pool_.end();)
		{
			if (!(iter->second.lock()))
			{
				iter = effect_pool_.erase(iter);
			}
			else
			{
				++ iter;
			}
		}

		effect_pool_type::iterator eiter = effect_pool_.find(effectName);
		if (eiter == effect_pool_.end())
		{
			ResIdentifierPtr file(ResLoader::Instance().Load(effectName));

			file->seekg(0, std::ios_base::end);
			std::vector<char> data(file->tellg());
			file->seekg(0);
			file->read(&data[0], static_cast<std::streamsize>(data.size()));

			ret = this->DoMakeRenderEffect(std::string(data.begin(), data.end()));
			if (ret)
			{
				effect_pool_.insert(std::make_pair(effectName, boost::weak_ptr<RenderEffect>(ret)));
			}
		}
		else
		{
			ret = RenderEffectPtr(eiter->second);
		}

		return ret;
	}
}
