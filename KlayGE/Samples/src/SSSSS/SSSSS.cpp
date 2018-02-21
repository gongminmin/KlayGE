#include <KlayGE/KlayGE.hpp>
#include <KFL/CXX17/iterator.hpp>
#include <KFL/Util.hpp>
#include <KFL/Math.hpp>
#include <KlayGE/GraphicsBuffer.hpp>
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
#include <KlayGE/Texture.hpp>
#include <KlayGE/SceneObjectHelper.hpp>
#include <KlayGE/PostProcess.hpp>
#include <KlayGE/Light.hpp>
#include <KlayGE/Camera.hpp>
#include <KlayGE/UI.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/InputFactory.hpp>
#include <KlayGE/SSSBlur.hpp>
#include <KlayGE/DeferredRenderingLayer.hpp>

#include <vector>
#include <sstream>

#include "SampleCommon.hpp"
#include "SSSSS.hpp"

using namespace KlayGE;

namespace
{
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

	SSSSSApp app;
	app.Create();
	app.Run();

	return 0;
}

SSSSSApp::SSSSSApp()
	: App3DFramework("SSSSS"),
		obj_controller_(true, MB_Left, MB_Middle, 0),
		light_controller_(true, MB_Right, 0, 0)
{
	ResLoader::Instance().AddPath("../../Samples/media/SSSSS");
}

void SSSSSApp::OnCreate()
{
	RenderablePtr scene_model = ASyncLoadModel("ScifiRoom.meshml",
		EAH_GPU_Read | EAH_Immutable);
	RenderablePtr sss_model = ASyncLoadModel("Infinite-Level_02.meshml",
		EAH_GPU_Read | EAH_Immutable);
	TexturePtr c_cube = ASyncLoadTexture("Lake_CraterLake03_filtered_c.dds",
		EAH_GPU_Read | EAH_Immutable);
	TexturePtr y_cube = ASyncLoadTexture("Lake_CraterLake03_filtered_y.dds",
		EAH_GPU_Read | EAH_Immutable);

	font_ = SyncLoadFont("gkai00mp.kfont");

	deferred_rendering_ = Context::Instance().DeferredRenderingLayerInstance();
	deferred_rendering_->SSVOEnabled(0, false);
	  
	this->LookAt(float3(0.5f, 5, -0.5f), float3(0, 5, 0));
	this->Proj(0.05f, 200.0f);

	AmbientLightSourcePtr ambient_light = MakeSharedPtr<AmbientLightSource>();
	ambient_light->SkylightTex(y_cube, c_cube);
	ambient_light->Color(float3(0.3f, 0.3f, 0.3f));
	ambient_light->AddToSceneManager();
	
	light_ = MakeSharedPtr<SpotLightSource>();
	light_->Attrib(0);
	light_->Color(float3(5.0f, 5.0f, 5.0f));
	light_->Falloff(float3(1, 1, 0));
	light_->Position(float3(0, 0, -2));
	light_->Direction(float3(0, 0, 1));
	light_->OuterAngle(PI / 6);
	light_->InnerAngle(PI / 8);
	light_->AddToSceneManager();

	light_proxy_ = MakeSharedPtr<SceneObjectLightSourceProxy>(light_);
	checked_pointer_cast<SceneObjectLightSourceProxy>(light_proxy_)->Scaling(0.1f, 0.1f, 0.1f);
	light_proxy_->AddToSceneManager();

	RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();

	scene_camera_ = re.DefaultFrameBuffer()->GetViewport()->camera;

	obj_controller_.AttachCamera(*scene_camera_);
	obj_controller_.Scalers(0.01f, 0.005f);

	light_camera_ = MakeSharedPtr<Camera>();
	light_controller_.AttachCamera(*light_camera_);
	light_controller_.Scalers(0.01f, 0.005f);
	light_camera_->ViewParams(light_->SMCamera(0)->EyePos(), light_->SMCamera(0)->LookAt(), light_->SMCamera(0)->UpVec());
	light_camera_->ProjParams(light_->SMCamera(0)->FOV(), light_->SMCamera(0)->Aspect(), light_->SMCamera(0)->NearPlane(), light_->SMCamera(0)->FarPlane());

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

	UIManager::Instance().Load(ResLoader::Instance().Open("SSSSS.uiml"));
	dialog_params_ = UIManager::Instance().GetDialog("Parameters");
	id_sss_ = dialog_params_->IDFromName("SSS");
	id_sss_strength_static_ = dialog_params_->IDFromName("SSSStrengthStatic");
	id_sss_strength_slider_ = dialog_params_->IDFromName("SSSStrengthSlider");
	id_sss_correction_static_ = dialog_params_->IDFromName("SSSCorrectionStatic");
	id_sss_correction_slider_ = dialog_params_->IDFromName("SSSCorrectionSlider");
	id_translucency_ = dialog_params_->IDFromName("Translucency");
	id_translucency_strength_static_ = dialog_params_->IDFromName("TranslucencyStrengthStatic");
	id_translucency_strength_slider_ = dialog_params_->IDFromName("TranslucencyStrengthSlider");

	dialog_params_->Control<UICheckBox>(id_sss_)->OnChangedEvent().connect(
		[this](UICheckBox const & sender)
		{
			this->SSSHandler(sender);
		});
	this->SSSHandler(*dialog_params_->Control<UICheckBox>(id_sss_));
	dialog_params_->Control<UISlider>(id_sss_strength_slider_)->OnValueChangedEvent().connect(
		[this](UISlider const & sender)
		{
			this->SSSStrengthChangedHandler(sender);
		});
	this->SSSStrengthChangedHandler(*dialog_params_->Control<UISlider>(id_sss_strength_slider_));
	dialog_params_->Control<UISlider>(id_sss_correction_slider_)->OnValueChangedEvent().connect(
		[this](UISlider const & sender)
		{
			this->SSSCorrectionChangedHandler(sender);
		});
	this->SSSCorrectionChangedHandler(*dialog_params_->Control<UISlider>(id_sss_correction_slider_));
	dialog_params_->Control<UICheckBox>(id_translucency_)->OnChangedEvent().connect(
		[this](UICheckBox const & sender)
		{
			this->TranslucencyHandler(sender);
		});
	this->TranslucencyHandler(*dialog_params_->Control<UICheckBox>(id_translucency_));
	dialog_params_->Control<UISlider>(id_translucency_strength_slider_)->OnValueChangedEvent().connect(
		[this](UISlider const & sender)
		{
			this->TranslucencyStrengthChangedHandler(sender);
		});
	this->TranslucencyStrengthChangedHandler(*dialog_params_->Control<UISlider>(id_translucency_strength_slider_));

	SceneObjectPtr subsurface_obj = MakeSharedPtr<SceneObjectHelper>(sss_model, SceneObject::SOA_Cullable);
	subsurface_obj->ModelMatrix(MathLib::translation(0.0f, 5.0f, 0.0f));
	subsurface_obj->AddToSceneManager();

	SceneObjectPtr scene_obj = MakeSharedPtr<SceneObjectHelper>(scene_model, SceneObject::SOA_Cullable);
	scene_obj->AddToSceneManager();

	SceneObjectSkyBoxPtr sky_box = MakeSharedPtr<SceneObjectSkyBox>();
	sky_box->CompressedCubeMap(y_cube, c_cube);
	sky_box->AddToSceneManager();
}

void SSSSSApp::OnResize(uint32_t width, uint32_t height)
{
	App3DFramework::OnResize(width, height);

	RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
	deferred_rendering_->SetupViewport(0, re.CurFrameBuffer(), 0);

	UIManager::Instance().SettleCtrls();
}

void SSSSSApp::InputHandler(InputEngine const & /*sender*/, InputAction const & action)
{
	switch (action.first)
	{
	case Exit:
		this->Quit();
		break;
	}
}

void SSSSSApp::SSSHandler(KlayGE::UICheckBox const & sender)
{
	deferred_rendering_->SSSEnabled(sender.GetChecked());
}

void SSSSSApp::SSSStrengthChangedHandler(KlayGE::UISlider const & sender)
{
	float strength = sender.GetValue() * 0.1f;
	deferred_rendering_->SSSStrength(strength);

	std::wostringstream stream;
	stream << L"SSS Strength: " << strength;
	dialog_params_->Control<UIStatic>(id_sss_strength_static_)->SetText(stream.str());
}

void SSSSSApp::SSSCorrectionChangedHandler(KlayGE::UISlider const & sender)
{
	float correction = sender.GetValue() * 0.1f;
	deferred_rendering_->SSSCorrection(correction);

	std::wostringstream stream;
	stream << L"SSS Correction: " << correction;
	dialog_params_->Control<UIStatic>(id_sss_correction_static_)->SetText(stream.str());
}

void SSSSSApp::TranslucencyHandler(KlayGE::UICheckBox const & sender)
{
	deferred_rendering_->TranslucencyEnabled(sender.GetChecked());
}

void SSSSSApp::TranslucencyStrengthChangedHandler(KlayGE::UISlider const & sender)
{
	float strength = static_cast<float>(sender.GetValue());
	deferred_rendering_->TranslucencyStrength(strength);

	std::wostringstream stream;
	stream << L"Translucency Strength: " << strength;
	dialog_params_->Control<UIStatic>(id_translucency_strength_static_)->SetText(stream.str());
}

void SSSSSApp::DoUpdateOverlay()
{
	UIManager::Instance().Render();

	std::wostringstream stream;
	stream.precision(2);
	stream << std::fixed << this->FPS() << " FPS";
 
	font_->RenderText(0, 0, Color(1, 1, 0, 1), L"Screen Space Sub Surface Scattering", 16);
	font_->RenderText(0, 18, Color(1, 1, 0, 1), stream.str(), 16);
}

uint32_t SSSSSApp::DoUpdate(uint32_t pass)
{
	if (0 == pass)
	{
		light_->Position(float3(0, 5, 0) - light_camera_->ForwardVec() * 2.0f);
		light_->Direction(light_camera_->ForwardVec());
	}

	return deferred_rendering_->Update(pass);
}
