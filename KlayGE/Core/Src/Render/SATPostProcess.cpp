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
#include <KlayGE/Math.hpp>
#include <KlayGE/Util.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/Context.hpp>
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
		if (technique_)
		{
			child_sampler_ep_ = technique_->Effect().ParameterByName("child_sampler");
			addr_offset_ep_ = technique_->Effect().ParameterByName("addr_offset");
			length_ep_ = technique_->Effect().ParameterByName("length");
			scale_ep_ = technique_->Effect().ParameterByName("scale");
		}
	}

	void SATSeparableScanSweepPostProcess::ChildBuffer(TexturePtr const & tex)
	{
		*child_sampler_ep_ = tex;
	}

	void SATSeparableScanSweepPostProcess::AddrOffset(float3 offset)
	{
		*addr_offset_ep_ = offset;
	}

	void SATSeparableScanSweepPostProcess::Length(int length)
	{
		length_ = length;
		*length_ep_ = length;
	}

	void SATSeparableScanSweepPostProcess::Scale(float scale)
	{
		*scale_ep_ = scale;
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

		uint32_t const tex_width = tex->Width(0);
		uint32_t const tex_height = tex->Height(0);

		RenderFactory& rf = Context::Instance().RenderFactoryInstance();

		uint32_t width = tex_width;
		uint32_t height = tex_height;

		std::vector<uint32_t> widths;
		while (width >= 1)
		{
			widths.push_back(width);

			if (1 == width)
			{
				break;
			}
			width = (width + 3) / 4;
		}

		std::vector<uint32_t> heights;
		while (height >= 1)
		{
			heights.push_back(height);

			if (1 == height)
			{
				break;
			}
			height = (height + 3) / 4;
		}

		inter_tex_x_up_.resize(widths.size());
		inter_fb_x_up_.resize(widths.size());
		inter_tex_x_down_.resize(widths.size());
		inter_fb_x_down_.resize(widths.size());
		inter_tex_y_up_.resize(heights.size());
		inter_fb_y_up_.resize(heights.size());
		inter_tex_y_down_.resize(heights.size());
		inter_fb_y_down_.resize(heights.size());

		{
			inter_tex_x_up_[0] = tex;
			inter_fb_x_up_[0] = rf.MakeFrameBuffer();
			inter_fb_x_up_[0]->Attach(FrameBuffer::ATT_Color0, rf.Make2DRenderView(*inter_tex_x_up_[0], 0));
		}
		for (size_t i = 1; i < widths.size(); ++ i)
		{
			inter_tex_x_up_[i] = rf.MakeTexture2D(widths[i], tex_height, 1, EF_ABGR32F, 1, 0, EAH_GPU_Read | EAH_GPU_Write, NULL);
			inter_fb_x_up_[i] = rf.MakeFrameBuffer();
			inter_fb_x_up_[i]->Attach(FrameBuffer::ATT_Color0, rf.Make2DRenderView(*inter_tex_x_up_[i], 0));
		}
		{
			inter_tex_x_down_[0] = inter_tex_x_up_.back();
			inter_fb_x_down_[0] = inter_fb_x_up_.back();
		}
		for (size_t i = 1; i < widths.size(); ++ i)
		{
			inter_tex_x_down_[i] = rf.MakeTexture2D(widths[widths.size() - 1 - i], tex_height, 1, EF_ABGR32F, 1, 0, EAH_GPU_Read | EAH_GPU_Write, NULL);
			inter_fb_x_down_[i] = rf.MakeFrameBuffer();
			inter_fb_x_down_[i]->Attach(FrameBuffer::ATT_Color0, rf.Make2DRenderView(*inter_tex_x_down_[i], 0));
		}
		{
			inter_tex_y_up_[0] = inter_tex_x_down_.back();
			inter_fb_y_up_[0] = inter_fb_x_down_.back();
		}
		for (size_t i = 1; i < heights.size(); ++ i)
		{
			inter_tex_y_up_[i] = rf.MakeTexture2D(tex_width, heights[i], 1, EF_ABGR32F, 1, 0, EAH_GPU_Read | EAH_GPU_Write, NULL);
			inter_fb_y_up_[i] = rf.MakeFrameBuffer();
			inter_fb_y_up_[i]->Attach(FrameBuffer::ATT_Color0, rf.Make2DRenderView(*inter_tex_y_up_[i], 0));
		}
		{
			inter_tex_y_down_[0] = inter_tex_y_up_.back();
			inter_fb_y_down_[0] = inter_fb_y_up_.back();
		}
		for (size_t i = 1; i < heights.size(); ++ i)
		{
			inter_tex_y_down_[i] = rf.MakeTexture2D(tex_width, heights[heights.size() - 1 - i], 1, EF_ABGR32F, 1, 0, EAH_GPU_Read | EAH_GPU_Write, NULL);
			inter_fb_y_down_[i] = rf.MakeFrameBuffer();
			inter_fb_y_down_[i]->Attach(FrameBuffer::ATT_Color0, rf.Make2DRenderView(*inter_tex_y_down_[i], 0));
		}
	}

	void SummedAreaTablePostProcess::Apply()
	{
		for (size_t i = 0; i < inter_tex_x_up_.size() - 1; ++ i)
		{
			uint32_t const parent_length = inter_tex_x_up_[i + 1]->Width(0);
			uint32_t const child_length = inter_tex_x_up_[i]->Width(0);

			scan_x_up_.Length(child_length);
			scan_x_up_.AddrOffset(float3(0.5f / child_length, 1.5f / child_length, 0));
			scan_x_up_.Scale((parent_length * 4.0f) / child_length);
			if (0 == i)
			{
				scan_x_up_.Source(inter_tex_x_up_[i], flipping_);
			}
			else
			{
				scan_x_up_.Source(inter_tex_x_up_[i], inter_fb_x_up_[i]->RequiresFlipping());
			}
			scan_x_up_.Destinate(inter_fb_x_up_[i + 1]);
			scan_x_up_.Apply();
		}

		for (size_t i = 0; i < inter_tex_x_down_.size() - 1; ++ i)
		{
			uint32_t const parent_length = inter_tex_x_down_[i]->Width(0);
			uint32_t const child_length = inter_tex_x_down_[i + 1]->Width(0);

			scan_x_down_.Length(child_length);
			scan_x_down_.Source(inter_tex_x_down_[i], inter_fb_x_down_[i]->RequiresFlipping());
			scan_x_down_.ChildBuffer(inter_tex_x_up_[inter_tex_x_down_.size() - 2 - i]);
			scan_x_down_.AddrOffset(float3(1.0f / parent_length, 1.0f / child_length, 2.0f / child_length));
			scan_x_down_.Scale(child_length / (parent_length * 4.0f));
			scan_x_down_.Destinate(inter_fb_x_down_[i + 1]);
			scan_x_down_.Apply();
		}

		for (size_t i = 0; i < inter_tex_y_up_.size() - 1; ++ i)
		{
			uint32_t const parent_length = inter_tex_y_up_[i + 1]->Height(0);
			uint32_t const child_length = inter_tex_y_up_[i]->Height(0);

			scan_y_up_.Length(child_length);
			scan_y_up_.AddrOffset(float3(0.5f / child_length, 1.5f / child_length, 0));
			scan_y_up_.Scale((parent_length * 4.0f) / child_length);
			scan_y_up_.Source(inter_tex_y_up_[i], inter_fb_y_up_[i]->RequiresFlipping());
			scan_y_up_.Destinate(inter_fb_y_up_[i + 1]);
			scan_y_up_.Apply();
		}

		for (size_t i = 0; i < inter_tex_y_down_.size() - 1; ++ i)
		{
			uint32_t const parent_length = inter_tex_y_down_[i]->Height(0);
			uint32_t const child_length = inter_tex_y_down_[i + 1]->Height(0);

			scan_y_down_.Length(child_length);
			scan_y_down_.Source(inter_tex_y_down_[i], inter_fb_y_down_[i]->RequiresFlipping());
			scan_y_down_.ChildBuffer(inter_tex_y_up_[inter_tex_y_down_.size() - 2 - i]);
			scan_y_down_.AddrOffset(float3(1.0f / parent_length, 1.0f / child_length, 2.0f / child_length));
			scan_y_down_.Scale(child_length / (parent_length * 4.0f));
			scan_y_down_.Destinate(inter_fb_y_down_[i + 1]);
			scan_y_down_.Apply();
		}
	}

	TexturePtr SummedAreaTablePostProcess::SATTexture()
	{
		return inter_tex_y_down_.back();
	}
}
