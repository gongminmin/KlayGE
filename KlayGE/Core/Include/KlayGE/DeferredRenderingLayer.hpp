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
	enum
	{
		Opaque_GBuffer = 0,
		TransparencyBack_GBuffer,
		TransparencyFront_GBuffer,
		Num_GBuffers
	};

	enum VPAttribMask
	{		
		VPAM_Enabled = 1UL << 0,
		VPAM_NoOpaque = 1UL << 1,
		VPAM_NoTransparencyBack = 1UL << 2,
		VPAM_NoTransparencyFront = 1UL << 3,
		VPAM_NoSimpleForward = 1UL << 4,
		VPAM_NoGI = 1UL << 5,
		VPAM_NoSSVO = 1UL << 6,
		VPAM_NoSSGI = 1UL << 7
	};

	struct PerViewport
	{
		PerViewport()
			: attrib(0), ssgi_enabled(false), ssvo_enabled(true)
		{
		}

		uint32_t attrib;

		FrameBufferPtr frame_buffer;

		boost::array<bool, Num_GBuffers> g_buffer_enables;

		boost::array<FrameBufferPtr, Num_GBuffers> pre_depth_buffers;

		boost::array<FrameBufferPtr, Num_GBuffers> g_buffers;
		boost::array<TexturePtr, Num_GBuffers> g_buffer_rt0_texs;
		boost::array<TexturePtr, Num_GBuffers> g_buffer_rt1_texs;
		boost::array<TexturePtr, Num_GBuffers> g_buffer_ds_texs;
		boost::array<TexturePtr, Num_GBuffers> g_buffer_depth_texs;

		boost::array<FrameBufferPtr, Num_GBuffers> lighting_buffers;
		boost::array<TexturePtr, Num_GBuffers> lighting_texs;

		FrameBufferPtr shadowing_buffer;
		TexturePtr shadowing_tex;

		boost::array<FrameBufferPtr, Num_GBuffers> shading_buffers;
		boost::array<TexturePtr, Num_GBuffers> shading_texs;

		boost::array<FrameBufferPtr, Num_GBuffers> curr_merged_shading_buffers;
		TexturePtr curr_merged_shading_tex;
		boost::array<FrameBufferPtr, Num_GBuffers> curr_merged_depth_buffers;
		TexturePtr curr_merged_depth_tex;

		boost::array<FrameBufferPtr, Num_GBuffers> prev_merged_shading_buffers;
		TexturePtr prev_merged_shading_tex;
		boost::array<FrameBufferPtr, Num_GBuffers> prev_merged_depth_buffers;
		TexturePtr prev_merged_depth_tex;

		TexturePtr small_ssgi_tex;
		bool ssgi_enabled;

		TexturePtr small_ssvo_tex;
		bool ssvo_enabled;

		float4x4 view, proj;
		float4x4 inv_view, inv_proj;
		float3 depth_near_far_invfar;

		TexturePtr depth_deriative_tex;
		TexturePtr depth_deriative_small_tex;
		TexturePtr normal_cone_tex;
		TexturePtr normal_cone_small_tex;

		TexturePtr indirect_lighting_tex;
		TexturePtr indirect_lighting_pingpong_tex;
		std::vector<FrameBufferPtr> vpls_lighting_fbs;

		std::vector<bool> light_visibles;
	};

	class KLAYGE_CORE_API DeferredRenderingLayer
	{
	public:
		DeferredRenderingLayer();

		void SSGIEnabled(uint32_t vp, bool ssgi);
		void SSVOEnabled(uint32_t vp, bool ssvo);
		void SSREnabled(bool ssr);
		void TemporalAAEnabled(bool taa);

		void SetupViewport(uint32_t index, FrameBufferPtr const & fb, uint32_t attrib);
		void EnableViewport(uint32_t index, bool enable);
		uint32_t Update(uint32_t pass);

		void AtmosphericPostProcess(PostProcessPtr const & pp)
		{
			atmospheric_pp_ = pp;
		}

		RenderEffectPtr const & GBufferEffect() const
		{
			return g_buffer_effect_;
		}

		TexturePtr const & OpaqueLightingTex(uint32_t vp) const
		{
			return viewports_[vp].lighting_texs[Opaque_GBuffer];
		}
		TexturePtr const & TransparencyBackLightingTex(uint32_t vp) const
		{
			return viewports_[vp].lighting_texs[TransparencyBack_GBuffer];
		}
		TexturePtr const & TransparencyFrontLightingTex(uint32_t vp) const
		{
			return viewports_[vp].lighting_texs[TransparencyFront_GBuffer];
		}
		TexturePtr const & OpaqueShadingTex(uint32_t vp) const
		{
			return viewports_[vp].shading_texs[Opaque_GBuffer];
		}
		TexturePtr const & TransparencyBackShadingTex(uint32_t vp) const
		{
			return viewports_[vp].shading_texs[TransparencyBack_GBuffer];
		}
		TexturePtr const & TransparencyFrontShadingTex(uint32_t vp) const
		{
			return viewports_[vp].shading_texs[TransparencyFront_GBuffer];
		}
		TexturePtr const & PrevFrameShadingTex(uint32_t vp) const
		{
			return viewports_[vp].prev_merged_shading_tex;
		}
		TexturePtr const & PrevFrameDepthTex(uint32_t vp) const
		{
			return viewports_[vp].prev_merged_depth_tex;
		}

		TexturePtr const & SmallSSVOTex(uint32_t vp) const
		{
			return viewports_[vp].small_ssvo_tex;
		}

		TexturePtr const & OpaqueGBufferRT0Tex(uint32_t vp) const
		{
			return viewports_[vp].g_buffer_rt0_texs[Opaque_GBuffer];
		}
		TexturePtr const & OpaqueGBufferRT1Tex(uint32_t vp) const
		{
			return viewports_[vp].g_buffer_rt1_texs[Opaque_GBuffer];
		}
		TexturePtr const & OpaqueDepthTex(uint32_t vp) const
		{
			return viewports_[vp].g_buffer_depth_texs[Opaque_GBuffer];
		}
		TexturePtr const & TransparencyBackGBufferRT0Tex(uint32_t vp) const
		{
			return viewports_[vp].g_buffer_rt0_texs[TransparencyBack_GBuffer];
		}
		TexturePtr const & TransparencyBackGBufferRT1Tex(uint32_t vp) const
		{
			return viewports_[vp].g_buffer_rt1_texs[TransparencyBack_GBuffer];
		}
		TexturePtr const & TransparencyBackDepthTex(uint32_t vp) const
		{
			return viewports_[vp].g_buffer_depth_texs[TransparencyBack_GBuffer];
		}
		TexturePtr const & TransparencyFrontGBufferRT0Tex(uint32_t vp) const
		{
			return viewports_[vp].g_buffer_rt0_texs[TransparencyFront_GBuffer];
		}
		TexturePtr const & TransparencyFrontGBufferRT1Tex(uint32_t vp) const
		{
			return viewports_[vp].g_buffer_rt1_texs[TransparencyFront_GBuffer];
		}
		TexturePtr const & TransparencyFrontDepthTex(uint32_t vp) const
		{
			return viewports_[vp].g_buffer_depth_texs[TransparencyFront_GBuffer];
		}

		uint32_t ActiveViewport() const
		{
			return active_viewport_;
		}

		void DisplayIllum(int illum);
		void IndirectScale(float scale);

		void LightDistance(float dist)
		{
			light_scale_ = dist * 0.01f;
		}

	private:
		void CreateDepthDerivativeMipMap(uint32_t vp);
		void CreateNormalConeMipMap(uint32_t vp);
		void SetSubsplatStencil(uint32_t vp);
		void ExtractVPLs(uint32_t vp, CameraPtr const & rsm_camera, LightSourcePtr const & light);
		void VPLsLighting(uint32_t vp, LightSourcePtr const & light);
		void UpsampleMultiresLighting(uint32_t vp);
		void AccumulateToLightingTex(uint32_t vp);

		uint32_t ComposePassScanCode(uint32_t vp_index, int32_t pass_type, int32_t org_no, int32_t index_in_pass);
		void DecomposePassScanCode(uint32_t& vp_index, int32_t& pass_type, int32_t& org_no, int32_t& index_in_pass, uint32_t code);

	private:
		bool mrt_g_buffer_support_;
		bool depth_texture_support_;

		RenderEffectPtr g_buffer_effect_;
		RenderEffectPtr dr_effect_;

		boost::array<PerViewport, 8> viewports_;
		uint32_t active_viewport_;

		PostProcessPtr ssgi_pp_;
		PostProcessPtr ssgi_blur_pp_;

		PostProcessPtr ssvo_pp_;
		PostProcessPtr ssvo_blur_pp_;

		PostProcessPtr ssr_pp_;
		bool ssr_enabled_;

		PostProcessPtr taa_pp_;
		bool taa_enabled_;

		float light_scale_;
		RenderLayoutPtr rl_cone_;
		RenderLayoutPtr rl_pyramid_;
		RenderLayoutPtr rl_box_;
		RenderLayoutPtr rl_quad_;
		OBBox cone_obb_;
		OBBox pyramid_obb_;
		OBBox box_obb_;

		std::vector<LightSourcePtr> lights_;

		std::vector<uint32_t> pass_scaned_;

		boost::array<RenderTechniquePtr, LT_NumLightTypes> technique_shadows_;
		boost::array<RenderTechniquePtr, LT_NumLightTypes> technique_lights_;
		RenderTechniquePtr technique_light_depth_only_;
		RenderTechniquePtr technique_light_stencil_;
		RenderTechniquePtr technique_clear_stencil_;
		RenderTechniquePtr technique_no_lighting_;
		RenderTechniquePtr technique_shading_;
		boost::array<RenderTechniquePtr, 2> technique_merge_shadings_;
		boost::array<RenderTechniquePtr, 2> technique_merge_depths_;
		RenderTechniquePtr technique_copy_shading_depth_;
		RenderTechniquePtr technique_copy_depth_;

		FrameBufferPtr sm_buffer_;
		TexturePtr sm_tex_;
		TexturePtr sm_depth_tex_;
		TexturePtr blur_sm_tex_;
		TexturePtr sm_cube_tex_;

		boost::array<PostProcessPtr, 7> sm_filter_pps_;
		PostProcessPtr depth_to_vsm_pp_;
		PostProcessPtr depth_to_linear_pp_;

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
		RenderEffectParameterPtr inv_width_height_param_;
		RenderEffectParameterPtr shadowing_tex_param_;
		RenderEffectParameterPtr near_q_param_;

		std::vector<SceneObject*> visible_scene_objs_;
		bool has_reflective_objs_;
		bool has_simple_forward_objs_;

		PostProcessPtr atmospheric_pp_;

		FrameBufferPtr rsm_buffer_;
		boost::array<TexturePtr, 2> rsm_texs_;

		boost::array<PostProcessPtr, LT_NumLightTypes> rsm_to_vpls_pps_;
		TexturePtr vpl_tex_;
				
		RenderTechniquePtr subsplat_stencil_tech_;
		RenderTechniquePtr vpls_lighting_instance_id_tech_;
		RenderTechniquePtr vpls_lighting_no_instance_id_tech_;
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
		void ExtractVPLsNew(uint32_t vp, CameraPtr const & rsm_camera, LightSourcePtr const & light);

	private:
		TexturePtr rsm_depth_derivative_tex_;
		PostProcessPtr rsm_to_depth_derivate_pp_;
	};
}

#endif		// _DEFERREDRENDERINGLAYER_HPP
