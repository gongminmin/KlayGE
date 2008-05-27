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
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/Math.hpp>
#include <KlayGE/Sampler.hpp>
#include <KlayGE/RenderEffect.hpp>
#include <KlayGE/FrameBuffer.hpp>

#include <boost/assert.hpp>

#include <KlayGE/PostProcess.hpp>
#include <KlayGE/SATPostProcess.hpp>

namespace KlayGE
{
	SATSeparableScanSweepPostProcess::SATSeparableScanSweepPostProcess(RenderTechniquePtr tech, bool dir)
			: PostProcess(tech),
				dir_(dir)
	{
	}

	void SATSeparableScanSweepPostProcess::Step(int step)
	{
		*(technique_->Effect().ParameterByName("addr_offset")) = static_cast<float>(step) / length_ * (dir_ ? 0.5f : 1);
		*(technique_->Effect().ParameterByName("step")) = step;
	}

	void SATSeparableScanSweepPostProcess::Length(int length)
	{
		length_ = length;
		*(technique_->Effect().ParameterByName("length")) = length;
	}


	SummedAreaTablePostProcess::SummedAreaTablePostProcess()
		: PostProcess(RenderTechniquePtr()),
			scan_x_up_(Context::Instance().RenderFactoryInstance().LoadEffect("SummedAreaTable.kfx")->TechniqueByName("SATScanXUpSweep"), true),
			scan_x_down_(Context::Instance().RenderFactoryInstance().LoadEffect("SummedAreaTable.kfx")->TechniqueByName("SATScanXDownSweep"), false),
			scan_y_up_(Context::Instance().RenderFactoryInstance().LoadEffect("SummedAreaTable.kfx")->TechniqueByName("SATScanYUpSweep"), true),
			scan_y_down_(Context::Instance().RenderFactoryInstance().LoadEffect("SummedAreaTable.kfx")->TechniqueByName("SATScanYDownSweep"), false)
	{
	}

	void SummedAreaTablePostProcess::Source(TexturePtr const & tex, bool flipping)
	{
		PostProcess::Source(tex, flipping);

		uint32_t const width = tex->Width(0);
		uint32_t const height = tex->Height(0);

		scan_x_up_.Length(width);
		scan_x_down_.Length(width);
		scan_y_up_.Length(height);
		scan_y_down_.Length(height);

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
		index_ = true;
		uint32_t step = 2;
		do
		{
			scan_x_up_.Step(step);
			if (2 == step)
			{
				scan_x_up_.Source(src_texture_, flipping_);
			}
			else
			{
				scan_x_up_.Source(inter_tex_[!index_], inter_fb_[!index_]->RequiresFlipping());
			}
			scan_x_up_.Destinate(inter_fb_[index_]);
			scan_x_up_.Apply();

			index_ = !index_;

			step *= 2;
		} while (step <= src_texture_->Width(0));

		step = src_texture_->Width(0) / 2;
		do
		{
			scan_x_down_.Step(step);
			scan_x_down_.Source(inter_tex_[!index_], inter_fb_[!index_]->RequiresFlipping());
			scan_x_down_.Destinate(inter_fb_[index_]);
			scan_x_down_.Apply();

			index_ = !index_;

			step /= 2;
		} while (step >= 1);

		step = 2;
		do
		{
			scan_y_up_.Step(step);
			scan_y_up_.Source(inter_tex_[!index_], inter_fb_[!index_]->RequiresFlipping());
			scan_y_up_.Destinate(inter_fb_[index_]);
			scan_y_up_.Apply();

			index_ = !index_;
		
			step *= 2;
		} while (step <= src_texture_->Height(0));

		step = src_texture_->Height(0) / 2;
		do
		{
			scan_y_down_.Step(step);
			scan_y_down_.Source(inter_tex_[!index_], inter_fb_[!index_]->RequiresFlipping());
			scan_y_down_.Destinate(inter_fb_[index_]);
			scan_y_down_.Apply();

			index_ = !index_;

			step /= 2;
		} while (step >= 1);
	}

	TexturePtr SummedAreaTablePostProcess::SATTexture()
	{
		return inter_tex_[!index_];
	}
}
