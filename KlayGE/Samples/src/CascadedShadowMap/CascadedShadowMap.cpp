#include <KlayGE/KlayGE.hpp>
#include <KFL/Util.hpp>
#include <KFL/Math.hpp>
#include <KlayGE/Font.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/InputFactory.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/RenderEffect.hpp>
#include <KlayGE/FrameBuffer.hpp>
#include <KlayGE/SceneManager.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/ResLoader.hpp>
#include <KlayGE/RenderSettings.hpp>
#include <KlayGE/Mesh.hpp>
#include <KlayGE/SceneNode.hpp>
#include <KlayGE/SkyBox.hpp>
#include <KlayGE/PostProcess.hpp>
#include <KlayGE/Camera.hpp>
#include <KlayGE/DeferredRenderingLayer.hpp>

#include <iterator>
#include <sstream>

#include "SampleCommon.hpp"
#include "CascadedShadowMap.hpp"

using namespace std;
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

	CascadedShadowMapApp app;
	app.Create();
	app.Run();

	return 0;
}

CascadedShadowMapApp::CascadedShadowMapApp()
			: App3DFramework("CascadedShadowMap"),
				light_controller_(true, MB_Right, 0, 0),
				pssm_factor_(0.8f)
{
	ResLoader::Instance().AddPath("../../Samples/media/CascadedShadowMap");
}

void CascadedShadowMapApp::OnCreate()
{
	this->LookAt(float3(-25.72f, 29.65f, 24.57f), float3(-24.93f, 29.09f, 24.32f));
	this->Proj(0.05f, 300.0f);

	auto& root_node = Context::Instance().SceneManagerInstance().SceneRootNode();

	auto light_ctrl_camera_node =
		MakeSharedPtr<SceneNode>(SceneNode::SOA_Cullable | SceneNode::SOA_Moveable | SceneNode::SOA_NotCastShadow);
	light_ctrl_camera_ = MakeSharedPtr<Camera>();
	light_ctrl_camera_node->AddComponent(light_ctrl_camera_);
	float3 const light_pos = float3(-50, 50, -50);
	light_ctrl_camera_->LookAtDist(MathLib::length(light_pos));
	light_ctrl_camera_node->TransformToParent(MathLib::inverse(MathLib::look_at_lh(light_pos, float3(0, 0, 0), float3(0, 1, 0))));
	root_node.AddChild(light_ctrl_camera_node);
	light_controller_.AttachCamera(*light_ctrl_camera_);
	light_controller_.Scalers(0.003f, 0.003f);

	TexturePtr c_cube = ASyncLoadTexture("Lake_CraterLake03_filtered_c.dds", EAH_GPU_Read | EAH_Immutable);
	TexturePtr y_cube = ASyncLoadTexture("Lake_CraterLake03_filtered_y.dds", EAH_GPU_Read | EAH_Immutable);
	auto plane_model = ASyncLoadModel("plane.glb", EAH_GPU_Read | EAH_Immutable,
		SceneNode::SOA_Cullable,
		[](RenderModel& model)
		{
			model.RootNode()->TransformToParent(MathLib::scaling(200.0f, 1.0f, 200.0f));
			AddToSceneRootHelper(model);
		});
	auto katapult_model = ASyncLoadModel("katapult.glb", EAH_GPU_Read | EAH_Immutable,
		SceneNode::SOA_Cullable, AddToSceneRootHelper);

	font_ = SyncLoadFont("gkai00mp.kfont");

	deferred_rendering_ = Context::Instance().DeferredRenderingLayerInstance();
	deferred_rendering_->SSVOEnabled(0, false);

	AmbientLightSourcePtr ambient_light = MakeSharedPtr<AmbientLightSource>();
	ambient_light->SkylightTex(y_cube, c_cube);
	ambient_light->Color(float3(0.1f, 0.1f, 0.1f));
	root_node.AddComponent(ambient_light);

	auto sun_light = MakeSharedPtr<DirectionalLightSource>();
	sun_light->Attrib(0);
	sun_light->Color(float3(1, 1, 1));
	auto sun_light_node = MakeSharedPtr<SceneNode>(SceneNode::SOA_Cullable | SceneNode::SOA_Moveable);
	sun_light_node->OnMainThreadUpdate().Connect([this](SceneNode& node, float app_time, float elapsed_time) {
		KFL_UNUSED(app_time);
		KFL_UNUSED(elapsed_time);

		node.TransformToParent(light_ctrl_camera_->InverseViewMatrix());
	});
	sun_light_node->AddComponent(sun_light);
	root_node.AddChild(sun_light_node);

	fpcController_.Scalers(0.05f, 1.0f);

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

	UIManager::Instance().Load(*ResLoader::Instance().Open("CascadedShadowMap.uiml"));
	dialog_ = UIManager::Instance().GetDialogs()[0];

	id_csm_type_combo_ = dialog_->IDFromName("TypeCombo");
	id_cascades_combo_ = dialog_->IDFromName("CascadesCombo");
	id_pssm_factor_static_ = dialog_->IDFromName("PSSMFactorStatic");
	id_pssm_factor_slider_ = dialog_->IDFromName("PSSMFactorSlider");
	id_ctrl_camera_ = dialog_->IDFromName("CtrlCamera");

	dialog_->Control<UIComboBox>(id_csm_type_combo_)->OnSelectionChangedEvent().Connect(
		[this](UIComboBox const & sender)
		{
			this->CSMTypeChangedHandler(sender);
		});
	this->CSMTypeChangedHandler(*dialog_->Control<UIComboBox>(id_csm_type_combo_));

	dialog_->Control<UIComboBox>(id_cascades_combo_)->OnSelectionChangedEvent().Connect(
		[this](UIComboBox const & sender)
		{
			this->CascadesChangedHandler(sender);
		});
	this->CascadesChangedHandler(*dialog_->Control<UIComboBox>(id_cascades_combo_));

	dialog_->Control<UISlider>(id_pssm_factor_slider_)->OnValueChangedEvent().Connect(
		[this](UISlider const & sender)
		{
			this->PSSMFactorChangedHandler(sender);
		});
	this->PSSMFactorChangedHandler(*dialog_->Control<UISlider>(id_pssm_factor_slider_));

	dialog_->Control<UICheckBox>(id_ctrl_camera_)->OnChangedEvent().Connect(
		[this](UICheckBox const & sender)
		{
			this->CtrlCameraHandler(sender);
		});
	this->CtrlCameraHandler(*dialog_->Control<UICheckBox>(id_ctrl_camera_));

	auto skybox = MakeSharedPtr<RenderableSkyBox>();
	skybox->CompressedCubeMap(y_cube, c_cube);
	root_node.AddChild(MakeSharedPtr<SceneNode>(MakeSharedPtr<RenderableComponent>(skybox), SceneNode::SOA_NotCastShadow));

	RenderDeviceCaps const & caps = Context::Instance().RenderFactoryInstance().RenderEngineInstance().DeviceCaps();
	if (caps.max_shader_model < ShaderModel(5, 0))
	{
		dialog_->Control<UIComboBox>(id_csm_type_combo_)->SetSelectedByIndex(0);
		dialog_->Control<UIComboBox>(id_csm_type_combo_)->SetEnabled(false);
	}
}

void CascadedShadowMapApp::OnResize(uint32_t width, uint32_t height)
{
	App3DFramework::OnResize(width, height);

	RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
	deferred_rendering_->SetupViewport(0, re.CurFrameBuffer(), 0);

	UIManager::Instance().SettleCtrls();
}

void CascadedShadowMapApp::InputHandler(InputEngine const & /*sender*/, InputAction const & action)
{
	switch (action.first)
	{
	case Exit:
		this->Quit();
		break;
	}
}

void CascadedShadowMapApp::PSSMFactorChangedHandler(UISlider const & sender)
{
	pssm_factor_ = sender.GetValue() * 0.01f;

	std::wostringstream stream;
	stream << L"PSSM Factor: " << pssm_factor_;
	dialog_->Control<UIStatic>(id_pssm_factor_static_)->SetText(stream.str());

	deferred_rendering_->SetViewportCascades(0, num_cascades_, pssm_factor_);
}

void CascadedShadowMapApp::CSMTypeChangedHandler(UIComboBox const & sender)
{
	CascadedShadowLayerType type = static_cast<CascadedShadowLayerType>(sender.GetSelectedIndex() + 1);
	deferred_rendering_->SetCascadedShadowType(type);
	dialog_->Control<UIStatic>(id_pssm_factor_static_)->SetEnabled(CSLT_PSSM == type);
	dialog_->Control<UISlider>(id_pssm_factor_slider_)->SetEnabled(CSLT_PSSM == type);
}

void CascadedShadowMapApp::CascadesChangedHandler(UIComboBox const & sender)
{
	num_cascades_ = sender.GetSelectedIndex() + 1;
	deferred_rendering_->SetViewportCascades(0, num_cascades_, pssm_factor_);
}

void CascadedShadowMapApp::CtrlCameraHandler(UICheckBox const & sender)
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

void CascadedShadowMapApp::DoUpdateOverlay()
{
	RenderEngine& renderEngine(Context::Instance().RenderFactoryInstance().RenderEngineInstance());

	UIManager::Instance().Render();

	font_->RenderText(0, 0, Color(1, 1, 0, 1), L"Cascaded Shadow Map", 16);
	font_->RenderText(0, 18, Color(1, 1, 0, 1), renderEngine.ScreenFrameBuffer()->Description(), 16);

	std::wostringstream stream;
	stream.precision(2);
	stream << std::fixed << this->FPS() << " FPS";
	font_->RenderText(0, 36, Color(1, 1, 0, 1), stream.str(), 16);

	stream.str(L"");
	stream << deferred_rendering_->NumObjectsRendered() << " Scene objects "
		<< deferred_rendering_->NumRenderablesRendered() << " Renderables "
		<< deferred_rendering_->NumPrimitivesRendered() << " Primitives "
		<< deferred_rendering_->NumVerticesRendered() << " Vertices";
	font_->RenderText(0, 54, Color(1, 1, 1, 1), stream.str(), 16);

	uint32_t const num_loading_res = ResLoader::Instance().NumLoadingResources();
	if (num_loading_res > 0)
	{
		stream.str(L"");
		stream << "Loading " << num_loading_res << " resources...";
		font_->RenderText(100, 300, Color(1, 0, 0, 1), stream.str(), 48);
	}
}

uint32_t CascadedShadowMapApp::DoUpdate(uint32_t pass)
{
	return deferred_rendering_->Update(pass);
}
