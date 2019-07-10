/**
* @file SSSBlur.cpp
* @author Minmin Gong
*
* @section DESCRIPTION
*
* This source file is part of KlayGE
* For the latest info, see http://www.klayge.org
*
* @section LICENSE
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published
* by the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*
* You may alternatively use this source under the terms of
* the KlayGE Proprietary License (KPL). You can obtained such a license
* from http://www.klayge.org/licensing/.
*/

#include <KlayGE/KlayGE.hpp>
#include <KlayGE/RenderEffect.hpp>
#include <KlayGE/Camera.hpp>
#include <KlayGE/RenderView.hpp>

#include <KlayGE/SSSBlur.hpp>

namespace KlayGE
{
	SSSBlurPP::SSSBlurPP(bool multi_sample)
			: PostProcess(L"SSSBlurPP", false,
				MakeSpan<std::string>({"strength", "correction"}),
				MakeSpan<std::string>({
					multi_sample ? "src_tex_ms" : "src_tex",
					multi_sample ? "depth_tex_ms" : "depth_tex"
				}),
				MakeSpan<std::string>({"output"}),
				RenderEffectPtr(), nullptr)
	{
		RenderDeviceCaps const & caps = Context::Instance().RenderFactoryInstance().RenderEngineInstance().DeviceCaps();
		mrt_blend_support_ = (caps.max_simultaneous_rts > 1) && caps.independent_blend_support;

		RenderEffectPtr effect = SyncLoadRenderEffect("SSS.fxml");
		copy_tech_ = effect->TechniqueByName(multi_sample ? "SSSCopyMS" : "SSSCopy");
		blur_x_tech_ = effect->TechniqueByName(multi_sample ? "BlurXMS" : "BlurX");
		if (mrt_blend_support_)
		{
			std::string blur_y_name = multi_sample ? "BlurY1MS" : "BlurY1";
			for (uint32_t i = 0; i < 3; ++ i)
			{
				blur_y_name[5] = static_cast<char>('1' + i);
				blur_y_techs_[i] = effect->TechniqueByName(blur_y_name);
			}
		}
		else
		{
			blur_y_techs_[0] = blur_x_tech_;
			std::string accum_name = multi_sample ? "Accum1MS" : "Accum1";
			for (uint32_t i = 0; i < 3; ++ i)
			{
				accum_name[5] = static_cast<char>('1' + i);
				accum_techs_[i] = effect->TechniqueByName(accum_name);
			}
		}
		this->Technique(effect, blur_x_tech_);

		RenderFactory& rf = Context::Instance().RenderFactoryInstance();
		blur_x_fb_ = rf.MakeFrameBuffer();
		blur_y_fb_ = rf.MakeFrameBuffer();

		src_tex_param_ = effect->ParameterByName(multi_sample ? "src_tex_ms" : "src_tex");
		step_param_ = effect->ParameterByName("step");
		far_plane_param_ = effect->ParameterByName("far_plane");
	}

	void SSSBlurPP::InputPin(uint32_t index, ShaderResourceViewPtr const& srv)
	{
		PostProcess::InputPin(index, srv);

		if (0 == index)
		{
			auto const* tex = srv->TextureResource().get();
			RenderFactory& rf = Context::Instance().RenderFactoryInstance();
			blur_x_tex_ = rf.MakeTexture2D(tex->Width(0), tex->Height(0), 1, 1, tex->Format(), tex->SampleCount(), tex->SampleQuality(),
				EAH_GPU_Read | EAH_GPU_Write);
			blur_y_tex_ = rf.MakeTexture2D(tex->Width(0), tex->Height(0), 1, 1, tex->Format(), tex->SampleCount(), tex->SampleQuality(),
				EAH_GPU_Read | EAH_GPU_Write);

			blur_x_fb_->Attach(FrameBuffer::Attachment::Color0, rf.Make2DRtv(blur_x_tex_, 0, 1, 0));
			blur_x_fb_->Attach(frame_buffer_->AttachedDsv());
			blur_y_fb_->Attach(FrameBuffer::Attachment::Color0, rf.Make2DRtv(blur_y_tex_, 0, 1, 0));
			blur_y_fb_->Attach(frame_buffer_->AttachedDsv());

			if (mrt_blend_support_)
			{
				frame_buffer_->Attach(FrameBuffer::Attachment::Color1, rf.Make2DRtv(blur_y_tex_, 0, 1, 0));
			}
		}
	}

	void SSSBlurPP::Apply()
	{
		RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
		float sss_strength;
		params_[0].second->Value(sss_strength);

		Camera const& camera = *re.DefaultFrameBuffer()->Viewport()->Camera();
		*far_plane_param_ = camera.FarPlane();

		{
			re.BindFrameBuffer(blur_y_fb_);
			re.CurFrameBuffer()->AttachedRtv(FrameBuffer::Attachment::Color0)->ClearColor(Color(0, 0, 0, 0));
			technique_ = copy_tech_;
			*src_tex_param_ = this->InputPin(0);
			this->Render();
		}
		for (uint32_t i = 0; i < 3; ++ i)
		{
			re.BindFrameBuffer(blur_x_fb_);
			re.CurFrameBuffer()->AttachedRtv(FrameBuffer::Attachment::Color0)->ClearColor(Color(0, 0, 0, 0));
			technique_ = blur_x_tech_;
			*src_tex_param_ = blur_y_tex_;
			*step_param_ = float2((i + 1) * sss_strength / frame_buffer_->Width(), 0);
			this->Render();

			*src_tex_param_ = blur_x_tex_;
			*step_param_ = float2(0, (i + 1) * sss_strength / frame_buffer_->Height());
			if (mrt_blend_support_)
			{
				technique_ = blur_y_techs_[i];
			}
			else
			{
				re.BindFrameBuffer(blur_y_fb_);
				re.CurFrameBuffer()->AttachedRtv(FrameBuffer::Attachment::Color0)->ClearColor(Color(0, 0, 0, 0));
				technique_ = blur_y_techs_[0];
				this->Render();

				technique_ = accum_techs_[i];
				*src_tex_param_ = blur_y_tex_;
			}

			re.BindFrameBuffer(frame_buffer_);
			this->Render();
		}
	}
}
