#ifndef _DEFERREDRENDERINGLAYER_HPP
#define _DEFERREDRENDERINGLAYER_HPP

#include <KlayGE/PreDeclare.hpp>
#include <KlayGE/Light.hpp>
#include "MultiresPostProcess.hpp"

namespace KlayGE
{
	template <typename T>
	void CreateConeMesh(std::vector<T>& vb, std::vector<uint16_t>& ib, uint16_t vertex_base, float radius, float height, uint16_t n)
	{
		for (int i = 0; i < n; ++ i)
		{
			vb.push_back(T());
			vb.back().x() = vb.back().y() = vb.back().z() = 0;
		}

		float outer_radius = radius / cos(PI / n);
		for (int i = 0; i < n; ++ i)
		{
			vb.push_back(T());
			float angle = i * 2 * PI / n;
			vb.back().x() = outer_radius * cos(angle);
			vb.back().y() = outer_radius * sin(angle);
			vb.back().z() = height;
		}

		vb.push_back(T());
		vb.back().x() = vb.back().y() = 0;
		vb.back().z() = height;

		for (int i = 0; i < n; ++ i)
		{
			vb.push_back(T());
			vb.back() = vb[vertex_base + n + i];
		}

		for (uint16_t i = 0; i < n - 1; ++ i)
		{
			ib.push_back(vertex_base + i);
			ib.push_back(vertex_base + n + i + 1);
			ib.push_back(vertex_base + n + i);
		}
		ib.push_back(vertex_base + n - 1);
		ib.push_back(vertex_base + n + 0);
		ib.push_back(vertex_base + n + n - 1);

		for (uint16_t i = 0; i < n - 1; ++ i)
		{
			ib.push_back(vertex_base + 2 * n);
			ib.push_back(vertex_base + 2 * n + 1 + i);
			ib.push_back(vertex_base + 2 * n + 1 + i + 1);
		}
		ib.push_back(vertex_base + 2 * n);
		ib.push_back(vertex_base + 2 * n + 1 + n - 1);
		ib.push_back(vertex_base + 2 * n + 1);
	}

	template <typename T>
	void CreatePyramidMesh(std::vector<T>& vb, std::vector<uint16_t>& ib, uint16_t vertex_base, float radius, float height)
	{
		for (int i = 0; i < 4; ++ i)
		{
			vb.push_back(T());
			vb.back().x() = vb.back().y() = vb.back().z() = 0;
		}

		float outer_radius = radius * sqrt(2.0f);
		vb.push_back(T());
		vb.back().x() = -outer_radius;
		vb.back().y() = -outer_radius;
		vb.back().z() = height;
		vb.push_back(T());
		vb.back().x() = +outer_radius;
		vb.back().y() = -outer_radius;
		vb.back().z() = height;
		vb.push_back(T());
		vb.back().x() = +outer_radius;
		vb.back().y() = +outer_radius;
		vb.back().z() = height;
		vb.push_back(T());
		vb.back().x() = -outer_radius;
		vb.back().y() = +outer_radius;
		vb.back().z() = height;

		vb.push_back(T());
		vb.back().x() = vb.back().y() = 0;
		vb.back().z() = height;

		for (int i = 0; i < 4; ++ i)
		{
			vb.push_back(T());
			vb.back() = vb[vertex_base + 4 + i];
		}

		ib.push_back(vertex_base + 0);
		ib.push_back(vertex_base + 5);
		ib.push_back(vertex_base + 4);
		ib.push_back(vertex_base + 0);
		ib.push_back(vertex_base + 6);
		ib.push_back(vertex_base + 5);
		ib.push_back(vertex_base + 0);
		ib.push_back(vertex_base + 7);
		ib.push_back(vertex_base + 6);
		ib.push_back(vertex_base + 0);
		ib.push_back(vertex_base + 4);
		ib.push_back(vertex_base + 7);

		ib.push_back(vertex_base + 8);
		ib.push_back(vertex_base + 9);
		ib.push_back(vertex_base + 10);
		ib.push_back(vertex_base + 8);
		ib.push_back(vertex_base + 10);
		ib.push_back(vertex_base + 11);
		ib.push_back(vertex_base + 8);
		ib.push_back(vertex_base + 11);
		ib.push_back(vertex_base + 12);
		ib.push_back(vertex_base + 8);
		ib.push_back(vertex_base + 12);
		ib.push_back(vertex_base + 9);
	}

	template <typename T>
	void CreateBoxMesh(std::vector<T>& vb, std::vector<uint16_t>& ib, uint16_t vertex_base, float half_length)
	{
		vb.push_back(T());
		vb.back().x() = -half_length;
		vb.back().y() = +half_length;
		vb.back().z() = -half_length;
		vb.push_back(T());
		vb.back().x() = +half_length;
		vb.back().y() = +half_length;
		vb.back().z() = -half_length;
		vb.push_back(T());
		vb.back().x() = +half_length;
		vb.back().y() = -half_length;
		vb.back().z() = -half_length;
		vb.push_back(T());
		vb.back().x() = -half_length;
		vb.back().y() = -half_length;
		vb.back().z() = -half_length;

		vb.push_back(T());
		vb.back().x() = -half_length;
		vb.back().y() = +half_length;
		vb.back().z() = +half_length;
		vb.push_back(T());
		vb.back().x() = +half_length;
		vb.back().y() = +half_length;
		vb.back().z() = +half_length;
		vb.push_back(T());
		vb.back().x() = +half_length;
		vb.back().y() = -half_length;
		vb.back().z() = +half_length;
		vb.push_back(T());
		vb.back().x() = -half_length;
		vb.back().y() = -half_length;
		vb.back().z() = +half_length;

		ib.push_back(vertex_base + 0);
		ib.push_back(vertex_base + 1);
		ib.push_back(vertex_base + 2);
		ib.push_back(vertex_base + 2);
		ib.push_back(vertex_base + 3);
		ib.push_back(vertex_base + 0);

		ib.push_back(vertex_base + 5);
		ib.push_back(vertex_base + 4);
		ib.push_back(vertex_base + 7);
		ib.push_back(vertex_base + 7);
		ib.push_back(vertex_base + 6);
		ib.push_back(vertex_base + 5);

		ib.push_back(vertex_base + 4);
		ib.push_back(vertex_base + 5);
		ib.push_back(vertex_base + 1);
		ib.push_back(vertex_base + 1);
		ib.push_back(vertex_base + 0);
		ib.push_back(vertex_base + 4);

		ib.push_back(vertex_base + 1);
		ib.push_back(vertex_base + 5);
		ib.push_back(vertex_base + 6);
		ib.push_back(vertex_base + 6);
		ib.push_back(vertex_base + 2);
		ib.push_back(vertex_base + 1);

		ib.push_back(vertex_base + 3);
		ib.push_back(vertex_base + 2);
		ib.push_back(vertex_base + 6);
		ib.push_back(vertex_base + 6);
		ib.push_back(vertex_base + 7);
		ib.push_back(vertex_base + 3);

		ib.push_back(vertex_base + 4);
		ib.push_back(vertex_base + 0);
		ib.push_back(vertex_base + 3);
		ib.push_back(vertex_base + 3);
		ib.push_back(vertex_base + 7);
		ib.push_back(vertex_base + 4);
	}

	enum PassType
	{
		PT_GBuffer,
		PT_MRTGBuffer,
		PT_GenShadowMap,
		PT_GenReflectiveShadowMap,
		PT_Lighting,
		PT_IndirectLighting,
		PT_Shading,
		PT_SpecialShading
	};


	class DeferredRenderable
	{
	public:
		explicit DeferredRenderable(RenderEffectPtr const & effect);
		virtual ~DeferredRenderable()
		{
		}

		virtual void Pass(PassType type) = 0;
		virtual void LightingTex(TexturePtr const & tex);
		virtual void SSAOTex(TexturePtr const & tex);
		virtual void SSAOEnabled(bool ssao);

	protected:
		virtual RenderTechniquePtr const & Pass(PassType type, bool alpha) const;

	protected:
		RenderEffectPtr effect_;

		RenderTechniquePtr gbuffer_tech_;
		RenderTechniquePtr gbuffer_alpha_tech_;
		RenderTechniquePtr gbuffer_mrt_tech_;
		RenderTechniquePtr gbuffer_alpha_mrt_tech_;
		RenderTechniquePtr gen_sm_tech_;
		RenderTechniquePtr gen_sm_alpha_tech_;
		RenderTechniquePtr gen_rsm_tech_;
		RenderTechniquePtr gen_rsm_alpha_tech_;
		RenderTechniquePtr shading_tech_;
		RenderTechniquePtr special_shading_tech_;

		RenderEffectParameterPtr lighting_tex_param_;
		RenderEffectParameterPtr ssao_tex_param_;
		RenderEffectParameterPtr ssao_enabled_param_;
		RenderEffectParameterPtr g_buffer_1_tex_param_;
	};

	enum
	{
		SOA_Deferred = 1UL << 4
	};
	class DeferredSceneObject
	{
	public:
		virtual ~DeferredSceneObject()
		{
		}

		void AttachRenderable(DeferredRenderable* dr);

		virtual void Pass(PassType type) = 0;
		virtual void LightingTex(TexturePtr const & tex);
		virtual void SSAOTex(TexturePtr const & tex);
		virtual void SSAOEnabled(bool ssao);

	private:
		DeferredRenderable* dr_;
	};
	typedef boost::shared_ptr<DeferredSceneObject> DeferredSceneObjectPtr;


	class DeferredRenderingLayer
	{
	public:
		DeferredRenderingLayer();

		void SSAOTex(TexturePtr const & tex);
		void SSAOEnabled(bool ssao);

		void OnResize(uint32_t width, uint32_t height);
		uint32_t Update(uint32_t pass);

		TexturePtr const & GBufferTex() const
		{
			return g_buffer_tex_;
		}
		TexturePtr const & LightingTex() const
		{
			return lighting_tex_;
		}
		TexturePtr const & ShadingTex() const
		{
			return shading_tex_;
		}

		void DisplayIllum(int illum);

	private:
		void CreateDepthDerivativeMipMap();
		void CreateNormalConeMipMap();
		void SetSubsplatStencil();
		void UpsampleMultiresLighting();

	private:
		bool mrt_g_buffer_;

		RenderEffectPtr effect_;

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
		RenderViewPtr subsplat_ds_view_;
		bool indirect_lighting_enabled_;	

		PostProcessPtr gbuffer_to_depth_derivate_pp_;
		PostProcessPtr depth_derivate_mipmap_pp_;
		PostProcessPtr gbuffer_to_normal_cone_pp_;
		PostProcessPtr normal_cone_mipmap_pp_;
		int num_mipmap_levels_;

		MultiresPostProcessPtr set_subsplat_stencil_pp_;
		MultiresPostProcessPtr vpls_lighting_pp_;
		PostProcessPtr upsampling_pp_;

		int illum_;
		PostProcessPtr copy_to_light_buffer_pp_;
		PostProcessPtr copy_to_light_buffer_i_pp_;

		FrameBufferPtr g_buffer_;
		TexturePtr g_buffer_tex_;
		TexturePtr g_buffer_1_tex_;

		FrameBufferPtr shadowing_buffer_;
		TexturePtr shadowing_tex_;

		FrameBufferPtr lighting_buffer_;
		TexturePtr lighting_tex_;

		FrameBufferPtr shading_buffer_;
		TexturePtr shading_tex_;

		TexturePtr ssao_tex_;
		bool ssao_enabled_;

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
		RenderEffectParameterPtr ssao_tex_param_;
		RenderEffectParameterPtr ssao_enabled_param_;

		std::vector<DeferredSceneObject*> deferred_scene_objs_;
	};

	typedef boost::shared_ptr<DeferredRenderingLayer> DeferredRenderingLayerPtr;
}

#endif		// _DEFERREDRENDERINGLAYER_HPP
