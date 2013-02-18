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
#include <KlayGE/IndirectLightingLayer.hpp>

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
		VPAM_NoSSVO = 1UL << 6
	};

	struct PerViewport
	{
		PerViewport()
			: attrib(0), ssvo_enabled(true), ssgi_enable(false)
		{
		}

		uint32_t attrib;

		FrameBufferPtr frame_buffer;

		array<bool, Num_GBuffers> g_buffer_enables;

		array<FrameBufferPtr, Num_GBuffers> pre_depth_buffers;

		array<FrameBufferPtr, Num_GBuffers> g_buffers;
		array<FrameBufferPtr, Num_GBuffers> g_buffers_rt1;
		array<TexturePtr, Num_GBuffers> g_buffer_rt0_texs;
		array<TexturePtr, Num_GBuffers> g_buffer_rt1_texs;
		array<TexturePtr, Num_GBuffers> g_buffer_ds_texs;
		array<TexturePtr, Num_GBuffers> g_buffer_depth_texs;

		array<FrameBufferPtr, Num_GBuffers> lighting_buffers;
		array<TexturePtr, Num_GBuffers> lighting_texs;

		FrameBufferPtr shadowing_buffer;
		TexturePtr shadowing_tex;

		array<FrameBufferPtr, Num_GBuffers> shading_buffers;
		array<TexturePtr, Num_GBuffers> shading_texs;

		array<FrameBufferPtr, Num_GBuffers> curr_merged_shading_buffers;
		TexturePtr curr_merged_shading_tex;
		array<FrameBufferPtr, Num_GBuffers> curr_merged_depth_buffers;
		TexturePtr curr_merged_depth_tex;

		array<FrameBufferPtr, Num_GBuffers> prev_merged_shading_buffers;
		TexturePtr prev_merged_shading_tex;
		array<FrameBufferPtr, Num_GBuffers> prev_merged_depth_buffers;
		TexturePtr prev_merged_depth_tex;

		TexturePtr small_ssvo_tex;
		bool ssvo_enabled;

		float4x4 view, proj;
		float4x4 inv_view, inv_proj;
		float3 depth_near_far_invfar;
		float4x4 proj_to_prev;

		IndirectLightingLayerPtr il_layer;
		bool ssgi_enable;

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

		void AddDecal(RenderDecalPtr const & decal);

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
		TexturePtr const & CurrFrameShadingTex(uint32_t vp) const
		{
			return viewports_[vp].curr_merged_shading_tex;
		}
		TexturePtr const & CurrFrameDepthTex(uint32_t vp) const
		{
			return viewports_[vp].curr_merged_depth_tex;
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
		void SetupViewportGI(uint32_t vp);
		void AccumulateToLightingTex(uint32_t vp);

		uint32_t ComposePassScanCode(uint32_t vp_index, int32_t pass_type, int32_t org_no, int32_t index_in_pass);
		void DecomposePassScanCode(uint32_t& vp_index, int32_t& pass_type, int32_t& org_no, int32_t& index_in_pass, uint32_t code);

	private:
		bool mrt_g_buffer_support_;
		bool depth_texture_support_;

		RenderEffectPtr g_buffer_effect_;
		RenderEffectPtr dr_effect_;

		array<PerViewport, 8> viewports_;
		uint32_t active_viewport_;

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
		std::vector<RenderablePtr> decals_;

		std::vector<uint32_t> pass_scaned_;

		array<RenderTechniquePtr, LT_NumLightTypes> technique_shadows_;
		array<RenderTechniquePtr, LT_NumLightTypes> technique_lights_;
		RenderTechniquePtr technique_light_depth_only_;
		RenderTechniquePtr technique_light_stencil_;
		RenderTechniquePtr technique_clear_stencil_;
		RenderTechniquePtr technique_no_lighting_;
		RenderTechniquePtr technique_shading_;
		array<RenderTechniquePtr, 2> technique_merge_shadings_;
		array<RenderTechniquePtr, 2> technique_merge_depths_;
		RenderTechniquePtr technique_copy_shading_depth_;
		RenderTechniquePtr technique_copy_depth_;

		static int const NUM_SHADOWED_SPOT_LIGHTS = 4;
		static int const NUM_SHADOWED_POINT_LIGHTS = 1;

		std::vector<int32_t> sm_light_indices_;
		FrameBufferPtr sm_buffer_;
		TexturePtr sm_tex_;
		TexturePtr sm_depth_tex_;
		array<TexturePtr, NUM_SHADOWED_SPOT_LIGHTS> blur_sm_2d_texs_;
		array<TexturePtr, NUM_SHADOWED_POINT_LIGHTS> blur_sm_cube_texs_;

		PostProcessPtr sm_filter_pp_;
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
		RenderEffectParameterPtr projective_map_2d_tex_param_;
		RenderEffectParameterPtr projective_map_cube_tex_param_;
		RenderEffectParameterPtr shadow_map_2d_tex_param_;
		RenderEffectParameterPtr shadow_map_cube_tex_param_;
		RenderEffectParameterPtr inv_width_height_param_;
		RenderEffectParameterPtr shadowing_tex_param_;
		RenderEffectParameterPtr near_q_param_;

		std::vector<SceneObject*> visible_scene_objs_;
		bool has_reflective_objs_;
		bool has_simple_forward_objs_;

		PostProcessPtr atmospheric_pp_;

		FrameBufferPtr rsm_buffer_;
		array<TexturePtr, 2> rsm_texs_;

		bool indirect_lighting_enabled_;

		int illum_;
		float indirect_scale_;
		PostProcessPtr copy_to_light_buffer_pp_;
		PostProcessPtr copy_to_light_buffer_i_pp_;
	};
}

#endif		// _DEFERREDRENDERINGLAYER_HPP
