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
#include <KlayGE/CascadedShadowLayer.hpp>

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
			: attrib(0), ssvo_enabled(true)
		{
		}

		uint32_t attrib;

		FrameBufferPtr frame_buffer;

		array<bool, Num_GBuffers> g_buffer_enables;

		FrameBufferPtr pre_depth_buffer;

		FrameBufferPtr g_buffer;
		FrameBufferPtr g_buffer_rt1;
		TexturePtr g_buffer_rt0_tex;
		TexturePtr g_buffer_rt1_tex;
		TexturePtr g_buffer_ds_tex;
		TexturePtr g_buffer_depth_tex;
		TexturePtr g_buffer_rt0_backup_tex;

		FrameBufferPtr lighting_buffer;
		TexturePtr lighting_tex;

		FrameBufferPtr shadowing_buffer;
		TexturePtr shadowing_tex;

		FrameBufferPtr projective_shadowing_buffer;
		TexturePtr projective_shadowing_tex;

		FrameBufferPtr shading_buffer;
		TexturePtr shading_tex;

		uint32_t num_cascades;
		array<TexturePtr, CascadedShadowLayer::MAX_NUM_CASCADES> blur_cascaded_sm_texs;

		FrameBufferPtr curr_merged_shading_buffer;
		TexturePtr curr_merged_shading_tex;
		FrameBufferPtr curr_merged_depth_buffer;
		TexturePtr curr_merged_depth_tex;

		FrameBufferPtr prev_merged_shading_buffer;
		TexturePtr prev_merged_shading_tex;
		FrameBufferPtr prev_merged_depth_buffer;
		TexturePtr prev_merged_depth_tex;

		TexturePtr small_ssvo_tex;
		bool ssvo_enabled;

		float4x4 view, proj;
		float4x4 inv_view, inv_proj;
		float3 depth_near_far_invfar;
		float4x4 proj_to_prev;

		IndirectLightingLayerPtr il_layer;

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

		TexturePtr const & LightingTex(uint32_t vp) const
		{
			return viewports_[vp].lighting_tex;
		}
		TexturePtr const & ShadingTex(uint32_t vp) const
		{
			return viewports_[vp].shading_tex;
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

		TexturePtr const & GBufferRT0Tex(uint32_t vp) const
		{
			return viewports_[vp].g_buffer_rt0_tex;
		}
		TexturePtr const & GBufferRT1Tex(uint32_t vp) const
		{
			return viewports_[vp].g_buffer_rt1_tex;
		}
		TexturePtr const & DepthTex(uint32_t vp) const
		{
			return viewports_[vp].g_buffer_depth_tex;
		}
		TexturePtr const & GBufferRT0BackupTex(uint32_t vp) const
		{
			return viewports_[vp].g_buffer_rt0_backup_tex;
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

		void SetCascadedShadowType(CascadedShadowLayerType type);
		CascadedShadowLayerPtr const & GetCascadedShadowLayer() const
		{
			return cascaded_shadow_layer_;
		}
		int32_t CurrCascadeIndex() const
		{
			return curr_cascade_index_;
		}
		void SetViewportCascades(uint32_t vp, uint32_t num_cascades, float pssm_lambda);

	private:
		void SetupViewportGI(uint32_t vp, bool ssgi_enable);
		void AccumulateToLightingTex(PerViewport const & pvp);

		uint32_t ComposePassScanCode(uint32_t vp_index, PassType pass_type, int32_t org_no, int32_t index_in_pass);
		void DecomposePassScanCode(uint32_t& vp_index, PassType& pass_type, int32_t& org_no, int32_t& index_in_pass, uint32_t code);

		void BuildLightList();
		void BuildVisibleSceneObjList(bool& has_opaque_objs, bool& has_transparency_back_objs, bool& has_transparency_front_objs);
		void BuildPassScanList(bool has_opaque_objs, bool has_transparency_back_objs, bool has_transparency_front_objs);
		void CheckLightVisible(uint32_t vp_index, uint32_t light_index);
		void AppendGBufferPassScanCode(uint32_t vp_index, uint32_t g_buffer_index);
		void AppendShadowPassScanCode(uint32_t light_index);
		void AppendCascadedShadowPassScanCode(uint32_t vp_index, uint32_t light_index);
		void AppendShadowingPassScanCode(uint32_t vp_index, uint32_t g_buffer_index, uint32_t light_index);
		void AppendLightingPassScanCode(uint32_t vp_index, uint32_t g_buffer_index, uint32_t light_index);
		void AppendShadingPassScanCode(uint32_t vp_index, uint32_t g_buffer_index);
		void PreparePVP(PerViewport& pvp);
		void GenerateDepthBuffer(PerViewport const & pvp, uint32_t g_buffer_index);
		void GenerateGBuffer(PerViewport const & pvp, uint32_t g_buffer_index);
		void PostGenerateGBuffer(PerViewport const & pvp);
		void RenderDecals(PerViewport const & pvp, PassType pass_type);
		void PrepareLightCamera(PerViewport const & pvp, LightSourcePtr const & light,
			int32_t index_in_pass, PassType pass_type);
		void PostGenerateShadowMap(PerViewport const & pvp, int32_t org_no, int32_t index_in_pass);
		void UpdateShadowing(PerViewport const & pvp, int32_t org_no);
		void UpdateLighting(PerViewport const & pvp, LightSource::LightType type, int32_t org_no);
		void UpdateIndirectAndSSVO(PerViewport const & pvp);
		void UpdateShading(PerViewport const & pvp, uint32_t g_buffer_index);
		void MergeShadingAndDepth(PerViewport const & pvp, uint32_t g_buffer_index);
		void AddSSR(PerViewport const & pvp);
		void AddAtmospheric(PerViewport const & pvp);
		void AddTAA(PerViewport const & pvp);

	private:
		bool mrt_g_buffer_support_;
		bool depth_texture_support_;
		bool tex_array_support_;

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
		array<RenderLayoutPtr, LightSource::LT_NumLightTypes> light_volume_rl_;
		OBBox cone_obb_;
		OBBox pyramid_obb_;
		OBBox box_obb_;

		std::vector<LightSourcePtr> lights_;
		std::vector<RenderablePtr> decals_;

		std::vector<uint32_t> pass_scaned_;

		array<array<RenderTechniquePtr, 5>, LightSource::LT_NumLightTypes> technique_shadows_;
		array<RenderTechniquePtr, LightSource::LT_NumLightTypes> technique_lights_;
		RenderTechniquePtr technique_light_depth_only_;
		RenderTechniquePtr technique_light_stencil_;
		RenderTechniquePtr technique_clear_stencil_;
		RenderTechniquePtr technique_no_lighting_;
		RenderTechniquePtr technique_shading_;
		array<RenderTechniquePtr, 2> technique_merge_shadings_;
		array<RenderTechniquePtr, 2> technique_merge_depths_;
		RenderTechniquePtr technique_copy_shading_depth_;
		RenderTechniquePtr technique_copy_depth_;

		static uint32_t const MAX_NUM_SHADOWED_SPOT_LIGHTS = 4;
		static uint32_t const MAX_NUM_SHADOWED_POINT_LIGHTS = 1;

		int32_t projective_light_index_;
		std::vector<int32_t> sm_light_indices_;
		FrameBufferPtr sm_buffer_;
		TexturePtr sm_tex_;
		TexturePtr sm_depth_tex_;
		FrameBufferPtr cascaded_sm_buffer_;
		TexturePtr cascaded_sm_tex_;
		array<TexturePtr, MAX_NUM_SHADOWED_SPOT_LIGHTS + 1> blur_sm_2d_texs_;
		array<TexturePtr, MAX_NUM_SHADOWED_POINT_LIGHTS + 1> blur_sm_cube_texs_;

		PostProcessPtr sm_filter_pp_;
		PostProcessPtr depth_to_esm_pp_;
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
		RenderEffectParameterPtr projective_shadowing_tex_param_;
		RenderEffectParameterPtr shadowing_channel_param_;
		RenderEffectParameterPtr esm_scale_factor_param_;
		RenderEffectParameterPtr near_q_param_;
		RenderEffectParameterPtr cascade_intervals_param_;
		RenderEffectParameterPtr cascade_scale_bias_param_;
		RenderEffectParameterPtr num_cascades_param_;
		RenderEffectParameterPtr view_z_to_light_view_param_;
		RenderEffectParameterPtr cascaded_shadow_map_tex_array_param_;
		array<RenderEffectParameterPtr, CascadedShadowLayer::MAX_NUM_CASCADES> cascaded_shadow_map_texs_param_;

		std::vector<SceneObject*> visible_scene_objs_;
		bool has_reflective_objs_;
		bool has_simple_forward_objs_;

		PostProcessPtr atmospheric_pp_;

		FrameBufferPtr rsm_buffer_;
		array<TexturePtr, 2> rsm_texs_;

		bool indirect_lighting_enabled_;
		int32_t cascaded_shadow_index_;

		int illum_;
		float indirect_scale_;
		PostProcessPtr copy_to_light_buffer_pp_;
		PostProcessPtr copy_to_light_buffer_i_pp_;

		CascadedShadowLayerPtr cascaded_shadow_layer_;
		float2 blur_size_light_space_;
		int32_t curr_cascade_index_;
	};
}

#endif		// _DEFERREDRENDERINGLAYER_HPP
