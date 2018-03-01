#include <KlayGE/KlayGE.hpp>
#include <KFL/CXX17/iterator.hpp>
#include <KlayGE/ResLoader.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/FrameBuffer.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/InputFactory.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/RenderEffect.hpp>
#include <KlayGE/SceneObject.hpp>
#include <KlayGE/SceneObjectHelper.hpp>
#include <KlayGE/Camera.hpp>
#include <KlayGE/Mesh.hpp>
#include <KlayGE/PostProcess.hpp>
#include <KlayGE/Light.hpp>
#include <KlayGE/DeferredRenderingLayer.hpp>
#include <KlayGE/UI.hpp>
#include <KlayGE/RenderMaterial.hpp>
#include <KlayGE/RenderableHelper.hpp>

#include <KlayGE/SSRPostProcess.hpp>

#include <sstream>

#include "SampleCommon.hpp"
#include "Reflection.hpp"

using namespace KlayGE;

namespace
{
	class ReflectMesh : public StaticMesh
	{
	public:
		ReflectMesh(RenderModelPtr const & model, std::wstring const & name)
			: StaticMesh(model, name)
		{
		}

		virtual void DoBuildMeshInfo() override
		{
			StaticMesh::DoBuildMeshInfo();

			mtl_ = SyncLoadRenderMaterial("ReflectMesh.mtlml");

			effect_attrs_ |= EA_Reflection;

			this->BindDeferredEffect(SyncLoadRenderEffect("Reflection.fxml"));
			technique_ = special_shading_tech_;

			reflection_tech_ = effect_->TechniqueByName("ReflectReflectionTech");
			reflection_alpha_blend_back_tech_ = reflection_tech_;
			reflection_alpha_blend_front_tech_ = reflection_tech_;

			special_shading_tech_ = effect_->TechniqueByName("ReflectSpecialShadingTech");
			special_shading_alpha_blend_back_tech_ = special_shading_tech_;
			special_shading_alpha_blend_front_tech_ = special_shading_tech_;

			reflection_tex_param_ = effect_->ParameterByName("reflection_tex");
		}

		void OnRenderBegin()
		{
			StaticMesh::OnRenderBegin();
					
			switch (type_)
			{
			case PT_OpaqueReflection:
			case PT_TransparencyBackReflection:
			case PT_TransparencyFrontReflection:
				{
					App3DFramework const & app = Context::Instance().AppInstance();
					Camera const & camera = app.ActiveCamera();
					*(effect_->ParameterByName("proj")) = camera.ProjMatrix();
					*(effect_->ParameterByName("inv_proj")) = camera.InverseProjMatrix();
					float q = camera.FarPlane() / (camera.FarPlane() - camera.NearPlane());
					float3 near_q_far(camera.NearPlane() * q, q, camera.FarPlane());
					*(effect_->ParameterByName("near_q_far")) = near_q_far;
					*(effect_->ParameterByName("ray_length")) = camera.FarPlane() - camera.NearPlane();
					*(effect_->ParameterByName("inv_view")) = camera.InverseViewMatrix();
				}
				break;

			default:
				break;
			}
		}

		void FrontReflectionTex(TexturePtr const & tex)
		{
			*(effect_->ParameterByName("front_side_tex")) = tex;
		}
		void FrontReflectionDepthTex(TexturePtr const & tex)
		{
			*(effect_->ParameterByName("front_side_depth_tex")) = tex;
		}

		void BackReflectionTex(TexturePtr const & tex)
		{
			*(effect_->ParameterByName("back_side_tex")) = tex;
		}
		void BackReflectionDepthTex(TexturePtr const & tex)
		{
			*(effect_->ParameterByName("back_side_depth_tex")) = tex;
		}

		void BackCamera(CameraPtr const & camera)
		{
			float4x4 const & view = camera->ViewMatrix();
			float4x4 const & proj = camera->ProjMatrix();

			float4x4 const mv = model_mat_ * view;

			*(effect_->ParameterByName("back_model_view")) = mv;
			*(effect_->ParameterByName("back_mvp")) = mv * proj;

			App3DFramework const & app = Context::Instance().AppInstance();
			Camera const & scene_camera = app.ActiveCamera();
			*(effect_->ParameterByName("eye_in_back_camera")) = MathLib::transform_coord(scene_camera.EyePos(), view);
		}

		void MinSamples(int32_t samples)
		{
			*(effect_->ParameterByName("min_samples")) = samples;
		}
		void MaxSamples(int32_t samples)
		{
			*(effect_->ParameterByName("max_samples")) = samples;
		}

		void EnbleReflection(bool enable)
		{
			if (enable)
			{
				effect_attrs_ |= EA_SpecialShading;
			}
			else
			{
				effect_attrs_ &= ~EA_SpecialShading;
			}
		}

		void SkyBox(TexturePtr const & y_cube, TexturePtr const & c_cube)
		{
			*(effect_->ParameterByName("skybox_tex")) = y_cube;
			*(effect_->ParameterByName("skybox_C_tex")) = c_cube;
		}

	private:
		TexturePtr front_refl_tex_;
		TexturePtr front_refl_depth_tex_;
		TexturePtr back_refl_tex_;
		TexturePtr back_refl_depth_tex_;
	};

	class DinoMesh : public StaticMesh
	{
	public:
		DinoMesh(RenderModelPtr const & model, std::wstring const & name)
			: StaticMesh(model, name)
		{
		}

	private:
		virtual void DoBuildMeshInfo() override
		{
			StaticMesh::DoBuildMeshInfo();

			mtl_ = SyncLoadRenderMaterial("DinoMesh.mtlml");
		}
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

int SampleMain()
{
	ContextCfg cfg = Context::Instance().Config();
	cfg.deferred_rendering = true;
	Context::Instance().Config(cfg);

	ScreenSpaceReflectionApp app;
	app.Create();
	app.Run();
	
	return 0;
}

ScreenSpaceReflectionApp::ScreenSpaceReflectionApp()
	: App3DFramework("Reflection")
{
	ResLoader::Instance().AddPath("../../Samples/media/CausticsMap");
	ResLoader::Instance().AddPath("../../Samples/media/Reflection");
}

void ScreenSpaceReflectionApp::OnCreate()
{
	RenderFactory& rf = Context::Instance().RenderFactoryInstance();

	loading_percentage_ = 0;
	c_cube_ = ASyncLoadTexture("Lake_CraterLake03_filtered_c.dds", EAH_GPU_Read | EAH_Immutable);
	y_cube_ = ASyncLoadTexture("Lake_CraterLake03_filtered_y.dds", EAH_GPU_Read | EAH_Immutable);
	teapot_model_ = ASyncLoadModel("teapot.meshml", EAH_GPU_Read | EAH_Immutable,
		CreateModelFactory<RenderModel>(), CreateMeshFactory<ReflectMesh>());
	RenderablePtr dino_model = ASyncLoadModel("dino50.meshml", EAH_GPU_Read | EAH_Immutable,
		CreateModelFactory<RenderModel>(), CreateMeshFactory<DinoMesh>());

	this->LookAt(float3(2.0f, 2.0f, -5.0f), float3(0.0f, 1.0f, 0.0f), float3(0, 1, 0));
	this->Proj(0.1f, 500.0f);
	tb_controller_.AttachCamera(this->ActiveCamera());
	tb_controller_.Scalers(0.003f, 0.05f);

	screen_camera_path_ = LoadCameraPath(ResLoader::Instance().Open("Reflection.cam_path"));
	screen_camera_path_->AttachCamera(this->ActiveCamera());
	this->ActiveCamera().AddToSceneManager();

	AmbientLightSourcePtr ambient_light = MakeSharedPtr<AmbientLightSource>();
	ambient_light->SkylightTex(y_cube_, c_cube_);
	ambient_light->Color(float3(0.1f, 0.1f, 0.1f));
	ambient_light->AddToSceneManager();

	point_light_ = MakeSharedPtr<PointLightSource>();
	point_light_->Attrib(LightSource::LSA_NoShadow);
	point_light_->Color(float3(1, 1, 1));
	point_light_->Position(float3(0, 3, -2));
	point_light_->Falloff(float3(1, 0, 0.3f));
	point_light_->AddToSceneManager();

	SceneObjectPtr scene_obj = MakeSharedPtr<SceneObjectHelper>(dino_model, SceneObject::SOA_Cullable);
	scene_obj->ModelMatrix(MathLib::scaling(float3(2, 2, 2)) * MathLib::translation(0.0f, 1.0f, -2.5f));
	scene_obj->AddToSceneManager();

	deferred_rendering_ = Context::Instance().DeferredRenderingLayerInstance();

	font_ = SyncLoadFont("gkai00mp.kfont");

	sky_box_ = MakeSharedPtr<SceneObjectSkyBox>();
	sky_box_->AddToSceneManager();

	back_refl_fb_ = rf.MakeFrameBuffer();

	InputEngine& inputEngine(Context::Instance().InputFactoryInstance().InputEngineInstance());
	InputActionMap actionMap;
	actionMap.AddActions(actions, actions + std::size(actions));

	action_handler_t input_handler = MakeSharedPtr<input_signal>();
	input_handler->connect(
		[this](InputEngine const & sender, InputAction const & action)
		{
			this->InputHandler(sender, action);
		});
	inputEngine.ActionMap(actionMap, input_handler);

	UIManager::Instance().Load(ResLoader::Instance().Open("Reflection.uiml"));
	parameter_dialog_ = UIManager::Instance().GetDialog("Reflection");

	id_min_sample_num_static_ = parameter_dialog_->IDFromName("min_sample_num_static");
	id_min_sample_num_slider_ = parameter_dialog_->IDFromName("min_sample_num_slider");
	parameter_dialog_->Control<UISlider>(id_min_sample_num_slider_)->OnValueChangedEvent().connect(
		[this](UISlider const & sender)
		{
			this->MinSampleNumHandler(sender);
		});
	this->MinSampleNumHandler(*(parameter_dialog_->Control<UISlider>(id_min_sample_num_slider_)));

	id_max_sample_num_static_ = parameter_dialog_->IDFromName("max_sample_num_static");
	id_max_sample_num_slider_ = parameter_dialog_->IDFromName("max_sample_num_slider");
	parameter_dialog_->Control<UISlider>(id_max_sample_num_slider_)->OnValueChangedEvent().connect(
		[this](UISlider const & sender)
		{
			this->MaxSampleNumHandler(sender);
		});
	this->MaxSampleNumHandler(*(parameter_dialog_->Control<UISlider>(id_max_sample_num_slider_)));

	id_enable_reflection_ = parameter_dialog_->IDFromName("enable_reflection");
	parameter_dialog_->Control<UICheckBox>(id_enable_reflection_)->OnChangedEvent().connect(
		[this](UICheckBox const & sender)
		{
			this->EnbleReflectionHandler(sender);
		});
	this->EnbleReflectionHandler(*(parameter_dialog_->Control<UICheckBox>(id_enable_reflection_)));
}

void ScreenSpaceReflectionApp::OnResize(KlayGE::uint32_t width, KlayGE::uint32_t height)
{
	App3DFramework::OnResize(width, height);

	UIManager::Instance().SettleCtrls();

	RenderFactory& rf = Context::Instance().RenderFactoryInstance();
	RenderEngine& re = rf.RenderEngineInstance();

	deferred_rendering_->SetupViewport(1, re.CurFrameBuffer(), VPAM_NoSSVO | VPAM_NoGI);
	
	back_refl_tex_ = rf.MakeTexture2D(width / 2, width / 2, 1, 1, 
		deferred_rendering_->ShadingTex(1)->Format(), 1, 0, EAH_GPU_Read | EAH_GPU_Write);
	back_refl_ds_tex_ = rf.MakeTexture2D(width / 2, width / 2, 1, 1, 
		EF_D16, 1, 0, EAH_GPU_Read | EAH_GPU_Write);
	back_refl_fb_->Attach(FrameBuffer::ATT_Color0, rf.Make2DRenderView(*back_refl_tex_, 0, 1, 0));
	back_refl_fb_->Attach(FrameBuffer::ATT_DepthStencil, rf.Make2DDepthStencilRenderView(*back_refl_ds_tex_, 0, 1, 0));

	deferred_rendering_->SetupViewport(0, back_refl_fb_, VPAM_NoTransparencyBack | VPAM_NoTransparencyFront | VPAM_NoSimpleForward | VPAM_NoGI | VPAM_NoSSVO);

	screen_camera_ = re.CurFrameBuffer()->GetViewport()->camera;
}

void ScreenSpaceReflectionApp::InputHandler(KlayGE::InputEngine const & /*sender*/, KlayGE::InputAction const & action)
{
	switch (action.first)
	{
	case Exit:
		this->Quit();
		break;
	}
}

void ScreenSpaceReflectionApp::MinSampleNumHandler(KlayGE::UISlider const & sender)
{
	int32_t sample_num = sender.GetValue();
	if (teapot_)
	{
		checked_pointer_cast<ReflectMesh>(teapot_->GetRenderable())->MinSamples(sample_num);

		std::wostringstream oss;
		oss << "Min Samples: " << sample_num;
		parameter_dialog_->Control<UIStatic>(id_min_sample_num_static_)->SetText(oss.str());
	}
}

void ScreenSpaceReflectionApp::MaxSampleNumHandler(KlayGE::UISlider const & sender)
{
	int32_t sample_num = sender.GetValue();
	if (teapot_)
	{
		checked_pointer_cast<ReflectMesh>(teapot_->GetRenderable())->MaxSamples(sample_num);

		std::wostringstream oss;
		oss << "Max Samples: " << sample_num;
		parameter_dialog_->Control<UIStatic>(id_max_sample_num_static_)->SetText(oss.str());
	}
}

void ScreenSpaceReflectionApp::EnbleReflectionHandler(KlayGE::UICheckBox const & sender)
{
	bool enabled = sender.GetChecked();
	if (teapot_)
	{
		checked_pointer_cast<ReflectMesh>(teapot_->GetRenderable())->EnbleReflection(enabled);
	}
}

void ScreenSpaceReflectionApp::DoUpdateOverlay()
{
	UIManager::Instance().Render();

	font_->RenderText(0, 0, Color(1, 1, 0, 1), L"Reflection", 16);

	std::wostringstream stream;
	stream.precision(2);
	stream << std::fixed << this->FPS() << " FPS";
	font_->RenderText(0, 18, Color(1, 1, 0, 1), stream.str(), 16);
}

uint32_t ScreenSpaceReflectionApp::DoUpdate(KlayGE::uint32_t pass)
{
	uint32_t urt;

	if (0 == pass)
	{
		if (loading_percentage_ < 100)
		{
			if (loading_percentage_ < 30)
			{
				if (teapot_model_->HWResourceReady())
				{
					teapot_ = MakeSharedPtr<SceneObjectHelper>(teapot_model_->Subrenderable(0), SceneObjectHelper::SOA_Cullable);
					teapot_->ModelMatrix(MathLib::scaling(float3(15, 15, 15)));
					teapot_->AddToSceneManager();

					this->MinSampleNumHandler(*(parameter_dialog_->Control<UISlider>(id_min_sample_num_slider_)));
					this->MaxSampleNumHandler(*(parameter_dialog_->Control<UISlider>(id_max_sample_num_slider_)));
					this->EnbleReflectionHandler(*(parameter_dialog_->Control<UICheckBox>(id_enable_reflection_)));

					loading_percentage_ = 30;
				}
			}
			else if (loading_percentage_ < 40)
			{
				if (y_cube_->HWResourceReady() && c_cube_->HWResourceReady())
				{
					checked_pointer_cast<SceneObjectSkyBox>(sky_box_)->CompressedCubeMap(y_cube_, c_cube_);
					checked_pointer_cast<ReflectMesh>(teapot_->GetRenderable())->SkyBox(y_cube_, c_cube_);

					loading_percentage_ = 100;
				}
			}
		}
		else
		{
			CameraPtr const & back_camera = back_refl_fb_->GetViewport()->camera;

			float3 eye = screen_camera_->EyePos();
			float3 at = screen_camera_->LookAt();

			float3 center = MathLib::transform_coord(teapot_->GetRenderable()->PosBound().Center(), teapot_->ModelMatrix());
			float3 direction = eye - at;

			back_camera->ViewParams(center, center + direction, screen_camera_->UpVec());
			back_camera->ProjParams(PI / 2, 1, screen_camera_->NearPlane(), screen_camera_->FarPlane());

			checked_pointer_cast<ReflectMesh>(teapot_->GetRenderable())->BackCamera(back_camera);

			checked_pointer_cast<ReflectMesh>(teapot_->GetRenderable())->FrontReflectionTex(deferred_rendering_->PrevFrameShadingTex(1));
			checked_pointer_cast<ReflectMesh>(teapot_->GetRenderable())->FrontReflectionDepthTex(deferred_rendering_->PrevFrameDepthTex(1));
			checked_pointer_cast<ReflectMesh>(teapot_->GetRenderable())->BackReflectionTex(deferred_rendering_->CurrFrameShadingTex(0));
			checked_pointer_cast<ReflectMesh>(teapot_->GetRenderable())->BackReflectionDepthTex(deferred_rendering_->CurrFrameDepthTex(0));
		}
	}

	urt = deferred_rendering_->Update(pass);

	if (teapot_)
	{
		if (0 == deferred_rendering_->ActiveViewport())
		{
			teapot_->Visible(false);
		}
		else
		{
			teapot_->Visible(true);
		}
	}

	return urt;
}
