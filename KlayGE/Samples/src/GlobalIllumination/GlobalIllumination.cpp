#include <KlayGE/KlayGE.hpp>
#include <KlayGE/ThrowErr.hpp>
#include <KlayGE/Util.hpp>
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
#include <KlayGE/GraphicsBuffer.hpp>
#include <KlayGE/SceneObjectHelper.hpp>
#include <KlayGE/UI.hpp>
#include <KlayGE/Camera.hpp>
#include <KlayGE/DeferredRenderingLayer.hpp>

#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/InputFactory.hpp>

#include <vector>
#include <sstream>
#include <fstream>
#include <boost/bind.hpp>

#include "GlobalIllumination.hpp"

using namespace std;
using namespace KlayGE;

namespace
{
	class SpotLightSourceUpdate
	{
	public:
		void operator()(LightSource& light, float /*app_time*/, float /*elapsed_time*/)
		{
			light.Position(float3(0, 12, -4.8f));
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

int main()
{
	ResLoader::Instance().AddPath("../../Samples/media/Common");

	Context::Instance().LoadCfg("KlayGE.cfg");

	ContextCfg cfg = Context::Instance().Config();
	cfg.deferred_rendering = true;
	Context::Instance().Config(cfg);

	GlobalIlluminationApp app;
	app.Create();
	app.Run();

	return 0;
}

GlobalIlluminationApp::GlobalIlluminationApp()
			: App3DFramework("GlobalIllumination"),
				il_scale_(1.0f)
{
	ResLoader::Instance().AddPath("../../Samples/media/GlobalIllumination");
}

bool GlobalIlluminationApp::ConfirmDevice() const
{
	RenderDeviceCaps const & caps = Context::Instance().RenderFactoryInstance().RenderEngineInstance().DeviceCaps();
	if (caps.max_shader_model < 2)
	{
		return false;
	}
	return true;
}

void GlobalIlluminationApp::InitObjects()
{
	this->LookAt(float3(-14.5f, 18, -3), float3(-13.6f, 17.55f, -2.8f));
	this->Proj(0.1f, 500.0f);

	boost::function<RenderModelPtr()> model_ml = ASyncLoadModel("sponza_crytek.7z//sponza_crytek.meshml", EAH_GPU_Read | EAH_Immutable, CreateModelFactory<RenderModel>(), CreateMeshFactory<StaticMesh>());
	boost::function<TexturePtr()> y_cube_tl = ASyncLoadTexture("Lake_CraterLake03_y.dds", EAH_GPU_Read | EAH_Immutable);
	boost::function<TexturePtr()> c_cube_tl = ASyncLoadTexture("Lake_CraterLake03_c.dds", EAH_GPU_Read | EAH_Immutable);

	font_ = Context::Instance().RenderFactoryInstance().MakeFont("gkai00mp.kfont");

	deferred_rendering_ = Context::Instance().DeferredRenderingLayerInstance();

	spot_light_ = MakeSharedPtr<SpotLightSource>();
	spot_light_->Attrib(LSA_IndirectLighting);
	spot_light_->Position(float3(0, 12, -4.8f));
	spot_light_->Direction(float3(0, 0, 1));
	spot_light_->Color(float3(6.0f, 5.88f, 4.38f));
	spot_light_->Falloff(float3(1, 0.1f, 0));
	spot_light_->OuterAngle(PI / 4);
	spot_light_->InnerAngle(PI / 6);
	spot_light_->BindUpdateFunc(SpotLightSourceUpdate());
	spot_light_->AddToSceneManager();

	spot_light_src_ = MakeSharedPtr<SceneObjectLightSourceProxy>(spot_light_);
	checked_pointer_cast<SceneObjectLightSourceProxy>(spot_light_src_)->Scaling(0.1f, 0.1f, 0.1f);
	spot_light_src_->AddToSceneManager();

	fpcController_.Scalers(0.05f, 0.5f);

	InputEngine& inputEngine(Context::Instance().InputFactoryInstance().InputEngineInstance());
	InputActionMap actionMap;
	actionMap.AddActions(actions, actions + sizeof(actions) / sizeof(actions[0]));

	action_handler_t input_handler = MakeSharedPtr<input_signal>();
	input_handler->connect(boost::bind(&GlobalIlluminationApp::InputHandler, this, _1, _2));
	inputEngine.ActionMap(actionMap, input_handler, true);

	UIManager::Instance().Load(ResLoader::Instance().Open("GlobalIllumination.uiml"));
	dialog_ = UIManager::Instance().GetDialogs()[0];

	id_illum_combo_ = dialog_->IDFromName("IllumCombo");
	id_il_scale_static_ = dialog_->IDFromName("ILScaleStatic");
	id_il_scale_slider_ = dialog_->IDFromName("ILScaleSlider");
	id_ssgi_ = dialog_->IDFromName("SSGI");
	id_ssvo_ = dialog_->IDFromName("SSVO");
	id_hdr_ = dialog_->IDFromName("HDR");
	id_aa_ = dialog_->IDFromName("AA");
	id_cg_ = dialog_->IDFromName("CG");
	id_ctrl_camera_ = dialog_->IDFromName("CtrlCamera");

	dialog_->Control<UIComboBox>(id_illum_combo_)->OnSelectionChangedEvent().connect(boost::bind(&GlobalIlluminationApp::IllumChangedHandler, this, _1));
	this->IllumChangedHandler(*dialog_->Control<UIComboBox>(id_illum_combo_));

	dialog_->Control<UISlider>(id_il_scale_slider_)->SetValue(static_cast<int>(il_scale_ * 10));
	dialog_->Control<UISlider>(id_il_scale_slider_)->OnValueChangedEvent().connect(boost::bind(&GlobalIlluminationApp::ILScaleChangedHandler, this, _1));
	this->ILScaleChangedHandler(*dialog_->Control<UISlider>(id_il_scale_slider_));

	dialog_->Control<UICheckBox>(id_ssgi_)->OnChangedEvent().connect(boost::bind(&GlobalIlluminationApp::SSGIHandler, this, _1));
	this->SSGIHandler(*dialog_->Control<UICheckBox>(id_ssgi_));

	dialog_->Control<UICheckBox>(id_ssvo_)->OnChangedEvent().connect(boost::bind(&GlobalIlluminationApp::SSVOHandler, this, _1));
	this->SSVOHandler(*dialog_->Control<UICheckBox>(id_ssvo_));

	dialog_->Control<UICheckBox>(id_hdr_)->OnChangedEvent().connect(boost::bind(&GlobalIlluminationApp::HDRHandler, this, _1));
	this->HDRHandler(*dialog_->Control<UICheckBox>(id_hdr_));

	dialog_->Control<UICheckBox>(id_aa_)->OnChangedEvent().connect(boost::bind(&GlobalIlluminationApp::AAHandler, this, _1));
	this->AAHandler(*dialog_->Control<UICheckBox>(id_aa_));

	dialog_->Control<UICheckBox>(id_cg_)->OnChangedEvent().connect(boost::bind(&GlobalIlluminationApp::ColorGradingHandler, this, _1));
	this->ColorGradingHandler(*dialog_->Control<UICheckBox>(id_cg_));

	dialog_->Control<UICheckBox>(id_ctrl_camera_)->OnChangedEvent().connect(boost::bind(&GlobalIlluminationApp::CtrlCameraHandler, this, _1));

	scene_model_ = model_ml();
	scene_objs_.resize(scene_model_->NumMeshes());
	for (size_t i = 0; i < scene_model_->NumMeshes(); ++ i)
	{
		scene_objs_[i] = MakeSharedPtr<SceneObjectHelper>(scene_model_->Mesh(i), SceneObject::SOA_Cullable);
		scene_objs_[i]->AddToSceneManager();
	}

	sky_box_ = MakeSharedPtr<SceneObjectHDRSkyBox>();
	checked_pointer_cast<SceneObjectHDRSkyBox>(sky_box_)->CompressedCubeMap(y_cube_tl(), c_cube_tl());
	sky_box_->AddToSceneManager();
}

void GlobalIlluminationApp::OnResize(uint32_t width, uint32_t height)
{
	App3DFramework::OnResize(width, height);
	deferred_rendering_->OnResize(width, height);

	UIManager::Instance().SettleCtrls(width, height);
}

void GlobalIlluminationApp::InputHandler(InputEngine const & /*sender*/, InputAction const & action)
{
	switch (action.first)
	{
	case Exit:
		this->Quit();
		break;
	}
}

void GlobalIlluminationApp::IllumChangedHandler(UIComboBox const & sender)
{
	deferred_rendering_->DisplayIllum(sender.GetSelectedIndex());
}

void GlobalIlluminationApp::ILScaleChangedHandler(KlayGE::UISlider const & sender)
{
	il_scale_ = sender.GetValue() / 10.0f;
	deferred_rendering_->IndirectScale(il_scale_);

	std::wostringstream stream;
	stream << L"Scale: " << il_scale_ << " x";
	dialog_->Control<UIStatic>(id_il_scale_static_)->SetText(stream.str());
}

void GlobalIlluminationApp::SSGIHandler(UICheckBox const & sender)
{
	deferred_rendering_->SSGIEnabled(sender.GetChecked());
}

void GlobalIlluminationApp::SSVOHandler(UICheckBox const & sender)
{
	deferred_rendering_->SSVOEnabled(sender.GetChecked());
}

void GlobalIlluminationApp::HDRHandler(UICheckBox const & sender)
{
	Context::Instance().RenderFactoryInstance().RenderEngineInstance().HDREnabled(sender.GetChecked());
}

void GlobalIlluminationApp::AAHandler(UICheckBox const & sender)
{
	Context::Instance().RenderFactoryInstance().RenderEngineInstance().PPAAEnabled(sender.GetChecked() ? 1 : 0);
}

void GlobalIlluminationApp::ColorGradingHandler(UICheckBox const & sender)
{
	Context::Instance().RenderFactoryInstance().RenderEngineInstance().ColorGradingEnabled(sender.GetChecked());
}

void GlobalIlluminationApp::CtrlCameraHandler(UICheckBox const & sender)
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

void GlobalIlluminationApp::DoUpdateOverlay()
{
	RenderEngine& renderEngine(Context::Instance().RenderFactoryInstance().RenderEngineInstance());

	UIManager::Instance().Render();

	font_->RenderText(0, 0, Color(1, 1, 0, 1), L"Global Illumination", 16);
	font_->RenderText(0, 18, Color(1, 1, 0, 1), renderEngine.ScreenFrameBuffer()->Description(), 16);

	std::wostringstream stream;
	stream.precision(2);
	stream << std::fixed << this->FPS() << " FPS";
	font_->RenderText(0, 36, Color(1, 1, 0, 1), stream.str(), 16);
}

uint32_t GlobalIlluminationApp::DoUpdate(uint32_t pass)
{
	return deferred_rendering_->Update(pass);
}
