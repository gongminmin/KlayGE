// RenderFactory.cpp
// KlayGE 渲染工厂类 实现文件
// Ver 3.8.0
// 版权所有(C) 龚敏敏, 2003-2008
// Homepage: http://www.klayge.org
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
#include <KFL/Util.hpp>
#include <KFL/Math.hpp>
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
#include <KlayGE/Fence.hpp>

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
			ElementInitData const * /*init_data*/)
		{
			return Texture::NullObject();
		}
		TexturePtr MakeTexture2D(uint32_t /*width*/, uint32_t /*height*/, uint32_t /*numMipMaps*/, uint32_t /*array_size*/,
			ElementFormat /*format*/, uint32_t /*sample_count*/, uint32_t /*sample_quality*/, uint32_t /*access_hint*/,
			ElementInitData const * /*init_data*/)
		{
			return Texture::NullObject();
		}
		TexturePtr MakeTexture3D(uint32_t /*width*/, uint32_t /*height*/, uint32_t /*depth*/, uint32_t /*numMipMaps*/, uint32_t /*array_size*/,
			ElementFormat /*format*/, uint32_t /*sample_count*/, uint32_t /*sample_quality*/, uint32_t /*access_hint*/,
			ElementInitData const * /*init_data*/)
		{
			return Texture::NullObject();
		}
		TexturePtr MakeTextureCube(uint32_t /*size*/, uint32_t /*numMipMaps*/, uint32_t /*array_size*/,
			ElementFormat /*format*/, uint32_t /*sample_count*/, uint32_t /*sample_quality*/, uint32_t /*access_hint*/,
			ElementInitData const * /*init_data*/)
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

		GraphicsBufferPtr MakeVertexBuffer(BufferUsage /*usage*/, uint32_t /*access_hint*/, uint32_t /*size_in_bytes*/, void const * /*init_data*/, ElementFormat /*fmt*/)
		{
			return GraphicsBuffer::NullObject();
		}
		GraphicsBufferPtr MakeIndexBuffer(BufferUsage /*usage*/, uint32_t /*access_hint*/, uint32_t /*size_in_bytes*/, void const * /*init_data*/, ElementFormat /*fmt*/)
		{
			return GraphicsBuffer::NullObject();
		}
		GraphicsBufferPtr MakeConstantBuffer(BufferUsage /*usage*/, uint32_t /*access_hint*/, uint32_t /*size_in_bytes*/, void const * /*init_data*/, ElementFormat /*fmt*/)
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

		QueryPtr MakeTimerQuery()
		{
			return Query::NullObject();
		}

		virtual FencePtr MakeFence() KLAYGE_OVERRIDE
		{
			return Fence::NullObject();
		}

		RenderViewPtr Make1DRenderView(Texture& /*texture*/, int /*first_array_index*/, int /*array_size*/, int /*level*/)
		{
			return RenderView::NullObject();
		}

		RenderViewPtr Make2DRenderView(Texture& /*texture*/, int /*first_array_index*/, int /*array_size*/, int /*level*/)
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

		RenderViewPtr Make1DDepthStencilRenderView(Texture& /*texture*/, int /*first_array_index*/, int /*array_size*/, int /*level*/)
		{
			return RenderView::NullObject();
		}

		RenderViewPtr Make2DDepthStencilRenderView(Texture& /*texture*/, int /*first_array_index*/, int /*array_size*/, int /*level*/)
		{
			return RenderView::NullObject();
		}

		RenderViewPtr Make2DDepthStencilRenderView(Texture& /*texture*/, int /*array_index*/, Texture::CubeFaces /*face*/, int /*level*/)
		{
			return RenderView::NullObject();
		}

		RenderViewPtr Make2DDepthStencilRenderView(Texture& /*texture*/, int /*array_index*/, uint32_t /*slice*/, int /*level*/)
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

		UnorderedAccessViewPtr Make1DUnorderedAccessView(Texture& /*texture*/, int /*first_array_index*/, int /*array_size*/, int /*level*/)
		{
			return UnorderedAccessView::NullObject();
		}

		UnorderedAccessViewPtr Make2DUnorderedAccessView(Texture& /*texture*/, int /*first_array_index*/, int /*array_size*/, int /*level*/)
		{
			return UnorderedAccessView::NullObject();
		}

		UnorderedAccessViewPtr Make2DUnorderedAccessView(Texture& /*texture*/, int /*array_index*/, Texture::CubeFaces /*face*/, int /*level*/)
		{
			return UnorderedAccessView::NullObject();
		}

		UnorderedAccessViewPtr Make2DUnorderedAccessView(Texture& /*texture*/, int /*array_index*/, uint32_t /*slice*/, int /*level*/)
		{
			return UnorderedAccessView::NullObject();
		}

		UnorderedAccessViewPtr MakeCubeUnorderedAccessView(Texture& /*texture*/, int /*array_index*/, int /*level*/)
		{
			return UnorderedAccessView::NullObject();
		}

		UnorderedAccessViewPtr Make3DUnorderedAccessView(Texture& /*texture*/, int /*array_index*/, uint32_t /*first_slice*/, uint32_t /*num_slices*/, int /*level*/)
		{
			return UnorderedAccessView::NullObject();
		}

		UnorderedAccessViewPtr MakeGraphicsBufferUnorderedAccessView(GraphicsBuffer& /*gbuffer*/, ElementFormat /*pf*/)
		{
			return UnorderedAccessView::NullObject();
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

	private:
		virtual void DoSuspend() KLAYGE_OVERRIDE
		{
		}
		virtual void DoResume() KLAYGE_OVERRIDE
		{
		}
	};


	RenderFactory::~RenderFactory()
	{
		for (auto& rs : rs_pool_)
		{
			rs.second.reset();
		}
		for (auto& dss : dss_pool_)
		{
			dss.second.reset();
		}
		for (auto& bs : bs_pool_)
		{
			bs.second.reset();
		}
		for (auto& ss : ss_pool_)
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
	
	void RenderFactory::Suspend()
	{
		if (re_)
		{
			re_->Suspend();
		}
		this->DoSuspend();
	}
	
	void RenderFactory::Resume()
	{
		this->DoResume();
		if (re_)
		{
			re_->Resume();
		}
	}

	RasterizerStateObjectPtr RenderFactory::MakeRasterizerStateObject(RasterizerStateDesc const & desc)
	{
		RasterizerStateObjectPtr ret;

		auto iter = rs_pool_.find(desc);
		if (iter == rs_pool_.end())
		{
			ret = this->DoMakeRasterizerStateObject(desc);
			KLAYGE_EMPLACE(rs_pool_, desc, ret);
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

		auto iter = dss_pool_.find(desc);
		if (iter == dss_pool_.end())
		{
			ret = this->DoMakeDepthStencilStateObject(desc);
			KLAYGE_EMPLACE(dss_pool_, desc, ret);
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

		auto iter = bs_pool_.find(desc);
		if (iter == bs_pool_.end())
		{
			ret = this->DoMakeBlendStateObject(desc);
			KLAYGE_EMPLACE(bs_pool_, desc, ret);
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

		auto iter = ss_pool_.find(desc);
		if (iter == ss_pool_.end())
		{
			ret = this->DoMakeSamplerStateObject(desc);
			KLAYGE_EMPLACE(ss_pool_, desc, ret);
		}
		else
		{
			ret = iter->second;
		}

		return ret;
	}
}
