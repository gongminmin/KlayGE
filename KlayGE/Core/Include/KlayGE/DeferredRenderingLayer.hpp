// DeferredRenderingLayer.hpp
// KlayGE Deferred Rendering Layer header file
// Ver 4.0.0
// Copyright(C) Minmin Gong, 2011
// Homepage: http://www.klayge.org
//
// 4.0.0
// First release (2011.8.28)
//
// CHANGE LIST
//////////////////////////////////////////////////////////////////////////////////

#ifndef _DEFERREDRENDERINGLAYER_HPP
#define _DEFERREDRENDERINGLAYER_HPP

#pragma once

#ifndef KLAYGE_CORE_SOURCE
#define KLAYGE_LIB_NAME KlayGE_Core
#include <KlayGE/config/auto_link.hpp>
#endif

#include <KlayGE/PreDeclare.hpp>
#include <KlayGE/Light.hpp>

namespace KlayGE
{
	class KLAYGE_CORE_API DeferredRenderingLayer
	{
	public:
		DeferredRenderingLayer();

		void SSVOEnabled(bool ssvo);
		void HDREnabled(bool hdr);
		void AAEnabled(int aa);

		void OnResize(uint32_t width, uint32_t height);
		uint32_t Update(uint32_t pass);

		RenderEffectPtr const & GBufferEffect() const
		{
			return g_buffer_effect_;
		}

		TexturePtr const & OpaqueLightingTex() const
		{
			return opaque_lighting_tex_;
		}
		TexturePtr const & ShadingTex() const
		{
			return shading_tex_;
		}
		TexturePtr const & SSVOTex() const
		{
			return ssvo_tex_;
		}

		TexturePtr const & OpaqueGBufferRT0Tex() const
		{
			return opaque_g_buffer_rt0_tex_;
		}
		TexturePtr const & OpaqueGBufferRT1Tex() const
		{
			return opaque_g_buffer_rt1_tex_;
		}

		void DisplayIllum(int illum);
		void IndirectScale(float scale);

	private:
		void CreateDepthDerivativeMipMap();
		void CreateNormalConeMipMap();
		void SetSubsplatStencil();
		void ExtractVPLs(CameraPtr const & rsm_camera, LightSourcePtr const & light);
		void VPLsLighting(LightSourcePtr const & light);
		void UpsampleMultiresLighting();
		void AccumulateToLightingTex();

	private:
		bool mrt_g_buffer_;

		RenderEffectPtr g_buffer_effect_;
		RenderEffectPtr dr_effect_;

		FrameBufferPtr opaque_g_buffer_;
		TexturePtr opaque_g_buffer_rt0_tex_;
		TexturePtr opaque_g_buffer_rt1_tex_;
		TexturePtr opaque_ds_tex_;

		FrameBufferPtr transparency_back_g_buffer_;
		TexturePtr transparency_back_g_buffer_rt0_tex_;
		TexturePtr transparency_back_g_buffer_rt1_tex_;
		TexturePtr transparency_back_ds_tex_;

		FrameBufferPtr transparency_front_g_buffer_;
		TexturePtr transparency_front_g_buffer_rt0_tex_;
		TexturePtr transparency_front_g_buffer_rt1_tex_;
		TexturePtr transparency_front_ds_tex_;

		FrameBufferPtr shadowing_buffer_;
		TexturePtr shadowing_tex_;

		FrameBufferPtr opaque_lighting_buffer_;
		TexturePtr opaque_lighting_tex_;

		FrameBufferPtr transparency_back_lighting_buffer_;
		TexturePtr transparency_back_lighting_tex_;

		FrameBufferPtr transparency_front_lighting_buffer_;
		TexturePtr transparency_front_lighting_tex_;

		FrameBufferPtr shading_buffer_;
		TexturePtr shading_tex_;

		TexturePtr ldr_tex_;

		PostProcessPtr ssvo_pp_;
		PostProcessPtr blur_pp_;
		TexturePtr small_ssvo_tex_;
		TexturePtr ssvo_tex_;
		bool ssvo_enabled_;

		PostProcessPtr hdr_pp_;
		PostProcessPtr skip_hdr_pp_;
		bool hdr_enabled_;

		PostProcessPtr aa_pp_;
		PostProcessPtr skip_aa_pp_;
		int aa_enabled_;

		RenderLayoutPtr rl_cone_;
		RenderLayoutPtr rl_pyramid_;
		RenderLayoutPtr rl_box_;
		RenderLayoutPtr rl_quad_;
		Box cone_bbox_;
		Box pyramid_bbox_;
		Box box_bbox_;

		std::vector<LightSourcePtr> lights_;

		std::vector<uint32_t> pass_scaned_;

		RenderTechniquePtr technique_shadows_[LT_NumLightTypes];
		RenderTechniquePtr technique_lights_[LT_NumLightTypes];
		RenderTechniquePtr technique_light_depth_only_;
		RenderTechniquePtr technique_light_stencil_;
		RenderTechniquePtr technique_clear_stencil_;
		RenderTechniquePtr technique_shading_;
		RenderTechniquePtr technique_shading_alpha_blend_;

		FrameBufferPtr sm_buffer_;
		TexturePtr sm_tex_;
		TexturePtr sm_depth_tex_;
		TexturePtr blur_sm_tex_;
		TexturePtr sm_cube_tex_;

		PostProcessPtr sm_filter_pps_[7];
		PostProcessPtr depth_to_vsm_pp_;

		float4x4 view_, proj_;
		float4x4 inv_view_, inv_proj_;
		float3 depth_near_far_invfar_;

		RenderEffectParameterPtr g_buffer_tex_param_;
		RenderEffectParameterPtr g_buffer_1_tex_param_;
		RenderEffectParameterPtr lighting_tex_param_;
		RenderEffectParameterPtr depth_near_far_invfar_param_;
		RenderEffectParameterPtr light_attrib_param_;
		RenderEffectParameterPtr light_color_param_;
		RenderEffectParameterPtr light_falloff_param_;
		RenderEffectParameterPtr light_view_proj_param_;
		RenderEffectParameterPtr light_volume_mv_param_;
		RenderEffectParameterPtr light_volume_mvp_param_;
		RenderEffectParameterPtr view_to_light_model_param_;
		RenderEffectParameterPtr light_pos_es_param_;
		RenderEffectParameterPtr light_dir_es_param_;
		RenderEffectParameterPtr ssvo_enabled_param_;

		std::vector<SceneObject*> opaque_scene_objs_;
		std::vector<SceneObject*> transparency_scene_objs_;

		FrameBufferPtr rsm_buffer_;
		TexturePtr rsm_texs_[2];

		PostProcessPtr rsm_to_vpls_pps[LT_NumLightTypes];
		TexturePtr vpl_tex_;
		
		TexturePtr depth_deriative_tex_;
		TexturePtr depth_deriative_small_tex_;
		TexturePtr normal_cone_tex_;
		TexturePtr normal_cone_small_tex_;
		
		TexturePtr indirect_lighting_tex_;
		TexturePtr indirect_lighting_pingpong_tex_;
		RenderTechniquePtr subsplat_stencil_tech_;
		RenderTechniquePtr vpls_lighting_instance_id_tech_;
		RenderTechniquePtr vpls_lighting_no_instance_id_tech_;
		std::vector<FrameBufferPtr> vpls_lighting_fbs_;
		bool indirect_lighting_enabled_;	

		PostProcessPtr gbuffer_to_depth_derivate_pp_;
		PostProcessPtr depth_derivate_mipmap_pp_;
		PostProcessPtr gbuffer_to_normal_cone_pp_;
		PostProcessPtr normal_cone_mipmap_pp_;

		PostProcessPtr upsampling_pp_;

		int illum_;
		float indirect_scale_;
		PostProcessPtr copy_to_light_buffer_pp_;
		PostProcessPtr copy_to_light_buffer_i_pp_;

		RenderEffectParameterPtr subsplat_cur_lower_level_param_;
		RenderEffectParameterPtr subsplat_is_not_first_last_level_param_;

		RenderEffectParameterPtr vpl_view_param_;
		RenderEffectParameterPtr vpl_proj_param_;
		RenderEffectParameterPtr vpl_depth_near_far_invfar_param_;
		RenderEffectParameterPtr vpl_light_pos_es_param_;
		RenderEffectParameterPtr vpl_light_color_param_;
		RenderEffectParameterPtr vpl_x_coord_param_;

		RenderLayoutPtr rl_vpl_;
	};
}

#endif		// _DEFERREDRENDERINGLAYER_HPP
