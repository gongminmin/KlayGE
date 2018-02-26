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

#include <KlayGE/PreDeclare.hpp>
#include <vector>
#include <KlayGE/RenderMaterial.hpp>

namespace KlayGE
{
	enum PassCategory
	{
		PC_GBuffer = 0,
		PC_ShadowMap,
		PC_Shadowing,
		PC_IndirectLighting,
		PC_Shading,
		PC_Reflection,
		PC_SpecialShading,
		PC_SimpleForward,
		PC_VDM,
		PC_None
	};

	enum PassTargetBuffer
	{
		PTB_Opaque = 0,
		PTB_TransparencyBack,
		PTB_TransparencyFront,
		PTB_None
	};

	enum PassRT
	{
		PRT_MRT = 0,
		PRT_ShadowMap,
		PRT_CascadedShadowMap,
		PRT_ReflectiveShadowMap,
		PRT_None
	};

	// pass type is a 32-bit value:
	// 0000000000000000000000 C3 C2 C1 C0 TB1 TB0 RT3 RT2 RT1 RT0

	template <PassRT rt, PassTargetBuffer tb, PassCategory cat>
	struct MakePassType
	{
		static uint32_t const value = (rt << 0) | (tb << 4) | (cat << 6);
	};

	enum PassType
	{
		PT_OpaqueGBufferMRT = MakePassType<PRT_MRT, PTB_Opaque, PC_GBuffer>::value,
		PT_TransparencyBackGBufferMRT = MakePassType<PRT_MRT, PTB_TransparencyBack, PC_GBuffer>::value,
		PT_TransparencyFrontGBufferMRT = MakePassType<PRT_MRT, PTB_TransparencyFront, PC_GBuffer>::value,
		
		PT_GenShadowMap = MakePassType<PRT_ShadowMap, PTB_None, PC_ShadowMap>::value,
		PT_GenCascadedShadowMap = MakePassType<PRT_CascadedShadowMap, PTB_None, PC_ShadowMap>::value,
		PT_GenReflectiveShadowMap = MakePassType<PRT_ReflectiveShadowMap, PTB_None, PC_ShadowMap>::value,
		
		PT_Shadowing = MakePassType<PRT_None, PTB_None, PC_Shadowing>::value,

		PT_IndirectLighting = MakePassType<PRT_None, PTB_None, PC_IndirectLighting>::value,
		
		PT_OpaqueShading = MakePassType<PRT_None, PTB_Opaque, PC_Shading>::value,
		PT_TransparencyBackShading = MakePassType<PRT_None, PTB_TransparencyBack, PC_Shading>::value,
		PT_TransparencyFrontShading = MakePassType<PRT_None, PTB_TransparencyFront, PC_Shading>::value,

		PT_OpaqueReflection = MakePassType<PRT_None, PTB_Opaque, PC_Reflection>::value,
		PT_TransparencyBackReflection = MakePassType<PRT_None, PTB_TransparencyBack, PC_Reflection>::value,
		PT_TransparencyFrontReflection = MakePassType<PRT_None, PTB_TransparencyFront, PC_Reflection>::value,
		
		PT_OpaqueSpecialShading = MakePassType<PRT_None, PTB_Opaque, PC_SpecialShading>::value,
		PT_TransparencyBackSpecialShading = MakePassType<PRT_None, PTB_TransparencyBack, PC_SpecialShading>::value,
		PT_TransparencyFrontSpecialShading = MakePassType<PRT_None, PTB_TransparencyFront, PC_SpecialShading>::value,
		
		PT_SimpleForward = MakePassType<PRT_None, PTB_None, PC_SimpleForward>::value,

		PT_VDM = MakePassType<PRT_None, PTB_None, PC_VDM>::value
	};

	inline PassRT GetPassRT(PassType pt)
	{
		return static_cast<PassRT>(pt & 0xF);
	}
	inline PassTargetBuffer GetPassTargetBuffer(PassType pt)
	{
		return static_cast<PassTargetBuffer>((pt >> 4) & 0x3);
	}
	inline PassCategory GetPassCategory(PassType pt)
	{
		return static_cast<PassCategory>((pt >> 6) & 0xF);
	}
	inline PassType ComposePassType(PassRT rt, PassTargetBuffer tb, PassCategory cat)
	{
		return static_cast<PassType>((rt << 0) | (tb << 4) | (cat << 6));
	}

	// Abstract class defining the interface all renderable objects must implement.
	class KLAYGE_CORE_API Renderable : boost::noncopyable
	{
	public:
		enum EffectAttribute
		{
			EA_SpecialShading = 1UL << 0,
			EA_TransparencyBack = 1UL << 1,
			EA_TransparencyFront = 1UL << 2,
			EA_AlphaTest = 1UL << 3,
			EA_Reflection = 1UL << 4,
			EA_SimpleForward = 1UL << 5,
			EA_SSS = 1UL << 6,
			EA_VDM = 1UL << 7
		};

	public:
		Renderable();
		virtual ~Renderable();

		virtual RenderEffectPtr const & GetRenderEffect() const
		{
			return effect_;
		}
		virtual RenderTechnique* GetRenderTechnique() const
		{
			return technique_;
		}

		virtual void NumLods(uint32_t lods);
		virtual uint32_t NumLods() const;
		virtual void ActiveLod(int32_t lod);
		virtual int32_t ActiveLod() const
		{
			return active_lod_;
		}
		virtual RenderLayout& GetRenderLayout() const = 0;
		virtual RenderLayout& GetRenderLayout(uint32_t lod) const;
		virtual std::wstring const & Name() const = 0;

		virtual void OnRenderBegin();
		virtual void OnRenderEnd();

		// These two functions are used for non-instancing rendering
		virtual void OnInstanceBegin(uint32_t id);
		virtual void OnInstanceEnd(uint32_t id);

		virtual AABBox const & PosBound() const = 0;
		virtual AABBox const & TexcoordBound() const = 0;

		virtual void AddToRenderQueue();

		virtual void Render();

		template <typename Iterator>
		void AssignInstances(Iterator begin, Iterator end)
		{
			this->ClearInstances();
			for (Iterator iter = begin; iter != end; ++ iter)
			{
				this->AddInstance(*iter);
			}
		}
		void AddInstance(SceneObject const * obj);
		void ClearInstances();
		uint32_t NumInstances() const
		{
			return static_cast<uint32_t>(instances_.size());
		}
		SceneObject const * GetInstance(uint32_t index) const
		{
			return instances_[index];
		}

		virtual void ModelMatrix(float4x4 const & mat);

		template <typename ForwardIterator>
		void AssignSubrenderables(ForwardIterator first, ForwardIterator last)
		{
			subrenderables_.assign(first, last);

			this->UpdateBoundBox();
		}
		RenderablePtr const & Subrenderable(size_t id) const
		{
			return subrenderables_[id];
		}
		uint32_t NumSubrenderables() const
		{
			return static_cast<uint32_t>(subrenderables_.size());
		}

		virtual bool HWResourceReady() const
		{
			return true;
		}
		bool AllHWResourceReady() const;

		// For select mode

		virtual void ObjectID(uint32_t id);
		virtual void SelectMode(bool select_mode);
		bool SelectMode() const
		{
			return select_mode_on_;
		}

		// For deferred only

		virtual void Pass(PassType type);

		virtual bool SpecialShading() const
		{
			return effect_attrs_ & EA_SpecialShading ? true : false;
		}
		virtual bool TransparencyBackFace() const
		{
			return effect_attrs_ & EA_TransparencyBack ? true : false;
		}
		virtual bool TransparencyFrontFace() const
		{
			return effect_attrs_ & EA_TransparencyFront ? true : false;
		}		
		virtual bool AlphaTest() const
		{
			return effect_attrs_ & EA_AlphaTest ? true : false;
		}
		virtual bool SSS() const
		{
			return effect_attrs_ & EA_SSS ? true : false;
		}
		virtual bool Reflection() const
		{
			return effect_attrs_ & EA_Reflection ? true : false;
		}
		virtual bool SimpleForward() const
		{
			return effect_attrs_ & EA_SimpleForward ? true : false;
		}
		virtual bool VDM() const
		{
			return effect_attrs_ & EA_VDM ? true : false;
		}

	protected:
		virtual void UpdateInstanceStream();
		virtual void UpdateBoundBox();

		float CalcLod(float3 const & eye_pos, float fov_scale) const;

		// For deferred only
		void BindDeferredEffect(RenderEffectPtr const & deferred_effect);
		virtual RenderTechnique* PassTech(PassType type) const;
		virtual void UpdateTechniques();

	protected:
		std::vector<SceneObject const *> instances_;

		RenderEffectPtr effect_;
		RenderTechnique* technique_;

		int32_t active_lod_;

		// For select mode

		RenderTechnique* select_mode_tech_;
		RenderEffectParameter* select_mode_object_id_param_;
		float4 select_mode_object_id_;
		bool select_mode_on_;

		// For deferred only

		RenderEffectPtr deferred_effect_;

		RenderTechnique* gbuffer_mrt_tech_;
		RenderTechnique* gbuffer_alpha_blend_back_mrt_tech_;
		RenderTechnique* gbuffer_alpha_blend_front_mrt_tech_;
		RenderTechnique* gen_sm_tech_;
		RenderTechnique* gen_cascaded_sm_tech_;
		RenderTechnique* gen_rsm_tech_;
		RenderTechnique* reflection_tech_;
		RenderTechnique* reflection_alpha_blend_back_tech_;
		RenderTechnique* reflection_alpha_blend_front_tech_;
		RenderTechnique* special_shading_tech_;
		RenderTechnique* special_shading_alpha_blend_back_tech_;
		RenderTechnique* special_shading_alpha_blend_front_tech_;
		RenderTechnique* simple_forward_tech_;
		RenderTechnique* vdm_tech_;

		float4x4 model_mat_;

		PassType type_;
		uint32_t effect_attrs_;

		RenderMaterialPtr mtl_;

		RenderEffectParameter* mvp_param_;
		RenderEffectParameter* model_view_param_;
		RenderEffectParameter* forward_vec_param_;
		RenderEffectParameter* frame_size_param_;
		RenderEffectParameter* height_offset_scale_param_;
		RenderEffectParameter* tess_factors_param_;
		RenderEffectParameter* pos_center_param_;
		RenderEffectParameter* pos_extent_param_;
		RenderEffectParameter* tc_center_param_;
		RenderEffectParameter* tc_extent_param_;
		RenderEffectParameter* albedo_map_enabled_param_;
		RenderEffectParameter* albedo_tex_param_;
		RenderEffectParameter* albedo_clr_param_;
		RenderEffectParameter* metalness_tex_param_;
		RenderEffectParameter* metalness_clr_param_;
		RenderEffectParameter* glossiness_tex_param_;
		RenderEffectParameter* glossiness_clr_param_;
		RenderEffectParameter* emissive_tex_param_;
		RenderEffectParameter* emissive_clr_param_;
		RenderEffectParameter* normal_map_enabled_param_;
		RenderEffectParameter* normal_tex_param_;
		RenderEffectParameter* height_map_parallax_enabled_param_;
		RenderEffectParameter* height_map_tess_enabled_param_;
		RenderEffectParameter* height_tex_param_;
		RenderEffectParameter* opaque_depth_tex_param_;
		RenderEffectParameter* reflection_tex_param_;
		RenderEffectParameter* alpha_test_threshold_param_;

		std::array<TexturePtr, RenderMaterial::TS_NumTextureSlots> textures_;

		std::vector<RenderablePtr> subrenderables_;
	};
}

#endif		//_RENDERABLE_HPP
