#include <KlayGE/KlayGE.hpp>
#include <KFL/CXX17/iterator.hpp>
#include <KFL/Util.hpp>
#include <KFL/Math.hpp>
#include <KlayGE/Font.hpp>
#include <KlayGE/SceneNode.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/FrameBuffer.hpp>
#include <KlayGE/SceneManager.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/ResLoader.hpp>
#include <KlayGE/RenderSettings.hpp>
#include <KlayGE/Mesh.hpp>
#include <KlayGE/RenderEffect.hpp>
#include <KlayGE/Window.hpp>
#include <KlayGE/Light.hpp>
#include <KlayGE/Camera.hpp>

#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/InputFactory.hpp>

#include <sstream>

#include "SampleCommon.hpp"
#include "SubSurface.hpp"

using namespace KlayGE;
using namespace std;

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
	SubSurfaceApp app;
	app.Create();
	app.Run();

	return 0;
}

SubSurfaceApp::SubSurfaceApp()
					: App3DFramework("SubSurface")
{
	ResLoader::Instance().AddPath("../../Samples/media/SubSurface");
}

void SubSurfaceApp::OnCreate()
{
	font_ = SyncLoadFont("gkai00mp.kfont");

	model_ = SyncLoadModel("Dragon.glb", EAH_GPU_Read | EAH_Immutable,
		SceneNode::SOA_Cullable, AddToSceneRootHelper,
		CreateModelFactory<RenderModel>, CreateMeshFactory<DetailedMesh>);

	this->LookAt(float3(-0.4f, 1, 3.9f), float3(0, 1, 0), float3(0.0f, 1.0f, 0.0f));
	this->Proj(0.1f, 200.0f);

	tbController_.AttachCamera(this->ActiveCamera());
	tbController_.Scalers(0.01f, 0.01f);

	light_ = MakeSharedPtr<PointLightSource>();
	light_->Attrib(0);
	light_->Color(float3(1.5f, 1.5f, 1.5f));
	light_->Falloff(float3(1, 0.5f, 0.0f));

	auto light_proxy = LoadLightSourceProxyModel(light_);
	light_proxy->RootNode()->TransformToParent(MathLib::scaling(0.05f, 0.05f, 0.05f) * light_proxy->RootNode()->TransformToParent());

	auto light_node = MakeSharedPtr<SceneNode>(SceneNode::SOA_Cullable);
	light_node->TransformToParent(MathLib::translation(0.0f, 2.0f, -3.0f));
	light_node->AddComponent(light_);
	light_node->AddChild(light_proxy->RootNode());
	Context::Instance().SceneManagerInstance().SceneRootNode().AddChild(light_node);

	InputEngine& inputEngine(Context::Instance().InputFactoryInstance().InputEngineInstance());
	InputActionMap actionMap;
	actionMap.AddActions(actions, actions + std::size(actions));

	action_handler_t input_handler = MakeSharedPtr<input_signal>();
	input_handler->Connect(
		[this](InputEngine const & sender, InputAction const & action)
		{
			this->InputHandler(sender, action);
		});
	inputEngine.ActionMap(actionMap, input_handler);

	UIManager::Instance().Load(ResLoader::Instance().Open("SubSurface.uiml"));
	dialog_params_ = UIManager::Instance().GetDialog("Parameters");
	id_sigma_static_ = dialog_params_->IDFromName("SigmaStatic");
	id_sigma_slider_ = dialog_params_->IDFromName("SigmaSlider");
	id_mtl_thickness_static_ = dialog_params_->IDFromName("MtlThicknessStatic");
	id_mtl_thickness_slider_ = dialog_params_->IDFromName("MtlThicknessSlider");

	dialog_params_->Control<UISlider>(id_sigma_slider_)->OnValueChangedEvent().Connect(
		[this](UISlider const & sender)
		{
			this->SigmaChangedHandler(sender);
		});
	this->SigmaChangedHandler(*dialog_params_->Control<UISlider>(id_sigma_slider_));

	dialog_params_->Control<UISlider>(id_mtl_thickness_slider_)->OnValueChangedEvent().Connect(
		[this](UISlider const & sender)
		{
			this->MtlThicknessChangedHandler(sender);
		});
	this->MtlThicknessChangedHandler(*dialog_params_->Control<UISlider>(id_mtl_thickness_slider_));

	RenderFactory& rf = Context::Instance().RenderFactoryInstance();
	RenderEngine& re = rf.RenderEngineInstance();
	RenderDeviceCaps const & caps = re.DeviceCaps();
	depth_texture_support_ = caps.depth_texture_support;

	back_face_depth_fb_ = rf.MakeFrameBuffer();
	FrameBufferPtr screen_buffer = rf.RenderEngineInstance().CurFrameBuffer();
	back_face_depth_fb_->Viewport()->Camera(screen_buffer->Viewport()->Camera());
}

void SubSurfaceApp::OnResize(uint32_t width, uint32_t height)
{
	App3DFramework::OnResize(width, height);

	RenderFactory& rf = Context::Instance().RenderFactoryInstance();
	RenderEngine& re = rf.RenderEngineInstance();
	RenderDeviceCaps const & caps = re.DeviceCaps();

	KlayGE::TexturePtr back_face_depth_tex;
	KlayGE::TexturePtr back_face_ds_tex;
	KlayGE::DepthStencilViewPtr back_face_ds_view;
	ElementFormat fmt;
	if (depth_texture_support_)
	{
		fmt = caps.BestMatchTextureRenderTargetFormat(MakeSpan({EF_ABGR8, EF_ARGB8}), 1, 0);
		BOOST_ASSERT(fmt != EF_Unknown);

		// Just dummy
		back_face_depth_tex = rf.MakeTexture2D(width, height, 1, 1, fmt, 1, 0, EAH_GPU_Read | EAH_GPU_Write);

		fmt = caps.BestMatchTextureRenderTargetFormat(MakeSpan({EF_D24S8, EF_D16}), 1, 0);
		BOOST_ASSERT(fmt != EF_Unknown);

		float4 constexpr back_face_ds_clear_value(0, 0, 0, 0);
		back_face_ds_tex = rf.MakeTexture2D(width, height, 1, 1, fmt, 1, 0, EAH_GPU_Read | EAH_GPU_Write, {}, &back_face_ds_clear_value);
		back_face_ds_view = rf.Make2DDsv(back_face_ds_tex, 0, 1, 0);

		model_->ForEachMesh([back_face_ds_tex](Renderable& mesh)
			{
				checked_cast<DetailedMesh&>(mesh).BackFaceDepthTex(back_face_ds_tex);
			});
	}
	else
	{
		if (caps.pack_to_rgba_required)
		{
			fmt = caps.BestMatchTextureRenderTargetFormat(MakeSpan({EF_ABGR8, EF_ARGB8}), 1, 0);
			BOOST_ASSERT(fmt != EF_Unknown);
		}
		else
		{
			fmt = EF_R16F;
		}
		back_face_depth_tex = rf.MakeTexture2D(width, height, 1, 1, fmt, 1, 0, EAH_GPU_Read | EAH_GPU_Write);
		back_face_ds_view = rf.Make2DDsv(width, height, EF_D16, 1, 0);

		model_->ForEachMesh([back_face_depth_tex](Renderable& mesh)
			{
				checked_cast<DetailedMesh&>(mesh).BackFaceDepthTex(back_face_depth_tex);
			});
	}

	back_face_depth_fb_->Attach(FrameBuffer::Attachment::Color0, rf.Make2DRtv(back_face_depth_tex, 0, 1, 0));
	back_face_depth_fb_->Attach(back_face_ds_view);

	UIManager::Instance().SettleCtrls();
}

void SubSurfaceApp::InputHandler(InputEngine const & /*sender*/, InputAction const & action)
{
	switch (action.first)
	{
	case Exit:
		this->Quit();
		break;
	}
}

void SubSurfaceApp::SigmaChangedHandler(KlayGE::UISlider const & sender)
{
	float sigma_t = sender.GetValue() * 0.2f;
	model_->ForEachMesh([sigma_t](Renderable& mesh)
		{
			checked_cast<DetailedMesh&>(mesh).SigmaT(sigma_t);
		});

	std::wostringstream stream;
	stream << L"Sigma_t: " << sigma_t;
	dialog_params_->Control<UIStatic>(id_sigma_static_)->SetText(stream.str());
}

void SubSurfaceApp::MtlThicknessChangedHandler(KlayGE::UISlider const & sender)
{
	float mtl_thickness = sender.GetValue() * 0.1f;
	model_->ForEachMesh([mtl_thickness](Renderable& mesh)
		{
			checked_cast<DetailedMesh&>(mesh).MtlThickness(mtl_thickness);
		});

	std::wostringstream stream;
	stream << L"Material thickness: " << mtl_thickness;
	dialog_params_->Control<UIStatic>(id_mtl_thickness_static_)->SetText(stream.str());
}

void SubSurfaceApp::DoUpdateOverlay()
{
	RenderEngine& renderEngine(Context::Instance().RenderFactoryInstance().RenderEngineInstance());

	UIManager::Instance().Render();

	std::wostringstream stream;
	stream.precision(2);
	stream << std::fixed << this->FPS() << " FPS";

	font_->RenderText(0, 0, Color(1, 1, 0, 1), renderEngine.Name(), 16);
	font_->RenderText(0, 18, Color(1, 1, 0, 1), renderEngine.ScreenFrameBuffer()->Description(), 16);
	font_->RenderText(0, 36, Color(1, 1, 0, 1), stream.str(), 16);
}

uint32_t SubSurfaceApp::DoUpdate(KlayGE::uint32_t pass)
{
	RenderEngine& renderEngine(Context::Instance().RenderFactoryInstance().RenderEngineInstance());

	switch (pass)
	{
	case 0:
		renderEngine.BindFrameBuffer(back_face_depth_fb_);
		renderEngine.CurFrameBuffer()->Clear(FrameBuffer::CBM_Color | FrameBuffer::CBM_Depth, Color(0, 0, 0, 0), 0.0f, 0);
		model_->ForEachMesh([](Renderable& mesh)
			{
				checked_cast<DetailedMesh&>(mesh).BackFaceDepthPass(true);
			});
		return App3DFramework::URV_NeedFlush;

	default:
		renderEngine.BindFrameBuffer(FrameBufferPtr());
		Color clear_clr(0.2f, 0.4f, 0.6f, 1);
		if (Context::Instance().Config().graphics_cfg.gamma)
		{
			clear_clr.r() = 0.029f;
			clear_clr.g() = 0.133f;
			clear_clr.b() = 0.325f;
		}
		renderEngine.CurFrameBuffer()->Clear(FrameBuffer::CBM_Color | FrameBuffer::CBM_Depth, clear_clr, 1.0f, 0);

		model_->ForEachMesh([this](Renderable& mesh)
			{
				auto& detailed_mesh = checked_cast<DetailedMesh&>(mesh);

				detailed_mesh.LightPos(light_->Position());
				detailed_mesh.LightColor(light_->Color());
				detailed_mesh.LightFalloff(light_->Falloff());
				detailed_mesh.EyePos(this->ActiveCamera().EyePos());
				detailed_mesh.BackFaceDepthPass(false);
			});

		return App3DFramework::URV_NeedFlush | App3DFramework::URV_Finished;
	}
}
