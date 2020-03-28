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
#include <KFL/ErrorHandling.hpp>
#include <KFL/Math.hpp>
#include <KlayGE/App3D.hpp>
#include <KlayGE/SceneManager.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/SceneNode.hpp>
#include <KlayGE/RenderEffect.hpp>
#include <KlayGE/RenderView.hpp>
#include <KlayGE/FrameBuffer.hpp>
#include <KlayGE/Camera.hpp>
#include <KlayGE/RenderMaterial.hpp>
#include <KlayGE/DeferredRenderingLayer.hpp>

#include <KlayGE/Renderable.hpp>

namespace KlayGE
{
	Renderable::Renderable()
		: Renderable(L"")
	{
	}

	Renderable::Renderable(std::wstring_view name)
		: name_(name), rls_(1)
	{
		auto drl = Context::Instance().DeferredRenderingLayerInstance();
		if (drl)
		{
			this->BindDeferredEffect(drl->GBufferEffect(nullptr, false, false));
		}

		if (Context::Instance().RenderFactoryValid())
		{
			auto const& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();

			auto const& mesh_cb = re.PredefinedMeshCBufferInstance();
			auto* mesh_cbuff = mesh_cb.CBuffer();
			mesh_cbuffer_ = mesh_cbuff->Clone(mesh_cbuff->OwnerEffect());

			auto const& model_cb = re.PredefinedModelCBufferInstance();
			auto* model_cbuff = model_cb.CBuffer();
			model_cbuffer_ = model_cbuff->Clone(model_cbuff->OwnerEffect());

			auto const& camera_cb = re.PredefinedCameraCBufferInstance();
			auto* camera_cbuff = camera_cb.CBuffer();
			camera_cbuffer_ = camera_cbuff->Clone(camera_cbuff->OwnerEffect());
		}
	}

	Renderable::~Renderable()
	{
	}

	void Renderable::NumLods(uint32_t lods)
	{
		rls_.resize(lods);
	}

	uint32_t Renderable::NumLods() const
	{
		return static_cast<uint32_t>(rls_.size());
	}

	void Renderable::ActiveLod(int32_t lod)
	{
		if (lod >= 0)
		{
			active_lod_ = std::min(lod, static_cast<int32_t>(this->NumLods() - 1));
		}
		else
		{
			// -1 means automatic choose lod
			active_lod_ = -1;
		}
	}

	RenderLayout& Renderable::GetRenderLayout() const
	{
		return this->GetRenderLayout(active_lod_);
	}

	RenderLayout& Renderable::GetRenderLayout(uint32_t lod) const
	{
		return *rls_[lod];
	}

	std::wstring const & Renderable::Name() const
	{
		return name_;
	}

	void Renderable::OnRenderBegin()
	{
		RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
		auto* drl = Context::Instance().DeferredRenderingLayerInstance();

		{
			uint32_t const mesh_cbuff_index = effect_->FindCBuffer("klayge_mesh");
			if ((mesh_cbuff_index != static_cast<uint32_t>(-1)) && (effect_->CBufferByIndex(mesh_cbuff_index)->Size() > 0))
			{
				if (&mesh_cbuffer_->OwnerEffect() != effect_.get())
				{
					mesh_cbuffer_ = mesh_cbuffer_->Clone(*effect_);
				}

				effect_->BindCBufferByIndex(mesh_cbuff_index, mesh_cbuffer_);
			}
		}

		{
			uint32_t const camera_cbuff_index = effect_->FindCBuffer("klayge_camera");
			if ((camera_cbuff_index != static_cast<uint32_t>(-1)) && (effect_->CBufferByIndex(camera_cbuff_index)->Size() > 0))
			{
				if (&camera_cbuffer_->OwnerEffect() != effect_.get())
				{
					camera_cbuffer_ = camera_cbuffer_->Clone(*effect_);
				}

				auto const& pccb = re.PredefinedCameraCBufferInstance();

				auto const& viewport = *re.CurFrameBuffer()->Viewport();
				uint32_t const num_cameras = viewport.NumCameras();
				visible_in_cameras_ = 0;
				for (uint32_t i = 0; i < num_cameras; ++i)
				{
					if ((curr_node_ == nullptr) || (curr_node_->VisibleMark(i) != BoundOverlap::No))
					{
						Camera const& camera = *viewport.Camera(i);

						float4x4 cascade_crop_mat = float4x4::Identity();
						bool need_cascade_crop_mat = false;
						if (drl)
						{
							int32_t const cas_index = drl->CurrCascadeIndex();
							if (cas_index >= 0)
							{
								cascade_crop_mat = drl->GetCascadedShadowLayer().CascadeCropMatrix(cas_index);
								need_cascade_crop_mat = true;
							}
						}

						camera.Active(*camera_cbuffer_, visible_in_cameras_, model_mat_, inv_model_mat_, prev_model_mat_, model_mat_dirty_,
							cascade_crop_mat, need_cascade_crop_mat);
						pccb.CameraIndices(*camera_cbuffer_, visible_in_cameras_) = i;

						++visible_in_cameras_;
					}
				}

				pccb.NumCameras(*camera_cbuffer_) = visible_in_cameras_;

				effect_->BindCBufferByIndex(camera_cbuff_index, camera_cbuffer_);
			}
		}

		{
			uint32_t const model_cbuff_index = effect_->FindCBuffer("klayge_model");
			if ((model_cbuff_index != static_cast<uint32_t>(-1)) && (effect_->CBufferByIndex(model_cbuff_index)->Size() > 0))
			{
				if (&model_cbuffer_->OwnerEffect() != effect_.get())
				{
					model_cbuffer_ = model_cbuffer_->Clone(*effect_);
				}

				if (model_mat_dirty_)
				{
					auto const& pmcb = re.PredefinedModelCBufferInstance();

					pmcb.Model(*model_cbuffer_) = MathLib::transpose(model_mat_);
					pmcb.InvModel(*model_cbuffer_) = MathLib::transpose(inv_model_mat_);

					model_mat_dirty_ = false;
				}

				effect_->BindCBufferByIndex(model_cbuff_index, model_cbuffer_);
			}
		}

		if (select_mode_on_)
		{
			*select_mode_object_id_param_ = select_mode_object_id_;
		}
		else
		{
			auto const& mtl = mtl_ ? mtl_ : re.DefaultMaterial();
			mtl->Active(*effect_);

			if (drl)
			{
				FrameBufferPtr const & fb = re.CurFrameBuffer();
				*frame_size_param_ = int2(fb->Width(), fb->Height());

				*half_exposure_x_framerate_param_ = drl->MotionBlurExposure() / 2 / Context::Instance().AppInstance().FrameTime();
				*motion_blur_radius_param_ = static_cast<float>(drl->MotionBlurRadius());

				switch (type_)
				{
				case PT_OpaqueGBuffer:
				case PT_TransparencyBackGBuffer:
				case PT_TransparencyFrontGBuffer:
				case PT_GenReflectiveShadowMap:
					*opaque_depth_tex_param_ = drl->ResolvedDepthTex(drl->ActiveViewport());
					break;

				case PT_OpaqueSpecialShading:
				case PT_TransparencyBackSpecialShading:
				case PT_TransparencyFrontSpecialShading:
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
	}

	void Renderable::OnRenderEnd()
	{
	}

	AABBox const & Renderable::PosBound() const
	{
		return pos_aabb_;
	}

	AABBox const & Renderable::TexcoordBound() const
	{
		return tc_aabb_;
	}

	void Renderable::AddToRenderQueue()
	{
		Context::Instance().SceneManagerInstance().AddRenderable(this);
	}

	void Renderable::Render()
	{
		this->UpdateInstanceStream();

		RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();

		int32_t lod;
		if (active_lod_ < 0)
		{
			auto const& camera = *re.CurFrameBuffer()->Viewport()->Camera();
			lod = MathLib::clamp(static_cast<int32_t>(this->CalcLod(camera.EyePos(), camera.ProjMatrix()(0, 0)) + 0.5f),
				0, static_cast<int32_t>(this->NumLods() - 1));
		}
		else
		{
			lod = active_lod_;
		}
		RenderLayout const & layout = this->GetRenderLayout(lod);
		GraphicsBufferPtr const & inst_stream = layout.InstanceStream();
		RenderTechnique const & tech = *this->GetRenderTechnique();
		auto const & effect = *this->GetRenderEffect();
		if (inst_stream)
		{
			if (layout.NumInstances() > 0)
			{
				this->OnRenderBegin();
				re.Render(effect, tech, layout);
				this->OnRenderEnd();
			}
		}
		else
		{
			if (instances_.empty())
			{
				this->OnRenderBegin();
				re.Render(effect, tech, layout);
				this->OnRenderEnd();
			}
			else
			{
				for (auto const * node : instances_)
				{
					this->BindSceneNode(node);

					this->OnRenderBegin();

					bool const auto_set_camera_instances = (re.NumCameraInstances() == 0);
					if (auto_set_camera_instances)
					{
						re.NumCameraInstances(visible_in_cameras_);
					}
					re.Render(effect, tech, layout);
					if (auto_set_camera_instances)
					{
						re.NumCameraInstances(0);
					}

					this->OnRenderEnd();
				}
			}
		}
	}

	void Renderable::AddInstance(SceneNode const * node)
	{
		instances_.push_back(node);
	}

	void Renderable::ClearInstances()
	{
		instances_.resize(0);
	}

	void Renderable::UpdateInstanceStream()
	{
		if (!instances_.empty() && !instances_[0]->InstanceFormat().empty())
		{
			auto const & vet = instances_[0]->InstanceFormat();
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
		if (memcmp(&model_mat_, &mat, sizeof(mat)) != 0)
		{
			model_mat_ = prev_model_mat_ = mat;
			model_mat_dirty_ = true;
		}
	}

	void Renderable::InverseModelMatrix(float4x4 const& mat)
	{
		if (memcmp(&inv_model_mat_, &mat, sizeof(mat)) != 0)
		{
			inv_model_mat_ = mat;
			model_mat_dirty_ = true;
		}
	}

	void Renderable::PrevModelMatrix(float4x4 const& mat)
	{
		if (memcmp(&prev_model_mat_, &mat, sizeof(mat)) != 0)
		{
			prev_model_mat_ = mat;
			model_mat_dirty_ = true;
		}
	}

	void Renderable::BindSceneNode(SceneNode const * node)
	{
		curr_node_ = node;
		this->ModelMatrix(node->TransformToWorld());
		this->InverseModelMatrix(node->InverseTransformToWorld());
		this->PrevModelMatrix(node->PrevTransformToWorld());
	}

	void Renderable::UpdateBoundBox()
	{
		auto const& pmcb = Context::Instance().RenderFactoryInstance().RenderEngineInstance().PredefinedMeshCBufferInstance();

		AABBox const & pos_bb = this->PosBound();
		AABBox const & tc_bb = this->TexcoordBound();

		pmcb.PosCenter(*mesh_cbuffer_) = pos_bb.Center();
		pmcb.PosExtent(*mesh_cbuffer_) = pos_bb.HalfSize();
		pmcb.TcCenter(*mesh_cbuffer_) = float2(tc_bb.Center().x(), tc_bb.Center().y());
		pmcb.TcExtent(*mesh_cbuffer_) = float2(tc_bb.HalfSize().x(), tc_bb.HalfSize().y());

		mesh_cbuffer_->Dirty(true);
	}

	float Renderable::CalcLod(float3 const & eye_pos, float fov_scale) const
	{
		auto const aabb_ws = MathLib::transform_aabb(this->PosBound(), model_mat_);
		float3 view_dir = aabb_ws.Center() - eye_pos;
		float const dist_sq = MathLib::length_sq(view_dir);
		view_dir *= MathLib::recip_sqrt(dist_sq);
		float const area = MathLib::ortho_area(view_dir, aabb_ws);
		return dist_sq / area / fov_scale;
	}

	bool Renderable::AllHWResourceReady() const
	{
		bool ready = this->HWResourceReady();
		for (size_t i = 0; i < RenderMaterial::TS_NumTextureSlots; ++ i)
		{
			auto const& srv = mtl_ ? mtl_->Texture(static_cast<RenderMaterial::TextureSlot>(i)) : ShaderResourceViewPtr();
			if (ready && srv)
			{
				ready = srv->TextureResource()->HWResourceReady();
			}
		}
		return ready;
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

	void Renderable::Material(RenderMaterialPtr const& mtl)
	{
		mtl_ = mtl;

		if (mtl_->Transparent())
		{
			effect_attrs_ |= EA_TransparencyBack;
			effect_attrs_ |= EA_TransparencyFront;
		}
		if (mtl_->AlphaTestThreshold() > 0)
		{
			effect_attrs_ |= EA_AlphaTest;
		}
		if (mtl_->Sss())
		{
			effect_attrs_ |= EA_SSS;
		}

		if ((mtl_->Emissive().x() > 0) || (mtl_->Emissive().y() > 0) || (mtl_->Emissive().z() > 0) ||
			mtl_->Texture(RenderMaterial::TS_Emissive) || (effect_attrs_ & EA_TransparencyBack) || (effect_attrs_ & EA_TransparencyFront) ||
			(effect_attrs_ & EA_Reflection))
		{
			effect_attrs_ |= EA_SpecialShading;
		}

		auto drl = Context::Instance().DeferredRenderingLayerInstance();
		if (drl)
		{
			this->BindDeferredEffect(drl->GBufferEffect(mtl_.get(), false, is_skinned_));
		}
	}

	void Renderable::BindDeferredEffect(RenderEffectPtr const & deferred_effect)
	{
		effect_ = deferred_effect;

		this->UpdateTechniques();

		frame_size_param_ = effect_->ParameterByName("frame_size");
		opaque_depth_tex_param_ = effect_->ParameterByName("opaque_depth_tex");
		reflection_tex_param_ = nullptr;
		select_mode_object_id_param_ = effect_->ParameterByName("object_id");
		half_exposure_x_framerate_param_ = effect_->ParameterByName("half_exposure_x_framerate");
		motion_blur_radius_param_ = effect_->ParameterByName("motion_blur_radius");
	}

	void Renderable::UpdateTechniques()
	{
		auto& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
		bool const vp_rt_index_at_every_stage_support = re.DeviceCaps().vp_rt_index_at_every_stage_support;

		if (this->AlphaTest())
		{
			gbuffer_tech_ = effect_->TechniqueByName("GBufferAlphaTestTech");
			gen_shadow_map_tech_ = effect_->TechniqueByName("GenShadowMapAlphaTestTech");
			gen_csm_tech_ = effect_->TechniqueByName("GenCascadedShadowMapAlphaTestTech");
			gen_rsm_tech_ = effect_->TechniqueByName("GenReflectiveShadowMapAlphaTestTech");
			if (vp_rt_index_at_every_stage_support)
			{
				gbuffer_multi_view_tech_ = effect_->TechniqueByName("GBufferAlphaTestMultiViewTech");
				gen_shadow_map_multi_view_tech_ = effect_->TechniqueByName("GenShadowMapAlphaTestMultiViewTech");
				gen_csm_multi_view_tech_ = effect_->TechniqueByName("GenCascadedShadowMapAlphaTestMultiViewTech");
				gen_rsm_multi_view_tech_ = effect_->TechniqueByName("GenReflectiveShadowMapAlphaTestMultiViewTech");
			}
			else
			{
				gbuffer_multi_view_tech_ = effect_->TechniqueByName("GBufferAlphaTestMultiViewNoVpRtTech");
				gen_shadow_map_multi_view_tech_ = effect_->TechniqueByName("GenShadowMapAlphaTestMultiViewNoVpRtTech");
				gen_csm_multi_view_tech_ = effect_->TechniqueByName("GenCascadedShadowMapAlphaTestMultiViewNoVpRtTech");
				gen_rsm_multi_view_tech_ = effect_->TechniqueByName("GenReflectiveShadowMapAlphaTestMultiViewNoVpRtTech");
			}
		}
		else
		{
			gbuffer_tech_ = effect_->TechniqueByName("GBufferTech");
			gen_shadow_map_tech_ = effect_->TechniqueByName("GenShadowMapTech");
			gen_csm_tech_ = effect_->TechniqueByName("GenCascadedShadowMapTech");
			gen_rsm_tech_ = effect_->TechniqueByName("GenReflectiveShadowMapTech");
			if (vp_rt_index_at_every_stage_support)
			{
				gbuffer_multi_view_tech_ = effect_->TechniqueByName("GBufferMultiViewTech");
				gen_shadow_map_multi_view_tech_ = effect_->TechniqueByName("GenShadowMapMultiViewTech");
				gen_csm_multi_view_tech_ = effect_->TechniqueByName("GenCascadedShadowMapMultiViewTech");
				gen_rsm_multi_view_tech_ = effect_->TechniqueByName("GenReflectiveShadowMapMultiViewTech");
			}
			else
			{
				gbuffer_multi_view_tech_ = effect_->TechniqueByName("GBufferMultiViewNoVpRtTech");
				gen_shadow_map_multi_view_tech_ = effect_->TechniqueByName("GenShadowMapMultiViewNoVpRtTech");
				gen_csm_multi_view_tech_ = effect_->TechniqueByName("GenCascadedShadowMapMultiViewNoVpRtTech");
				gen_rsm_multi_view_tech_ = effect_->TechniqueByName("GenReflectiveShadowMapMultiViewNoVpRtTech");
			}
		}
		gbuffer_alpha_blend_back_tech_ = effect_->TechniqueByName("GBufferAlphaBlendBackTech");
		gbuffer_alpha_blend_front_tech_ = effect_->TechniqueByName("GBufferAlphaBlendFrontTech");
		special_shading_tech_ = effect_->TechniqueByName("SpecialShadingTech");
		special_shading_alpha_blend_back_tech_ = effect_->TechniqueByName("SpecialShadingAlphaBlendBackTech");
		special_shading_alpha_blend_front_tech_ = effect_->TechniqueByName("SpecialShadingAlphaBlendFrontTech");
		if (vp_rt_index_at_every_stage_support)
		{
			gbuffer_alpha_blend_back_multi_view_tech_ = effect_->TechniqueByName("GBufferAlphaBlendBackMultiViewTech");
			gbuffer_alpha_blend_front_multi_view_tech_ = effect_->TechniqueByName("GBufferAlphaBlendFrontMultiViewTech");
			special_shading_multi_view_tech_ = effect_->TechniqueByName("SpecialShadingMultiViewTech");
			special_shading_alpha_blend_back_multi_view_tech_ = effect_->TechniqueByName("SpecialShadingAlphaBlendBackMultiViewTech");
			special_shading_alpha_blend_front_multi_view_tech_ = effect_->TechniqueByName("SpecialShadingAlphaBlendFrontMultiViewTech");
		}
		else
		{
			gbuffer_alpha_blend_back_multi_view_tech_ = effect_->TechniqueByName("GBufferAlphaBlendBackMultiViewNoVpRtTech");
			gbuffer_alpha_blend_front_multi_view_tech_ = effect_->TechniqueByName("GBufferAlphaBlendFrontMultiViewNoVpRtTech");
			special_shading_multi_view_tech_ = effect_->TechniqueByName("SpecialShadingMultiViewNoVpRtTech");
			special_shading_alpha_blend_back_multi_view_tech_ = effect_->TechniqueByName("SpecialShadingAlphaBlendBackMultiViewNoVpRtTech");
			special_shading_alpha_blend_front_multi_view_tech_ =
				effect_->TechniqueByName("SpecialShadingAlphaBlendFrontMultiViewNoVpRtTech");
		}

		select_mode_tech_ = effect_->TechniqueByName("SelectModeTech");
	}

	RenderTechnique* Renderable::PassTech(PassType type) const
	{
		switch (type)
		{
		case PT_OpaqueGBuffer:
			return gbuffer_tech_;

		case PT_TransparencyBackGBuffer:
			return gbuffer_alpha_blend_back_tech_;

		case PT_TransparencyFrontGBuffer:
			return gbuffer_alpha_blend_front_tech_;

		case PT_OpaqueGBufferMultiView:
			return gbuffer_multi_view_tech_;

		case PT_TransparencyBackGBufferMultiView:
			return gbuffer_alpha_blend_back_multi_view_tech_;

		case PT_TransparencyFrontGBufferMultiView:
			return gbuffer_alpha_blend_front_multi_view_tech_;

		case PT_GenShadowMap:
			return gen_shadow_map_tech_;

		case PT_GenCascadedShadowMap:
			return gen_csm_tech_;

		case PT_GenReflectiveShadowMap:
			return gen_rsm_tech_;

		case PT_GenShadowMapMultiView:
			return gen_shadow_map_multi_view_tech_;

		case PT_GenCascadedShadowMapMultiView:
			return gen_csm_multi_view_tech_;

		case PT_GenReflectiveShadowMapMultiView:
			return gen_rsm_multi_view_tech_;

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

		case PT_OpaqueSpecialShadingMultiView:
			return special_shading_multi_view_tech_;

		case PT_TransparencyBackSpecialShadingMultiView:
			return special_shading_alpha_blend_back_multi_view_tech_;

		case PT_TransparencyFrontSpecialShadingMultiView:
			return special_shading_alpha_blend_front_multi_view_tech_;

		case PT_SimpleForward:
			return simple_forward_tech_;

		case PT_VDM:
			return vdm_tech_;

		default:
			KFL_UNREACHABLE("Invalid pass type");
		}
	}


	RenderableComponent::RenderableComponent(RenderablePtr const& renderable)
		: renderable_(renderable)
	{
		BOOST_ASSERT(renderable);
	}

	SceneComponentPtr RenderableComponent::Clone() const
	{
		return MakeSharedPtr<RenderableComponent>(renderable_);
	}

	Renderable& RenderableComponent::BoundRenderable() const
	{
		return *renderable_;
	}
}
