/**
 * @file MotionBlur.hpp
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

#ifndef KLAYGE_CORE_MOTION_BLUR_HPP
#define KLAYGE_CORE_MOTION_BLUR_HPP

#pragma once

#include <KlayGE/PreDeclare.hpp>

namespace KlayGE
{
	class KLAYGE_CORE_API MotionBlurPostProcess final : public PostProcess
	{
	public:
		enum VisualizeType
		{
			VT_Velocity,
			VT_VelocityTileMax,
			VT_VelocityNeighborMax,
			VT_Result
		};

	public:
		MotionBlurPostProcess();

		void Exposure(float exposure);
		float Exposure() const;

		void BlurRadius(uint32_t blur_radius);
		uint32_t BlurRadius() const;

		void ReconstructionSamples(uint32_t reconstruction_samples);
		uint32_t ReconstructionSamples() const;

		using PostProcess::InputPin;
		void InputPin(uint32_t index, ShaderResourceViewPtr const& srv) override;

		using PostProcess::OutputPin;
		void OutputPin(uint32_t index, RenderTargetViewPtr const& rtv) override;

		using PostProcess::SetParam;
		void SetParam(uint32_t index, uint32_t const & value) override;
		void SetParam(uint32_t index, float const & value) override;

		void Apply() override;

	private:
		void RecreateTextures(TexturePtr const& tex, ShaderResourceViewPtr const& srv);
		void BindVisualizeTextures();

	private:
		VisualizeType visualize_velocity_type_;

		float exposure_;
		uint32_t blur_radius_;
		uint32_t reconstruction_samples_;

		PostProcessPtr motion_blur_tile_max_x_dir_pp_;
		PostProcessPtr motion_blur_tile_max_y_dir_pp_;
		PostProcessPtr motion_blur_neighbor_max_pp_;
		PostProcessPtr motion_blur_gather_pp_;
		PostProcessPtr motion_blur_visualize_pp_;
	};
}

#endif		// KLAYGE_CORE_MOTION_BLUR_HPP
