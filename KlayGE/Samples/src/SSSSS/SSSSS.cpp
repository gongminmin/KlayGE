#include <KlayGE/KlayGE.hpp>
#include <KFL/Util.hpp>
#include <KFL/ThrowErr.hpp>
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

#include <vector>
#include <sstream>

#include "SampleCommon.hpp"
#include "SSSSS.hpp"
#include "SubsurfaceMesh.hpp"
#include "SSSBlur.hpp"

uint32_t const SHADOW_MAP_SIZE = 512;

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

void SSSSSApp::InitObjects()
{
	font_ = SyncLoadFont("gkai00mp.kfont");
	  
	this->LookAt(float3(0.5f, 0, -0.5f), float3(0, 0, 0));
	this->Proj(0.05f, 20.0f);
	
	light_ = MakeSharedPtr<SpotLightSource>();
	light_->Attrib(0);
	light_->Color(float3(1.0f, 1.0f, 1.0f));
	light_->Falloff(float3(1, 0, 1));
	light_->Position(float3(0, 0, -2));
	light_->Direction(float3(0, 0, 1));
	light_->OuterAngle(PI / 12);
	light_->InnerAngle(PI / 16);
	light_->AddToSceneManager();

	light_proxy_ = MakeSharedPtr<SceneObjectLightSourceProxy>(light_);
	checked_pointer_cast<SceneObjectLightSourceProxy>(light_proxy_)->Scaling(0.1f, 0.1f, 0.1f);
	light_proxy_->AddToSceneManager();

	subsurface_obj_ = MakeSharedPtr<MySceneObjectHelper>("Infinite-Level_02.meshml");
	subsurface_obj_->AddToSceneManager();

	RenderFactory& rf = Context::Instance().RenderFactoryInstance();
	RenderEngine& re = rf.RenderEngineInstance();
	RenderDeviceCaps const & caps = re.DeviceCaps();
	ElementFormat ds_fmt;
	if (caps.rendertarget_format_support(EF_D24S8, 1, 0))
	{
		ds_fmt = EF_D24S8;
	}
	else
	{
		BOOST_ASSERT(caps.rendertarget_format_support(EF_D16, 1, 0));

		ds_fmt = EF_D16;
	}

	shadow_tex_ = rf.MakeTexture2D(SHADOW_MAP_SIZE, SHADOW_MAP_SIZE, 1, 1, EF_R32F, 1, 0, EAH_GPU_Read | EAH_GPU_Write, nullptr);
	shadow_ds_tex_ = rf.MakeTexture2D(SHADOW_MAP_SIZE, SHADOW_MAP_SIZE, 1, 1, ds_fmt, 1, 0, EAH_GPU_Read | EAH_GPU_Write, nullptr);

	depth_ls_fb_ = rf.MakeFrameBuffer();
	depth_ls_fb_->Attach(FrameBuffer::ATT_Color0, rf.Make2DRenderView(*shadow_tex_, 0, 1, 0));
	depth_ls_fb_->Attach(FrameBuffer::ATT_DepthStencil, rf.Make2DDepthStencilRenderView(*shadow_ds_tex_, 0, 1, 0));
	depth_ls_fb_->GetViewport()->camera = light_->SMCamera(0);

	scene_camera_ = re.DefaultFrameBuffer()->GetViewport()->camera;

	color_fb_ = rf.MakeFrameBuffer();
	color_fb_->GetViewport()->camera = scene_camera_;

	sss_fb_ = rf.MakeFrameBuffer();
	sss_fb_->GetViewport()->camera = scene_camera_;

	sss_blur_pp_ = MakeSharedPtr<SSSBlurPP>();
	translucency_pp_ = SyncLoadPostProcess("Translucency.ppml", "Translucency");

	obj_controller_.AttachCamera(*scene_camera_);
	obj_controller_.Scalers(0.01f, 0.005f);

	light_camera_ = MakeSharedPtr<Camera>();
	light_controller_.AttachCamera(*light_camera_);
	light_controller_.Scalers(0.01f, 0.005f);
	light_camera_->ViewParams(light_->SMCamera(0)->EyePos(), light_->SMCamera(0)->LookAt(), light_->SMCamera(0)->UpVec());
	light_camera_->ProjParams(light_->SMCamera(0)->FOV(), light_->SMCamera(0)->Aspect(), light_->SMCamera(0)->NearPlane(), light_->SMCamera(0)->FarPlane());

	depth_to_linear_pp_ = SyncLoadPostProcess("DepthToSM.ppml", "DepthToSM");
	copy_pp_ = SyncLoadPostProcess("Copy.ppml", "copy");

	InputEngine& inputEngine(Context::Instance().InputFactoryInstance().InputEngineInstance());
	InputActionMap actionMap;
	actionMap.AddActions(actions, actions + sizeof(actions) / sizeof(actions[0]));

	action_handler_t input_handler = MakeSharedPtr<input_signal>();
	input_handler->connect(KlayGE::bind(&SSSSSApp::InputHandler, this, KlayGE::placeholders::_1, KlayGE::placeholders::_2));
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

	dialog_params_->Control<UICheckBox>(id_sss_)->OnChangedEvent().connect(KlayGE::bind(&SSSSSApp::SSSHandler, this, KlayGE::placeholders::_1));
	this->SSSHandler(*dialog_params_->Control<UICheckBox>(id_sss_));
	dialog_params_->Control<UISlider>(id_sss_strength_slider_)->OnValueChangedEvent().connect(KlayGE::bind(&SSSSSApp::SSSStrengthChangedHandler, this, KlayGE::placeholders::_1));
	this->SSSStrengthChangedHandler(*dialog_params_->Control<UISlider>(id_sss_strength_slider_));
	dialog_params_->Control<UISlider>(id_sss_correction_slider_)->OnValueChangedEvent().connect(KlayGE::bind(&SSSSSApp::SSSCorrectionChangedHandler, this, KlayGE::placeholders::_1));
	this->SSSCorrectionChangedHandler(*dialog_params_->Control<UISlider>(id_sss_correction_slider_));
	dialog_params_->Control<UICheckBox>(id_translucency_)->OnChangedEvent().connect(KlayGE::bind(&SSSSSApp::TranslucencyHandler, this, KlayGE::placeholders::_1));
	this->TranslucencyHandler(*dialog_params_->Control<UICheckBox>(id_translucency_));
	dialog_params_->Control<UISlider>(id_translucency_strength_slider_)->OnValueChangedEvent().connect(KlayGE::bind(&SSSSSApp::TranslucencyStrengthChangedHandler, this, KlayGE::placeholders::_1));
	this->TranslucencyStrengthChangedHandler(*dialog_params_->Control<UISlider>(id_translucency_strength_slider_));
}

void SSSSSApp::OnResize(uint32_t width, uint32_t height)
{
	App3DFramework::OnResize(width, height);

	RenderFactory& rf = Context::Instance().RenderFactoryInstance();
	RenderEngine& re = rf.RenderEngineInstance();
	RenderDeviceCaps const & caps = re.DeviceCaps();
	ElementFormat ds_fmt;
	if (caps.rendertarget_format_support(EF_D24S8, 1, 0))
	{
		ds_fmt = EF_D24S8;
	}
	else
	{
		BOOST_ASSERT(caps.rendertarget_format_support(EF_D16, 1, 0));

		ds_fmt = EF_D16;
	}

	shading_tex_ = rf.MakeTexture2D(width, height, 1, 1, EF_ABGR16F, 1, 0, EAH_GPU_Read | EAH_GPU_Write, nullptr);
	normal_tex_ = rf.MakeTexture2D(width, height, 1, 1, EF_ABGR16F, 1, 0, EAH_GPU_Read | EAH_GPU_Write, nullptr);
	albedo_tex_ = rf.MakeTexture2D(width, height, 1, 1, EF_ABGR16F, 1, 0, EAH_GPU_Read | EAH_GPU_Write, nullptr);
	ds_tex_ = rf.MakeTexture2D(width, height, 1, 1, ds_fmt, 1, 0, EAH_GPU_Read | EAH_GPU_Write, nullptr);
	depth_tex_ = rf.MakeTexture2D(width, height, 1, 1, EF_R32F, 1, 0, EAH_GPU_Read | EAH_GPU_Write, nullptr);

	RenderViewPtr ds_view = rf.Make2DDepthStencilRenderView(*ds_tex_, 0, 1, 0);

	color_fb_->Attach(FrameBuffer::ATT_Color0, rf.Make2DRenderView(*normal_tex_, 0, 1, 0));
	color_fb_->Attach(FrameBuffer::ATT_Color1, rf.Make2DRenderView(*albedo_tex_, 0, 1, 0));
	color_fb_->Attach(FrameBuffer::ATT_Color2, rf.Make2DRenderView(*shading_tex_, 0, 1, 0));
	color_fb_->Attach(FrameBuffer::ATT_DepthStencil, ds_view);

	sss_fb_->Attach(FrameBuffer::ATT_Color0, rf.Make2DRenderView(*shading_tex_, 0, 1, 0));
	sss_fb_->Attach(FrameBuffer::ATT_DepthStencil, ds_view);

	sss_blur_pp_->OutputFrameBuffer()->Attach(FrameBuffer::ATT_DepthStencil, ds_view);
	sss_blur_pp_->InputPin(0, shading_tex_);
	sss_blur_pp_->InputPin(1, depth_tex_);
	sss_blur_pp_->OutputPin(0, shading_tex_);

	translucency_pp_->InputPin(0, normal_tex_);
	translucency_pp_->InputPin(1, albedo_tex_);
	translucency_pp_->InputPin(2, depth_tex_);
	translucency_pp_->InputPin(3, shadow_tex_);
	translucency_pp_->OutputPin(0, shading_tex_);
	translucency_pp_->OutputFrameBuffer()->Attach(FrameBuffer::ATT_DepthStencil, ds_view);
	translucency_pp_->SetParam(3, float3(light_->Color()));
	translucency_pp_->SetParam(4, 50.0f);

	copy_pp_->InputPin(0, shading_tex_);

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
	sss_on_ = sender.GetChecked();
}

void SSSSSApp::SSSStrengthChangedHandler(KlayGE::UISlider const & sender)
{
	float strength = sender.GetValue() * 0.1f;
	sss_blur_pp_->SetParam(0, strength);

	std::wostringstream stream;
	stream << L"SSS strength: " << strength;
	dialog_params_->Control<UIStatic>(id_sss_strength_static_)->SetText(stream.str());
}

void SSSSSApp::SSSCorrectionChangedHandler(KlayGE::UISlider const & sender)
{
	float correction = sender.GetValue() * 0.1f;
	sss_blur_pp_->SetParam(1, correction);

	std::wostringstream stream;
	stream << L"SSS Correction: " << correction;
	dialog_params_->Control<UIStatic>(id_sss_correction_static_)->SetText(stream.str());
}

void SSSSSApp::TranslucencyHandler(KlayGE::UICheckBox const & sender)
{
	translucency_on_ = sender.GetChecked();
}

void SSSSSApp::TranslucencyStrengthChangedHandler(KlayGE::UISlider const & sender)
{
	float strength = static_cast<float>(sender.GetValue());
	translucency_pp_->SetParam(4, strength);

	std::wostringstream stream;
	stream << L"Translucency strength: " << strength;
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
	RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
 
	switch (pass)
	{
	case 0:
		light_proxy_->Visible(false);
		subsurface_obj_->Visible(true);
		checked_pointer_cast<SubsurfaceMesh>(subsurface_obj_->GetRenderable())->Pass(PT_GenShadowMap);

		light_->Position(-light_camera_->ForwardVec() * 2.0f);
		light_->Direction(light_camera_->ForwardVec());

		re.BindFrameBuffer(depth_ls_fb_);
		re.CurFrameBuffer()->Clear(FrameBuffer::CBM_Color | FrameBuffer::CBM_Depth | FrameBuffer::CBM_Stencil,
			Color(0.0f, 0.0f, 0.0f, 0.0f), 1.0f, 0);
		return App3DFramework::URV_NeedFlush;
	
	case 1:
		light_proxy_->Visible(false);
		subsurface_obj_->Visible(true);
		checked_pointer_cast<SubsurfaceMesh>(subsurface_obj_->GetRenderable())->Pass(PT_OpaqueShading);

		{
			float q = light_->SMCamera(0)->FarPlane() / (light_->SMCamera(0)->FarPlane() - light_->SMCamera(0)->NearPlane());
			float2 near_q(light_->SMCamera(0)->NearPlane() * q, q);
			depth_to_linear_pp_->SetParam(0, near_q);
			depth_to_linear_pp_->InputPin(0, shadow_ds_tex_);
			depth_to_linear_pp_->OutputPin(0, shadow_tex_);
			depth_to_linear_pp_->Apply();
		}

		re.BindFrameBuffer(color_fb_);
		checked_pointer_cast<MySceneObjectHelper>(subsurface_obj_)->LightPosition(light_->Position());
		checked_pointer_cast<MySceneObjectHelper>(subsurface_obj_)->LightColor(light_->Color());
		checked_pointer_cast<MySceneObjectHelper>(subsurface_obj_)->EyePosition(scene_camera_->EyePos());
		re.CurFrameBuffer()->Clear(FrameBuffer::CBM_Color | FrameBuffer::CBM_Depth | FrameBuffer::CBM_Stencil,
			Color(0.0f, 0.0f, 0.0f, 0), 1.0f, 0);
		return App3DFramework::URV_NeedFlush;

	case 2:
		light_proxy_->Visible(true);
		subsurface_obj_->Visible(false);

		{
			float q = scene_camera_->FarPlane() / (scene_camera_->FarPlane() - scene_camera_->NearPlane());
			float2 near_q(scene_camera_->NearPlane() * q, q);
			depth_to_linear_pp_->SetParam(0, near_q);
			depth_to_linear_pp_->InputPin(0, ds_tex_);
			depth_to_linear_pp_->OutputPin(0, depth_tex_);
			depth_to_linear_pp_->Apply();
		}

		re.BindFrameBuffer(sss_fb_);

		if (sss_on_)
		{
			sss_blur_pp_->Apply();
		}

		if (translucency_on_)
		{
			translucency_pp_->SetParam(0, scene_camera_->InverseViewMatrix() * light_->SMCamera(0)->ViewProjMatrix());
			translucency_pp_->SetParam(1, scene_camera_->InverseProjMatrix());
			translucency_pp_->SetParam(2, MathLib::transform_coord(light_->Position(), scene_camera_->ViewMatrix()));
			translucency_pp_->Apply();
		}

		return App3DFramework::URV_NeedFlush;

	default:
		copy_pp_->Apply();
		return App3DFramework::URV_Finished;
	}
}
