#include <KlayGE/KlayGE.hpp>
#include <KlayGE/Util.hpp>
#include <KlayGE/ThrowErr.hpp>
#include <KlayGE/GraphicsBuffer.hpp>
#include <KlayGE/Math.hpp>
#include <KlayGE/Font.hpp>
#include <KlayGE/Renderable.hpp>
#include <KlayGE/RenderableHelper.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/RenderEffect.hpp>
#include <KlayGE/FrameBuffer.hpp>
#include <KlayGE/SceneManager.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/ResLoader.hpp>
#include <KlayGE/RenderSettings.hpp>
#include <KlayGE/Mesh.hpp>
#include <KlayGE/Mesh.hpp>
#include <KlayGE/Texture.hpp>
#include <KlayGE/SceneObjectHelper.hpp>
#include <KlayGE/PostProcess.hpp>
#include <KlayGE/Light.hpp>

#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/InputFactory.hpp>

#include <vector>
#include <sstream>
#include <ctime>
#include <boost/bind.hpp>

#include "ShadowCubeMap.hpp"

using namespace std;
using namespace KlayGE;
using namespace KlayGE::MathLib;

namespace
{
	uint32_t const SHADOW_MAP_SIZE = 512;

	class ShadowMapped
	{
	public:
		ShadowMapped(uint32_t shadow_map_size)
			: shadow_map_size_(shadow_map_size)
		{
			FrameBufferPtr fb = Context::Instance().RenderFactoryInstance().MakeFrameBuffer();
			flipping_ = static_cast<int32_t>(fb->RequiresFlipping() ? +1 : -1);
		}

		float4x4 LightViewProj() const
		{
			return light_view_ * light_proj_;
		}

		virtual void GenShadowMapPass(bool gen_sm)
		{
			gen_sm_pass_ = gen_sm;
		}

		void LightSrc(LightSourcePtr const & light_src)
		{
			light_pos_ = light_src->Position();

			float4x4 light_model = MathLib::to_matrix(light_src->Rotation()) * MathLib::translation(light_src->Position());
			inv_light_model_ = MathLib::inverse(light_model);

			App3DFramework const & app = Context::Instance().AppInstance();
			light_view_ = app.ActiveCamera().ViewMatrix();
			light_proj_ = app.ActiveCamera().ProjMatrix();

			light_falloff_ = light_src->Falloff();
		}

		void ShadowMapTexture(TexturePtr const & cube_tex)
		{
			sm_cube_tex_ = cube_tex;
		}

		void LampTexture(TexturePtr const & tex)
		{
			lamp_tex_ = tex;
		}

	protected:
		void OnRenderBegin(float4x4 const & model, RenderEffectPtr effect)
		{
			App3DFramework const & app = Context::Instance().AppInstance();

			*(effect->ParameterByName("model")) = model;
			*(effect->ParameterByName("obj_model_to_light_model")) = model * inv_light_model_;

			if (gen_sm_pass_)
			{
				*(effect->ParameterByName("mvp")) = model * this->LightViewProj();
			}
			else
			{
				float4x4 const & view = app.ActiveCamera().ViewMatrix();
				float4x4 const & proj = app.ActiveCamera().ProjMatrix();

				*(effect->ParameterByName("mvp")) = model * view * proj;
				*(effect->ParameterByName("light_pos")) = light_pos_;

				*(effect->ParameterByName("flipping")) = flipping_;

				*(effect->ParameterByName("light_projective_tex")) = lamp_tex_;
				*(effect->ParameterByName("shadow_cube_tex")) = sm_cube_tex_;

				*(effect->ParameterByName("light_falloff")) = light_falloff_;
			}
		}

	protected:
		uint32_t shadow_map_size_;

		int32_t flipping_;

		bool gen_sm_pass_;
		TexturePtr sm_cube_tex_;

		float3 light_pos_;
		float4x4 inv_light_model_;
		float4x4 light_view_, light_proj_;
		float3 light_falloff_;

		TexturePtr lamp_tex_;
	};

	class OccluderMesh : public StaticMesh, public ShadowMapped
	{
	public:
		OccluderMesh(RenderModelPtr const & model, std::wstring const & name)
			: StaticMesh(model, name),
				ShadowMapped(SHADOW_MAP_SIZE)
		{
			effect_ = Context::Instance().RenderFactoryInstance().LoadEffect("ShadowCubeMap.fxml");
		}

		void BuildMeshInfo()
		{
			RenderModel::Material const & mtl = model_.lock()->GetMaterial(this->MaterialID());

			// 建立纹理
			TexturePtr dm, sm, em;
			RenderModel::TextureSlotsType const & texture_slots = mtl.texture_slots;
			for (RenderModel::TextureSlotsType::const_iterator iter = texture_slots.begin();
				iter != texture_slots.end(); ++ iter)
			{
				if (("DiffuseMap" == iter->first) || ("Diffuse Color" == iter->first) || ("Diffuse Color Map" == iter->first))
				{
					if (!ResLoader::Instance().Locate(iter->second).empty())
					{
						dm = LoadTexture(iter->second, EAH_GPU_Read)();
					}
				}
				else
				{
					if (("SpecularMap" == iter->first) || ("Specular Level" == iter->first) || ("Reflection Glossiness Map" == iter->first))
					{
						if (!ResLoader::Instance().Locate(iter->second).empty())
						{
							sm = LoadTexture(iter->second, EAH_GPU_Read)();
						}
					}
					else
					{
						if (("EmitMap" == iter->first) || ("Self-Illumination" == iter->first))
						{
							if (!ResLoader::Instance().Locate(iter->second).empty())
							{
								em = LoadTexture(iter->second, EAH_GPU_Read)();
							}
						}
					}
				}
			}
			*(effect_->ParameterByName("diffuse_tex")) = dm;
			*(effect_->ParameterByName("specular_tex")) = sm;
			*(effect_->ParameterByName("emit_tex")) = em;

			*(effect_->ParameterByName("ambient_clr")) = float4(mtl.ambient.x(), mtl.ambient.y(), mtl.ambient.z(), 1);
			*(effect_->ParameterByName("diffuse_clr")) = float4(mtl.diffuse.x(), mtl.diffuse.y(), mtl.diffuse.z(), bool(dm));
			*(effect_->ParameterByName("specular_clr")) = float4(mtl.specular.x(), mtl.specular.y(), mtl.specular.z(), bool(sm));
			*(effect_->ParameterByName("emit_clr")) = float4(mtl.emit.x(), mtl.emit.y(), mtl.emit.z(), bool(em));

			*(effect_->ParameterByName("specular_level")) = mtl.specular_level;
			*(effect_->ParameterByName("shininess")) = std::max(1e-6f, mtl.shininess);
		}

		void MinVariance(float min_variance)
		{
			*(effect_->ParameterByName("min_variance")) = min_variance;
		}

		void BleedingReduce(float bleeding_reduce)
		{
			*(effect_->ParameterByName("bleeding_reduce")) = bleeding_reduce;
		}

		void SetModelMatrix(float4x4 const & model)
		{
			model_matrix_ = model;
		}

		void GenShadowMapPass(bool gen_sm)
		{
			ShadowMapped::GenShadowMapPass(gen_sm);

			if (gen_sm)
			{
				technique_ = effect_->TechniqueByName("GenShadowMap");
			}
			else
			{
				technique_ = effect_->TechniqueByName("RenderScene");
			}
		}

		void OnRenderBegin()
		{
			ShadowMapped::OnRenderBegin(model_matrix_, effect_);
		}

	private:
		float4x4 model_matrix_;
	
		RenderEffectPtr effect_;
	};

	class OccluderObject : public SceneObjectHelper
	{
	public:
		explicit OccluderObject(std::string const & model_name)
			: SceneObjectHelper(SOA_Cullable | SOA_Moveable)
		{
			renderable_ = LoadModel(model_name, EAH_GPU_Read, CreateModelFactory<RenderModel>(), CreateMeshFactory<OccluderMesh>())()->Mesh(0);
			checked_pointer_cast<OccluderMesh>(renderable_)->SetModelMatrix(model_);
		}

		void Update()
		{
			model_ = MathLib::scaling(5.0f, 5.0f, 5.0f) * MathLib::translation(5.0f, 5.0f, 0.0f) * MathLib::rotation_y(-std::clock() / 1500.0f);
			checked_pointer_cast<OccluderMesh>(renderable_)->SetModelMatrix(model_);
		}

		float4x4 const & GetModelMatrix() const
		{
			return model_;
		}

	private:
		float4x4 model_;
	};

	class RoomObject : public SceneObjectHelper
	{
	public:
		explicit RoomObject(StaticMeshPtr const & mesh)
			: SceneObjectHelper(SOA_Cullable | SOA_Moveable)
		{
			model_ = float4x4::Identity();

			renderable_ = mesh;
			checked_pointer_cast<OccluderMesh>(renderable_)->SetModelMatrix(model_);
		}

		void Update()
		{
		}

		float4x4 const & GetModelMatrix() const
		{
			return model_;
		}

	private:
		float4x4 model_;
	};


	class PointLightSourceUpdate
	{
	public:
		void operator()(LightSource& light)
		{
			light.ModelMatrix(MathLib::rotation_z(0.4f)
				* MathLib::rotation_y(static_cast<float>(timer_.current_time()) / 1.4f)
				* MathLib::translation(2.0f, 12.0f, 4.0f));
		}

	private:
		Timer timer_;
	};


	enum
	{
		Exit,
	};

	InputActionDefine actions[] =
	{
		InputActionDefine(Exit, KS_Escape),
	};
}


int main()
{
	ResLoader::Instance().AddPath("../Samples/media/Common");

	Context::Instance().LoadCfg("KlayGE.cfg");

	ShadowCubeMap app;
	app.Create();
	app.Run();

	return 0;
}

ShadowCubeMap::ShadowCubeMap()
				: App3DFramework("ShadowCubeMap")
{
	ResLoader::Instance().AddPath("../Samples/media/ShadowCubeMap");
}

bool ShadowCubeMap::ConfirmDevice() const
{
	RenderDeviceCaps const & caps = Context::Instance().RenderFactoryInstance().RenderEngineInstance().DeviceCaps();
	if (caps.max_shader_model < 2)
	{
		return false;
	}
	if (!(caps.rendertarget_format_support(EF_D16, 1, 0)
		&& (caps.rendertarget_format_support(EF_GR16F, 1, 0) || caps.rendertarget_format_support(EF_ABGR16F, 1, 0))))
	{
		return false;
	}

	return true;
}

void ShadowCubeMap::InitObjects()
{
	RenderFactory& rf = Context::Instance().RenderFactoryInstance();

	// 建立字体
	font_ = rf.MakeFont("gkai00mp.kfont");

	boost::function<RenderModelPtr()> model_ml = LoadModel("ScifiRoom.7z//ScifiRoom.meshml", EAH_GPU_Read, CreateModelFactory<RenderModel>(), CreateMeshFactory<OccluderMesh>());

	this->LookAt(float3(0.0f, 10.0f, -25.0f), float3(0, 10.0f, 0));
	this->Proj(0.01f, 500);

	lamp_tex_ = LoadTexture("lamp.dds", EAH_GPU_Read)();

	RenderModelPtr scene_model = model_ml();
	scene_objs_.resize(scene_model->NumMeshes() + 1);
	for (size_t i = 0; i < scene_model->NumMeshes(); ++ i)
	{
		scene_objs_[i] = MakeSharedPtr<RoomObject>(scene_model->Mesh(i));
	}
	scene_objs_.back() = MakeSharedPtr<OccluderObject>("teapot.meshml");

	for (size_t i = 0; i < scene_objs_.size(); ++ i)
	{
		scene_objs_[i]->AddToSceneManager();
		checked_pointer_cast<OccluderMesh>(scene_objs_[i]->GetRenderable())->LampTexture(lamp_tex_);
	}

	RenderViewPtr depth_view = rf.Make2DDepthStencilRenderView(SHADOW_MAP_SIZE, SHADOW_MAP_SIZE, EF_D16, 1, 0);
	shadow_buffer_ = rf.MakeFrameBuffer();
	ElementFormat fmt;
	if (rf.RenderEngineInstance().DeviceCaps().rendertarget_format_support(EF_GR16F, 1, 0))
	{
		fmt = EF_GR16F;
	}
	else
	{
		BOOST_ASSERT(rf.RenderEngineInstance().DeviceCaps().rendertarget_format_support(EF_ABGR16F, 1, 0));

		fmt = EF_ABGR16F;
	}
	shadow_tex_ = rf.MakeTexture2D(SHADOW_MAP_SIZE, SHADOW_MAP_SIZE, 1, 1, fmt, 1, 0, EAH_GPU_Read | EAH_GPU_Write, NULL);
	shadow_buffer_->Attach(FrameBuffer::ATT_Color0, rf.Make2DRenderView(*shadow_tex_, 0, 0));
	shadow_buffer_->Attach(FrameBuffer::ATT_DepthStencil, depth_view);

	shadow_cube_tex_ = rf.MakeTextureCube(SHADOW_MAP_SIZE, 1, 1, shadow_tex_->Format(), 1, 0, EAH_GPU_Read | EAH_GPU_Write, NULL);
	for (size_t i = 0; i < scene_objs_.size(); ++ i)
	{
		checked_pointer_cast<OccluderMesh>(scene_objs_[i]->GetRenderable())->ShadowMapTexture(shadow_cube_tex_);
	}

	light_ = MakeSharedPtr<PointLightSource>();
	light_->Attrib(0);
	light_->Color(float3(1, 1, 1));
	light_->Falloff(float3(0, 0.05f, 0));
	light_->ProjectiveTexture(lamp_tex_);
	light_->BindUpdateFunc(PointLightSourceUpdate());
	light_->AddToSceneManager();

	light_proxy_ = MakeSharedPtr<SceneObjectLightSourceProxy>(light_);
	checked_pointer_cast<SceneObjectLightSourceProxy>(light_proxy_)->Scaling(0.5f, 0.5f, 0.5f);
	light_proxy_->AddToSceneManager();

	for (int i = 0; i < 6; ++ i)
	{
		sm_filter_pps_[i] = MakeSharedPtr<BlurPostProcess<SeparableGaussianFilterPostProcess> >(3, 1.0f);
	
		sm_filter_pps_[i]->InputPin(0, shadow_tex_);
		sm_filter_pps_[i]->OutputPin(0, shadow_cube_tex_, 0, 0, i);
		if (!shadow_buffer_->RequiresFlipping())
		{
			switch (i)
			{
			case Texture::CF_Positive_Y:
				sm_filter_pps_[i]->OutputPin(0, shadow_cube_tex_, 0, 0, Texture::CF_Negative_Y);
				break;

			case Texture::CF_Negative_Y:
				sm_filter_pps_[i]->OutputPin(0, shadow_cube_tex_, 0, 0, Texture::CF_Positive_Y);
				break;

			default:
				break;
			}
		}
	}

	fpcController_.Scalers(0.05f, 1.0f);

	InputEngine& inputEngine(Context::Instance().InputFactoryInstance().InputEngineInstance());
	InputActionMap actionMap;
	actionMap.AddActions(actions, actions + sizeof(actions) / sizeof(actions[0]));

	action_handler_t input_handler = MakeSharedPtr<input_signal>();
	input_handler->connect(boost::bind(&ShadowCubeMap::InputHandler, this, _1, _2));
	inputEngine.ActionMap(actionMap, input_handler, true);

	UIManager::Instance().Load(ResLoader::Instance().Load("ShadowCubemap.uiml"));
	dialog_ = UIManager::Instance().GetDialogs()[0];

	id_min_variance_static_ = dialog_->IDFromName("MinVarianceStatic");
	id_min_variance_slider_ = dialog_->IDFromName("MinVarianceSlider");
	id_bleeding_reduce_static_ = dialog_->IDFromName("BleedingReduceStatic");
	id_bleeding_reduce_slider_ = dialog_->IDFromName("BleedingReduceSlider");
	id_ctrl_camera_ = dialog_->IDFromName("CtrlCamera");

	dialog_->Control<UISlider>(id_min_variance_slider_)->OnValueChangedEvent().connect(boost::bind(&ShadowCubeMap::MinVarianceChangedHandler, this, _1));
	dialog_->Control<UISlider>(id_bleeding_reduce_slider_)->OnValueChangedEvent().connect(boost::bind(&ShadowCubeMap::BleedingReduceChangedHandler, this, _1));
	dialog_->Control<UICheckBox>(id_ctrl_camera_)->OnChangedEvent().connect(boost::bind(&ShadowCubeMap::CtrlCameraHandler, this, _1));

	this->MinVarianceChangedHandler(*dialog_->Control<UISlider>(id_min_variance_slider_));
	this->BleedingReduceChangedHandler(*dialog_->Control<UISlider>(id_bleeding_reduce_slider_));
}

void ShadowCubeMap::OnResize(uint32_t width, uint32_t height)
{
	App3DFramework::OnResize(width, height);

	UIManager::Instance().SettleCtrls(width, height);
}

void ShadowCubeMap::InputHandler(InputEngine const & /*sender*/, InputAction const & action)
{
	switch (action.first)
	{
	case Exit:
		this->Quit();
		break;
	}
}

void ShadowCubeMap::MinVarianceChangedHandler(KlayGE::UISlider const & sender)
{
	float min_var = sender.GetValue() / 2000.0f + 0.001f;
	for (size_t i = 0; i < scene_objs_.size(); ++ i)
	{
		checked_pointer_cast<OccluderMesh>(scene_objs_[i]->GetRenderable())->MinVariance(min_var);
	}
}

void ShadowCubeMap::BleedingReduceChangedHandler(KlayGE::UISlider const & sender)
{
	float bleeding = sender.GetValue() / 500.0f + 0.45f;
	for (size_t i = 0; i < scene_objs_.size(); ++ i)
	{
		checked_pointer_cast<OccluderMesh>(scene_objs_[i]->GetRenderable())->BleedingReduce(bleeding);
	}
}

void ShadowCubeMap::CtrlCameraHandler(KlayGE::UICheckBox const & sender)
{
	if (sender.GetChecked())
	{
		fpcController_.AttachCamera(this->ActiveCamera());
	}
	else
	{
		fpcController_.DetachCamera();
	}
}

void ShadowCubeMap::DoUpdateOverlay()
{
	UIManager::Instance().Render();

	std::wostringstream stream;
	stream.precision(2);
	stream << std::fixed << this->FPS() << " FPS";

	font_->RenderText(0, 0, Color(1, 1, 0, 1), L"ShadowCubeMap", 16);
	font_->RenderText(0, 18, Color(1, 1, 0, 1), stream.str(), 16);
}

uint32_t ShadowCubeMap::DoUpdate(uint32_t pass)
{
	RenderEngine& renderEngine = Context::Instance().RenderFactoryInstance().RenderEngineInstance();

	if (pass > 0)
	{
		sm_filter_pps_[pass - 1]->Apply();
	}

	switch (pass)
	{
	case 0:
		{
			for (size_t i = 0; i < scene_objs_.size(); ++ i)
			{
				checked_pointer_cast<OccluderMesh>(scene_objs_[i]->GetRenderable())->GenShadowMapPass(true);
			}
		}

	case 1:
	case 2:
	case 3:
	case 4:
	case 5:
		{
			renderEngine.BindFrameBuffer(shadow_buffer_);
			renderEngine.CurFrameBuffer()->Clear(FrameBuffer::CBM_Color | FrameBuffer::CBM_Depth, Color(0.2f, 0.4f, 0.6f, 1), 1.0f, 0);

			shadow_buffer_->GetViewport().camera = light_->SMCamera(pass);

			for (size_t i = 0; i < scene_objs_.size(); ++ i)
			{
				checked_pointer_cast<OccluderMesh>(scene_objs_[i]->GetRenderable())->LightSrc(light_);
			}
		}
		return App3DFramework::URV_Need_Flush;

	default:
		{
			renderEngine.BindFrameBuffer(FrameBufferPtr());
			renderEngine.CurFrameBuffer()->Clear(FrameBuffer::CBM_Color | FrameBuffer::CBM_Depth, Color(0.2f, 0.4f, 0.6f, 1), 1.0f, 0);

			for (size_t i = 0; i < scene_objs_.size(); ++ i)
			{
				checked_pointer_cast<OccluderMesh>(scene_objs_[i]->GetRenderable())->GenShadowMapPass(false);
			}
		}
		return App3DFramework::URV_Need_Flush | App3DFramework::URV_Finished;
	}
}
