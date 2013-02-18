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

#ifndef KLAYGE_CORE_SOURCE
#define KLAYGE_LIB_NAME KlayGE_Core
#include <KlayGE/config/auto_link.hpp>
#endif

#include <KlayGE/PreDeclare.hpp>

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

		virtual void UpdateGBuffer(CameraPtr const & vp_camera) = 0;
		virtual void UpdateRSM(CameraPtr const & rsm_camera, LightSourcePtr const & light) = 0;
		virtual void CalcIndirectLighting(TexturePtr const & prev_shading_tex, float4x4 const & proj_to_prev) = 0;

	protected:
		TexturePtr indirect_lighting_tex_;
	};

	class KLAYGE_CORE_API MultiResSILLayer : public IndirectLightingLayer
	{
	public:
		MultiResSILLayer();

		virtual void GBuffer(TexturePtr const & rt0_tex, TexturePtr const & rt1_tex, TexturePtr const & depth_tex);
		virtual void RSM(TexturePtr const & rt0_tex, TexturePtr const & rt1_tex, TexturePtr const & depth_tex);

		virtual void UpdateGBuffer(CameraPtr const & vp_camera);
		virtual void UpdateRSM(CameraPtr const & rsm_camera, LightSourcePtr const & light);
		virtual void CalcIndirectLighting(TexturePtr const & prev_shading_tex, float4x4 const & proj_to_prev);

	private:
		void CreateDepthDerivativeMipMap();
		void CreateNormalConeMipMap();
		void SetSubsplatStencil();
		void ExtractVPLs(CameraPtr const & rsm_camera, LightSourcePtr const & light);
		void VPLsLighting(LightSourcePtr const & light);
		void UpsampleMultiresLighting();

	private:
		RenderLayoutPtr rl_quad_;

		array<TexturePtr, 2> g_buffer_texs_;
		TexturePtr g_buffer_depth_tex_;
		CameraPtr g_buffer_camera_;

		array<TexturePtr, 2> rsm_texs_;
		TexturePtr rsm_depth_tex_;

		array<PostProcessPtr, LT_NumLightTypes> rsm_to_vpls_pps_;
		TexturePtr vpl_tex_;

		TexturePtr depth_deriative_tex_;
		TexturePtr depth_deriative_small_tex_;
		TexturePtr normal_cone_tex_;
		TexturePtr normal_cone_small_tex_;

		TexturePtr indirect_lighting_pingpong_tex_;
		std::vector<FrameBufferPtr> vpls_lighting_fbs_;
				
		RenderTechniquePtr subsplat_stencil_tech_;
		RenderTechniquePtr vpls_lighting_instance_id_tech_;
		RenderTechniquePtr vpls_lighting_no_instance_id_tech_;

		PostProcessPtr gbuffer_to_depth_derivate_pp_;
		PostProcessPtr depth_derivate_mipmap_pp_;
		PostProcessPtr gbuffer_to_normal_cone_pp_;
		PostProcessPtr normal_cone_mipmap_pp_;

		PostProcessPtr upsampling_pp_;

		RenderEffectParameterPtr subsplat_cur_lower_level_param_;
		RenderEffectParameterPtr subsplat_is_not_first_last_level_param_;
		RenderEffectParameterPtr subsplat_depth_deriv_tex_param_;
		RenderEffectParameterPtr subsplat_normal_cone_tex_param_;
		RenderEffectParameterPtr subsplat_depth_normal_threshold_param_;

		RenderEffectParameterPtr vpl_view_param_;
		RenderEffectParameterPtr vpl_proj_param_;
		RenderEffectParameterPtr vpl_depth_near_far_invfar_param_;
		RenderEffectParameterPtr vpl_light_pos_es_param_;
		RenderEffectParameterPtr vpl_light_color_param_;
		RenderEffectParameterPtr vpl_light_falloff_param_;
		RenderEffectParameterPtr vpl_x_coord_param_;
		RenderEffectParameterPtr vpl_gbuffer_tex_param_;
		RenderEffectParameterPtr vpl_depth_tex_param_;

		RenderLayoutPtr rl_vpl_;

		// USE_NEW_LIGHT_SAMPLING
	private:
		void ExtractVPLsNew(CameraPtr const & rsm_camera, LightSourcePtr const & light);

	private:
		TexturePtr rsm_depth_derivative_tex_;
		PostProcessPtr rsm_to_depth_derivate_pp_;
	};

	class KLAYGE_CORE_API SSGILayer : public IndirectLightingLayer
	{
	public:
		SSGILayer();

		virtual void GBuffer(TexturePtr const & rt0_tex, TexturePtr const & rt1_tex, TexturePtr const & depth_tex);
		virtual void RSM(TexturePtr const & rt0_tex, TexturePtr const & rt1_tex, TexturePtr const & depth_tex);

		virtual void UpdateGBuffer(CameraPtr const & vp_camera);
		virtual void UpdateRSM(CameraPtr const & rsm_camera, LightSourcePtr const & light);
		virtual void CalcIndirectLighting(TexturePtr const & prev_shading_tex, float4x4 const & proj_to_prev);

	private:
		array<TexturePtr, 2> g_buffer_texs_;
		TexturePtr g_buffer_depth_tex_;
		CameraPtr g_buffer_camera_;

		PostProcessPtr ssgi_pp_;
		PostProcessPtr ssgi_blur_pp_;

		TexturePtr small_ssgi_tex_;
	};
}

#endif		// _INDIRECTLIGHTINGLAYER_HPP
