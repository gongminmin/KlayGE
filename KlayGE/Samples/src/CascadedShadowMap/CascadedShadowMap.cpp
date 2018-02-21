#include <KlayGE/KlayGE.hpp>
#include <KFL/CXX17/iterator.hpp>
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
#include <KlayGE/SceneObjectHelper.hpp>
#include <KlayGE/PostProcess.hpp>
#include <KlayGE/Camera.hpp>
#include <KlayGE/DeferredRenderingLayer.hpp>

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

	light_ctrl_camera_.ViewParams(float3(-50, 50, -50), float3(0, 0, 0), float3(0, 1, 0));
	light_controller_.AttachCamera(light_ctrl_camera_);
	light_controller_.Scalers(0.003f, 0.003f);

	TexturePtr c_cube = ASyncLoadTexture("Lake_CraterLake03_filtered_c.dds", EAH_GPU_Read | EAH_Immutable);
	TexturePtr y_cube = ASyncLoadTexture("Lake_CraterLake03_filtered_y.dds", EAH_GPU_Read | EAH_Immutable);
	RenderablePtr plane_model = ASyncLoadModel("plane.meshml", EAH_GPU_Read | EAH_Immutable,
		CreateModelFactory<RenderModel>(), CreateMeshFactory<StaticMesh>());
	RenderablePtr katapult_model = ASyncLoadModel("katapult.meshml", EAH_GPU_Read | EAH_Immutable);

	font_ = SyncLoadFont("gkai00mp.kfont");

	deferred_rendering_ = Context::Instance().DeferredRenderingLayerInstance();
	deferred_rendering_->SSVOEnabled(0, false);

	AmbientLightSourcePtr ambient_light = MakeSharedPtr<AmbientLightSource>();
	ambient_light->SkylightTex(y_cube, c_cube);
	ambient_light->Color(float3(0.1f, 0.1f, 0.1f));
	ambient_light->AddToSceneManager();
	
	sun_light_ = MakeSharedPtr<DirectionalLightSource>();
	sun_light_->Attrib(0);
	sun_light_->Direction(MathLib::normalize(float3(50, -50, 50)));
	sun_light_->Color(float3(1, 1, 1));
	sun_light_->AddToSceneManager();

	SceneObjectPtr plane_so = MakeSharedPtr<SceneObjectHelper>(plane_model, SceneObject::SOA_Cullable);
	plane_so->ModelMatrix(MathLib::scaling(200.0f, 1.0f, 200.0f));
	plane_so->AddToSceneManager();

	SceneObjectPtr katapult_so = MakeSharedPtr<SceneObjectHelper>(katapult_model, SceneObject::SOA_Cullable);
	katapult_so->AddToSceneManager();

	fpcController_.Scalers(0.05f, 1.0f);

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

	UIManager::Instance().Load(ResLoader::Instance().Open("CascadedShadowMap.uiml"));
	dialog_ = UIManager::Instance().GetDialogs()[0];

	id_csm_type_combo_ = dialog_->IDFromName("TypeCombo");
	id_cascades_combo_ = dialog_->IDFromName("CascadesCombo");
	id_pssm_factor_static_ = dialog_->IDFromName("PSSMFactorStatic");
	id_pssm_factor_slider_ = dialog_->IDFromName("PSSMFactorSlider");
	id_ctrl_camera_ = dialog_->IDFromName("CtrlCamera");

	dialog_->Control<UIComboBox>(id_csm_type_combo_)->OnSelectionChangedEvent().connect(
		[this](UIComboBox const & sender)
		{
			this->CSMTypeChangedHandler(sender);
		});
	this->CSMTypeChangedHandler(*dialog_->Control<UIComboBox>(id_csm_type_combo_));

	dialog_->Control<UIComboBox>(id_cascades_combo_)->OnSelectionChangedEvent().connect(
		[this](UIComboBox const & sender)
		{
			this->CascadesChangedHandler(sender);
		});
	this->CascadesChangedHandler(*dialog_->Control<UIComboBox>(id_cascades_combo_));

	dialog_->Control<UISlider>(id_pssm_factor_slider_)->OnValueChangedEvent().connect(
		[this](UISlider const & sender)
		{
			this->PSSMFactorChangedHandler(sender);
		});
	this->PSSMFactorChangedHandler(*dialog_->Control<UISlider>(id_pssm_factor_slider_));

	dialog_->Control<UICheckBox>(id_ctrl_camera_)->OnChangedEvent().connect(
		[this](UICheckBox const & sender)
		{
			this->CtrlCameraHandler(sender);
		});
	this->CtrlCameraHandler(*dialog_->Control<UICheckBox>(id_ctrl_camera_));

	sky_box_ = MakeSharedPtr<SceneObjectSkyBox>();
	checked_pointer_cast<SceneObjectSkyBox>(sky_box_)->CompressedCubeMap(y_cube, c_cube);
	sky_box_->AddToSceneManager();

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
}

uint32_t CascadedShadowMapApp::DoUpdate(uint32_t pass)
{
	if (0 == pass)
	{
		sun_light_->Direction(light_ctrl_camera_.ForwardVec());
	}

	return deferred_rendering_->Update(pass);
}
