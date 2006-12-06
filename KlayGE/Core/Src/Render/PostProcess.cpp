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
			: RenderableHelper(L"PostProcess")
	{
		RenderFactory& rf = Context::Instance().RenderFactoryInstance();

		rl_ = rf.MakeRenderLayout(RenderLayout::BT_TriangleFan);

		pos_vb_ = rf.MakeVertexBuffer(BU_Static);
		pos_vb_->Resize(sizeof(float2) * 4);
		{
			GraphicsBuffer::Mapper mapper(*pos_vb_, BA_Write_Only);
			float2* addr = mapper.Pointer<float2>();
			addr[0] = float2(-1, +1);
			addr[1] = float2(+1, +1);
			addr[2] = float2(+1, -1);
			addr[3] = float2(-1, -1);
			box_ = Box(float3(-1, -1, -1), float3(1, 1, 1));
		}
		rl_->BindVertexStream(pos_vb_, boost::make_tuple(vertex_element(VEU_Position, 0, EF_GR32F)));

		technique_ = tech;
	}

	void PostProcess::Source(TexturePtr const & tex, bool flipping)
	{
		src_texture_ = tex;
		flipping_ = flipping;
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
		*(technique_->Effect().ParameterByName("src_sampler")) = src_texture_;

		RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
		float4 texel_to_pixel = re.TexelToPixelOffset();
		texel_to_pixel.x() /= render_target_->Width() / 2;
		texel_to_pixel.y() /= render_target_->Height() / 2;
		*(technique_->Effect().ParameterByName("texel_to_pixel_offset")) = texel_to_pixel;

		*(technique_->Effect().ParameterByName("flipping")) = flipping_ ? -1 : +1;
	}
}
