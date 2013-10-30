#include <KlayGE/KlayGE.hpp>
#include <KFL/ThrowErr.hpp>
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
				num_objs_rendered_(0), num_renderable_rendered_(0),
				num_primitives_rendered_(0), num_vertices_rendered_(0),
				pssm_factor_(0.8f)
{
	ResLoader::Instance().AddPath("../../Samples/media/CascadedShadowMap");
}

bool CascadedShadowMapApp::ConfirmDevice() const
{
	RenderDeviceCaps const & caps = Context::Instance().RenderFactoryInstance().RenderEngineInstance().DeviceCaps();
	if (caps.max_shader_model < 2)
	{
		return false;
	}
	return true;
}

void CascadedShadowMapApp::InitObjects()
{
	this->LookAt(float3(-25.72f, 29.65f, 24.57f), float3(-24.93f, 29.09f, 24.32f));
	this->Proj(0.05f, 300.0f);

	light_ctrl_camera_.ViewParams(float3(-50, 50, -50), float3(0, 0, 0), float3(0, 1, 0));
	light_controller_.AttachCamera(light_ctrl_camera_);
	light_controller_.Scalers(0.003f, 0.003f);

	loading_percentage_ = 0;
	c_cube_tl_ = ASyncLoadTexture("Lake_CraterLake03_c.dds", EAH_GPU_Read | EAH_Immutable);
	y_cube_tl_ = ASyncLoadTexture("Lake_CraterLake03_y.dds", EAH_GPU_Read | EAH_Immutable);
	ground_ml_ = ASyncLoadModel("plane.meshml", EAH_GPU_Read | EAH_Immutable);
	model_ml_ = ASyncLoadModel("katapult.meshml", EAH_GPU_Read | EAH_Immutable);

	font_ = SyncLoadFont("gkai00mp.kfont");

	deferred_rendering_ = Context::Instance().DeferredRenderingLayerInstance();
	deferred_rendering_->SSVOEnabled(0, false);
	
	sun_light_ = MakeSharedPtr<SunLightSource>();
	sun_light_->Attrib(0);
	sun_light_->Direction(MathLib::normalize(float3(50, -50, 50)));
	sun_light_->Color(float3(1, 1, 1));
	sun_light_->AddToSceneManager();

	fpcController_.Scalers(0.05f, 1.0f);

	InputEngine& inputEngine(Context::Instance().InputFactoryInstance().InputEngineInstance());
	InputActionMap actionMap;
	actionMap.AddActions(actions, actions + sizeof(actions) / sizeof(actions[0]));

	action_handler_t input_handler = MakeSharedPtr<input_signal>();
	input_handler->connect(KlayGE::bind(&CascadedShadowMapApp::InputHandler, this, KlayGE::placeholders::_1, KlayGE::placeholders::_2));
	inputEngine.ActionMap(actionMap, input_handler);

	UIManager::Instance().Load(ResLoader::Instance().Open("CascadedShadowMap.uiml"));
	dialog_ = UIManager::Instance().GetDialogs()[0];

	id_csm_type_combo_ = dialog_->IDFromName("TypeCombo");
	id_cascades_combo_ = dialog_->IDFromName("CascadesCombo");
	id_pssm_factor_static_ = dialog_->IDFromName("PSSMFactorStatic");
	id_pssm_factor_slider_ = dialog_->IDFromName("PSSMFactorSlider");
	id_ctrl_camera_ = dialog_->IDFromName("CtrlCamera");

	dialog_->Control<UIComboBox>(id_csm_type_combo_)->OnSelectionChangedEvent().connect(KlayGE::bind(&CascadedShadowMapApp::CSMTypeChangedHandler, this, KlayGE::placeholders::_1));
	this->CSMTypeChangedHandler(*dialog_->Control<UIComboBox>(id_csm_type_combo_));

	dialog_->Control<UIComboBox>(id_cascades_combo_)->OnSelectionChangedEvent().connect(KlayGE::bind(&CascadedShadowMapApp::CascadesChangedHandler, this, KlayGE::placeholders::_1));
	this->CascadesChangedHandler(*dialog_->Control<UIComboBox>(id_cascades_combo_));

	dialog_->Control<UISlider>(id_pssm_factor_slider_)->OnValueChangedEvent().connect(KlayGE::bind(&CascadedShadowMapApp::PSSMFactorChangedHandler, this, KlayGE::placeholders::_1));
	this->PSSMFactorChangedHandler(*dialog_->Control<UISlider>(id_pssm_factor_slider_));

	dialog_->Control<UICheckBox>(id_ctrl_camera_)->OnChangedEvent().connect(KlayGE::bind(&CascadedShadowMapApp::CtrlCameraHandler, this, KlayGE::placeholders::_1));
	this->CtrlCameraHandler(*dialog_->Control<UICheckBox>(id_ctrl_camera_));

	sky_box_ = MakeSharedPtr<SceneObjectSkyBox>();
	sky_box_->AddToSceneManager();

	RenderDeviceCaps const & caps = Context::Instance().RenderFactoryInstance().RenderEngineInstance().DeviceCaps();
	if (caps.max_shader_model < 5)
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
	stream << num_objs_rendered_ << " Scene objects "
		<< num_renderable_rendered_ << " Renderables "
		<< num_primitives_rendered_ << " Primitives "
		<< num_vertices_rendered_ << " Vertices";
	font_->RenderText(0, 54, Color(1, 1, 1, 1), stream.str(), 16);
}

uint32_t CascadedShadowMapApp::DoUpdate(uint32_t pass)
{
	SceneManager& sceneMgr(Context::Instance().SceneManagerInstance());

	if (0 == pass)
	{
		if (loading_percentage_ < 100)
		{
			if (loading_percentage_ < 10)
			{
				TexturePtr y_cube_tex = y_cube_tl_();
				TexturePtr c_cube_tex = c_cube_tl_();
				checked_pointer_cast<SceneObjectSkyBox>(sky_box_)->CompressedCubeMap(y_cube_tex, c_cube_tex);
				if (!!y_cube_tex && !!c_cube_tex)
				{
					loading_percentage_ = 10;
				}
			}
			else if (loading_percentage_ < 15)
			{
				ground_model_ = ground_ml_();
				if (ground_model_)
				{
					SceneObjectPtr so = MakeSharedPtr<SceneObjectHelper>(ground_model_->Mesh(0),
							SceneObject::SOA_Cullable);
					so->ModelMatrix(MathLib::scaling(200.0f, 1.0f, 200.0f));
					so->AddToSceneManager();
					scene_objs_.push_back(so);

					loading_percentage_ = 15;
				}
			}
			else if (loading_percentage_ < 80)
			{
				scene_model_ = model_ml_();
				if (scene_model_)
				{
					loading_percentage_ = 80;
				}
			}
			else
			{
				uint32_t n = (scene_model_->NumMeshes() + 20 - 1) / 20;
				uint32_t s = (loading_percentage_ - 80) * n;
				uint32_t e = std::min(s + n, scene_model_->NumMeshes());
				for (uint32_t i = s; i < e; ++ i)
				{
					SceneObjectPtr so = MakeSharedPtr<SceneObjectHelper>(scene_model_->Mesh(i),
						SceneObject::SOA_Cullable);
					so->AddToSceneManager();
					scene_objs_.push_back(so);
				}

				++ loading_percentage_;
			}
		}
		else
		{
			sun_light_->Direction(light_ctrl_camera_.ForwardVec());
		}
	}
	else if (1 == pass)
	{
		num_objs_rendered_ = sceneMgr.NumObjectsRendered();
		num_renderable_rendered_ = sceneMgr.NumRenderablesRendered();
		num_primitives_rendered_ = sceneMgr.NumPrimitivesRendered();
		num_vertices_rendered_ = sceneMgr.NumVerticesRendered();
	}

	return deferred_rendering_->Update(pass);
}
