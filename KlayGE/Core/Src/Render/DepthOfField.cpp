/**
 * @file DepthOfField.cpp
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
#include <KFL/CXX2a/span.hpp>

#include <KFL/ErrorHandling.hpp>
#include <KlayGE/PostProcess.hpp>
#include <KlayGE/RenderEffect.hpp>
#include <KlayGE/SATPostProcess.hpp>

#include <KlayGE/DepthOfField.hpp>

namespace KlayGE
{
	DepthOfField::DepthOfField()
		: PostProcess(L"DepthOfField", false, MakeSpan<std::string>(), MakeSpan<std::string>({"color_tex", "depth_tex"}),
			  MakeSpan<std::string>({"output"}), RenderEffectPtr(), nullptr)
	{
		RenderDeviceCaps const& caps = Context::Instance().RenderFactoryInstance().RenderEngineInstance().DeviceCaps();
		cs_support_ = caps.cs_support && (caps.max_shader_model >= ShaderModel(5, 0));

		RenderEffectPtr effect = SyncLoadRenderEffect("DepthOfField.fxml");
		this->Technique(effect, effect->TechniqueByName("DepthOfFieldNormalization"));

		RenderFactory& rf = Context::Instance().RenderFactoryInstance();
		spread_fb_ = rf.MakeFrameBuffer();

		if (cs_support_)
		{
			spreading_pp_ = SyncLoadPostProcess("DepthOfField.ppml", "SpreadingCS");
			sat_pp_ = MakeSharedPtr<SATPostProcessCS>();
		}
		else
		{
			spreading_pp_ = SyncLoadPostProcess("DepthOfField.ppml", "Spreading");
			sat_pp_ = MakeSharedPtr<SATPostProcess>();
		}
		spreading_pp_->SetParam(1, static_cast<float>(max_radius_));

		normalization_rl_ = rf.MakeRenderLayout();
		normalization_rl_->TopologyType(RenderLayout::TT_TriangleStrip);

		normalization_tech_ = effect_->TechniqueByName("DepthOfFieldNormalization");
		blur_factor_tech_ = effect_->TechniqueByName("DepthOfFieldBlurFactor");

		focus_plane_inv_range_param_ = effect_->ParameterByName("focus_plane_inv_range");
		depth_tex_param_ = effect_->ParameterByName("depth_tex");
		src_tex_param_ = effect_->ParameterByName("src_tex");
	}

	void DepthOfField::FocusPlane(float focus_plane)
	{
		focus_plane_ = focus_plane;
	}

	float DepthOfField::FocusPlane() const
	{
		return focus_plane_;
	}

	void DepthOfField::FocusRange(float focus_range)
	{
		focus_range_ = focus_range;
	}

	float DepthOfField::FocusRange() const
	{
		return focus_range_;
	}

	void DepthOfField::ShowBlurFactor(bool show)
	{
		show_blur_factor_ = show;
		if (show_blur_factor_)
		{
			technique_ = blur_factor_tech_;
		}
		else
		{
			technique_ = normalization_tech_;
		}
	}

	bool DepthOfField::ShowBlurFactor() const
	{
		return show_blur_factor_;
	}

	void DepthOfField::InputPin(uint32_t index, ShaderResourceViewPtr const& srv)
	{
		PostProcess::InputPin(index, srv);

		if (0 == index)
		{
			auto const* tex = srv->TextureResource().get();
			uint32_t const width = tex->Width(0) + max_radius_ * 4 + 1;
			uint32_t const height = tex->Height(0) + max_radius_ * 4 + 1;
			if (!spread_tex_ || spread_tex_->Width(0) != width || spread_tex_->Height(0) != height)
			{
				auto const& caps = Context::Instance().RenderFactoryInstance().RenderEngineInstance().DeviceCaps();
				auto const fmt = caps.BestMatchTextureRenderTargetFormat(MakeSpan({EF_ABGR32F, EF_ABGR16F}), 1, 0);
				BOOST_ASSERT(fmt != EF_Unknown);

				RenderFactory& rf = Context::Instance().RenderFactoryInstance();
				spread_tex_ =
					rf.MakeTexture2D(width, height, 1, 1, fmt, 1, 0, EAH_GPU_Read | EAH_GPU_Write | (cs_support_ ? EAH_GPU_Unordered : 0));
				auto spread_rtv = rf.Make2DRtv(spread_tex_, 0, 1, 0);
				spread_fb_->Attach(FrameBuffer::Attachment::Color0, spread_rtv);

				spreading_pp_->SetParam(0, float4(static_cast<float>(width), static_cast<float>(height), 1.0f / width, 1.0f / height));

				{
					float4 const pos[] = {float4(-1, +1, 0 + (max_radius_ * 2 + 0.0f) / width, 0 + (max_radius_ * 2 + 0.0f) / height),
						float4(+1, +1, 1 - (max_radius_ * 2 + 1.0f) / width, 0 + (max_radius_ * 2 + 0.0f) / height),
						float4(-1, -1, 0 + (max_radius_ * 2 + 0.0f) / width, 1 - (max_radius_ * 2 + 1.0f) / height),
						float4(+1, -1, 1 - (max_radius_ * 2 + 1.0f) / width, 1 - (max_radius_ * 2 + 1.0f) / height)};

					GraphicsBufferPtr pos_vb = rf.MakeVertexBuffer(BU_Static, EAH_GPU_Read | EAH_Immutable, sizeof(pos), &pos[0]);
					normalization_rl_->BindVertexStream(pos_vb, VertexElement(VEU_Position, 0, EF_ABGR32F));
				}

				sat_pp_->InputPin(0, rf.MakeTextureSrv(spread_tex_));

				if (cs_support_)
				{
					auto spread_uav = rf.Make2DUav(spread_tex_, 0, 1, 0);
					spreading_pp_->OutputPin(0, spread_uav);
					sat_pp_->OutputPin(0, spread_uav);
				}
				else
				{
					spreading_pp_->OutputPin(0, spread_rtv);
					sat_pp_->OutputPin(0, spread_rtv);
				}
			}
		}
	}

	void DepthOfField::Apply()
	{
		if (show_blur_factor_)
		{
			*focus_plane_inv_range_param_ = float3(focus_plane_, -focus_plane_ / focus_range_, 1.0f / focus_range_);
			*depth_tex_param_ = this->InputPin(1);
			PostProcess::Apply();
		}
		else
		{
			RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
			spreading_pp_->SetParam(2, float3(focus_plane_, -focus_plane_ / focus_range_, 1.0f / focus_range_));
			spreading_pp_->InputPin(0, this->InputPin(0));
			spreading_pp_->InputPin(1, this->InputPin(1));
			spreading_pp_->Apply();

			sat_pp_->Apply();

			*src_tex_param_ = spread_tex_;

			re.BindFrameBuffer(frame_buffer_);
			re.Render(*effect_, *technique_, *normalization_rl_);
		}
	}


	BokehFilter::BokehFilter()
		: PostProcess(L"BokehFilter", false, MakeSpan<std::string>(), MakeSpan<std::string>({"color_tex", "depth_tex"}),
			  MakeSpan<std::string>({"output"}), RenderEffectPtr(), nullptr)
	{
		RenderDeviceCaps const& caps = Context::Instance().RenderFactoryInstance().RenderEngineInstance().DeviceCaps();
		gs_support_ = caps.gs_support;

		RenderEffectPtr effect = SyncLoadRenderEffect("DepthOfField.fxml");
		if (gs_support_)
		{
			this->Technique(effect, effect->TechniqueByName("SeparateBokeh4"));
		}
		else
		{
			this->Technique(effect, effect->TechniqueByName("SeparateBokeh"));
		}

		*(effect->ParameterByName("max_radius")) = static_cast<float>(max_radius_);

		RenderFactory& rf = Context::Instance().RenderFactoryInstance();
		bokeh_fb_ = rf.MakeFrameBuffer();

		bokeh_rl_ = rf.MakeRenderLayout();

		merge_bokeh_pp_ = SyncLoadPostProcess("DepthOfField.ppml", "MergeBokeh");
		merge_bokeh_pp_->SetParam(2, static_cast<float>(max_radius_));

		focus_plane_inv_range_param_ = effect_->ParameterByName("focus_plane_inv_range");
		lum_threshold_param_ = effect_->ParameterByName("lum_threshold");
		color_tex_param_ = effect_->ParameterByName("color_tex");
		depth_tex_param_ = effect_->ParameterByName("depth_tex");
	}

	void BokehFilter::FocusPlane(float focus_plane)
	{
		focus_plane_ = focus_plane;
	}

	float BokehFilter::FocusPlane() const
	{
		return focus_plane_;
	}

	void BokehFilter::FocusRange(float focus_range)
	{
		focus_range_ = focus_range;
	}

	float BokehFilter::FocusRange() const
	{
		return focus_range_;
	}

	void BokehFilter::LuminanceThreshold(float lum_threshold)
	{
		lum_threshold_ = lum_threshold;
	}

	float BokehFilter::LuminanceThreshold() const
	{
		return lum_threshold_;
	}

	void BokehFilter::InputPin(uint32_t index, ShaderResourceViewPtr const& srv)
	{
		PostProcess::InputPin(index, srv);

		if (0 == index)
		{
			auto const* tex = srv->TextureResource().get();
			uint32_t const in_width = tex->Width(0) / 2;
			uint32_t const in_height = tex->Height(0) / 2;
			uint32_t const out_width = in_width * 2 + max_radius_ * 4;
			uint32_t const out_height = in_height;
			if (!bokeh_tex_ || (bokeh_tex_->Width(0) != out_width) || (bokeh_tex_->Height(0) != out_height))
			{
				*(effect_->ParameterByName("in_width_height")) =
					float4(static_cast<float>(in_width), static_cast<float>(in_height), 1.0f / in_width, 1.0f / in_height);
				*(effect_->ParameterByName("bokeh_width_height")) =
					float4(static_cast<float>(out_width), static_cast<float>(out_height), 1.0f / out_width, 1.0f / out_height);
				*(effect_->ParameterByName("background_offset")) = static_cast<float>(in_width + max_radius_ * 4);

				RenderFactory& rf = Context::Instance().RenderFactoryInstance();
				RenderDeviceCaps const& caps = rf.RenderEngineInstance().DeviceCaps();
				bokeh_tex_ = rf.MakeTexture2D(out_width, out_height, 1, 1, tex->Format(), 1, 0, EAH_GPU_Read | EAH_GPU_Write);
				bokeh_fb_->Attach(FrameBuffer::Attachment::Color0, rf.Make2DRtv(bokeh_tex_, 0, 1, 0));

				if (gs_support_)
				{
					bokeh_rl_->TopologyType(RenderLayout::TT_PointList);

					std::vector<float2> points;
					for (uint32_t y = 0; y < in_height; y += 2)
					{
						for (uint32_t x = 0; x < in_width; x += 2)
						{
							points.push_back(float2((x + 0.5f) / in_width, (y + 0.5f) / in_height));
						}
					}

					GraphicsBufferPtr pos_vb = rf.MakeVertexBuffer(
						BU_Static, EAH_GPU_Read | EAH_Immutable, static_cast<uint32_t>(points.size() * sizeof(points[0])), &points[0]);
					bokeh_rl_->BindVertexStream(pos_vb, VertexElement(VEU_Position, 0, EF_GR32F));
				}
				else
				{
					RenderEngine& re = rf.RenderEngineInstance();
					float const flipping = re.RequiresFlipping() ? -1.0f : +1.0f;

					std::vector<float4> points;
					for (uint32_t y = 0; y < in_height; y += 2)
					{
						for (uint32_t x = 0; x < in_width; x += 2)
						{
							points.push_back(float4((x + 0.5f) / in_width, (y + 0.5f) / in_height, -1, +1 * flipping));
							points.push_back(float4((x + 0.5f) / in_width, (y + 0.5f) / in_height, +1, +1 * flipping));
							points.push_back(float4((x + 0.5f) / in_width, (y + 0.5f) / in_height, -1, -1 * flipping));
							points.push_back(float4((x + 0.5f) / in_width, (y + 0.5f) / in_height, +1, -1 * flipping));
						}
					}

					std::vector<uint32_t> indices;
					uint32_t base = 0;
					if (caps.primitive_restart_support)
					{
						bokeh_rl_->TopologyType(RenderLayout::TT_TriangleStrip);

						for (uint32_t y = 0; y < in_height; y += 2)
						{
							for (uint32_t x = 0; x < in_width; x += 2)
							{
								indices.push_back(base + 0);
								indices.push_back(base + 1);
								indices.push_back(base + 2);
								indices.push_back(base + 3);
								indices.push_back(0xFFFFFFFF);

								base += 4;
							}
						}
					}
					else
					{
						bokeh_rl_->TopologyType(RenderLayout::TT_TriangleList);

						for (uint32_t y = 0; y < in_height; y += 2)
						{
							for (uint32_t x = 0; x < in_width; x += 2)
							{
								indices.push_back(base + 0);
								indices.push_back(base + 1);
								indices.push_back(base + 2);

								indices.push_back(base + 2);
								indices.push_back(base + 1);
								indices.push_back(base + 3);

								base += 4;
							}
						}
					}

					GraphicsBufferPtr pos_vb = rf.MakeVertexBuffer(
						BU_Static, EAH_GPU_Read | EAH_Immutable, static_cast<uint32_t>(points.size() * sizeof(points[0])), &points[0]);
					bokeh_rl_->BindVertexStream(pos_vb, VertexElement(VEU_Position, 0, EF_ABGR32F));

					GraphicsBufferPtr pos_ib = rf.MakeIndexBuffer(
						BU_Static, EAH_GPU_Read | EAH_Immutable, static_cast<uint32_t>(indices.size() * sizeof(indices[0])), &indices[0]);
					bokeh_rl_->BindIndexStream(pos_ib, EF_R32UI);
				}

				merge_bokeh_pp_->SetParam(
					0, float4(static_cast<float>(in_width), static_cast<float>(in_height), 1.0f / in_width, 1.0f / in_height));
				merge_bokeh_pp_->SetParam(
					1, float4(static_cast<float>(out_width), static_cast<float>(out_height), 1.0f / out_width, 1.0f / out_height));
				merge_bokeh_pp_->SetParam(4, static_cast<float>(in_width + max_radius_ * 4));
				merge_bokeh_pp_->InputPin(0, rf.MakeTextureSrv(bokeh_tex_));
			}
		}
		else
		{
			merge_bokeh_pp_->InputPin(index, srv);
		}
	}

	void BokehFilter::OutputPin(uint32_t index, RenderTargetViewPtr const& rtv)
	{
		merge_bokeh_pp_->OutputPin(index, rtv);
	}

	void BokehFilter::OutputPin(uint32_t index, UnorderedAccessViewPtr const& uav)
	{
		merge_bokeh_pp_->OutputPin(index, uav);
	}

	RenderTargetViewPtr const& BokehFilter::RtvOutputPin(uint32_t index) const
	{
		return merge_bokeh_pp_->RtvOutputPin(index);
	}

	UnorderedAccessViewPtr const& BokehFilter::UavOutputPin(uint32_t index) const
	{
		return merge_bokeh_pp_->UavOutputPin(index);
	}

	void BokehFilter::Apply()
	{
		RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
		*focus_plane_inv_range_param_ = float3(focus_plane_, -focus_plane_ / focus_range_, 1.0f / focus_range_);
		*lum_threshold_param_ = lum_threshold_;
		*color_tex_param_ = this->InputPin(0);
		*depth_tex_param_ = this->InputPin(1);

		re.BindFrameBuffer(bokeh_fb_);
		bokeh_fb_->Clear(FrameBuffer::CBM_Color, Color(0, 0, 0, 0), 1, 0);
		re.Render(*effect_, *technique_, *bokeh_rl_);

		merge_bokeh_pp_->SetParam(3, float3(focus_plane_, -focus_plane_ / focus_range_, 1.0f / focus_range_));
		merge_bokeh_pp_->Apply();
	}
} // namespace KlayGE
