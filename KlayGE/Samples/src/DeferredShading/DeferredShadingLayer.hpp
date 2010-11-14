#ifndef _DEFERREDSHADINGLAYER_HPP
#define _DEFERREDSHADINGLAYER_HPP

#include <KlayGE/PreDeclare.hpp>

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

	enum LightType
	{
		LT_Ambient = 0,
		LT_Point,
		LT_Directional,
		LT_Spot,

		LT_NumLightTypes
	};

	enum LightSrcAttrib
	{
		LSA_NoShadow = 1UL << 0,
		LSA_NoDiffuse = 1UL << 1,
		LSA_NoSpecular = 1UL << 2
	};

	enum PassType
	{
		PT_GBuffer,
		PT_GenShadowMap,
		PT_Lighting,
		PT_Shading
	};

	
	class DeferredLightSource
	{
	public:
		explicit DeferredLightSource(LightType type);
		virtual ~DeferredLightSource();

		LightType Type() const;

		int32_t Attrib() const;
		void Attrib(int32_t attrib);

		bool Enabled() const;
		void Enabled(bool enabled);

		float4 const & Color() const;
		void Color(float3 const & clr);

		virtual float3 const & Position() const;
		virtual float3 const & Direction() const;
		virtual float3 const & Falloff() const;
		virtual float CosInnerAngle() const;
		virtual float CosOuterAngle() const;
		virtual float4 const & CosOuterInner() const;

	protected:
		LightType type_;
		int32_t attrib_;
		bool enabled_;
		float4 color_;
	};
	typedef boost::shared_ptr<DeferredLightSource> DeferredLightSourcePtr;

	class DeferredAmbientLightSource : public DeferredLightSource
	{
	public:
		DeferredAmbientLightSource();
		virtual ~DeferredAmbientLightSource();
	};
	typedef boost::shared_ptr<DeferredAmbientLightSource> DeferredAmbientLightSourcePtr;

	class DeferredPointLightSource : public DeferredLightSource
	{
	public:
		DeferredPointLightSource();
		virtual ~DeferredPointLightSource();

		float3 const & Position() const;
		void Position(float3 const & pos);

		float3 const & Falloff() const;
		void Falloff(float3 const & fall_off);

	protected:
		float3 pos_;
		float3 falloff_;
	};
	typedef boost::shared_ptr<DeferredPointLightSource> DeferredPointLightSourcePtr;

	class DeferredSpotLightSource : public DeferredLightSource
	{
	public:
		DeferredSpotLightSource();
		virtual ~DeferredSpotLightSource();

		float3 const & Position() const;
		void Position(float3 const & pos);

		float3 const & Direction() const;
		void Direction(float3 const & dir);

		float3 const & Falloff() const;
		void Falloff(float3 const & falloff);

		float CosInnerAngle() const;
		void InnerAngle(float angle);

		float CosOuterAngle() const;
		void OuterAngle(float angle);

		float4 const & CosOuterInner() const;

	protected:
		float3 pos_;
		float3 dir_;
		float3 falloff_;
		float4 cos_outer_inner_;
	};
	typedef boost::shared_ptr<DeferredSpotLightSource> DeferredSpotLightSourcePtr;

	class DeferredDirectionalLightSource : public DeferredLightSource
	{
	public:
		DeferredDirectionalLightSource();
		virtual ~DeferredDirectionalLightSource();

		float3 const & Direction() const;
		void Direction(float3 const & dir);

		float3 const & Falloff() const;
		void Falloff(float3 const & falloff);

	protected:
		float3 dir_;
		float3 falloff_;
	};
	typedef boost::shared_ptr<DeferredDirectionalLightSource> DeferredDirectionalLightSourcePtr;


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
		RenderTechniquePtr gen_sm_tech_;
		RenderTechniquePtr gen_sm_alpha_tech_;
		RenderTechniquePtr shading_tech_;

		RenderEffectParameterPtr lighting_tex_param_;
		RenderEffectParameterPtr ssao_tex_param_;
		RenderEffectParameterPtr ssao_enabled_param_;
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


	class DeferredShadingLayer
	{
	public:
		DeferredShadingLayer();

		DeferredAmbientLightSourcePtr AddAmbientLight(float3 const & clr);
		DeferredPointLightSourcePtr AddPointLight(int32_t attr, float3 const & pos, float3 const & clr, float3 const & falloff);
		DeferredDirectionalLightSourcePtr AddDirectionalLight(int32_t attr, float3 const & dir, float3 const & clr, float3 const & falloff);
		DeferredSpotLightSourcePtr AddSpotLight(int32_t attr, float3 const & pos, float3 const & dir, float outer, float inner, float3 const & clr, float3 const & falloff);

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

	private:
		RenderEffectPtr effect_;

		FrameBufferPtr g_buffer_;
		TexturePtr g_buffer_tex_;

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

		std::vector<DeferredLightSourcePtr> lights_;
		std::vector<std::vector<ConditionalRenderPtr> > light_crs_;

		std::vector<uint32_t> pass_scaned_;

		RenderTechniquePtr technique_shadows_[LT_NumLightTypes];
		RenderTechniquePtr technique_lights_[LT_NumLightTypes];
		RenderTechniquePtr technique_light_depth_only_;
		RenderTechniquePtr technique_light_stencil_eiv_;
		RenderTechniquePtr technique_light_stencil_eov_;
		RenderTechniquePtr technique_clear_stencil_;

		FrameBufferPtr sm_buffer_;
		TexturePtr sm_aa_tex_;
		TexturePtr sm_tex_;
		TexturePtr sm_depth_tex_;
		TexturePtr blur_sm_tex_;
		TexturePtr sm_cube_tex_;

		PostProcessPtr sm_filter_pps_[7];
		PostProcessPtr depth_to_vsm_pp_;

		float4x4 view_, proj_;
		float4x4 inv_view_, inv_proj_;

		RenderEffectParameterPtr depth_near_far_invfar_param_;
		RenderEffectParameterPtr inv_view_param_;
		RenderEffectParameterPtr light_attrib_param_;
		RenderEffectParameterPtr light_color_param_;
		RenderEffectParameterPtr light_falloff_param_;
		RenderEffectParameterPtr light_view_proj_param_;
		RenderEffectParameterPtr light_volume_mv_param_;
		RenderEffectParameterPtr light_volume_mvp_param_;
		RenderEffectParameterPtr light_pos_es_param_;
		RenderEffectParameterPtr light_dir_es_param_;

		std::vector<DeferredSceneObjectPtr> deferred_scene_objs_;
	};

	typedef boost::shared_ptr<DeferredShadingLayer> DeferredShadingLayerPtr;
}

#endif		// _DEFERREDSHADINGLAYER_HPP
