// Renderable.cpp
// KlayGE 可渲染对象类 实现文件
// Ver 3.4.0
// 版权所有(C) 龚敏敏, 2003-2006
// Homepage: http://www.klayge.org
//
// 3.4.0
// 修正了Render没有设置Technique的bug (2006.7.26)
//
// 2.7.0
// GetWorld改名为GetModelMatrix (2005.6.17)
//
// 2.3.0
// 增加了Render (2005.1.15)
//
// 修改记录
//////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KFL/Math.hpp>
#include <KlayGE/SceneManager.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/SceneObject.hpp>
#include <KlayGE/RenderEffect.hpp>
#include <KlayGE/FrameBuffer.hpp>
#include <KlayGE/Camera.hpp>
#include <KlayGE/DeferredRenderingLayer.hpp>

#include <KlayGE/Renderable.hpp>

namespace KlayGE
{
	Renderable::Renderable()
		: select_mode_on_(false),
			model_mat_(float4x4::Identity()), effect_attrs_(0)
	{
		auto drl = Context::Instance().DeferredRenderingLayerInstance();
		if (drl)
		{
			this->BindDeferredEffect(drl->GBufferEffect());
			opacity_map_enabled_ = false;
		}
	}

	Renderable::~Renderable()
	{
	}

	void Renderable::OnRenderBegin()
	{
		RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
		Camera const & camera = *re.CurFrameBuffer()->GetViewport()->camera;
		float4x4 const & view = camera.ViewMatrix();
		float4x4 const & proj = camera.ProjMatrix();
		float4x4 mv = model_mat_ * view;
		float4x4 mvp = mv * proj;
		AABBox const & pos_bb = this->PosBound();
		AABBox const & tc_bb = this->TexcoordBound();

		auto drl = Context::Instance().DeferredRenderingLayerInstance();

		if (drl)
		{
			int32_t cas_index = drl->CurrCascadeIndex();
			if (cas_index >= 0)
			{
				mvp *= drl->GetCascadedShadowLayer()->CascadeCropMatrix(cas_index);
			}
		}

		if (select_mode_on_)
		{
			*mvp_param_ = mvp;
			*pos_center_param_ = pos_bb.Center();
			*pos_extent_param_ = pos_bb.HalfSize();
			*select_mode_object_id_param_ = select_mode_object_id_;
		}
		else if (deferred_effect_)
		{
			*mvp_param_ = mvp;
			*model_view_param_ = mv;
			*far_plane_param_ = float2(camera.FarPlane(), 1.0f / camera.FarPlane());

			*pos_center_param_ = pos_bb.Center();
			*pos_extent_param_ = pos_bb.HalfSize();
			*tc_center_param_ = float2(tc_bb.Center().x(), tc_bb.Center().y());
			*tc_extent_param_ = float2(tc_bb.HalfSize().x(), tc_bb.HalfSize().y());

			switch (type_)
			{
			case PT_OpaqueDepth:
			case PT_TransparencyBackDepth:
			case PT_TransparencyFrontDepth:
				*diffuse_tex_param_ = diffuse_tex_;
				*diffuse_clr_param_ = float4(mtl_ ? mtl_->diffuse.x() : 0, mtl_ ? mtl_->diffuse.y() : 0, mtl_ ? mtl_->diffuse.z() : 0, static_cast<float>(!!diffuse_tex_));
				*opaque_depth_tex_param_ = drl->CurrFrameDepthTex(drl->ActiveViewport());
				break;

			case PT_OpaqueGBufferRT0:
			case PT_TransparencyBackGBufferRT0:
			case PT_TransparencyFrontGBufferRT0:
			case PT_OpaqueGBufferRT1:
			case PT_TransparencyBackGBufferRT1:
			case PT_TransparencyFrontGBufferRT1:
			case PT_OpaqueGBufferMRT:
			case PT_TransparencyBackGBufferMRT:
			case PT_TransparencyFrontGBufferMRT:
			case PT_GenReflectiveShadowMap:
				*diffuse_tex_param_ = diffuse_tex_;
				*diffuse_clr_param_ = float4(mtl_ ? mtl_->diffuse.x() : 0, mtl_ ? mtl_->diffuse.y() : 0, mtl_ ? mtl_->diffuse.z() : 0, static_cast<float>(!!diffuse_tex_));
				*normal_map_enabled_param_ = static_cast<int32_t>(!!normal_tex_);
				*normal_tex_param_ = normal_tex_;
				*height_map_enabled_param_ = static_cast<int32_t>(!!height_tex_);
				*height_tex_param_ = height_tex_;
				*specular_tex_param_ = specular_tex_;
				*specular_clr_param_ = float4(mtl_ ? mtl_->specular.x() : 0, mtl_ ? mtl_->specular.y() : 0, mtl_ ? mtl_->specular.z() : 0, static_cast<float>(!!specular_tex_));
				*shininess_clr_param_ = float2(MathLib::clamp(static_cast<float>(mtl_ ? log(mtl_->shininess) * INV_LOG_8192 : 0), 1e-6f, 0.999f), static_cast<float>(!!shininess_tex_));
				*shininess_tex_param_ = shininess_tex_;
				*opacity_clr_param_ = mtl_ ? mtl_->opacity : 1.0f;
				*opaque_depth_tex_param_ = drl->CurrFrameDepthTex(drl->ActiveViewport());
				break;

			case PT_GenShadowMap:
			case PT_GenCascadedShadowMap:
			case PT_GenShadowMapWODepthTexture:
				*diffuse_tex_param_ = diffuse_tex_;
				break;

			case PT_OpaqueShading:
			case PT_TransparencyBackShading:
			case PT_TransparencyFrontShading:
				*shininess_clr_param_ = float2(MathLib::clamp(static_cast<float>(mtl_ ? log(mtl_->shininess) * INV_LOG_8192 : 0), 1e-6f, 0.999f), static_cast<float>(!!shininess_tex_));
				*shininess_tex_param_ = shininess_tex_;
				*diffuse_tex_param_ = diffuse_tex_;
				*diffuse_clr_param_ = float4(mtl_ ? mtl_->diffuse.x() : 0, mtl_ ? mtl_->diffuse.y() : 0, mtl_ ? mtl_->diffuse.z() : 0, static_cast<float>(!!diffuse_tex_));
				*specular_tex_param_ = specular_tex_;
				*specular_clr_param_ = float4(mtl_ ? mtl_->specular.x() : 0, mtl_ ? mtl_->specular.y() : 0, mtl_ ? mtl_->specular.z() : 0, static_cast<float>(!!specular_tex_));
				*emit_tex_param_ = emit_tex_;
				*emit_clr_param_ = float4(mtl_ ? mtl_->emit.x() : 0, mtl_ ? mtl_->emit.y() : 0, mtl_ ? mtl_->emit.z() : 0, static_cast<float>(!!emit_tex_));
				*opacity_clr_param_ = mtl_ ? mtl_->opacity : 1.0f;
				*opacity_map_enabled_param_ = static_cast<int32_t>(opacity_map_enabled_);
				break;

			case PT_OpaqueReflection:
			case PT_TransparencyBackReflection:
			case PT_TransparencyFrontReflection:
				*specular_tex_param_ = specular_tex_;
				*specular_clr_param_ = float4(mtl_ ? mtl_->specular.x() : 0, mtl_ ? mtl_->specular.y() : 0, mtl_ ? mtl_->specular.z() : 0, static_cast<float>(!!specular_tex_));
				break;

			case PT_OpaqueSpecialShading:
			case PT_TransparencyBackSpecialShading:
			case PT_TransparencyFrontSpecialShading:
				*diffuse_tex_param_ = diffuse_tex_;
				*emit_tex_param_ = emit_tex_;
				*emit_clr_param_ = float4(mtl_ ? mtl_->emit.x() : 0, mtl_ ? mtl_->emit.y() : 0, mtl_ ? mtl_->emit.z() : 0, static_cast<float>(!!emit_tex_));
				*opacity_clr_param_ = mtl_ ? mtl_->opacity : 1;
				*opacity_map_enabled_param_ = static_cast<int32_t>(opacity_map_enabled_);
				if (reflection_tex_param_)
				{
					*reflection_tex_param_ = drl->ReflectionTex(drl->ActiveViewport());
				}
				break;

			default:
				break;
			}
		}
	}

	void Renderable::OnRenderEnd()
	{
	}

	void Renderable::OnInstanceBegin(uint32_t /*id*/)
	{
	}

	void Renderable::OnInstanceEnd(uint32_t /*id*/)
	{
	}

	void Renderable::AddToRenderQueue()
	{
		Context::Instance().SceneManagerInstance().AddRenderable(this);
	}

	void Renderable::Render()
	{
		this->UpdateInstanceStream();

		RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();

		RenderLayout const & layout = this->GetRenderLayout();
		GraphicsBufferPtr const & inst_stream = layout.InstanceStream();
		RenderTechnique const & tech = *this->GetRenderTechnique();
		if (inst_stream)
		{
			if (layout.NumInstances() > 0)
			{
				this->OnRenderBegin();
				re.Render(tech, layout);
				this->OnRenderEnd();
			}
		}
		else
		{
			this->OnRenderBegin();
			if (instances_.empty())
			{
				re.Render(tech, layout);
			}
			else
			{
				for (uint32_t i = 0; i < instances_.size(); ++ i)
				{
					this->OnInstanceBegin(i);
					re.Render(tech, layout);
					this->OnInstanceEnd(i);
				}
			}
			this->OnRenderEnd();
		}
	}

	void Renderable::AddInstance(SceneObject const * obj)
	{
		instances_.push_back(obj);
	}

	void Renderable::UpdateInstanceStream()
	{
		if (!instances_.empty() && !instances_[0]->InstanceFormat().empty())
		{
			vertex_elements_type const & vet = instances_[0]->InstanceFormat();
			uint32_t size = 0;
			for (size_t i = 0; i < vet.size(); ++ i)
			{
				size += vet[i].element_size();
			}

			uint32_t const inst_size = static_cast<uint32_t>(size * instances_.size());

			RenderLayout& rl = this->GetRenderLayout();

			GraphicsBufferPtr inst_stream = rl.InstanceStream();
			if (inst_stream && (inst_stream->Size() >= inst_size))
			{
				for (size_t i = 0; i < instances_.size(); ++ i)
				{
					BOOST_ASSERT(rl.InstanceStreamFormat() == instances_[i]->InstanceFormat());
				}
			}
			else
			{
				RenderFactory& rf(Context::Instance().RenderFactoryInstance());
				inst_stream = rf.MakeVertexBuffer(BU_Dynamic, EAH_CPU_Write | EAH_GPU_Read, inst_size, nullptr);
				rl.BindVertexStream(inst_stream, vet, RenderLayout::ST_Instance, 1);
				rl.InstanceStream(inst_stream);
			}

			{
				GraphicsBuffer::Mapper mapper(*inst_stream, BA_Write_Only);
				for (size_t i = 0; i < instances_.size(); ++ i)
				{
					uint8_t const * src = static_cast<uint8_t const *>(instances_[i]->InstanceData());
					std::copy(src, src + size, mapper.Pointer<uint8_t>() + i * size);
				}
			}

			for (uint32_t i = 0; i < rl.NumVertexStreams(); ++ i)
			{
				rl.VertexStreamFrequencyDivider(i, RenderLayout::ST_Geometry, static_cast<uint32_t>(instances_.size()));
			}
		}
	}

	void Renderable::ModelMatrix(float4x4 const & mat)
	{
		model_mat_ = mat;
	}

	void Renderable::UpdateBoundBox()
	{
	}

	void Renderable::ObjectID(uint32_t id)
	{
		select_mode_object_id_ = float4(((id & 0xFF) + 0.5f) / 255.0f,
			(((id >> 8) & 0xFF) + 0.5f) / 255.0f, (((id >> 16) & 0xFF) + 0.5f) / 255.0f, 0.0f);
	}

	void Renderable::SelectMode(bool select_mode)
	{
		select_mode_on_ = select_mode;
		if (select_mode_on_)
		{
			technique_ = select_mode_tech_;
		}
	}

	void Renderable::Pass(PassType type)
	{
		type_ = type;
		technique_ = this->PassTech(type);
	}

	void Renderable::BindDeferredEffect(RenderEffectPtr const & deferred_effect)
	{
		deferred_effect_ = deferred_effect;

		this->UpdateTechniques();

		mvp_param_ = deferred_effect_->ParameterByName("mvp");
		model_view_param_ = deferred_effect_->ParameterByName("model_view");
		far_plane_param_ = deferred_effect_->ParameterByName("far_plane");
		pos_center_param_ = deferred_effect_->ParameterByName("pos_center");
		pos_extent_param_ = deferred_effect_->ParameterByName("pos_extent");
		tc_center_param_ = deferred_effect_->ParameterByName("tc_center");
		tc_extent_param_ = deferred_effect_->ParameterByName("tc_extent");
		shininess_clr_param_ = deferred_effect_->ParameterByName("shininess_clr");
		shininess_tex_param_ = deferred_effect_->ParameterByName("shininess_tex");
		normal_map_enabled_param_ = deferred_effect_->ParameterByName("normal_map_enabled");
		normal_tex_param_ = deferred_effect_->ParameterByName("normal_tex");
		height_map_enabled_param_ = deferred_effect_->ParameterByName("height_map_enabled");
		height_tex_param_ = deferred_effect_->ParameterByName("height_tex");
		diffuse_tex_param_ = deferred_effect_->ParameterByName("diffuse_tex");
		diffuse_clr_param_ = deferred_effect_->ParameterByName("diffuse_clr");
		specular_tex_param_ = deferred_effect_->ParameterByName("specular_tex");
		specular_clr_param_ = deferred_effect_->ParameterByName("specular_clr");
		emit_tex_param_ = deferred_effect_->ParameterByName("emit_tex");
		emit_clr_param_ = deferred_effect_->ParameterByName("emit_clr");
		opacity_clr_param_ = deferred_effect_->ParameterByName("opacity_clr");
		opacity_map_enabled_param_ = deferred_effect_->ParameterByName("opacity_map_enabled");
		opaque_depth_tex_param_ = deferred_effect_->ParameterByName("opaque_depth_tex");
		select_mode_object_id_param_ = deferred_effect_->ParameterByName("object_id");
	}

	void Renderable::UpdateTechniques()
	{
		if (this->AlphaTest())
		{
			depth_tech_ = deferred_effect_->TechniqueByName("DepthAlphaTestTech");
			gbuffer_rt0_tech_ = deferred_effect_->TechniqueByName("GBufferAlphaTestRT0Tech");
			gbuffer_rt1_tech_ = deferred_effect_->TechniqueByName("GBufferAlphaTestRT1Tech");
			gbuffer_mrt_tech_ = deferred_effect_->TechniqueByName("GBufferAlphaTestMRTTech");
			gen_rsm_tech_ = deferred_effect_->TechniqueByName("GenReflectiveShadowMapAlphaTestTech");
			gen_sm_tech_ = deferred_effect_->TechniqueByName("GenShadowMapAlphaTestTech");
			gen_cascaded_sm_tech_ = deferred_effect_->TechniqueByName("GenCascadedShadowMapAlphaTestTech");
			gen_sm_wo_dt_tech_ = deferred_effect_->TechniqueByName("GenShadowMapWODepthTextureAlphaTestTech");
		}
		else
		{
			depth_tech_ = deferred_effect_->TechniqueByName("DepthTech");
			gbuffer_rt0_tech_ = deferred_effect_->TechniqueByName("GBufferRT0Tech");
			gbuffer_rt1_tech_ = deferred_effect_->TechniqueByName("GBufferRT1Tech");
			gbuffer_mrt_tech_ = deferred_effect_->TechniqueByName("GBufferMRTTech");
			gen_rsm_tech_ = deferred_effect_->TechniqueByName("GenReflectiveShadowMapTech");
			gen_sm_tech_ = deferred_effect_->TechniqueByName("GenShadowMapTech");
			gen_cascaded_sm_tech_ = deferred_effect_->TechniqueByName("GenCascadedShadowMapTech");
			gen_sm_wo_dt_tech_ = deferred_effect_->TechniqueByName("GenShadowMapWODepthTextureTech");
		}
		depth_alpha_blend_back_tech_ = deferred_effect_->TechniqueByName("DepthAlphaBlendBackTech");
		depth_alpha_blend_front_tech_ = deferred_effect_->TechniqueByName("DepthAlphaBlendFrontTech");
		gbuffer_alpha_blend_back_rt0_tech_ = deferred_effect_->TechniqueByName("GBufferAlphaBlendBackRT0Tech");
		gbuffer_alpha_blend_front_rt0_tech_ = deferred_effect_->TechniqueByName("GBufferAlphaBlendFrontRT0Tech");
		gbuffer_alpha_blend_back_rt1_tech_ = deferred_effect_->TechniqueByName("GBufferAlphaBlendBackRT1Tech");
		gbuffer_alpha_blend_front_rt1_tech_ = deferred_effect_->TechniqueByName("GBufferAlphaBlendFrontRT1Tech");
		gbuffer_alpha_blend_back_mrt_tech_ = deferred_effect_->TechniqueByName("GBufferAlphaBlendBackMRTTech");
		gbuffer_alpha_blend_front_mrt_tech_ = deferred_effect_->TechniqueByName("GBufferAlphaBlendFrontMRTTech");
		special_shading_tech_ = deferred_effect_->TechniqueByName("SpecialShadingTech");
		special_shading_alpha_blend_back_tech_ = deferred_effect_->TechniqueByName("SpecialShadingAlphaBlendBackTech");
		special_shading_alpha_blend_front_tech_ = deferred_effect_->TechniqueByName("SpecialShadingAlphaBlendFrontTech");
		select_mode_tech_ = deferred_effect_->TechniqueByName("SelectModeTech");
	}

	RenderTechniquePtr const & Renderable::PassTech(PassType type) const
	{
		switch (type)
		{
		case PT_OpaqueDepth:
			return depth_tech_;

		case PT_TransparencyBackDepth:
			return depth_alpha_blend_back_tech_;

		case PT_TransparencyFrontDepth:
			return depth_alpha_blend_front_tech_;

		case PT_OpaqueGBufferRT0:
			return gbuffer_rt0_tech_;

		case PT_OpaqueGBufferRT1:
			return gbuffer_rt1_tech_;

		case PT_TransparencyBackGBufferRT0:
			return gbuffer_alpha_blend_back_rt0_tech_;

		case PT_TransparencyBackGBufferRT1:
			return gbuffer_alpha_blend_back_rt1_tech_;

		case PT_TransparencyFrontGBufferRT0:
			return gbuffer_alpha_blend_front_rt0_tech_;

		case PT_TransparencyFrontGBufferRT1:
			return gbuffer_alpha_blend_front_rt1_tech_;

		case PT_OpaqueGBufferMRT:
			return gbuffer_mrt_tech_;

		case PT_TransparencyBackGBufferMRT:
			return gbuffer_alpha_blend_back_mrt_tech_;

		case PT_TransparencyFrontGBufferMRT:
			return gbuffer_alpha_blend_front_mrt_tech_;

		case PT_GenReflectiveShadowMap:
			return gen_rsm_tech_;

		case PT_GenShadowMap:
			return gen_sm_tech_;

		case PT_GenCascadedShadowMap:
			return gen_cascaded_sm_tech_;

		case PT_GenShadowMapWODepthTexture:
			return gen_sm_wo_dt_tech_;

		case PT_OpaqueReflection:
			return reflection_tech_;

		case PT_TransparencyBackReflection:
			return reflection_alpha_blend_back_tech_;

		case PT_TransparencyFrontReflection:
			return reflection_alpha_blend_front_tech_;

		case PT_OpaqueSpecialShading:
			return special_shading_tech_;

		case PT_TransparencyBackSpecialShading:
			return special_shading_alpha_blend_back_tech_;

		case PT_TransparencyFrontSpecialShading:
			return special_shading_alpha_blend_front_tech_;

		case PT_SimpleForward:
			return simple_forward_tech_;

		default:
			BOOST_ASSERT(false);
			return gbuffer_rt0_tech_;
		}
	}
}
