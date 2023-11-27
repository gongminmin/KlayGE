#include <KlayGE/KlayGE.hpp>

#include <KFL/Math.hpp>
#include <KFL/Util.hpp>
#include <KlayGE/Camera.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/DeferredRenderingLayer.hpp>
#include <KlayGE/Font.hpp>
#include <KlayGE/FrameBuffer.hpp>
#include <KlayGE/InputFactory.hpp>
#include <KlayGE/Light.hpp>
#include <KlayGE/Mesh.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/RenderSettings.hpp>
#include <KlayGE/ResLoader.hpp>
#include <KlayGE/SceneManager.hpp>
#include <KlayGE/SceneNode.hpp>
#include <KlayGE/SkyBox.hpp>
#include <KlayGE/UI.hpp>

#include <sstream>

#include "SampleCommon.hpp"
#include "SkinnedMesh.hpp"

using namespace KlayGE;

namespace
{
	enum
	{
		Exit,
	};

	InputActionDefine actions[] = {
		InputActionDefine(Exit, KS_Escape),
	};
} // namespace

int SampleMain()
{
	ContextCfg cfg = Context::Instance().Config();
	cfg.deferred_rendering = true;
	Context::Instance().Config(cfg);

	SkinnedMeshApp app;
	app.Create();
	app.Run();

	return 0;
}

SkinnedMeshApp::SkinnedMeshApp()
	: App3DFramework("SkinnedMesh"), obj_controller_(true, MB_Left, MB_Middle, 0)
{
	Context::Instance().ResLoaderInstance().AddPath("../../Samples/media/SkinnedMesh");
}

void SkinnedMeshApp::OnCreate()
{
	skinned_model_ = checked_pointer_cast<SkinnedModel>(SyncLoadModel("archer_attacking.glb", EAH_GPU_Read | EAH_Immutable,
		SceneNode::SOA_Cullable, AddToSceneRootHelper, CreateModelFactory<SkinnedModel>, CreateMeshFactory<SkinnedMesh>));
	TexturePtr c_cube = ASyncLoadTexture("Lake_CraterLake03_filtered_c.dds", EAH_GPU_Read | EAH_Immutable);
	TexturePtr y_cube = ASyncLoadTexture("Lake_CraterLake03_filtered_y.dds", EAH_GPU_Read | EAH_Immutable);

	font_ = SyncLoadFont("gkai00mp.kfont");

	auto& context = Context::Instance();
	BOOST_ASSERT(context.DeferredRenderingLayerValid());
	deferred_rendering_ = &context.DeferredRenderingLayerInstance();

	this->LookAt(float3(-2, 2, -3), float3(0, 1.2f, 0));
	this->Proj(0.1f, 20.0f);

	auto& root_node = context.SceneManagerInstance().SceneRootNode();

	AmbientLightSourcePtr ambient_light = MakeSharedPtr<AmbientLightSource>();
	ambient_light->SkylightTex(y_cube, c_cube);
	ambient_light->Color(float3(0.8f, 0.8f, 0.8f));
	root_node.AddComponent(ambient_light);

	RenderEngine& re = context.RenderFactoryInstance().RenderEngineInstance();

	scene_camera_ = re.DefaultFrameBuffer()->Viewport()->Camera();

	obj_controller_.AttachCamera(*scene_camera_);
	obj_controller_.Scalers(0.01f, 0.005f);

	InputEngine& inputEngine(context.InputFactoryInstance().InputEngineInstance());
	InputActionMap actionMap;
	actionMap.AddActions(actions, actions + std::size(actions));

	action_handler_t input_handler = MakeSharedPtr<input_signal>();
	input_handler->Connect([this](InputEngine const& sender, InputAction const& action) { this->InputHandler(sender, action); });
	inputEngine.ActionMap(actionMap, input_handler);

	auto& ui_mgr = context.UIManagerInstance();
	ui_mgr.Load(*context.ResLoaderInstance().Open("SkinnedMesh.uiml"));
	dialog_params_ = ui_mgr.GetDialog("Parameters");
	id_skinning_ = dialog_params_->IDFromName("Skinning");
	id_playing_ = dialog_params_->IDFromName("Playing");

	dialog_params_->Control<UICheckBox>(id_skinning_)->OnChangedEvent().Connect([this](UICheckBox const& sender) {
		this->SkinningHandler(sender);
	});
	this->SkinningHandler(*dialog_params_->Control<UICheckBox>(id_skinning_));
	dialog_params_->Control<UICheckBox>(id_playing_)->OnChangedEvent().Connect([this](UICheckBox const& sender) {
		this->PlayingHandler(sender);
	});
	this->PlayingHandler(*dialog_params_->Control<UICheckBox>(id_playing_));

	auto skybox = MakeSharedPtr<RenderableSkyBox>();
	skybox->CompressedCubeMap(y_cube, c_cube);
	root_node.AddChild(MakeSharedPtr<SceneNode>(MakeSharedPtr<RenderableComponent>(skybox), SceneNode::SOA_NotCastShadow));
}

void SkinnedMeshApp::OnResize(uint32_t width, uint32_t height)
{
	App3DFramework::OnResize(width, height);

	auto& context = Context::Instance();
	RenderEngine& re = context.RenderFactoryInstance().RenderEngineInstance();
	deferred_rendering_->SetupViewport(0, re.CurFrameBuffer(), 0);

	context.UIManagerInstance().SettleCtrls();
}

void SkinnedMeshApp::InputHandler([[maybe_unused]] InputEngine const& sender, InputAction const& action)
{
	switch (action.first)
	{
	case Exit:
		this->Quit();
		break;
	}
}

void SkinnedMeshApp::SkinningHandler(UICheckBox const& sender)
{
	skinning_ = sender.GetChecked();
	if (skinning_)
	{
		skinned_model_->RebindJoints();
	}
	else
	{
		skinned_model_->UnbindJoints();
	}
	dialog_params_->Control<UICheckBox>(id_playing_)->SetEnabled(skinning_);
}

void SkinnedMeshApp::PlayingHandler(UICheckBox const& sender)
{
	playing_ = sender.GetChecked();
}

void SkinnedMeshApp::DoUpdateOverlay()
{
	auto& context = Context::Instance();
	context.UIManagerInstance().Render();

	std::wostringstream stream;
	stream.precision(2);
	stream << std::fixed << this->FPS() << " FPS";

	font_->RenderText(0, 0, Color(1, 1, 0, 1), L"Skinned Mesh", 16);
	font_->RenderText(0, 18, Color(1, 1, 0, 1), stream.str(), 16);

	uint32_t const num_loading_res = context.ResLoaderInstance().NumLoadingResources();
	if (num_loading_res > 0)
	{
		stream.str(L"");
		stream << "Loading " << num_loading_res << " resources...";
		font_->RenderText(100, 300, Color(1, 0, 0, 1), stream.str(), 48);
	}
}

uint32_t SkinnedMeshApp::DoUpdate(uint32_t pass)
{
	if ((pass == 0) && skinning_ && playing_)
	{
		frame_ += skinned_model_->FrameRate() * frame_time_;
		skinned_model_->SetFrame(frame_);
	}

	return deferred_rendering_->Update(pass);
}
