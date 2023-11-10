/**
 * @file PPRPostProcess.hpp
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

#ifndef KLAYGE_CORE_PPR_POST_PROCESS_HPP
#define KLAYGE_CORE_PPR_POST_PROCESS_HPP

#pragma once

#include <KlayGE/PostProcess.hpp>

namespace KlayGE
{
	class KLAYGE_CORE_API PPRPostProcess final : public PostProcess
	{
	public:
		explicit PPRPostProcess(bool multi_sample);

		void Apply() override;

		void SetParam(uint32_t index, float4 const& value) override;
		using PostProcess::SetParam;

		void GetParam(uint32_t index, float4& value) override;
		using PostProcess::GetParam;

		void InputPin(uint32_t index, ShaderResourceViewPtr const& srv) override;
		using PostProcess::InputPin;

	private:
		RenderTechnique* coord_tech_;
		RenderTechnique* project_tech_;

		RenderEffectParameter* inv_view_param_;
		RenderEffectParameter* inv_proj_param_;
		RenderEffectParameter* view_to_reflected_proj_param_;
		RenderEffectParameter* reflection_width_height_param_;
		RenderEffectParameter* upper_left_param_;
		RenderEffectParameter* xy_dir_param_;
		RenderEffectParameter* far_plane_param_;

		RenderEffectParameter* rw_coord_tex_param_;
		RenderEffectParameter* coord_tex_param_;

		Plane reflect_plane_;

		TexturePtr coord_tex_;
		UnorderedAccessViewPtr coord_uav_;
	};
}

#endif		// KLAYGE_CORE_PPR_POST_PROCESS_HPP
