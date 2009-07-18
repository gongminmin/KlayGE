#ifndef _DEFERREDSHADINGLAYER_HPP
#define _DEFERREDSHADINGLAYER_HPP

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
			vb.back().z() = outer_radius * sin(angle);
			vb.back().y() = -height;
		}

		vb.push_back(T());
		vb.back().x() = vb.back().z() = 0;
		vb.back().y() = -height;

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
		vb.back().z() = -outer_radius;
		vb.back().y() = -height;
		vb.push_back(T());
		vb.back().x() = +outer_radius;
		vb.back().z() = -outer_radius;
		vb.back().y() = -height;
		vb.push_back(T());
		vb.back().x() = +outer_radius;
		vb.back().z() = +outer_radius;
		vb.back().y() = -height;
		vb.push_back(T());
		vb.back().x() = -outer_radius;
		vb.back().z() = +outer_radius;
		vb.back().y() = -height;

		vb.push_back(T());
		vb.back().x() = vb.back().z() = 0;
		vb.back().y() = -height;

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

	enum LightType
	{
		LT_Ambient = 0,
		LT_Point,
		LT_Directional,
		LT_Spot
	};

	enum LightSrcAttrib
	{
		LSA_NoShadow = 1UL << 0,
		LSA_NoDiffuse = 1UL << 1,
		LSA_NoSpecular = 1UL << 2
	};

	class DeferredShadingLayer : public RenderableHelper
	{
	public:
		DeferredShadingLayer();

		int AddAmbientLight(int32_t attr, float3 const & clr);
		int AddPointLight(int32_t attr, float3 const & pos, float3 const & clr, float3 const & falloff);
		int AddDirectionalLight(int32_t attr, float3 const & dir, float3 const & clr, float3 const & falloff);
		int AddSpotLight(int32_t attr, float3 const & pos, float3 const & dir, float outer, float inner, float3 const & clr, float3 const & falloff);

		void LightAttrib(int index, uint32_t attr);
		void LightColor(int index, float3 const & clr);
		void LightDir(int index, float3 const & dir);
		void LightPos(int index, float3 const & pos);
		void LightFalloff(int index, float3 const & falloff);
		void SpotLightAngle(int index, float outer, float inner);

		float3 LightColor(int index) const;
		float3 LightDir(int index) const;
		float3 LightPos(int index) const;
		float3 LightFalloff(int index) const;
		float2 SpotLightAngle(int index) const;

		void LightEnabled(int index, bool enable);
		bool LightEnabled(int index) const;

		void Destinate(FrameBufferPtr const & fb);

		void GBufferTexs(TexturePtr const & nd_tex, TexturePtr const & clr_tex, bool flipping);
		void SSAOTex(TexturePtr const & tex);
		void SSAOEnabled(bool ssao);
		void BufferType(int buffer_type);

		uint32_t Update(uint32_t pass);

	private:
		void ScanLightSrc();

	private:
		FrameBufferPtr frame_buffer_;

		RenderLayoutPtr rl_cone_;
		RenderLayoutPtr rl_pyramid_;
		RenderLayoutPtr rl_quad_;

		std::vector<char> light_enabled_;
		std::vector<int32_t> light_attrib_;
		std::vector<float4> light_clr_type_;
		std::vector<float4> light_pos_;
		std::vector<float4> light_dir_;
		std::vector<float4> light_cos_outer_inner_;
		std::vector<float4> light_falloff_;

		std::vector<uint32_t> light_scaned_;

		RenderTechniquePtr technique_ambient_;
		RenderTechniquePtr technique_directional_;
		RenderTechniquePtr technique_point_;
		RenderTechniquePtr technique_spot_;

		int32_t buffer_type_;

		FrameBufferPtr sm_buffer_;
		TexturePtr sm_tex_;

		float4x4 view_, proj_;
		float4x4 inv_view_;

		RenderEffectParameterPtr texel_to_pixel_offset_param_;
		RenderEffectParameterPtr depth_near_far_invfar_param_;
		RenderEffectParameterPtr upper_left_param_;
		RenderEffectParameterPtr upper_right_param_;
		RenderEffectParameterPtr lower_left_param_;
		RenderEffectParameterPtr lower_right_param_;
		RenderEffectParameterPtr inv_view_param_;
		RenderEffectParameterPtr show_skybox_param_;
		RenderEffectParameterPtr light_attrib_param_;
		RenderEffectParameterPtr light_clr_type_param_;
		RenderEffectParameterPtr light_falloff_param_;
		RenderEffectParameterPtr light_view_proj_param_;
		RenderEffectParameterPtr light_volume_mvp_param_;
		RenderEffectParameterPtr light_pos_es_param_;
		RenderEffectParameterPtr light_dir_es_param_;
	};

	typedef boost::shared_ptr<DeferredShadingLayer> DeferredShadingLayerPtr;
}

#endif		// _DEFERREDSHADINGLAYER_HPP
