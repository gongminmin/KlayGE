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

#include <boost/array.hpp>

namespace KlayGE
{
	class KLAYGE_CORE_API DeferredRenderingLayer
	{
	public:
		DeferredRenderingLayer();

		void SSGIEnabled(bool ssgi);
		void SSVOEnabled(bool ssvo);
		void SSREnabled(bool ssr);

		void OutputPin(TexturePtr const & tex);

		void OnResize(uint32_t width, uint32_t height);
		uint32_t Update(uint32_t pass);

		RenderEffectPtr const & GBufferEffect() const
		{
			return g_buffer_effect_;
		}

		TexturePtr const & OpaqueLightingTex() const
		{
			return lighting_texs_[0];
		}
		TexturePtr const & TransparencyBackLightingTex() const
		{
			return lighting_texs_[1];
		}
		TexturePtr const & TransparencyFrontLightingTex() const
		{
			return lighting_texs_[2];
		}
		TexturePtr const & OpaqueShadingTex() const
		{
			return shading_texs_[0];
		}
		TexturePtr const & TransparencyBackShadingTex() const
		{
			return shading_texs_[1];
		}
		TexturePtr const & TransparencyFrontShadingTex() const
		{
			return shading_texs_[2];
		}

		TexturePtr const & SmallSSVOTex() const
		{
			return small_ssvo_tex_;
		}

		TexturePtr const & OpaqueGBufferRT0Tex() const
		{
			return g_buffer_rt0_texs_[0];
		}
		TexturePtr const & OpaqueGBufferRT1Tex() const
		{
			return g_buffer_rt1_texs_[0];
		}
		TexturePtr const & OpaqueDepthTex() const
		{
			return g_buffer_depth_texs_[0];
		}
		TexturePtr const & TransparencyBackGBufferRT0Tex() const
		{
			return g_buffer_rt0_texs_[1];
		}
		TexturePtr const & TransparencyBackGBufferRT1Tex() const
		{
			return g_buffer_rt1_texs_[1];
		}
		TexturePtr const & TransparencyBackDepthTex() const
		{
			return g_buffer_depth_texs_[1];
		}
		TexturePtr const & TransparencyFrontGBufferRT0Tex() const
		{
			return g_buffer_rt0_texs_[2];
		}
		TexturePtr const & TransparencyFrontGBufferRT1Tex() const
		{
			return g_buffer_rt1_texs_[2];
		}
		TexturePtr const & TransparencyFrontDepthTex() const
		{
			return g_buffer_depth_texs_[2];
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
		bool depth_texture_;

		RenderEffectPtr g_buffer_effect_;
		RenderEffectPtr dr_effect_;

		enum
		{
			NUM_G_BUFFERS = 3
		};

		boost::array<bool, NUM_G_BUFFERS> g_buffer_enables_;

		boost::array<FrameBufferPtr, NUM_G_BUFFERS> pre_depth_buffers_;

		boost::array<FrameBufferPtr, NUM_G_BUFFERS> g_buffers_;
		boost::array<TexturePtr, NUM_G_BUFFERS> g_buffer_rt0_texs_;
		boost::array<TexturePtr, NUM_G_BUFFERS> g_buffer_rt1_texs_;
		boost::array<TexturePtr, NUM_G_BUFFERS> g_buffer_ds_texs_;
		boost::array<TexturePtr, NUM_G_BUFFERS> g_buffer_depth_texs_;

		FrameBufferPtr shadowing_buffer_;
		TexturePtr shadowing_tex_;

		boost::array<FrameBufferPtr, NUM_G_BUFFERS> lighting_buffers_;
		boost::array<TexturePtr, NUM_G_BUFFERS> lighting_texs_;

		boost::array<FrameBufferPtr, NUM_G_BUFFERS + 1> shading_buffers_;
		boost::array<TexturePtr, NUM_G_BUFFERS + 1> shading_texs_;
		int curr_frame_index_;

		FrameBufferPtr output_buffer_;
		TexturePtr output_tex_;

		PostProcessPtr ssgi_pp_;
		PostProcessPtr ssgi_blur_pp_;
		TexturePtr small_ssgi_tex_;
		bool ssgi_enabled_;

		PostProcessPtr ssvo_pp_;
		PostProcessPtr ssvo_blur_pp_;
		TexturePtr small_ssvo_tex_;
		bool ssvo_enabled_;

		PostProcessPtr ssr_pp_;
		bool ssr_enabled_;

		RenderLayoutPtr rl_cone_;
		RenderLayoutPtr rl_pyramid_;
		RenderLayoutPtr rl_box_;
		RenderLayoutPtr rl_quad_;
		OBBox cone_obb_;
		OBBox pyramid_obb_;
		OBBox box_obb_;

		std::vector<LightSourcePtr> lights_;
		std::vector<bool> light_visibles_;

		std::vector<uint32_t> pass_scaned_;

		RenderTechniquePtr technique_shadows_[LT_NumLightTypes];
		RenderTechniquePtr technique_lights_[LT_NumLightTypes];
		RenderTechniquePtr technique_light_depth_only_;
		RenderTechniquePtr technique_light_stencil_;
		RenderTechniquePtr technique_clear_stencil_;
		RenderTechniquePtr technique_no_lighting_;
		RenderTechniquePtr technique_shading_;
		RenderTechniquePtr technique_merge_shadings_[2];

		FrameBufferPtr sm_buffer_;
		TexturePtr sm_tex_;
		TexturePtr sm_depth_tex_;
		TexturePtr blur_sm_tex_;
		TexturePtr sm_cube_tex_;

		PostProcessPtr sm_filter_pps_[7];
		PostProcessPtr depth_to_vsm_pp_;
		PostProcessPtr depth_to_linear_pp_;

		float4x4 view_, proj_;
		float4x4 inv_view_, inv_proj_;
		float3 depth_near_far_invfar_;

		RenderEffectParameterPtr g_buffer_tex_param_;
		RenderEffectParameterPtr g_buffer_1_tex_param_;
		RenderEffectParameterPtr depth_tex_param_;
		RenderEffectParameterPtr lighting_tex_param_;
		RenderEffectParameterPtr shading_tex_param_;
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
		RenderEffectParameterPtr projective_map_tex_param_;
		RenderEffectParameterPtr projective_map_cube_tex_param_;

		std::vector<SceneObject*> visible_scene_objs_;
		bool has_transparency_back_objs_;
		bool has_transparency_front_objs_;
		bool has_reflective_objs_;
		bool has_simple_forward_objs_;

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
		RenderEffectParameterPtr vpl_light_falloff_param_;
		RenderEffectParameterPtr vpl_x_coord_param_;

		RenderLayoutPtr rl_vpl_;

		// USE_NEW_LIGHT_SAMPLING
	private:
		void ExtractVPLsNew(CameraPtr const & rsm_camera, LightSourcePtr const & light);

	private:
		TexturePtr rsm_depth_derivative_tex_;
		PostProcessPtr rsm_to_depth_derivate_pp_;
	};
}

#endif		// _DEFERREDRENDERINGLAYER_HPP
