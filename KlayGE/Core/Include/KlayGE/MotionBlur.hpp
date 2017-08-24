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
	class KLAYGE_CORE_API MotionBlurPostProcess : public PostProcess
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

		using PostProcess::InputPin;
		void InputPin(uint32_t index, TexturePtr const & tex) override;

		using PostProcess::SetParam;
		void SetParam(uint32_t index, uint32_t const & value) override;
		void SetParam(uint32_t index, float const & value) override;

		void Apply() override;

	private:
		void RecreateTextures(TexturePtr const & tex);
		void BindVisualizeTextures();

	private:
		VisualizeType visualize_velocity_type_;

		float exposure_;
		uint32_t blur_radius_;
		uint32_t reconstruction_samples_;

		TexturePtr velocity_tile_max_x_dir_tex_;
		TexturePtr velocity_tile_max_tex_;
		TexturePtr velocity_neighbor_max_tex_;
		TexturePtr random_tex_;

		PostProcessPtr motion_blur_tile_max_x_dir_pp_;
		PostProcessPtr motion_blur_tile_max_y_dir_pp_;
		PostProcessPtr motion_blur_neighbor_max_pp_;
		PostProcessPtr motion_blur_gather_pp_;
		PostProcessPtr motion_blur_visualize_pp_;

	};
}

#endif		// KLAYGE_CORE_MOTION_BLUR_HPP
