/**
 * @file IndirectLightingLayer.hpp
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

#ifndef _INDIRECTLIGHTINGLAYER_HPP
#define _INDIRECTLIGHTINGLAYER_HPP

#pragma once

#include <KlayGE/PreDeclare.hpp>

#include <array>

namespace KlayGE
{
	class KLAYGE_CORE_API IndirectLightingLayer
	{
	public:
		virtual ~IndirectLightingLayer()
		{
		}

		virtual void GBuffer(TexturePtr const & rt0_tex, TexturePtr const & rt1_tex, TexturePtr const & depth_tex) = 0;
		virtual void RSM(TexturePtr const & rt0_tex, TexturePtr const & rt1_tex, TexturePtr const & depth_tex) = 0;

		TexturePtr IndirectLightingTex() const
		{
			return indirect_lighting_tex_;
		}

		virtual void UpdateGBuffer(Camera const & vp_camera) = 0;
		virtual void UpdateRSM(Camera const & rsm_camera, LightSource const & light) = 0;
		virtual void CalcIndirectLighting(TexturePtr const & prev_shading_tex, float4x4 const & proj_to_prev) = 0;

	protected:
		TexturePtr indirect_lighting_tex_;
	};

	class KLAYGE_CORE_API MultiResSILLayer : public IndirectLightingLayer
	{
	public:
		MultiResSILLayer();

		virtual void GBuffer(TexturePtr const & rt0_tex, TexturePtr const & rt1_tex, TexturePtr const & depth_tex) override;
		virtual void RSM(TexturePtr const & rt0_tex, TexturePtr const & rt1_tex, TexturePtr const & depth_tex) override;

		virtual void UpdateGBuffer(Camera const & vp_camera) override;
		virtual void UpdateRSM(Camera const & rsm_camera, LightSource const & light) override;
		virtual void CalcIndirectLighting(TexturePtr const & prev_shading_tex, float4x4 const & proj_to_prev) override;

	private:
		void ExtractVPLs(Camera const & rsm_camera, LightSource const & light);
		void VPLsLighting(LightSource const & light);

	private:
		MultiResLayerPtr multi_res_layer_;

		TexturePtr g_buffer_rt0_tex_;
		TexturePtr g_buffer_depth_tex_;
		Camera const * g_buffer_camera_;

		std::array<TexturePtr, 2> rsm_texs_;
		TexturePtr rsm_depth_tex_;

		std::array<PostProcessPtr, LightSource::LT_NumLightTypes> rsm_to_vpls_pps_;
		TexturePtr vpl_tex_;

		RenderTechniquePtr vpls_lighting_instance_id_tech_;
		RenderTechniquePtr vpls_lighting_no_instance_id_tech_;

		RenderEffectParameterPtr vpl_view_param_;
		RenderEffectParameterPtr vpl_proj_param_;
		RenderEffectParameterPtr vpl_depth_near_far_invfar_param_;
		RenderEffectParameterPtr vpl_light_pos_es_param_;
		RenderEffectParameterPtr vpl_light_color_param_;
		RenderEffectParameterPtr vpl_light_falloff_param_;
		RenderEffectParameterPtr vpl_x_coord_param_;
		RenderEffectParameterPtr vpl_gbuffer_tex_param_;
		RenderEffectParameterPtr vpl_depth_tex_param_;

		RenderablePtr vpl_renderable_;

		TexturePtr rsm_depth_derivative_tex_;
		PostProcessPtr rsm_to_depth_derivate_pp_;
	};

	class KLAYGE_CORE_API SSGILayer : public IndirectLightingLayer
	{
	public:
		SSGILayer();

		virtual void GBuffer(TexturePtr const & rt0_tex, TexturePtr const & rt1_tex, TexturePtr const & depth_tex) override;
		virtual void RSM(TexturePtr const & rt0_tex, TexturePtr const & rt1_tex, TexturePtr const & depth_tex) override;

		virtual void UpdateGBuffer(Camera const & vp_camera) override;
		virtual void UpdateRSM(Camera const & rsm_camera, LightSource const & light) override;
		virtual void CalcIndirectLighting(TexturePtr const & prev_shading_tex, float4x4 const & proj_to_prev) override;

	private:
		MultiResLayerPtr multi_res_layer_;

		TexturePtr g_buffer_rt0_tex_;
		TexturePtr g_buffer_depth_tex_;

		PostProcessPtr ssgi_pp_;
		PostProcessPtr ssgi_blur_pp_;

		TexturePtr small_ssgi_tex_;
	};
}

#endif		// _INDIRECTLIGHTINGLAYER_HPP
