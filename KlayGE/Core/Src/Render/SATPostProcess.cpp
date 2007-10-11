// SATPostProcess.cpp
// KlayGE Summed-Area Table后期处理类 头文件
// Ver 3.7.0
// 版权所有(C) 龚敏敏, 2007
// Homepage: http://klayge.sourceforge.net
//
// 3.7.0
// 初次建立 (2006.10.10)
//
// 修改记录
//////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/RenderEffect.hpp>
#include <KlayGE/FrameBuffer.hpp>

#include <boost/assert.hpp>

#include <KlayGE/PostProcess.hpp>
#include <KlayGE/SATPostProcess.hpp>

namespace KlayGE
{
	SATSeparableScanPostProcess::SATSeparableScanPostProcess(RenderTechniquePtr tech)
			: PostProcess(tech)
	{
	}

	SATSeparableScanPostProcess::~SATSeparableScanPostProcess()
	{
	}

	void SATSeparableScanPostProcess::Pass(uint32_t pass)
	{
		*(technique_->Effect().ParameterByName("addr_offset")) = pow(4.0f, static_cast<float>(pass)) / length_;
	}

	void SATSeparableScanPostProcess::Length(uint32_t length)
	{
		length_ = length;
	}


	SATScanXPostProcess::SATScanXPostProcess()
			: SATSeparableScanPostProcess(Context::Instance().RenderFactoryInstance().LoadEffect("SummedAreaTable.kfx")->TechniqueByName("SATScanX"))
	{
	}

	SATScanYPostProcess::SATScanYPostProcess()
			: SATSeparableScanPostProcess(Context::Instance().RenderFactoryInstance().LoadEffect("SummedAreaTable.kfx")->TechniqueByName("SATScanY"))
	{
	}


	SummedAreaTablePostProcess::SummedAreaTablePostProcess()
		: PostProcess(RenderTechniquePtr())
	{
	}

	void SummedAreaTablePostProcess::Source(TexturePtr const & tex, bool flipping)
	{
		PostProcess::Source(tex, flipping);

		uint32_t const width = tex->Width(0);
		uint32_t const height = tex->Height(0);

		scan_x_.Length(width);
		scan_y_.Length(height);

		num_pass_x_ = static_cast<uint32_t>(ceil(log(static_cast<float>(width)) / log(4.0f)));
		num_pass_y_ = static_cast<uint32_t>(ceil(log(static_cast<float>(height)) / log(4.0f)));

		RenderFactory& rf = Context::Instance().RenderFactoryInstance();

		inter_tex_[0] = rf.MakeTexture2D(width, height, 1, EF_ABGR32F);
		inter_tex_[1] = rf.MakeTexture2D(width, height, 1, EF_ABGR32F);

		inter_fb_[0] = rf.MakeFrameBuffer();
		inter_fb_[0]->Attach(FrameBuffer::ATT_Color0, rf.Make2DRenderView(*inter_tex_[0], 0));
		inter_fb_[1] = rf.MakeFrameBuffer();
		inter_fb_[1]->Attach(FrameBuffer::ATT_Color0, rf.Make2DRenderView(*inter_tex_[1], 0));
	}

	void SummedAreaTablePostProcess::Apply()
	{
		scan_x_.Pass(0);
		scan_x_.Source(src_texture_, flipping_);
		scan_x_.Destinate(inter_fb_[0]);
		scan_x_.Apply();

		index_ = true;
		for (uint32_t i = 1; i < num_pass_x_; ++ i)
		{
			scan_x_.Pass(i);
			scan_x_.Source(inter_tex_[!index_], inter_fb_[!index_]->RequiresFlipping());
			scan_x_.Destinate(inter_fb_[index_]);
			scan_x_.Apply();

			index_ = !index_;
		}

		for (uint32_t i = 0; i < num_pass_y_; ++ i)
		{
			scan_y_.Pass(i);
			scan_y_.Source(inter_tex_[!index_], inter_fb_[!index_]->RequiresFlipping());
			scan_y_.Destinate(inter_fb_[index_]);
			scan_y_.Apply();

			index_ = !index_;
		}
	}

	TexturePtr SummedAreaTablePostProcess::SATTexture()
	{
		return inter_tex_[!index_];
	}
}
