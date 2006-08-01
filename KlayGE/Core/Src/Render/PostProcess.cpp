// PostProcess.cpp
// KlayGE 后期处理类 实现文件
// Ver 3.3.0
// 版权所有(C) 龚敏敏, 2006
// Homepage: http://klayge.sourceforge.net
//
// 3.3.0
// 初次建立 (2006.6.23)
//
// 修改记录
//////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/RenderEffect.hpp>
#include <KlayGE/RenderableHelper.hpp>
#include <KlayGE/Sampler.hpp>
#include <KlayGE/FrameBuffer.hpp>

#include <KlayGE/PostProcess.hpp>

namespace KlayGE
{
	PostProcess::PostProcess(KlayGE::RenderTechniquePtr tech)
			: RenderableHelper(L"PostProcess"),
				src_sampler_(new Sampler)
	{
		RenderFactory& rf = Context::Instance().RenderFactoryInstance();

		rl_ = rf.MakeRenderLayout(RenderLayout::BT_TriangleStrip);

		std::vector<float3> pos;
		pos.push_back(float3(-1, 1, 0));
		pos.push_back(float3(1, 1, 0));
		pos.push_back(float3(-1, -1, 0));
		pos.push_back(float3(1, -1, 0));

		GraphicsBufferPtr pos_vb = rf.MakeVertexBuffer(BU_Static);
		pos_vb->Resize(static_cast<uint32_t>(sizeof(pos[0]) * pos.size()));
		{
			GraphicsBuffer::Mapper mapper(*pos_vb, BA_Write_Only);
			std::copy(pos.begin(), pos.end(), mapper.Pointer<float3>());
		}
		rl_->BindVertexStream(pos_vb, boost::make_tuple(vertex_element(VEU_Position, 0, EF_BGR32F)));

		std::vector<float2> tex;
		tex.push_back(float2(0, 0));
		tex.push_back(float2(1, 0));
		tex.push_back(float2(0, 1));
		tex.push_back(float2(1, 1));

		GraphicsBufferPtr tex_vb = rf.MakeVertexBuffer(BU_Static);
		tex_vb->Resize(static_cast<uint32_t>(sizeof(tex[0]) * tex.size()));
		{
			GraphicsBuffer::Mapper mapper(*tex_vb, BA_Write_Only);
			std::copy(tex.begin(), tex.end(), mapper.Pointer<float2>());
		}
		rl_->BindVertexStream(tex_vb, boost::make_tuple(vertex_element(VEU_TextureCoord, 0, EF_GR32F)));

		std::vector<uint16_t> index;
		index.push_back(0);
		index.push_back(1);
		index.push_back(2);
		index.push_back(3);

		GraphicsBufferPtr ib = rf.MakeIndexBuffer(BU_Static);
		ib->Resize(static_cast<uint32_t>(index.size() * sizeof(index[0])));
		{
			GraphicsBuffer::Mapper mapper(*ib, BA_Write_Only);
			std::copy(index.begin(), index.end(), mapper.Pointer<uint16_t>());
		}
		rl_->BindIndexStream(ib, EF_R16);

		box_ = MathLib::compute_bounding_box<float>(pos.begin(), pos.end());

		technique_ = tech;

		src_sampler_->AddressingMode(Sampler::TAT_Addr_U, Sampler::TAM_Clamp);
		src_sampler_->AddressingMode(Sampler::TAT_Addr_V, Sampler::TAM_Clamp);
	}

	void PostProcess::Source(TexturePtr const & tex, Sampler::TexFilterOp filter)
	{
		src_sampler_->Filtering(filter);
		src_sampler_->SetTexture(tex);
	}

	void PostProcess::Destinate(RenderTargetPtr const & rt)
	{
		render_target_ = rt;
	}

	void PostProcess::Apply()
	{
		RenderEngine& render_engine(Context::Instance().RenderFactoryInstance().RenderEngineInstance());

		render_engine.BindRenderTarget(render_target_);
		render_engine.Clear(RenderEngine::CBM_Color);

		this->Render();

		render_engine.BindRenderTarget(RenderTargetPtr());
	}

	void PostProcess::OnRenderBegin()
	{
		*(technique_->Effect().ParameterByName("src_sampler")) = src_sampler_;
	}
}
