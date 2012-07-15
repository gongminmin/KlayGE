// Renderable.hpp
// KlayGE 可渲染对象类 头文件
// Ver 3.1.0
// 版权所有(C) 龚敏敏, 2003-2005
// Homepage: http://www.klayge.org
//
// 3.1.0
// 拆出SceneObject (2005.11.2)
//
// 2.7.0
// GetWorld改名为GetModelMatrix (2005.6.17)
//
// 2.3.0
// 增加了Render (2005.1.15)
//
// 修改记录
//////////////////////////////////////////////////////////////////////////////////

#ifndef _RENDERABLE_HPP
#define _RENDERABLE_HPP

#pragma once

#ifndef KLAYGE_CORE_SOURCE
#define KLAYGE_LIB_NAME KlayGE_Core
#include <KlayGE/config/auto_link.hpp>
#endif

#include <KlayGE/PreDeclare.hpp>
#include <vector>

namespace KlayGE
{
	enum PassType
	{
		PT_OpaqueDepth,
		PT_TransparencyBackDepth,
		PT_TransparencyFrontDepth,
		PT_OpaqueGBuffer,
		PT_TransparencyBackGBuffer,
		PT_TransparencyFrontGBuffer,
		PT_OpaqueMRTGBuffer,
		PT_TransparencyBackMRTGBuffer,
		PT_TransparencyFrontMRTGBuffer,
		PT_GenShadowMap,
		PT_GenShadowMapWODepthTexture,
		PT_GenReflectiveShadowMap,
		PT_Lighting,
		PT_IndirectLighting,
		PT_OpaqueShading,
		PT_TransparencyBackShading,
		PT_TransparencyFrontShading,
		PT_OpaqueSpecialShading,
		PT_TransparencyBackSpecialShading,
		PT_TransparencyFrontSpecialShading,
		PT_SimpleForward
	};

	typedef std::vector<std::pair<std::string, std::string> > TextureSlotsType;
	struct KLAYGE_CORE_API RenderMaterial
	{
		float3 ambient;
		float3 diffuse;
		float3 specular;
		float3 emit;
		float opacity;
		float specular_level;
		float shininess;

		TextureSlotsType texture_slots;
	};

	// Abstract class defining the interface all renderable objects must implement.
	class KLAYGE_CORE_API Renderable : public boost::enable_shared_from_this<Renderable>
	{
	public:
		Renderable();
		virtual ~Renderable();

		virtual RenderTechniquePtr const & GetRenderTechnique() const
		{
			return technique_;
		}
		virtual RenderLayoutPtr const & GetRenderLayout() const = 0;
		virtual std::wstring const & Name() const = 0;

		virtual void OnRenderBegin();
		virtual void OnRenderEnd();

		// These two functions are used for non-instancing rendering
		virtual void OnInstanceBegin(uint32_t id);
		virtual void OnInstanceEnd(uint32_t id);

		virtual AABBox const & Bound() const = 0;

		virtual void AddToRenderQueue();

		virtual void Render();

		template <typename Iterator>
		void AssignInstances(Iterator begin, Iterator end)
		{
			instances_.resize(0);
			for (Iterator iter = begin; iter != end; ++ iter)
			{
				this->AddInstance(*iter);
			}
		}
		void AddInstance(SceneObjectPtr const & obj);

		uint32_t NumInstances() const
		{
			return static_cast<uint32_t>(instances_.size());
		}
		SceneObjectPtr GetInstance(uint32_t index) const
		{
			return instances_[index].lock();
		}

		// For deferred only

		virtual void ModelMatrix(float4x4 const & mat);

		virtual void Pass(PassType type);
		virtual void LightingTex(TexturePtr const & tex);

		virtual bool SpecialShading() const
		{
			return special_shading_;
		}
		virtual bool TransparencyBackFace() const
		{
			return need_transparency_back_;
		}
		virtual bool TransparencyFrontFace() const
		{
			return need_transparency_front_;
		}
		virtual bool Reflection() const
		{
			return need_reflection_;
		}
		virtual bool SimpleForward() const
		{
			return need_simple_forward_;
		}

	protected:
		virtual void UpdateInstanceStream();

		// For deferred only
		virtual void BindDeferredEffect(RenderEffectPtr const & deferred_effect);
		virtual RenderTechniquePtr const & PassTech(PassType type) const;

	protected:
		std::vector<boost::weak_ptr<SceneObject> > instances_;

		RenderTechniquePtr technique_;

		// For deferred only

		RenderEffectPtr deferred_effect_;

		RenderTechniquePtr depth_tech_;
		RenderTechniquePtr depth_alpha_test_tech_;
		RenderTechniquePtr depth_alpha_blend_back_tech_;
		RenderTechniquePtr depth_alpha_blend_front_tech_;
		RenderTechniquePtr gbuffer_tech_;
		RenderTechniquePtr gbuffer_alpha_test_tech_;
		RenderTechniquePtr gbuffer_alpha_blend_back_tech_;
		RenderTechniquePtr gbuffer_alpha_blend_front_tech_;
		RenderTechniquePtr gbuffer_mrt_tech_;
		RenderTechniquePtr gbuffer_alpha_test_mrt_tech_;
		RenderTechniquePtr gbuffer_alpha_blend_back_mrt_tech_;
		RenderTechniquePtr gbuffer_alpha_blend_front_mrt_tech_;
		RenderTechniquePtr gen_sm_tech_;
		RenderTechniquePtr gen_sm_alpha_test_tech_;
		RenderTechniquePtr gen_sm_wo_dt_tech_;
		RenderTechniquePtr gen_sm_wo_dt_alpha_test_tech_;
		RenderTechniquePtr gen_rsm_tech_;
		RenderTechniquePtr gen_rsm_alpha_test_tech_;
		RenderTechniquePtr shading_tech_;
		RenderTechniquePtr shading_alpha_blend_back_tech_;
		RenderTechniquePtr shading_alpha_blend_front_tech_;
		RenderTechniquePtr special_shading_tech_;
		RenderTechniquePtr special_shading_alpha_blend_back_tech_;
		RenderTechniquePtr special_shading_alpha_blend_front_tech_;

		RenderEffectParameterPtr lighting_tex_param_;
		RenderEffectParameterPtr g_buffer_1_tex_param_;

		float4x4 model_mat_;

		PassType type_;
		bool opacity_map_enabled_;
		bool special_shading_;
		bool need_transparency_back_;
		bool need_transparency_front_;
		bool need_alpha_test_;
		bool need_reflection_;
		bool need_simple_forward_;

		RenderMaterialPtr mtl_;

		RenderEffectParameterPtr mvp_param_;
		RenderEffectParameterPtr model_view_param_;
		RenderEffectParameterPtr shininess_param_;
		RenderEffectParameterPtr specular_tex_param_;
		RenderEffectParameterPtr normal_map_enabled_param_;
		RenderEffectParameterPtr normal_tex_param_;
		RenderEffectParameterPtr height_map_enabled_param_;
		RenderEffectParameterPtr height_tex_param_;
		RenderEffectParameterPtr diffuse_tex_param_;
		RenderEffectParameterPtr diffuse_clr_param_;
		RenderEffectParameterPtr emit_tex_param_;
		RenderEffectParameterPtr emit_clr_param_;
		RenderEffectParameterPtr specular_level_param_;
		RenderEffectParameterPtr opacity_clr_param_;
		RenderEffectParameterPtr opacity_map_enabled_param_;
		RenderEffectParameterPtr opaque_depth_tex_param_;

		TexturePtr diffuse_tex_;
		TexturePtr specular_tex_;
		TexturePtr normal_tex_;
		TexturePtr height_tex_;
		TexturePtr emit_tex_;
	};
}

#endif		//_RENDERABLE_HPP
