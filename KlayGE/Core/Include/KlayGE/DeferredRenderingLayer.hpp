/**
 * @file DeferredRenderingLayer.hpp
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

#ifndef KLAYGE_CORE_DEFERRED_RENDERING_LAYER_HPP
#define KLAYGE_CORE_DEFERRED_RENDERING_LAYER_HPP

#pragma once

#include <KlayGE/PreDeclare.hpp>

#include <array>
#include <functional>

#include <KlayGE/Light.hpp>
#include <KlayGE/IndirectLightingLayer.hpp>
#include <KlayGE/CascadedShadowLayer.hpp>
#include <KlayGE/Renderable.hpp>

#define TRIDITIONAL_DEFERRED 0
#define LIGHT_INDEXED_DEFERRED 1
#define DEFAULT_DEFERRED LIGHT_INDEXED_DEFERRED

namespace KlayGE
{
	enum VPAttribMask
	{
		VPAM_Enabled = 1UL << 0,
		VPAM_NoOpaque = 1UL << 1,
		VPAM_NoTransparencyBack = 1UL << 2,
		VPAM_NoTransparencyFront = 1UL << 3,
		VPAM_NoSimpleForward = 1UL << 4,
		VPAM_NoGI = 1UL << 5,
		VPAM_NoSSVO = 1UL << 6,
		VPAM_NoDoF = 1UL << 7,
		VPAM_NoSSS = 1UL << 8,
		VPAM_NoSSR = 1UL << 9,
		VPAM_NoVDM = 1UL << 10,
		VPAM_NoAtmospheric = 1UL << 11,
		VPAM_NoTAA = 1UL << 12,
		VPAM_NoMotionBlur = 1UL << 13,
		VPAM_NoPPR = 1UL << 14,
	};

	struct PerViewport
	{
		PerViewport()
			: attrib(0), num_cascades(4), curr_merged_buffer_index(0), ssvo_enabled(true)
		{
		}

		uint32_t attrib;

		FrameBufferPtr frame_buffer;
		uint32_t sample_count = 1;
		uint32_t sample_quality = 0;

		std::array<bool, PTB_None> g_buffer_enables;

		FrameBufferPtr g_buffer_fb;
		FrameBufferPtr g_buffer_resolved_fb;
		TexturePtr g_buffer_rt0_tex;
		ShaderResourceViewPtr g_buffer_rt0_srv;
		TexturePtr g_buffer_rt1_tex;
		ShaderResourceViewPtr g_buffer_rt1_srv;
		TexturePtr g_buffer_rt2_tex;
		ShaderResourceViewPtr g_buffer_rt2_srv;
		TexturePtr g_buffer_ds_tex;
		ShaderResourceViewPtr g_buffer_ds_srv;
		TexturePtr g_buffer_depth_tex;
		ShaderResourceViewPtr g_buffer_depth_srv;
		RenderTargetViewPtr g_buffer_depth_rtv;
		TexturePtr g_buffer_resolved_rt0_tex;
		ShaderResourceViewPtr g_buffer_resolved_rt0_srv;
		TexturePtr g_buffer_resolved_rt1_tex;
		ShaderResourceViewPtr g_buffer_resolved_rt1_srv;
		TexturePtr g_buffer_resolved_rt2_tex;
		ShaderResourceViewPtr g_buffer_resolved_rt2_srv;
		TexturePtr g_buffer_resolved_depth_tex;
		ShaderResourceViewPtr g_buffer_resolved_depth_srv;
		RenderTargetViewPtr g_buffer_resolved_depth_rtv;
		TexturePtr g_buffer_rt0_backup_tex;
#if DEFAULT_DEFERRED == LIGHT_INDEXED_DEFERRED
		std::vector<TexturePtr> g_buffer_min_max_depth_texs;
		std::vector<ShaderResourceViewPtr> g_buffer_min_max_depth_srvs;
		std::vector<RenderTargetViewPtr> g_buffer_min_max_depth_rtvs;
		ShaderResourceViewPtr g_buffer_stencil_srv;
#endif
		std::vector<TexturePtr> g_buffer_vdm_max_ds_texs;
		std::vector<ShaderResourceViewPtr> g_buffer_vdm_max_ds_srvs;
		std::vector<DepthStencilViewPtr> g_buffer_vdm_max_ds_dsvs;

		FrameBufferPtr shadowing_fb;
		TexturePtr shadowing_tex;

		FrameBufferPtr projective_shadowing_fb;
		TexturePtr projective_shadowing_tex;

		FrameBufferPtr reflection_fb;
		TexturePtr reflection_tex;

		FrameBufferPtr vdm_fb;
		TexturePtr vdm_color_tex;
		ShaderResourceViewPtr vdm_color_srv;
		RenderTargetViewPtr vdm_color_rtv;
		TexturePtr vdm_transition_tex;
		ShaderResourceViewPtr vdm_transition_srv;
		TexturePtr vdm_count_tex;
		ShaderResourceViewPtr vdm_count_srv;

		FrameBufferPtr shading_fb;
		TexturePtr shading_tex;
		ShaderResourceViewPtr shading_srv;
		RenderTargetViewPtr shading_rtv;

		uint32_t num_cascades;
		std::array<TexturePtr, CascadedShadowLayer::MAX_NUM_CASCADES> filtered_csm_texs;
		std::array<ShaderResourceViewPtr, CascadedShadowLayer::MAX_NUM_CASCADES> filtered_csm_srvs;
		std::array<RenderTargetViewPtr, CascadedShadowLayer::MAX_NUM_CASCADES> filtered_csm_slice_rtvs;

		std::array<FrameBufferPtr, 2> merged_shading_fbs;
		std::array<TexturePtr, 2> merged_shading_texs;
		std::array<ShaderResourceViewPtr, 2> merged_shading_srvs;
		std::array<RenderTargetViewPtr, 2> merged_shading_rtvs;
		std::array<FrameBufferPtr, 2> merged_depth_fbs;
		std::array<TexturePtr, 2> merged_depth_texs;
		std::array<ShaderResourceViewPtr, 2> merged_depth_srvs;
		std::array<RenderTargetViewPtr, 2> merged_depth_rtvs;
		std::array<TexturePtr, 2> merged_shading_resolved_texs;
		std::array<ShaderResourceViewPtr, 2> merged_shading_resolved_srvs;
		std::array<RenderTargetViewPtr, 2> merged_shading_resolved_rtvs;
		std::array<FrameBufferPtr, 2> merged_depth_resolved_fbs;
		std::array<TexturePtr, 2> merged_depth_resolved_texs;
		std::array<ShaderResourceViewPtr, 2> merged_depth_resolved_srvs;
		std::array<RenderTargetViewPtr, 2> merged_depth_resolved_rtvs;
		uint32_t curr_merged_buffer_index;

		TexturePtr dof_tex;
		ShaderResourceViewPtr dof_srv;
		RenderTargetViewPtr dof_rtv;

		TexturePtr motion_blur_tex;
		ShaderResourceViewPtr motion_blur_srv;
		RenderTargetViewPtr motion_blur_rtv;

		TexturePtr small_ssvo_tex;
		ShaderResourceViewPtr small_ssvo_srv;
		RenderTargetViewPtr small_ssvo_rtv;
		bool ssvo_enabled;

		TexturePtr merged_shading_resolved_before_ssr_tex;
		ShaderResourceViewPtr merged_shading_resolved_before_ssr_srv;

		float4x4 view, proj;
		float4x4 inv_view, inv_proj;
		float4x4 proj_to_prev;

		std::unique_ptr<IndirectLightingLayer> il_layer;

		std::vector<char> light_visibles;

#if DEFAULT_DEFERRED == TRIDITIONAL_DEFERRED
		FrameBufferPtr lighting_fb;
		TexturePtr lighting_tex;
		ShaderResourceViewPtr lighting_srv;
#elif DEFAULT_DEFERRED == LIGHT_INDEXED_DEFERRED
		FrameBufferPtr light_index_fb;
		TexturePtr light_index_tex;

		FrameBufferPtr temp_shading_fb;
		TexturePtr temp_shading_tex;
		ShaderResourceViewPtr temp_shading_srv;

		TexturePtr temp_shading_tex_array;

		TexturePtr multi_sample_mask_tex;

		TexturePtr lights_start_tex;
		TexturePtr intersected_light_indices_tex;
#endif
	};

	class KLAYGE_CORE_API DeferredRenderingLayer final : boost::noncopyable
	{
		class DeferredRenderingJob final : boost::noncopyable
		{
		public:
			explicit DeferredRenderingJob(std::function<uint32_t()> job_func)
				: func_(std::move(job_func))
			{
			}

			uint32_t Run()
			{
				return func_();
			}

		private:
			std::function<uint32_t()> func_;
		};


	public:
		enum DisplayType
		{
			DT_Final,
			DT_Position,
			DT_Normal,
			DT_Depth,
			DT_Diffuse,
			DT_Specular,
			DT_Shininess,
			DT_MotionVec,
			DT_Occlusion,
			DT_Edge,
			DT_SSVO,
#if DEFAULT_DEFERRED == TRIDITIONAL_DEFERRED
			DT_DiffuseLighting,
			DT_SpecularLighting,
#endif
			Num_DT
		};

	public:
		DeferredRenderingLayer();

		static void Register();

		void Suspend();
		void Resume();

		void SSGIEnabled(uint32_t vp, bool ssgi);
		void SSVOEnabled(uint32_t vp, bool ssvo);
		void SSSEnabled(bool sss);
		void SSSStrength(float strength);
		void SSSCorrection(float correction);
		void TranslucencyEnabled(bool trans);
		void TranslucencyStrength(float strength);
		void SSREnabled(bool ssr);
		void PPREnabled(bool ppr);
		void PPRPlane(Plane const& plane);
		void TemporalAAEnabled(bool taa);
		void DepthOfFieldEnabled(bool dof, bool bokeh);
		void DepthFocus(float plane, float range);
		void BokehLuminanceThreshold(float lum_threshold);
		void MotionBlurEnabled(bool mb);
		void MotionBlurExposure(float exposure);
		float MotionBlurExposure() const;
		void MotionBlurRadius(uint32_t blur_radius);
		uint32_t MotionBlurRadius() const;
		void MotionBlurReconstructionSamples(uint32_t reconstruction_samples);

		void AddDecal(RenderDecalPtr const & decal);

		void SetupViewport(uint32_t index, FrameBufferPtr const & fb, uint32_t attrib);
		void SetupViewport(uint32_t index, FrameBufferPtr const & fb, uint32_t attrib, uint32_t sample_count, uint32_t sample_quality);
		void EnableViewport(uint32_t index, bool enable);
		uint32_t Update(uint32_t pass);

		void AtmosphericPostProcess(PostProcessPtr const & pp)
		{
			atmospheric_pp_ = pp;
		}

		RenderEffectPtr const & GBufferEffect(RenderMaterial const * material, bool line, bool skinning) const;

#if DEFAULT_DEFERRED == TRIDITIONAL_DEFERRED
		ShaderResourceViewPtr const & LightingSrv(uint32_t vp) const
		{
			return viewports_[vp].lighting_srv;
		}
#endif
		TexturePtr const & ShadingTex(uint32_t vp) const
		{
			return viewports_[vp].shading_tex;
		}
		TexturePtr const & CurrFrameShadingTex(uint32_t vp) const
		{
			return viewports_[vp].merged_shading_texs[viewports_[vp].curr_merged_buffer_index];
		}
		TexturePtr const & CurrFrameResolvedShadingTex(uint32_t vp) const
		{
			return viewports_[vp].merged_shading_resolved_texs[viewports_[vp].curr_merged_buffer_index];
		}
		TexturePtr const & CurrFrameDepthTex(uint32_t vp) const
		{
			return viewports_[vp].merged_depth_texs[viewports_[vp].curr_merged_buffer_index];
		}
		TexturePtr const & CurrFrameResolvedDepthTex(uint32_t vp) const
		{
			return viewports_[vp].merged_depth_resolved_texs[viewports_[vp].curr_merged_buffer_index];
		}
		TexturePtr const & PrevFrameShadingTex(uint32_t vp) const
		{
			return viewports_[vp].merged_shading_texs[!viewports_[vp].curr_merged_buffer_index];
		}
		TexturePtr const & PrevFrameResolvedShadingTex(uint32_t vp) const
		{
			return viewports_[vp].merged_shading_resolved_texs[!viewports_[vp].curr_merged_buffer_index];
		}
		ShaderResourceViewPtr const & PrevFrameResolvedShadingSrv(uint32_t vp) const
		{
			return viewports_[vp].merged_shading_resolved_srvs[!viewports_[vp].curr_merged_buffer_index];
		}
		TexturePtr const & PrevFrameDepthTex(uint32_t vp) const
		{
			return viewports_[vp].merged_depth_texs[!viewports_[vp].curr_merged_buffer_index];
		}
		ShaderResourceViewPtr const & PrevFrameDepthSrv(uint32_t vp) const
		{
			return viewports_[vp].merged_depth_srvs[!viewports_[vp].curr_merged_buffer_index];
		}
		TexturePtr const & PrevFrameResolvedDepthTex(uint32_t vp) const
		{
			return viewports_[vp].merged_depth_resolved_texs[!viewports_[vp].curr_merged_buffer_index];
		}
		ShaderResourceViewPtr const & PrevFrameResolvedDepthSrv(uint32_t vp) const
		{
			return viewports_[vp].merged_depth_resolved_srvs[!viewports_[vp].curr_merged_buffer_index];
		}

		TexturePtr const & SmallSSVOTex(uint32_t vp) const
		{
			return viewports_[vp].small_ssvo_tex;
		}
		ShaderResourceViewPtr const & SmallSSVOSrv(uint32_t vp) const
		{
			return viewports_[vp].small_ssvo_srv;
		}

		TexturePtr const & GBufferRT0Tex(uint32_t vp) const
		{
			return viewports_[vp].g_buffer_rt0_tex;
		}
		TexturePtr const & GBufferRT1Tex(uint32_t vp) const
		{
			return viewports_[vp].g_buffer_rt1_tex;
		}
		TexturePtr const& GBufferRT2Tex(uint32_t vp) const
		{
			return viewports_[vp].g_buffer_rt2_tex;
		}
		TexturePtr const & DepthTex(uint32_t vp) const
		{
			return viewports_[vp].g_buffer_depth_tex;
		}
		TexturePtr const & GBufferResolvedRT0Tex(uint32_t vp) const
		{
			return viewports_[vp].g_buffer_resolved_rt0_tex;
		}
		ShaderResourceViewPtr const& GBufferResolvedRT0Srv(uint32_t vp) const
		{
			return viewports_[vp].g_buffer_resolved_rt0_srv;
		}
		TexturePtr const & GBufferResolvedRT1Tex(uint32_t vp) const
		{
			return viewports_[vp].g_buffer_resolved_rt1_tex;
		}
		ShaderResourceViewPtr const& GBufferResolvedRT1Srv(uint32_t vp) const
		{
			return viewports_[vp].g_buffer_resolved_rt1_srv;
		}
		TexturePtr const& GBufferResolvedRT2Tex(uint32_t vp) const
		{
			return viewports_[vp].g_buffer_resolved_rt2_tex;
		}
		ShaderResourceViewPtr const& GBufferResolvedRT2Srv(uint32_t vp) const
		{
			return viewports_[vp].g_buffer_resolved_rt2_srv;
		}
		TexturePtr const & ResolvedDepthTex(uint32_t vp) const
		{
			return viewports_[vp].g_buffer_resolved_depth_tex;
		}
		ShaderResourceViewPtr const& ResolvedDepthSrv(uint32_t vp) const
		{
			return viewports_[vp].g_buffer_resolved_depth_srv;
		}
		TexturePtr const & GBufferRT0BackupTex(uint32_t vp) const
		{
			return viewports_[vp].g_buffer_rt0_backup_tex;
		}

		TexturePtr const & ReflectionTex(uint32_t vp) const
		{
			return viewports_[vp].reflection_tex;
		}

		uint32_t ActiveViewport() const
		{
			return active_viewport_;
		}

		uint32_t ViewportSampleCount(uint32_t vp) const
		{
			return viewports_[vp].sample_count;
		}
		uint32_t ViewportSampleQuality(uint32_t vp) const
		{
			return viewports_[vp].sample_quality;
		}

		void DisplayIllum(int illum);
		void IndirectScale(float scale);

		void LightDistance(float dist)
		{
			light_scale_ = dist * 0.01f;
		}

		void SetCascadedShadowType(CascadedShadowLayerType type);
		CascadedShadowLayer& GetCascadedShadowLayer() const
		{
			return *cascaded_shadow_layer_;
		}
		int32_t CurrCascadeIndex() const
		{
			return curr_cascade_index_;
		}
		void SetViewportCascades(uint32_t vp, uint32_t num_cascades, float pssm_lambda);

		// For debug only
		void ForceLineMode(bool line)
		{
			force_line_mode_ = line;
		}
		bool ForceLineMode() const
		{
			return force_line_mode_;
		}

		void Display(DisplayType display_type);
		void DumpIntermediaTextures();

		uint32_t NumObjectsRendered() const;
		uint32_t NumRenderablesRendered() const;
		uint32_t NumPrimitivesRendered() const;
		uint32_t NumVerticesRendered() const;

#ifndef KLAYGE_SHIP
		PerfRegion const& ShadowMapPerf() const noexcept
		{
			return *shadow_map_perf_;
		}
		PerfRegion const& GBufferPerf(PassTargetBuffer ptb) const noexcept
		{
			return *gbuffer_perfs_[ptb];
		}
		PerfRegion const& ShadowingPerf(PassTargetBuffer ptb) const noexcept
		{
			return *shadowing_perfs_[ptb];
		}
		PerfRegion const& IndirectLightingPerf(PassTargetBuffer ptb) const noexcept
		{
			return *indirect_lighting_perfs_[ptb];
		}
		PerfRegion const& ShadingPerf(PassTargetBuffer ptb) const noexcept
		{
			return *shading_perfs_[ptb];
		}
		PerfRegion const& SpecialShadingPerf(PassTargetBuffer ptb) const noexcept
		{
			return *special_shading_perfs_[ptb];
		}
#endif

	private:
		static bool ConfirmDevice();

		void SetupViewportGI(uint32_t vp, bool ssgi_enable);
		void AccumulateToLightingTex(PerViewport const & pvp, PassTargetBuffer pass_tb);

		uint32_t ComposePassScanCode(uint32_t vp_index, PassType pass_type,
			int32_t light_index, int32_t index_in_pass, bool is_profile) const;
		void DecomposePassScanCode(uint32_t& vp_index, PassType& pass_type,
			int32_t& light_index, int32_t& index_in_pass, bool& is_profile, uint32_t code) const;

		void BuildLightList();
		void BuildVisibleSceneObjList(bool& has_opaque_objs, bool& has_transparency_back_objs, bool& has_transparency_front_objs);
		void BuildPassScanList(bool has_opaque_objs, bool has_transparency_back_objs, bool has_transparency_front_objs);
		void CheckLightVisible(uint32_t vp_index, uint32_t light_index);
		void AppendGBufferPassScanCode(uint32_t vp_index, PassTargetBuffer pass_tb);
		void AppendShadowPassScanCode(uint32_t light_index);
		void AppendCascadedShadowPassScanCode(uint32_t vp_index, uint32_t light_index);
		void AppendIndirectLightingPassScanCode(uint32_t vp_index, uint32_t light_index);
		void AppendShadingPassScanCode(uint32_t vp_index, PassTargetBuffer pass_tb);
		void PreparePVP(PerViewport& pvp);
		void GenerateGBuffer(PerViewport const & pvp, PassTargetBuffer pass_tb);
		void PostGenerateGBuffer(PerViewport const & pvp);
		void BuildLinearDepthMipmap(PerViewport const & pvp);
		void RenderDecals(PerViewport const & pvp, PassType pass_type);
		void PrepareLightCamera(PerViewport const & pvp, LightSource const & light,
			int32_t index_in_pass, PassType pass_type);
		void PostGenerateShadowMap(PerViewport const & pvp, int32_t light_index, int32_t index_in_pass);
		void UpdateShadowing(PerViewport const & pvp);
#if DEFAULT_DEFERRED == LIGHT_INDEXED_DEFERRED
		void UpdateShadowingCS(PerViewport const & pvp);
#endif
		void MergeIndirectLighting(PerViewport const & pvp, PassTargetBuffer pass_tb);
		void MergeSSVO(PerViewport const & pvp, PassTargetBuffer pass_tb);
		void MergeShadingAndDepth(PerViewport const & pvp, PassTargetBuffer pass_tb);
		void AddTranslucency(uint32_t light_index, PerViewport const & pvp, PassTargetBuffer pass_tb);
		void AddSSS(PerViewport const & pvp);
		void AddSSR(PerViewport const & pvp);
		void AddPPR(PerViewport const& pvp);
		void AddVDM(PerViewport const & pvp);
		void AddAtmospheric(PerViewport const & pvp);
		void AddTAA(PerViewport const & pvp);

#if DEFAULT_DEFERRED == TRIDITIONAL_DEFERRED
		void UpdateLighting(PerViewport const & pvp, LightSource::LightType type, int32_t light_index);
		void UpdateShading(PerViewport const & pvp, PassTargetBuffer pass_tb);
#elif DEFAULT_DEFERRED == LIGHT_INDEXED_DEFERRED
		void UpdateLightIndexedLighting(PerViewport const & pvp, PassTargetBuffer pass_tb);
		void UpdateLightIndexedLightingAmbientSun(PerViewport const & pvp, LightSource::LightType type,
			int32_t light_index, PassTargetBuffer pass_tb);
		void UpdateLightIndexedLightingDirectional(PerViewport const & pvp, PassTargetBuffer pass_tb,
			std::vector<uint32_t>::const_iterator iter_beg, std::vector<uint32_t>::const_iterator iter_end);
		void UpdateLightIndexedLightingPointSpotArea(PerViewport const & pvp, PassTargetBuffer pass_tb,
			std::vector<uint32_t>::const_iterator iter_beg, std::vector<uint32_t>::const_iterator iter_end);
		void CreateDepthMinMaxMap(PerViewport const & pvp);

		void UpdateClusteredLighting(PerViewport const & pvp, PassTargetBuffer pass_tb);
		void CreateDepthMinMaxMapCS(PerViewport const & pvp);
#endif
		void CreateVDMDepthMaxMap(PerViewport const & pvp);

		uint32_t BeginPerfProfileDRJob(PerfRegion& perf);
		uint32_t EndPerfProfileDRJob(PerfRegion& perf);
		uint32_t RenderingStatsDRJob();
		uint32_t GBufferGenerationDRJob(PerViewport& pvp, PassType pass_type);
		uint32_t GBufferProcessingDRJob(PerViewport const & pvp);
		uint32_t OpaqueGBufferProcessingDRJob(PerViewport const & pvp);
		uint32_t ShadowMapGenerationDRJob(PerViewport const & pvp, PassType pass_type, int32_t light_index, int32_t index_in_pass);
		uint32_t IndirectLightingDRJob(PerViewport const & pvp, int32_t light_index);
		uint32_t ShadowingDRJob(PerViewport const & pvp, PassTargetBuffer pass_tb);
		uint32_t ShadingDRJob(PerViewport const & pvp, PassType pass_type, int32_t index_in_pass);
		uint32_t ReflectionDRJob(PerViewport const & pvp, PassType pass_type);
		uint32_t VDMDRJob(PerViewport const & pvp);
		uint32_t SpecialShadingDRJob(PerViewport& pvp, PassType pass_type);
		uint32_t MergeShadingAndDepthDRJob(PerViewport& pvp, PassTargetBuffer pass_tb);
		uint32_t PostEffectsDRJob(PerViewport& pvp);
		uint32_t SimpleForwardDRJob(PerViewport& pvp);
		uint32_t PostSimpleForwardDRJob(PerViewport& pvp);
		uint32_t FinishingViewportDRJob(PerViewport& pvp);
		uint32_t FinishingDRJob();
		uint32_t SwitchViewportDRJob(uint32_t vp_index);
		uint32_t VisualizeGBufferDRJob();
		uint32_t VisualizeLightingDRJob();
		uint32_t ClearOnlyDRJob();

	private:
		bool tex_array_support_;
		bool flexible_srvs_support_;

		// TODO: Remove the magic number
		mutable RenderEffectPtr g_buffer_effects_[64];

		RenderEffectPtr dr_effect_;
#if DEFAULT_DEFERRED == LIGHT_INDEXED_DEFERRED
		uint32_t light_batch_;
		uint32_t num_depth_slices_;
		std::vector<float> depth_slices_;
		bool cs_cldr_;
		bool typed_uav_;
#endif

		std::array<PerViewport, 8> viewports_;
		uint32_t active_viewport_;

		PostProcessPtr ssvo_pp_;
		PostProcessPtr ssvo_blur_pp_;
		PostProcessPtr ssvo_upsample_pp_;

		PostProcessPtr sss_blur_pps_[2];
		bool sss_enabled_;

		PostProcessPtr translucency_pps_[2];
		bool translucency_enabled_;

		PostProcessPtr ssr_pps_[2];
		bool ssr_enabled_;

		PostProcessPtr ppr_pps_[2];
		bool ppr_enabled_;

		PostProcessPtr taa_pp_;
		bool taa_enabled_;

		PostProcessPtr vdm_composition_pp_;

		KlayGE::PostProcessPtr depth_of_field_pp_;
		bool depth_of_field_enabled_ = false;

		KlayGE::PostProcessPtr bokeh_filter_pp_;
		bool bokeh_filter_enabled_ = false;

		KlayGE::PostProcessPtr motion_blur_pp_;
		bool motion_blur_enabled_ = false;

		float light_scale_;
		RenderLayoutPtr rl_cone_;
		RenderLayoutPtr rl_pyramid_;
		RenderLayoutPtr rl_box_;
		RenderLayoutPtr rl_quad_;
		std::array<RenderLayoutPtr, LightSource::LT_NumLightTypes> light_volume_rl_;
		AABBox cone_aabb_;
		AABBox pyramid_aabb_;
		AABBox box_aabb_;

		LightSourcePtr default_ambient_light_;
		LightSourcePtr merged_ambient_light_;
		std::vector<LightSource*> lights_;
		std::vector<RenderablePtr> decals_;

		std::vector<std::unique_ptr<DeferredRenderingJob>> jobs_;
		std::vector<std::unique_ptr<DeferredRenderingJob>>::iterator curr_job_iter_;

		std::array<std::array<RenderTechnique*, 5>, LightSource::LT_NumLightTypes> technique_shadows_;
		RenderTechnique* technique_no_lighting_;
		RenderTechnique* technique_shading_;
		RenderTechnique* technique_merge_shading_[2];
		RenderTechnique* technique_merge_depth_[2];
		RenderTechnique* technique_copy_shading_depth_;
#if DEFAULT_DEFERRED == TRIDITIONAL_DEFERRED
		std::array<RenderTechnique*, LightSource::LT_NumLightTypes> technique_lights_;
		RenderTechnique* technique_light_depth_only_;
		RenderTechnique* technique_light_stencil_;
#elif DEFAULT_DEFERRED == LIGHT_INDEXED_DEFERRED
		RenderTechnique* technique_draw_light_index_point_;
		RenderTechnique* technique_draw_light_index_spot_;
		RenderTechnique* technique_lidr_ambient_;
		RenderTechnique* technique_lidr_directional_shadow_;
		RenderTechnique* technique_lidr_directional_no_shadow_;
		RenderTechnique* technique_lidr_point_shadow_;
		RenderTechnique* technique_lidr_point_no_shadow_;
		RenderTechnique* technique_lidr_spot_shadow_;
		RenderTechnique* technique_lidr_spot_no_shadow_;
		RenderTechnique* technique_lidr_sphere_area_shadow_;
		RenderTechnique* technique_lidr_sphere_area_no_shadow_;
		RenderTechnique* technique_lidr_tube_area_shadow_;
		RenderTechnique* technique_lidr_tube_area_no_shadow_;

		RenderTechnique* technique_cldr_shadowing_unified_[2];
		RenderTechnique* technique_cldr_light_intersection_unified_;
		RenderTechnique* technique_cldr_unified_[2];

		RenderTechnique* technique_depth_to_tiled_min_max_[2];
		RenderTechnique* technique_resolve_g_buffers_;
		RenderTechnique* technique_resolve_merged_depth_;
		RenderTechnique* technique_array_to_multiSample_;
#endif
		static uint32_t const MAX_NUM_SHADOWED_LIGHTS = 4;
		static uint32_t const MAX_NUM_SHADOWED_SPOT_LIGHTS = 4;
		static uint32_t const MAX_NUM_SHADOWED_POINT_LIGHTS = 1;
		static uint32_t const MAX_NUM_PROJECTIVE_SHADOWED_SPOT_LIGHTS = 1;
		static uint32_t const MAX_NUM_PROJECTIVE_SHADOWED_POINT_LIGHTS = 1;

		int32_t projective_light_index_;
		std::vector<std::pair<int32_t, uint32_t>> shadow_map_light_indices_;
		FrameBufferPtr shadow_map_fb_;
		TexturePtr shadow_map_tex_;
		RenderTargetViewPtr shadow_map_rtv_;
		TexturePtr shadow_map_depth_tex_;
		ShaderResourceViewPtr shadow_map_depth_srv_;
		FrameBufferPtr shadow_map_array_fb_;
		TexturePtr shadow_map_array_tex_;
		RenderTargetViewPtr shadow_map_array_rtv_;
		TexturePtr shadow_map_array_depth_tex_;
		ShaderResourceViewPtr shadow_map_array_depth_srvs_[6];
		FrameBufferPtr csm_fb_;
		TexturePtr csm_tex_;
		TexturePtr unfiltered_shadow_map_2d_texs_[MAX_NUM_SHADOWED_SPOT_LIGHTS + MAX_NUM_PROJECTIVE_SHADOWED_SPOT_LIGHTS];
		ShaderResourceViewPtr unfiltered_shadow_map_2d_srvs_[MAX_NUM_SHADOWED_SPOT_LIGHTS + MAX_NUM_PROJECTIVE_SHADOWED_SPOT_LIGHTS];
		TexturePtr filtered_shadow_map_2d_texs_[MAX_NUM_SHADOWED_SPOT_LIGHTS + MAX_NUM_PROJECTIVE_SHADOWED_SPOT_LIGHTS];
		ShaderResourceViewPtr filtered_shadow_map_2d_srvs_[MAX_NUM_SHADOWED_SPOT_LIGHTS + MAX_NUM_PROJECTIVE_SHADOWED_SPOT_LIGHTS];
		RenderTargetViewPtr filtered_shadow_map_2d_slice_rtvs_[MAX_NUM_SHADOWED_SPOT_LIGHTS + MAX_NUM_PROJECTIVE_SHADOWED_SPOT_LIGHTS];
		TexturePtr filtered_shadow_map_cube_texs_[MAX_NUM_SHADOWED_POINT_LIGHTS + MAX_NUM_PROJECTIVE_SHADOWED_POINT_LIGHTS];
		ShaderResourceViewPtr filtered_shadow_map_cube_srvs_[MAX_NUM_SHADOWED_POINT_LIGHTS + MAX_NUM_PROJECTIVE_SHADOWED_POINT_LIGHTS];
		RenderTargetViewPtr
			filtered_shadow_map_cube_face_rtvs_[(MAX_NUM_SHADOWED_POINT_LIGHTS + MAX_NUM_PROJECTIVE_SHADOWED_POINT_LIGHTS) * 6];

		PostProcessPtr shadow_map_filter_pp_;
		PostProcessPtr csm_filter_pp_;
		PostProcessPtr depth_to_esm_pp_;
		PostProcessPtr depth_to_linear_pps_[2];
		PostProcessPtr depth_mipmap_pp_;

		RenderEffectParameter* g_buffer_rt0_tex_param_;
		RenderEffectParameter* g_buffer_rt1_tex_param_;
		RenderEffectParameter* g_buffer_rt2_tex_param_;
		RenderEffectParameter* depth_tex_param_;
		RenderEffectParameter* depth_tex_ms_param_;
		RenderEffectParameter* shading_tex_param_;
		RenderEffectParameter* shading_tex_ms_param_;
		RenderEffectParameter* light_attrib_param_;
		RenderEffectParameter* light_radius_extend_param_;
		RenderEffectParameter* light_color_param_;
		RenderEffectParameter* light_falloff_range_param_;
		RenderEffectParameter* light_view_proj_param_;
		RenderEffectParameter* light_volume_mv_param_;
		RenderEffectParameter* light_volume_mvp_param_;
		RenderEffectParameter* view_to_light_model_param_;
		RenderEffectParameter* light_pos_es_param_;
		RenderEffectParameter* light_dir_es_param_;
		RenderEffectParameter* projective_map_2d_tex_param_;
		RenderEffectParameter* projective_map_cube_tex_param_;
		RenderEffectParameter* filtered_shadow_map_2d_tex_param_;
		RenderEffectParameter* filtered_shadow_map_2d_tex_array_param_;
		RenderEffectParameter* filtered_shadow_map_2d_light_index_param_;
		RenderEffectParameter* filtered_shadow_map_cube_tex_param_;
		RenderEffectParameter* inv_width_height_param_;
		RenderEffectParameter* shadowing_tex_param_;
		RenderEffectParameter* projective_shadowing_tex_param_;
		RenderEffectParameter* shadowing_channel_param_;
		RenderEffectParameter* esm_scale_factor_param_;
		RenderEffectParameter* near_q_param_;
		RenderEffectParameter* cascade_intervals_param_;
		RenderEffectParameter* cascade_scale_bias_param_;
		RenderEffectParameter* num_cascades_param_;
		RenderEffectParameter* view_z_to_light_view_param_;
		std::array<RenderEffectParameter*, CascadedShadowLayer::MAX_NUM_CASCADES> filtered_csm_texs_param_;
		PostProcessPtr depth_to_max_pps_[2];
#if DEFAULT_DEFERRED == TRIDITIONAL_DEFERRED
		RenderEffectParameter* lighting_tex_param_;
#elif DEFAULT_DEFERRED == LIGHT_INDEXED_DEFERRED
		RenderEffectParameter* g_buffer_rt0_tex_ms_param_;
		RenderEffectParameter* g_buffer_rt1_tex_ms_param_;
		RenderEffectParameter* g_buffer_rt2_tex_ms_param_;
		RenderEffectParameter* g_buffer_ds_tex_ms_param_;
		RenderEffectParameter* g_buffer_depth_tex_ms_param_;
		RenderEffectParameter* g_buffer_stencil_tex_param_;
		RenderEffectParameter* g_buffer_stencil_tex_ms_param_;
		RenderEffectParameter* src_2d_tex_array_param_;

		RenderEffectParameter* min_max_depth_tex_param_;
		RenderEffectParameter* lights_color_param_;
		RenderEffectParameter* lights_pos_es_param_;
		RenderEffectParameter* lights_dir_es_param_;
		RenderEffectParameter* lights_falloff_range_param_;
		RenderEffectParameter* lights_attrib_param_;
		RenderEffectParameter* lights_radius_extend_param_;
		RenderEffectParameter* lights_aabb_min_param_;
		RenderEffectParameter* lights_aabb_max_param_;
		RenderEffectParameter* light_index_tex_param_;
		RenderEffectParameter* tile_scale_param_;
		RenderEffectParameter* camera_proj_01_param_;
		PostProcessPtr depth_to_min_max_pp_;
		PostProcessPtr reduce_min_max_pp_;

		RenderEffectParameter* projective_shadowing_rw_tex_param_;
		RenderEffectParameter* shadowing_rw_tex_param_;
		RenderEffectParameter* lights_view_proj_param_;
		RenderEffectParameter* filtered_shadow_maps_2d_light_index_param_;
		RenderEffectParameter* esms_scale_factor_param_;

		RenderEffectParameter* near_q_far_param_;
		RenderEffectParameter* width_height_param_;
		RenderEffectParameter* depth_to_tiled_ds_in_tex_param_;
		RenderEffectParameter* depth_to_tiled_linear_depth_in_tex_ms_param_;
		RenderEffectParameter* depth_to_tiled_min_max_depth_rw_tex_param_;
		RenderEffectParameter* linear_depth_rw_tex_param_;
		RenderEffectParameter* upper_left_param_;
		RenderEffectParameter* x_dir_param_;
		RenderEffectParameter* y_dir_param_;
		RenderEffectParameter* multi_sample_mask_tex_param_;
		RenderEffectParameter* shading_in_tex_param_;
		RenderEffectParameter* shading_in_tex_ms_param_;
		RenderEffectParameter* shading_rw_tex_param_;
		RenderEffectParameter* shading_rw_tex_array_param_;
		RenderEffectParameter* lights_type_param_;
		RenderEffectParameter* lights_start_in_tex_param_;
		RenderEffectParameter* lights_start_rw_tex_param_;
		RenderEffectParameter* intersected_light_indices_in_tex_param_;
		RenderEffectParameter* intersected_light_indices_rw_tex_param_;
		RenderEffectParameter* depth_slices_param_;
		RenderEffectParameter* depth_slices_shading_param_;
		PostProcessPtr copy_pps_[2];
#endif

		RenderEffectParameter* skylight_diff_spec_mip_param_;
		RenderEffectParameter* inv_view_param_;
		RenderEffectParameter* skylight_y_cube_tex_param_;
		RenderEffectParameter* skylight_c_cube_tex_param_;

		std::vector<SceneNode*> visible_scene_nodes_;
		bool has_sss_objs_;
		bool has_reflective_objs_;
		bool has_simple_forward_objs_;
		bool has_vdm_objs_;
		bool has_ssr_objs_;
		bool has_ppr_objs_;

		PostProcessPtr atmospheric_pp_;

		FrameBufferPtr rsm_fb_;
		std::array<TexturePtr, 2> rsm_texs_;

		bool indirect_lighting_enabled_;
		int32_t cascaded_shadow_index_;

		int illum_;
		float indirect_scale_;
		PostProcessPtr copy_to_light_buffer_pp_;
		PostProcessPtr copy_to_light_buffer_i_pp_;

		std::unique_ptr<CascadedShadowLayer> cascaded_shadow_layer_;
		float2 blur_size_light_space_;
		int32_t curr_cascade_index_;

		bool force_line_mode_;

		PostProcessPtr dr_debug_pp_;
		DisplayType display_type_;

		uint32_t num_objects_rendered_;
		uint32_t num_renderables_rendered_;
		uint32_t num_primitives_rendered_;
		uint32_t num_vertices_rendered_;

#ifndef KLAYGE_SHIP
		PerfRegion* shadow_map_perf_;
		std::array<PerfRegion*, PTB_None> gbuffer_perfs_;
		std::array<PerfRegion*, PTB_None> shadowing_perfs_;
		std::array<PerfRegion*, PTB_None> indirect_lighting_perfs_;
#if DEFAULT_DEFERRED == LIGHT_INDEXED_DEFERRED
		std::array<PerfRegion*, PTB_None> clustering_perfs_;
#endif
		std::array<PerfRegion*, PTB_None> shading_perfs_;
		std::array<PerfRegion*, PTB_None> reflection_perfs_;
		std::array<PerfRegion*, PTB_None> special_shading_perfs_;
		PerfRegion* sss_blur_pp_perf_;
		PerfRegion* ssr_pp_perf_;
		PerfRegion* ppr_pp_perf_;
		PerfRegion* atmospheric_pp_perf_;
		PerfRegion* taa_pp_perf_;
		PerfRegion* vdm_perf_;
		PerfRegion* vdm_composition_pp_perf_;
		PerfRegion* depth_of_field_perf_;
		PerfRegion* bokeh_filter_perf_;
#endif
	};
}

#endif		// KLAYGE_CORE_DEFERRED_RENDERING_LAYER_HPP
