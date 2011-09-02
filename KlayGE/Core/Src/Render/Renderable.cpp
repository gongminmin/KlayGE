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
#include <KlayGE/Math.hpp>
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
	{
		DeferredRenderingLayerPtr const & drl = Context::Instance().DeferredRenderingLayerInstance();
		if (drl)
		{
			deferred_effect_ = drl->GBufferEffect();
			
			gbuffer_tech_ = deferred_effect_->TechniqueByName("GBufferTech");
			gbuffer_alpha_tech_ = deferred_effect_->TechniqueByName("GBufferAlphaTech");
			gbuffer_mrt_tech_ = deferred_effect_->TechniqueByName("GBufferMRTTech");
			gbuffer_alpha_mrt_tech_ = deferred_effect_->TechniqueByName("GBufferAlphaMRTTech");
			gen_rsm_tech_ = deferred_effect_->TechniqueByName("GenReflectiveShadowMap");
			gen_rsm_alpha_tech_ = deferred_effect_->TechniqueByName("GenReflectiveShadowMapAlpha");
			gen_sm_tech_ = deferred_effect_->TechniqueByName("GenShadowMap");
			gen_sm_alpha_tech_ = deferred_effect_->TechniqueByName("GenShadowMapAlpha");
			shading_tech_ = deferred_effect_->TechniqueByName("Shading");
			special_shading_tech_ = deferred_effect_->TechniqueByName("SpecialShading");

			lighting_tex_param_ = deferred_effect_->ParameterByName("lighting_tex");
			ssvo_tex_param_ = deferred_effect_->ParameterByName("ssvo_tex");
			ssvo_enabled_param_ = deferred_effect_->ParameterByName("ssvo_enabled");
			g_buffer_1_tex_param_ = deferred_effect_->ParameterByName("g_buffer_1_tex");

			mvp_param_ = deferred_effect_->ParameterByName("mvp");
			model_view_param_ = deferred_effect_->ParameterByName("model_view");
			depth_near_far_invfar_param_ = deferred_effect_->ParameterByName("depth_near_far_invfar");
			shininess_param_ = deferred_effect_->ParameterByName("shininess");
			normal_map_enabled_param_ = deferred_effect_->ParameterByName("normal_map_enabled");
			normal_tex_param_ = deferred_effect_->ParameterByName("normal_tex");
			height_map_enabled_param_ = deferred_effect_->ParameterByName("height_map_enabled");
			height_tex_param_ = deferred_effect_->ParameterByName("height_tex");
			diffuse_tex_param_ = deferred_effect_->ParameterByName("diffuse_tex");
			diffuse_clr_param_ = deferred_effect_->ParameterByName("diffuse_clr");
			specular_tex_param_ = deferred_effect_->ParameterByName("specular_tex");
			emit_tex_param_ = deferred_effect_->ParameterByName("emit_tex");
			emit_clr_param_ = deferred_effect_->ParameterByName("emit_clr");
			specular_level_param_ = deferred_effect_->ParameterByName("specular_level");
			flipping_param_ = deferred_effect_->ParameterByName("flipping");

			model_mat_ = float4x4::Identity();
		}
	}

	Renderable::~Renderable()
	{
	}

	void Renderable::OnRenderBegin()
	{
		if (deferred_effect_)
		{
			RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
			Camera const & camera = *re.CurFrameBuffer()->GetViewport().camera;

			float4x4 const & view = camera.ViewMatrix();
			float4x4 const & proj = camera.ProjMatrix();

			float4x4 const mv = model_mat_ * view;
			*mvp_param_ = mv * proj;
			*model_view_param_ = mv;

			switch (type_)
			{
			case PT_GBuffer:
			case PT_MRTGBuffer:
			case PT_GenReflectiveShadowMap:
				*depth_near_far_invfar_param_ = float3(camera.NearPlane(), camera.FarPlane(), 1 / camera.FarPlane());
				*diffuse_tex_param_ = diffuse_tex_;
				*diffuse_clr_param_ = float4(mtl_->diffuse.x(), mtl_->diffuse.y(), mtl_->diffuse.z(), static_cast<float>(!!diffuse_tex_));
				*normal_map_enabled_param_ = static_cast<int32_t>(!!normal_tex_);
				*normal_tex_param_ = normal_tex_;
				*height_map_enabled_param_ = static_cast<int32_t>(!!height_tex_);
				*height_tex_param_ = normal_tex_;
				*specular_tex_param_ = specular_tex_;
				*specular_level_param_ = float4(mtl_->specular_level, mtl_->specular_level, mtl_->specular_level, static_cast<float>(!!specular_tex_));
				*shininess_param_ = MathLib::clamp(mtl_->shininess / 256.0f, 1e-6f, 0.999f);
				break;

			case PT_GenShadowMap:
				*diffuse_clr_param_ = float4(mtl_->diffuse.x(), mtl_->diffuse.y(), mtl_->diffuse.z(), static_cast<float>(!!diffuse_tex_));
				*diffuse_tex_param_ = diffuse_tex_;
				break;

			case PT_Shading:
				*shininess_param_ = std::max(1e-6f, mtl_->shininess);
				*diffuse_tex_param_ = diffuse_tex_;
				*diffuse_clr_param_ = float4(mtl_->diffuse.x(), mtl_->diffuse.y(), mtl_->diffuse.z(), static_cast<float>(!!diffuse_tex_));
				*emit_tex_param_ = emit_tex_;
				*emit_clr_param_ = float4(mtl_->emit.x(), mtl_->emit.y(), mtl_->emit.z(), static_cast<float>(!!emit_tex_));
				*flipping_param_ = static_cast<int32_t>(re.CurFrameBuffer()->RequiresFlipping() ? -1 : +1);
				break;

			case PT_SpecialShading:
				*emit_tex_param_ = emit_tex_;
				*emit_clr_param_ = float4(mtl_->emit.x(), mtl_->emit.y(), mtl_->emit.z(), static_cast<float>(!!emit_tex_));
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
		Context::Instance().SceneManagerInstance().AddRenderable(this->shared_from_this());
	}

	void Renderable::Render()
	{
		this->UpdateInstanceStream();

		RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();

		GraphicsBufferPtr const & inst_stream = this->GetRenderLayout()->InstanceStream();
		RenderTechniquePtr const & tech = this->GetRenderTechnique();
		RenderLayoutPtr const & layout = this->GetRenderLayout();
		if (inst_stream)
		{
			this->OnRenderBegin();
			re.Render(*tech, *layout);
			this->OnRenderEnd();
		}
		else
		{
			this->OnRenderBegin();
			if (instances_.empty())
			{
				re.Render(*tech, *layout);
			}
			else
			{
				for (uint32_t i = 0; i < instances_.size(); ++ i)
				{
					this->OnInstanceBegin(i);
					re.Render(*tech, *layout);
					this->OnInstanceEnd(i);
				}
			}
			this->OnRenderEnd();
		}
	}

	void Renderable::AddInstance(SceneObjectPtr const & obj)
	{
		instances_.push_back(boost::weak_ptr<SceneObject>(obj));
	}

	void Renderable::UpdateInstanceStream()
	{
		if (!instances_.empty() && !instances_[0].lock()->InstanceFormat().empty())
		{
			RenderLayoutPtr const & rl = this->GetRenderLayout();

			GraphicsBufferPtr inst_stream = rl->InstanceStream();
			if (!inst_stream)
			{
				RenderFactory& rf(Context::Instance().RenderFactoryInstance());

				inst_stream = rf.MakeVertexBuffer(BU_Dynamic, EAH_CPU_Write | EAH_GPU_Read, NULL);
				rl->BindVertexStream(inst_stream, instances_[0].lock()->InstanceFormat(), RenderLayout::ST_Instance, 1);
			}
			else
			{
				for (size_t i = 0; i < instances_.size(); ++ i)
				{
					BOOST_ASSERT(rl->InstanceStreamFormat() == instances_[i].lock()->InstanceFormat());
				}
			}

			uint32_t const size = rl->InstanceSize();

			inst_stream->Resize(static_cast<uint32_t>(size * instances_.size()));
			{
				GraphicsBuffer::Mapper mapper(*inst_stream, BA_Write_Only);
				for (size_t i = 0; i < instances_.size(); ++ i)
				{
					uint8_t const * src = static_cast<uint8_t const *>(instances_[i].lock()->InstanceData());
					std::copy(src, src + size, mapper.Pointer<uint8_t>() + i * size);
				}
			}

			for (uint32_t i = 0; i < rl->NumVertexStreams(); ++ i)
			{
				rl->VertexStreamFrequencyDivider(i, RenderLayout::ST_Geometry, static_cast<uint32_t>(instances_.size()));
			}
		}
	}

	void Renderable::SetModelMatrix(float4x4 const & mat)
	{
		model_mat_ = mat;
	}

	void Renderable::Pass(PassType type)
	{
		type_ = type;
		technique_ = this->PassTech(type, has_opacity_map_);
	}

	RenderTechniquePtr const & Renderable::PassTech(PassType type, bool alpha) const
	{
		switch (type)
		{
		case PT_GBuffer:
			if (alpha)
			{
				return gbuffer_alpha_tech_;
			}
			else
			{
				return gbuffer_tech_;
			}

		case PT_MRTGBuffer:
			if (alpha)
			{
				return gbuffer_alpha_mrt_tech_;
			}
			else
			{
				return gbuffer_mrt_tech_;
			}

		case PT_GenReflectiveShadowMap:
			if (alpha)
			{
				return gen_rsm_alpha_tech_;
			}
			else
			{
				return gen_rsm_tech_;
			}

		case PT_GenShadowMap:
			if (alpha)
			{
				return gen_sm_alpha_tech_;
			}
			else
			{
				return gen_sm_tech_;
			}

		case PT_Shading:
			return shading_tech_;

		case PT_SpecialShading:
			return special_shading_tech_;

		default:
			BOOST_ASSERT(false);
			return gbuffer_tech_;
		}
	}

	void Renderable::LightingTex(TexturePtr const & tex)
	{
		*lighting_tex_param_ = tex;
	}

	void Renderable::SSVOTex(TexturePtr const & tex)
	{
		*ssvo_tex_param_ = tex;
	}

	void Renderable::SSVOEnabled(bool ssvo)
	{
		*ssvo_enabled_param_ = static_cast<int32_t>(ssvo);
	}
}
