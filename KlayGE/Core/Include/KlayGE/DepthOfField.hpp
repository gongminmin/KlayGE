/**
 * @file DepthOfField.hpp
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

#ifndef KLAYGE_CORE_DEPTH_OF_FIELD_HPP
#define KLAYGE_CORE_DEPTH_OF_FIELD_HPP

#pragma once

#include <KlayGE/PreDeclare.hpp>

#include <KlayGE/PostProcess.hpp>

namespace KlayGE
{
	class KLAYGE_CORE_API DepthOfField final : public PostProcess
	{
	public:
		DepthOfField();

		void FocusPlane(float focus_plane);
		float FocusPlane() const;

		void FocusRange(float focus_range);
		float FocusRange() const;

		void ShowBlurFactor(bool show);
		bool ShowBlurFactor() const;

		void InputPin(uint32_t index, ShaderResourceViewPtr const& srv) override;
		using PostProcess::InputPin;

		void Apply() override;

	private:
		PostProcessPtr sat_pp_;

		bool cs_support_;

		int max_radius_ = 8;

		float focus_plane_ = 2;
		float focus_range_ = 2;
		bool show_blur_factor_ = false;

		TexturePtr spread_tex_;
		FrameBufferPtr spread_fb_;

		PostProcessPtr spreading_pp_;

		RenderLayoutPtr normalization_rl_;

		RenderTechnique* normalization_tech_;
		RenderTechnique* blur_factor_tech_;

		RenderEffectParameter* focus_plane_inv_range_param_;
		RenderEffectParameter* depth_tex_param_;
		RenderEffectParameter* src_tex_param_;
	};

	class KLAYGE_CORE_API BokehFilter final : public PostProcess
	{
	public:
		BokehFilter();

		void FocusPlane(float focus_plane);
		float FocusPlane() const;

		void FocusRange(float focus_range);
		float FocusRange() const;

		void LuminanceThreshold(float lum_threshold);
		float LuminanceThreshold() const;

		void InputPin(uint32_t index, ShaderResourceViewPtr const& srv) override;
		using PostProcess::InputPin;

		void OutputPin(uint32_t index, RenderTargetViewPtr const& rtv) override;
		void OutputPin(uint32_t index, UnorderedAccessViewPtr const& uav) override;
		RenderTargetViewPtr const& RtvOutputPin(uint32_t index) const override;
		UnorderedAccessViewPtr const& UavOutputPin(uint32_t index) const override;

		void Apply() override;

	private:
		bool gs_support_;

		int max_radius_ = 8;

		float focus_plane_ = 2;
		float focus_range_ = 2;

		float lum_threshold_ = 1.5f;

		TexturePtr bokeh_tex_;
		FrameBufferPtr bokeh_fb_;
		RenderLayoutPtr bokeh_rl_;

		PostProcessPtr merge_bokeh_pp_;

		RenderEffectParameter* focus_plane_inv_range_param_;
		RenderEffectParameter* lum_threshold_param_;
		RenderEffectParameter* color_tex_param_;
		RenderEffectParameter* depth_tex_param_;
	};
} // namespace KlayGE

#endif // KLAYGE_CORE_DEPTH_OF_FIELD_HPP
