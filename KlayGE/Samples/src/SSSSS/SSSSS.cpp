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

int SampleMain()
{
	MyAppFramework app;
	app.Create();
	app.Run();

	return 0;
}

MyAppFramework::MyAppFramework()
	: App3DFramework("SSSSS"),
		obj_controller_(true, MB_Left, MB_Middle, 0),
		light_controller_(true, MB_Right, 0, 0)
{
	ResLoader::Instance().AddPath("../../Samples/media/SSSSS");
}

void MyAppFramework::InitObjects()
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

	subsurfaceObject_ = MakeSharedPtr<MySceneObjectHelper>("Infinite-Level_02.meshml");
	subsurfaceObject_->AddToSceneManager();

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

	depth_in_ls_tex_ = rf.MakeTexture2D(SHADOW_MAP_SIZE, SHADOW_MAP_SIZE, 1, 1, EF_R32F, 1, 0, EAH_GPU_Read | EAH_GPU_Write, nullptr);
	depth_in_ls_ds_tex_ = rf.MakeTexture2D(SHADOW_MAP_SIZE, SHADOW_MAP_SIZE, 1, 1, ds_fmt, 1, 0, EAH_GPU_Read | EAH_GPU_Write, nullptr);

	depth_ls_fb_ = rf.MakeFrameBuffer();
	depth_ls_fb_->Attach(FrameBuffer::ATT_Color0, rf.Make2DRenderView(*depth_in_ls_tex_, 0, 1, 0));
	depth_ls_fb_->Attach(FrameBuffer::ATT_DepthStencil, rf.Make2DDepthStencilRenderView(*depth_in_ls_ds_tex_, 0, 1, 0));
	depth_ls_fb_->GetViewport()->camera = light_->SMCamera(0);

	scene_camera_ = re.DefaultFrameBuffer()->GetViewport()->camera;

	color_fbo_ = rf.MakeFrameBuffer();
	color_fbo_->GetViewport()->camera = scene_camera_;

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
}

void MyAppFramework::OnResize(uint32_t width, uint32_t height)
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

	sss_blurred_tex_ = rf.MakeTexture2D(width, height, 1, 1, EF_ABGR16F, 1, 0, EAH_GPU_Read | EAH_GPU_Write, nullptr);

	color_fbo_->Attach(FrameBuffer::ATT_Color0, rf.Make2DRenderView(*normal_tex_, 0, 1, 0));
	color_fbo_->Attach(FrameBuffer::ATT_Color1, rf.Make2DRenderView(*albedo_tex_, 0, 1, 0));
	color_fbo_->Attach(FrameBuffer::ATT_Color2, rf.Make2DRenderView(*shading_tex_, 0, 1, 0));
	color_fbo_->Attach(FrameBuffer::ATT_DepthStencil, rf.Make2DDepthStencilRenderView(*ds_tex_, 0, 1, 0));

	sss_blur_pp_->InputPin(0, shading_tex_);
	sss_blur_pp_->InputPin(1, depth_tex_);
	sss_blur_pp_->OutputPin(0, sss_blurred_tex_);

	translucency_pp_->InputPin(0, normal_tex_);
	translucency_pp_->InputPin(1, albedo_tex_);
	translucency_pp_->InputPin(2, sss_blurred_tex_);
	translucency_pp_->InputPin(3, depth_tex_);
	translucency_pp_->InputPin(4, depth_in_ls_tex_);
	translucency_pp_->SetParam(3, float3(light_->Color()));
	translucency_pp_->SetParam(4, 50.0f);

	UIManager::Instance().SettleCtrls();
}

void MyAppFramework::DoUpdateOverlay()
{
	std::wostringstream stream;
	stream.precision(2);
	stream << std::fixed << this->FPS() << " FPS";
 
	font_->RenderText(0, 0, Color(1, 1, 0, 1), L"Screen Space Sub Surface Scattering", 16);
	font_->RenderText(0, 18, Color(1, 1, 0, 1), stream.str(), 16);
}

uint32_t MyAppFramework::DoUpdate(uint32_t pass)
{
	RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
 
	switch (pass)
	{
	case 0:
		light_proxy_->Visible(false);
		subsurfaceObject_->Visible(true);
		checked_pointer_cast<SubsurfaceMesh>(subsurfaceObject_->GetRenderable())->Pass(PT_GenShadowMap);

		light_->Position(-light_camera_->ForwardVec() * 2.0f);
		light_->Direction(light_camera_->ForwardVec());

		re.BindFrameBuffer(depth_ls_fb_);
		re.CurFrameBuffer()->Clear(FrameBuffer::CBM_Color | FrameBuffer::CBM_Depth,
			Color(0.0f, 0.0f, 0.0f, 0.0f), 1.0f, 0);
		return App3DFramework::URV_Need_Flush;
	
	case 1:
		light_proxy_->Visible(false);
		subsurfaceObject_->Visible(true);
		checked_pointer_cast<SubsurfaceMesh>(subsurfaceObject_->GetRenderable())->Pass(PT_OpaqueShading);

		{
			float q = light_->SMCamera(0)->FarPlane() / (light_->SMCamera(0)->FarPlane() - light_->SMCamera(0)->NearPlane());
			float2 near_q(light_->SMCamera(0)->NearPlane() * q, q);
			depth_to_linear_pp_->SetParam(0, near_q);
			depth_to_linear_pp_->InputPin(0, depth_in_ls_ds_tex_);
			depth_to_linear_pp_->OutputPin(0, depth_in_ls_tex_);
			depth_to_linear_pp_->Apply();
		}

		re.BindFrameBuffer(color_fbo_);
		checked_pointer_cast<MySceneObjectHelper>(subsurfaceObject_)->LightPosition(light_->Position());
		checked_pointer_cast<MySceneObjectHelper>(subsurfaceObject_)->LightColor(light_->Color());
		checked_pointer_cast<MySceneObjectHelper>(subsurfaceObject_)->EyePosition(scene_camera_->EyePos());
		re.CurFrameBuffer()->Clear(FrameBuffer::CBM_Color | FrameBuffer::CBM_Depth,
			Color(0.0f, 0.0f, 0.0f, 0), 1.0f, 0);
		return App3DFramework::URV_Need_Flush;

	default:
		light_proxy_->Visible(true);
		subsurfaceObject_->Visible(false);

		{
			float q = scene_camera_->FarPlane() / (scene_camera_->FarPlane() - scene_camera_->NearPlane());
			float2 near_q(scene_camera_->NearPlane() * q, q);
			depth_to_linear_pp_->SetParam(0, near_q);
			depth_to_linear_pp_->InputPin(0, ds_tex_);
			depth_to_linear_pp_->OutputPin(0, depth_tex_);
			depth_to_linear_pp_->Apply();
		}

		sss_blur_pp_->SetParam(0, 1.0f);
		sss_blur_pp_->Apply();

		re.BindFrameBuffer(FrameBufferPtr());
		re.CurFrameBuffer()->Clear(FrameBuffer::CBM_Depth, Color(0.0f, 0.0f, 0.0f, 0), 1.0f, 0);

		translucency_pp_->SetParam(0, scene_camera_->InverseViewMatrix() * light_->SMCamera(0)->ViewProjMatrix());
		translucency_pp_->SetParam(1, scene_camera_->InverseProjMatrix());
		translucency_pp_->SetParam(2, MathLib::transform_coord(light_->Position(), scene_camera_->ViewMatrix()));
		translucency_pp_->Apply();

		return App3DFramework::URV_Need_Flush | App3DFramework::URV_Finished;
	}
}
