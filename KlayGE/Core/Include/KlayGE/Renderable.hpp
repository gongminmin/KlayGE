// Renderable.hpp
// KlayGE ����Ⱦ������ ͷ�ļ�
// Ver 3.1.0
// ��Ȩ����(C) ������, 2003-2005
// Homepage: http://www.klayge.org
//
// 3.1.0
// ���SceneObject (2005.11.2)
//
// 2.7.0
// GetWorld����ΪGetModelMatrix (2005.6.17)
//
// 2.3.0
// ������Render (2005.1.15)
//
// �޸ļ�¼
//////////////////////////////////////////////////////////////////////////////////

#ifndef _RENDERABLE_HPP
#define _RENDERABLE_HPP

#pragma once

#include <KlayGE/PreDeclare.hpp>
#include <vector>
#include <KlayGE/RenderMaterial.hpp>
#include <KlayGE/SceneComponent.hpp>

namespace KlayGE
{
	enum PassCategory
	{
		PC_GBuffer = 0,
		PC_ShadowMap,
		PC_Shadowing,
		PC_IndirectLighting,
		PC_Shading,
		PC_ObjectReflection,
		PC_ScreenSpaceReflection,
		PC_PixelProjectedReflection,
		PC_SpecialShading,
		PC_SpecialShadingMultiView,
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
		PRT_GBuffer = 0,
		PRT_GBufferMultiView,
		PRT_ShadowMap,
		PRT_ShadowMapMultiView,
		PRT_CascadedShadowMap,
		PRT_CascadedShadowMapMultiView,
		PRT_ReflectiveShadowMap,
		PRT_ReflectiveShadowMapMultiView,
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
		PT_OpaqueGBuffer = MakePassType<PRT_GBuffer, PTB_Opaque, PC_GBuffer>::value,
		PT_TransparencyBackGBuffer = MakePassType<PRT_GBuffer, PTB_TransparencyBack, PC_GBuffer>::value,
		PT_TransparencyFrontGBuffer = MakePassType<PRT_GBuffer, PTB_TransparencyFront, PC_GBuffer>::value,

		PT_OpaqueGBufferMultiView = MakePassType<PRT_GBufferMultiView, PTB_Opaque, PC_GBuffer>::value,
		PT_TransparencyBackGBufferMultiView = MakePassType<PRT_GBufferMultiView, PTB_TransparencyBack, PC_GBuffer>::value,
		PT_TransparencyFrontGBufferMultiView = MakePassType<PRT_GBufferMultiView, PTB_TransparencyFront, PC_GBuffer>::value,
		
		PT_GenShadowMap = MakePassType<PRT_ShadowMap, PTB_None, PC_ShadowMap>::value,
		PT_GenCascadedShadowMap = MakePassType<PRT_CascadedShadowMap, PTB_None, PC_ShadowMap>::value,
		PT_GenReflectiveShadowMap = MakePassType<PRT_ReflectiveShadowMap, PTB_None, PC_ShadowMap>::value,

		PT_GenShadowMapMultiView = MakePassType<PRT_ShadowMapMultiView, PTB_None, PC_ShadowMap>::value,
		PT_GenCascadedShadowMapMultiView = MakePassType<PRT_CascadedShadowMapMultiView, PTB_None, PC_ShadowMap>::value,
		PT_GenReflectiveShadowMapMultiView = MakePassType<PRT_ReflectiveShadowMapMultiView, PTB_None, PC_ShadowMap>::value,
		
		PT_Shadowing = MakePassType<PRT_None, PTB_None, PC_Shadowing>::value,

		PT_IndirectLighting = MakePassType<PRT_None, PTB_None, PC_IndirectLighting>::value,
		
		PT_OpaqueShading = MakePassType<PRT_None, PTB_Opaque, PC_Shading>::value,
		PT_TransparencyBackShading = MakePassType<PRT_None, PTB_TransparencyBack, PC_Shading>::value,
		PT_TransparencyFrontShading = MakePassType<PRT_None, PTB_TransparencyFront, PC_Shading>::value,

		PT_OpaqueReflection = MakePassType<PRT_None, PTB_Opaque, PC_ObjectReflection>::value,
		PT_TransparencyBackReflection = MakePassType<PRT_None, PTB_TransparencyBack, PC_ObjectReflection>::value,
		PT_TransparencyFrontReflection = MakePassType<PRT_None, PTB_TransparencyFront, PC_ObjectReflection>::value,
		
		PT_OpaqueSpecialShading = MakePassType<PRT_None, PTB_Opaque, PC_SpecialShading>::value,
		PT_TransparencyBackSpecialShading = MakePassType<PRT_None, PTB_TransparencyBack, PC_SpecialShading>::value,
		PT_TransparencyFrontSpecialShading = MakePassType<PRT_None, PTB_TransparencyFront, PC_SpecialShading>::value,

		PT_OpaqueSpecialShadingMultiView = MakePassType<PRT_None, PTB_Opaque, PC_SpecialShadingMultiView>::value,
		PT_TransparencyBackSpecialShadingMultiView = MakePassType<PRT_None, PTB_TransparencyBack, PC_SpecialShadingMultiView>::value,
		PT_TransparencyFrontSpecialShadingMultiView = MakePassType<PRT_None, PTB_TransparencyFront, PC_SpecialShadingMultiView>::value,

		PT_SimpleForward = MakePassType<PRT_None, PTB_None, PC_SimpleForward>::value,

		PT_VDM = MakePassType<PRT_None, PTB_None, PC_VDM>::value
	};

	constexpr PassRT GetPassRT(PassType pt)
	{
		return static_cast<PassRT>(pt & 0xF);
	}
	constexpr PassTargetBuffer GetPassTargetBuffer(PassType pt)
	{
		return static_cast<PassTargetBuffer>((pt >> 4) & 0x3);
	}
	constexpr PassCategory GetPassCategory(PassType pt)
	{
		return static_cast<PassCategory>((pt >> 6) & 0xF);
	}
	constexpr PassType ComposePassType(PassRT rt, PassTargetBuffer tb, PassCategory cat)
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
			EA_ObjectReflection = 1UL << 4,
			EA_SimpleForward = 1UL << 5,
			EA_SSS = 1UL << 6,
			EA_VDM = 1UL << 7,
			EA_ScreenSpaceReflection = 1UL << 8,
			EA_PixelProjectedReflection = 1UL << 9
		};

	public:
		Renderable();
		explicit Renderable(std::wstring_view name);
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
		virtual RenderLayout& GetRenderLayout() const;
		virtual RenderLayout& GetRenderLayout(uint32_t lod) const;
		virtual std::wstring const & Name() const;

		virtual void OnRenderBegin();
		virtual void OnRenderEnd();

		virtual AABBox const & PosBound() const;
		virtual AABBox const & TexcoordBound() const;

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
		void AddInstance(SceneNode const * node);
		void ClearInstances();
		uint32_t NumInstances() const
		{
			return static_cast<uint32_t>(instances_.size());
		}
		SceneNode const * GetInstance(uint32_t index) const
		{
			return instances_[index];
		}

		virtual void ModelMatrix(float4x4 const & mat);
		virtual void InverseModelMatrix(float4x4 const& mat);
		virtual void PrevModelMatrix(float4x4 const& mat);
		virtual void BindSceneNode(SceneNode const * node);
		SceneNode const * CurrSceneNode() const
		{
			return curr_node_;
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

		virtual void IsSkinned(bool is_skinned)
		{
			is_skinned_ = is_skinned;
		}

		virtual void Material(RenderMaterialPtr const& mtl);
		virtual RenderMaterialPtr const& Material() const
		{
			return mtl_;
		}

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
		virtual bool ObjectReflection() const
		{
			return effect_attrs_ & EA_ObjectReflection ? true : false;
		}
		virtual bool SimpleForward() const
		{
			return effect_attrs_ & EA_SimpleForward ? true : false;
		}
		virtual bool VDM() const
		{
			return effect_attrs_ & EA_VDM ? true : false;
		}
		virtual bool ScreenSpaceReflection() const
		{
			return effect_attrs_ & EA_ScreenSpaceReflection ? true : false;
		}
		virtual bool PixelProjectedReflection() const
		{
			return effect_attrs_ & EA_PixelProjectedReflection ? true : false;
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
		std::wstring name_;

		AABBox pos_aabb_;
		AABBox tc_aabb_;

		std::vector<SceneNode const *> instances_;
		SceneNode const * curr_node_ = nullptr;

		RenderEffectPtr effect_;
		RenderTechnique* technique_ = nullptr;

		std::vector<RenderLayoutPtr> rls_;

		int32_t active_lod_ = 0;

		// For select mode

		RenderTechnique* select_mode_tech_;
		RenderEffectParameter* select_mode_object_id_param_;
		float4 select_mode_object_id_;
		bool select_mode_on_ = false;

		// For deferred only

		RenderTechnique* gbuffer_tech_;
		RenderTechnique* gbuffer_multi_view_tech_;
		RenderTechnique* gbuffer_alpha_blend_back_tech_;
		RenderTechnique* gbuffer_alpha_blend_back_multi_view_tech_;
		RenderTechnique* gbuffer_alpha_blend_front_tech_;
		RenderTechnique* gbuffer_alpha_blend_front_multi_view_tech_;
		RenderTechnique* gen_shadow_map_tech_;
		RenderTechnique* gen_shadow_map_multi_view_tech_;
		RenderTechnique* gen_csm_tech_;
		RenderTechnique* gen_csm_multi_view_tech_;
		RenderTechnique* gen_rsm_tech_;
		RenderTechnique* gen_rsm_multi_view_tech_;
		RenderTechnique* reflection_tech_;
		RenderTechnique* reflection_alpha_blend_back_tech_;
		RenderTechnique* reflection_alpha_blend_front_tech_;
		RenderTechnique* special_shading_tech_;
		RenderTechnique* special_shading_multi_view_tech_;
		RenderTechnique* special_shading_alpha_blend_back_tech_;
		RenderTechnique* special_shading_alpha_blend_back_multi_view_tech_;
		RenderTechnique* special_shading_alpha_blend_front_tech_;
		RenderTechnique* special_shading_alpha_blend_front_multi_view_tech_;
		RenderTechnique* simple_forward_tech_;
		RenderTechnique* vdm_tech_;

		float4x4 model_mat_ = float4x4::Identity();
		float4x4 inv_model_mat_ = float4x4::Identity();
		float4x4 prev_model_mat_ = float4x4::Identity();
		bool model_mat_dirty_ = true;

		PassType type_;
		uint32_t effect_attrs_ = 0;
		bool is_skinned_ = false;

		RenderMaterialPtr mtl_;

		RenderEffectParameter* frame_size_param_;
		RenderEffectParameter* opaque_depth_tex_param_;
		RenderEffectParameter* reflection_tex_param_;
		RenderEffectParameter* half_exposure_x_framerate_param_;
		RenderEffectParameter* motion_blur_radius_param_;

		RenderEffectConstantBufferPtr mesh_cbuffer_;
		RenderEffectConstantBufferPtr model_cbuffer_;
		RenderEffectConstantBufferPtr camera_cbuffer_;
		uint32_t visible_in_cameras_ = 0;
	};

	// TODO: Consider merging this with Renderable
	class KLAYGE_CORE_API RenderableComponent : public SceneComponent
	{
	public:
#if defined(KLAYGE_COMPILER_CLANGCL) || defined(KLAYGE_COMPILER_CLANG)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Winconsistent-missing-override"
#endif
		BOOST_TYPE_INDEX_REGISTER_RUNTIME_CLASS((SceneComponent))
#if defined(KLAYGE_COMPILER_CLANGCL) || defined(KLAYGE_COMPILER_CLANG)
#pragma clang diagnostic pop
#endif

		explicit RenderableComponent(RenderablePtr const& renderable);

		SceneComponentPtr Clone() const override;

		Renderable& BoundRenderable() const;

		template <typename T>
		T& BoundRenderableOfType() const
		{
			return checked_cast<T&>(this->BoundRenderable());
		}

	private:
		RenderablePtr renderable_;
	};
}

#endif		//_RENDERABLE_HPP
