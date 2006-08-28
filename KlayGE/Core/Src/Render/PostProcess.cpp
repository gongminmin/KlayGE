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

		rl_ = rf.MakeRenderLayout(RenderLayout::BT_TriangleFan);

		float4 const & texel_to_pixel = rf.RenderEngineInstance().TexelToPixelOffset();
		if ((texel_to_pixel.x() != 0) || (texel_to_pixel.y() != 0))
		{
			pos_vb_ = rf.MakeVertexBuffer(BU_Static);
			pos_vb_->Resize(sizeof(float3) * 4);

			GraphicsBuffer::Mapper mapper(*pos_vb_, BA_Write_Only);
			float3* addr = mapper.Pointer<float3>();
			addr[0] = float3(-1, +1, 0);
			addr[1] = float3(+1, +1, 0);
			addr[2] = float3(+1, -1, 0);
			addr[3] = float3(-1, -1, 0);

			box_ = MathLib::compute_bounding_box<float>(&addr[0], &addr[4]);
		}
		else
		{
			pos_vb_ = rf.MakeVertexBuffer(BU_Dynamic);
			pos_vb_->Resize(sizeof(float3) * 4);
		}
		rl_->BindVertexStream(pos_vb_, boost::make_tuple(vertex_element(VEU_Position, 0, EF_BGR32F)));

		GraphicsBufferPtr tex_vb = rf.MakeVertexBuffer(BU_Static);
		tex_vb->Resize(sizeof(float2) * 4);
		{
			GraphicsBuffer::Mapper mapper(*tex_vb, BA_Write_Only);
			float2* addr = mapper.Pointer<float2>();
			addr[0] = float2(0, 0);
			addr[1] = float2(1, 0);
			addr[2] = float2(1, 1);
			addr[3] = float2(0, 1);
		}
		rl_->BindVertexStream(tex_vb, boost::make_tuple(vertex_element(VEU_TextureCoord, 0, EF_GR32F)));

		technique_ = tech;

		src_sampler_->AddressingMode(Sampler::TAT_Addr_U, Sampler::TAM_Clamp);
		src_sampler_->AddressingMode(Sampler::TAT_Addr_V, Sampler::TAM_Clamp);
	}

	void PostProcess::Source(TexturePtr const & tex, Sampler::TexFilterOp filter, Sampler::TexAddressingMode am)
	{
		src_sampler_->Filtering(filter);
		src_sampler_->SetTexture(tex);
		src_sampler_->AddressingMode(Sampler::TAT_Addr_U, am);
		src_sampler_->AddressingMode(Sampler::TAT_Addr_V, am);
	}

	void PostProcess::Destinate(RenderTargetPtr const & rt)
	{
		RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
		
		if (rt)
		{
			render_target_ = rt;
		}
		else
		{
			render_target_ = re.DefaultRenderTarget();
		}

		float4 const & texel_to_pixel = re.TexelToPixelOffset();
		float const x_offset = texel_to_pixel.x() / render_target_->Width();
		float const y_offset = texel_to_pixel.y() / render_target_->Height();

		{
			GraphicsBuffer::Mapper mapper(*pos_vb_, BA_Write_Only);
			float3* addr = mapper.Pointer<float3>();
			addr[0] = float3(-1 + x_offset, +1 + y_offset, 0);
			addr[1] = float3(+1 + x_offset, +1 + y_offset, 0);
			addr[2] = float3(+1 + x_offset, -1 + y_offset, 0);
			addr[3] = float3(-1 + x_offset, -1 + y_offset, 0);

			box_ = MathLib::compute_bounding_box<float>(&addr[0], &addr[4]);
		}
	}

	void PostProcess::Apply()
	{
		RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();

		RenderTargetPtr backup_rt = re.CurRenderTarget();
		re.BindRenderTarget(render_target_);

		this->Render();

		re.BindRenderTarget(backup_rt);
	}

	void PostProcess::OnRenderBegin()
	{
		*(technique_->Effect().ParameterByName("src_sampler")) = src_sampler_;
	}
}
