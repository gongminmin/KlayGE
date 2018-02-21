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
#include <KlayGE/ParticleSystem.hpp>
#include <KlayGE/PerfProfiler.hpp>

#include <sstream>

#include "SampleCommon.hpp"
#include "DeferredRendering.hpp"

using namespace std;
using namespace KlayGE;

namespace
{
	class SpotLightSourceUpdate
	{
	public:
		explicit SpotLightSourceUpdate(float3 const & clr)
			: random_dis_(0, 1000),
				color_(clr)
		{
		}

		void operator()(LightSource& light, float /*app_time*/, float /*elapsed_time*/)
		{
			light.Direction(float3(MathLib::clamp(random_dis_(gen_) * 0.0001f, 0.0f, 0.1f),
				1, MathLib::clamp(random_dis_(gen_) * 0.0001f, 0.0f, 0.1f)));
			light.Color(color_ * (0.85f + random_dis_(gen_) * 0.0003f));
		}

	private:
		ranlux24_base gen_;
		uniform_int_distribution<> random_dis_;
		float3 color_;
	};

	class GISpotLightSourceUpdate
	{
	public:
		GISpotLightSourceUpdate()
		{
		}

		void operator()(LightSource& light, float app_time, float /*elapsed_time*/)
		{
			light.Direction(float3(sin(app_time) * 0.3f, -1, 0.1f));
		}
	};

	class PointLightSourceUpdate
	{
	public:
		PointLightSourceUpdate(float move_speed, float3 const & pos)
			: move_speed_(move_speed), pos_(pos)
		{
		}

		void operator()(LightSource& light, float app_time, float /*elapsed_time*/)
		{
			light.ModelMatrix(MathLib::translation(sin(app_time * 1000 * move_speed_), 0.0f, 0.0f)
				* MathLib::translation(pos_));
		}

	private:
		float move_speed_;
		float3 pos_;
	};


	enum
	{
		Exit,
		Dump,
		Profile
	};

	InputActionDefine actions[] =
	{
		InputActionDefine(Exit, KS_Escape),
		InputActionDefine(Dump, KS_Enter),
		InputActionDefine(Profile, KS_P)
	};
}

int SampleMain()
{
	ContextCfg cfg = Context::Instance().Config();
	cfg.deferred_rendering = true;
	Context::Instance().Config(cfg);

	DeferredRenderingApp app;
	app.Create();
	app.Run();

	return 0;
}

DeferredRenderingApp::DeferredRenderingApp()
			: App3DFramework("DeferredRendering"),
				anti_alias_enabled_(1),
				il_scale_(1.0f)
{
	ResLoader::Instance().AddPath("../../Samples/media/DeferredRendering");
}

void DeferredRenderingApp::OnCreate()
{
	this->LookAt(float3(-14.5f, 18, -3), float3(-13.6f, 17.55f, -2.8f));
	this->Proj(0.1f, 500.0f);

	TexturePtr c_cube = ASyncLoadTexture("Lake_CraterLake03_filtered_c.dds", EAH_GPU_Read | EAH_Immutable);
	TexturePtr y_cube = ASyncLoadTexture("Lake_CraterLake03_filtered_y.dds", EAH_GPU_Read | EAH_Immutable);
	RenderablePtr scene_model = ASyncLoadModel("sponza_crytek.meshml", EAH_GPU_Read | EAH_Immutable);

	font_ = SyncLoadFont("gkai00mp.kfont");

	deferred_rendering_ = Context::Instance().DeferredRenderingLayerInstance();

	AmbientLightSourcePtr ambient_light = MakeSharedPtr<AmbientLightSource>();
	ambient_light->SkylightTex(y_cube, c_cube);
	ambient_light->Color(float3(0.1f, 0.1f, 0.1f));
	ambient_light->AddToSceneManager();
	
	spot_light_[0] = MakeSharedPtr<SpotLightSource>();
	spot_light_[0]->Attrib(0);
	spot_light_[0]->Color(float3(1.0f, 0.17f, 0.05f) * 10.0f);
	spot_light_[0]->Falloff(float3(1, 0.5f, 0));
	spot_light_[0]->Position(float3(+14.6f, 3.7f, -4.3f));
	spot_light_[0]->Direction(float3(0, 1, 0));
	spot_light_[0]->OuterAngle(PI / 2.5f);
	spot_light_[0]->InnerAngle(PI / 4);
	spot_light_[0]->BindUpdateFunc(SpotLightSourceUpdate(spot_light_[0]->Color()));
	spot_light_[0]->AddToSceneManager();

	spot_light_[1] = MakeSharedPtr<SpotLightSource>();
	spot_light_[1]->Attrib(0);
	spot_light_[1]->Color(float3(1.0f, 0.17f, 0.05f) * 10.0f);
	spot_light_[1]->Falloff(float3(1, 0.5f, 0));
	spot_light_[1]->Position(float3(-18.6f, 3.7f, +6.5f));
	spot_light_[1]->Direction(float3(0, 1, 0));
	spot_light_[1]->OuterAngle(PI / 2.5f);
	spot_light_[1]->InnerAngle(PI / 4);
	spot_light_[1]->BindUpdateFunc(SpotLightSourceUpdate(spot_light_[1]->Color()));
	spot_light_[1]->AddToSceneManager();

	spot_light_[2] = MakeSharedPtr<SpotLightSource>();
	spot_light_[2]->Attrib(LightSource::LSA_IndirectLighting);
	spot_light_[2]->Color(float3(6.0f, 5.88f, 4.38f) * 10.0f);
	spot_light_[2]->Position(float3(0.0f, 43.2f, -5.9f));
	spot_light_[2]->Direction(float3(0.0f, -1, 0.1f));
	spot_light_[2]->Falloff(float3(1, 0.1f, 0));
	spot_light_[2]->OuterAngle(PI / 8);
	spot_light_[2]->InnerAngle(PI / 12);
	spot_light_[2]->BindUpdateFunc(GISpotLightSourceUpdate());
	spot_light_[2]->AddToSceneManager();

	spot_light_src_[0] = MakeSharedPtr<SceneObjectLightSourceProxy>(spot_light_[0]);
	checked_pointer_cast<SceneObjectLightSourceProxy>(spot_light_src_[0])->Scaling(0.1f, 0.1f, 0.1f);
	spot_light_src_[1] = MakeSharedPtr<SceneObjectLightSourceProxy>(spot_light_[1]);
	checked_pointer_cast<SceneObjectLightSourceProxy>(spot_light_src_[1])->Scaling(0.1f, 0.1f, 0.1f);
	spot_light_src_[2] = MakeSharedPtr<SceneObjectLightSourceProxy>(spot_light_[2]);
	spot_light_src_[2]->AddToSceneManager();

	SceneObjectPtr scene_obj = MakeSharedPtr<SceneObjectHelper>(scene_model, SceneObject::SOA_Cullable);
	scene_obj->AddToSceneManager();

	fpcController_.Scalers(0.05f, 0.5f);

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

	UIManager::Instance().Load(ResLoader::Instance().Open("DeferredRendering.uiml"));
	dialog_ = UIManager::Instance().GetDialogs()[0];

	id_buffer_combo_ = dialog_->IDFromName("BufferCombo");
	id_illum_combo_ = dialog_->IDFromName("IllumCombo");
	id_il_scale_static_ = dialog_->IDFromName("ILScaleStatic");
	id_il_scale_slider_ = dialog_->IDFromName("ILScaleSlider");
	id_ssvo_ = dialog_->IDFromName("SSVO");
	id_hdr_ = dialog_->IDFromName("HDR");
	id_aa_ = dialog_->IDFromName("AA");
	id_num_lights_static_ = dialog_->IDFromName("NumLightsStatic");
	id_num_lights_slider_ = dialog_->IDFromName("NumLightsSlider");
	id_ctrl_camera_ = dialog_->IDFromName("CtrlCamera");

#if DEFAULT_DEFERRED != TRIDITIONAL_DEFERRED
	dialog_->Control<UIComboBox>(id_buffer_combo_)->RemoveItem(10);
	dialog_->Control<UIComboBox>(id_buffer_combo_)->RemoveItem(9);
#endif

	dialog_->Control<UIComboBox>(id_buffer_combo_)->OnSelectionChangedEvent().connect(
		[this](UIComboBox const & sender)
		{
			this->BufferChangedHandler(sender);
		});
	this->BufferChangedHandler(*dialog_->Control<UIComboBox>(id_buffer_combo_));

	dialog_->Control<UIComboBox>(id_illum_combo_)->OnSelectionChangedEvent().connect(
		[this](UIComboBox const & sender)
		{
			this->IllumChangedHandler(sender);
		});
	this->IllumChangedHandler(*dialog_->Control<UIComboBox>(id_illum_combo_));

	dialog_->Control<UISlider>(id_il_scale_slider_)->SetValue(static_cast<int>(il_scale_ * 10));
	dialog_->Control<UISlider>(id_il_scale_slider_)->OnValueChangedEvent().connect(
		[this](UISlider const & sender)
		{
			this->ILScaleChangedHandler(sender);
		});
	this->ILScaleChangedHandler(*dialog_->Control<UISlider>(id_il_scale_slider_));

	dialog_->Control<UICheckBox>(id_ssvo_)->OnChangedEvent().connect(
		[this](UICheckBox const & sender)
		{
			this->SSVOHandler(sender);
		});
	this->SSVOHandler(*dialog_->Control<UICheckBox>(id_ssvo_));
	dialog_->Control<UICheckBox>(id_hdr_)->OnChangedEvent().connect(
		[this](UICheckBox const & sender)
		{
			this->HDRHandler(sender);
		});
	this->HDRHandler(*dialog_->Control<UICheckBox>(id_hdr_));
	dialog_->Control<UICheckBox>(id_aa_)->OnChangedEvent().connect(
		[this](UICheckBox const & sender)
		{
			this->AntiAliasHandler(sender);
		});
	this->AntiAliasHandler(*dialog_->Control<UICheckBox>(id_aa_));
	dialog_->Control<UISlider>(id_num_lights_slider_)->OnValueChangedEvent().connect(
		[this](UISlider const & sender)
		{
			this->NumLightsChangedHandler(sender);
		});
	this->NumLightsChangedHandler(*dialog_->Control<UISlider>(id_num_lights_slider_));

	dialog_->Control<UICheckBox>(id_ctrl_camera_)->OnChangedEvent().connect(
		[this](UICheckBox const & sender)
		{
			this->CtrlCameraHandler(sender);
		});
	this->CtrlCameraHandler(*dialog_->Control<UICheckBox>(id_ctrl_camera_));

	sky_box_ = MakeSharedPtr<SceneObjectSkyBox>();
	checked_pointer_cast<SceneObjectSkyBox>(sky_box_)->CompressedCubeMap(y_cube, c_cube);
	sky_box_->AddToSceneManager();

	ps_ = SyncLoadParticleSystem("Fire.psml");
	ps_->Gravity(0.5f);
	ps_->MediaDensity(0.5f);
	ps_->AddToSceneManager();

	float const SCALE = 3;
	ps_->ModelMatrix(MathLib::scaling(SCALE, SCALE, SCALE));

	ParticleEmitterPtr emitter0 = ps_->Emitter(0);
	emitter0->ModelMatrix(MathLib::translation(spot_light_[0]->Position() / SCALE));

	ParticleEmitterPtr emitter1 = emitter0->Clone();
	emitter1->ModelMatrix(MathLib::translation(spot_light_[1]->Position() / SCALE));
	ps_->AddEmitter(emitter1);
}

void DeferredRenderingApp::OnResize(uint32_t width, uint32_t height)
{
	App3DFramework::OnResize(width, height);

	RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
	deferred_rendering_->SetupViewport(0, re.CurFrameBuffer(), 0);

	UIManager::Instance().SettleCtrls();
}

void DeferredRenderingApp::InputHandler(InputEngine const & /*sender*/, InputAction const & action)
{
	switch (action.first)
	{
	case Dump:
		deferred_rendering_->DumpIntermediaTextures();
		break;

	case Exit:
		this->Quit();
		break;

	case Profile:
#ifndef KLAYGE_SHIP
		PerfProfiler::Instance().ExportToCSV("profile.csv");
#endif
		break;
	}
}

void DeferredRenderingApp::BufferChangedHandler(UIComboBox const & sender)
{
	buffer_type_ = sender.GetSelectedIndex();
	deferred_rendering_->Display(static_cast<DeferredRenderingLayer::DisplayType>(buffer_type_));

	if (dialog_->Control<UICheckBox>(id_aa_)->GetChecked())
	{
		anti_alias_enabled_ = 1 + (DeferredRenderingLayer::DT_Edge == buffer_type_);
		RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
		re.PPAAEnabled(anti_alias_enabled_);
	}
}

void DeferredRenderingApp::IllumChangedHandler(UIComboBox const & sender)
{
	deferred_rendering_->DisplayIllum(sender.GetSelectedIndex());
}

void DeferredRenderingApp::ILScaleChangedHandler(KlayGE::UISlider const & sender)
{
	il_scale_ = sender.GetValue() / 10.0f;
	deferred_rendering_->IndirectScale(il_scale_);

	std::wostringstream stream;
	stream << L"Scale: " << il_scale_ << " x";
	dialog_->Control<UIStatic>(id_il_scale_static_)->SetText(stream.str());
}

void DeferredRenderingApp::SSVOHandler(UICheckBox const & sender)
{
	if ((DeferredRenderingLayer::DT_Final == buffer_type_) || (DeferredRenderingLayer::DT_SSVO == buffer_type_))
	{
		ssvo_enabled_ = sender.GetChecked();
		deferred_rendering_->SSVOEnabled(0, ssvo_enabled_);
		RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
		if (DeferredRenderingLayer::DT_SSVO == buffer_type_)
		{
			re.HDREnabled(false);
		}
		else
		{
			re.HDREnabled(hdr_enabled_);
		}
	}
}

void DeferredRenderingApp::HDRHandler(UICheckBox const & sender)
{
	if (DeferredRenderingLayer::DT_Final == buffer_type_)
	{
		hdr_enabled_ = sender.GetChecked();
		RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
		re.HDREnabled(hdr_enabled_);
	}
}

void DeferredRenderingApp::AntiAliasHandler(UICheckBox const & sender)
{
	if (DeferredRenderingLayer::DT_Final == buffer_type_)
	{
		anti_alias_enabled_ = sender.GetChecked();
		RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
		re.PPAAEnabled(anti_alias_enabled_);
	}
}

void DeferredRenderingApp::NumLightsChangedHandler(KlayGE::UISlider const & sender)
{
	int num_lights = sender.GetValue();

	for (size_t i = num_lights; i < particle_lights_.size(); ++ i)
	{
		particle_lights_[i]->DelFromSceneManager();
		particle_light_srcs_[i]->DelFromSceneManager();
	}

	size_t old_size = particle_lights_.size();

	particle_lights_.resize(num_lights);
	particle_light_srcs_.resize(num_lights);
	for (size_t i = old_size; i < particle_lights_.size(); ++ i)
	{
		particle_lights_[i] = MakeSharedPtr<PointLightSource>();
		particle_lights_[i]->Attrib(LightSource::LSA_NoShadow);
		particle_lights_[i]->Falloff(float3(1, 0, 1));
		particle_lights_[i]->AddToSceneManager();

		particle_light_srcs_[i] = MakeSharedPtr<SceneObjectLightSourceProxy>(particle_lights_[i]);
		checked_pointer_cast<SceneObjectLightSourceProxy>(particle_light_srcs_[i])->Scaling(0.1f, 0.1f, 0.1f);
		particle_light_srcs_[i]->AddToSceneManager();
	}

	std::wostringstream stream;
	stream << L"# lights: " << num_lights;
	dialog_->Control<UIStatic>(id_num_lights_static_)->SetText(stream.str());
}

void DeferredRenderingApp::CtrlCameraHandler(UICheckBox const & sender)
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

void DeferredRenderingApp::DoUpdateOverlay()
{
	RenderEngine& renderEngine(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
	SceneManager& scene_mgr = Context::Instance().SceneManagerInstance();

	UIManager::Instance().Render();

	font_->RenderText(0, 0, Color(1, 1, 0, 1), L"Deferred Rendering", 16);
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

	stream.str(L"");
	stream << ps_->NumActiveParticles() << " Particles";
	font_->RenderText(0, 72, Color(1, 1, 1, 1), stream.str(), 16);

	stream.str(L"");
	stream << scene_mgr.NumDrawCalls() << " Draws/frame "
		<< scene_mgr.NumDispatchCalls() << " Dispatches/frame";
	font_->RenderText(0, 90, Color(1, 1, 1, 1), stream.str(), 16);
}

uint32_t DeferredRenderingApp::DoUpdate(uint32_t pass)
{
	if (0 == pass)
	{
		for (uint32_t i = 0; i < particle_lights_.size(); ++ i)
		{
			float3 clr = MathLib::normalize(float3(sin(this->AppTime() * 0.3f + i * 10.0f),
				cos(this->AppTime() * 0.2f + 0.5f + i * 20.0f),
				sin(this->AppTime() * 0.1f + 1.0f + i * 30.0f))) * 0.3f + 0.1f;
			particle_lights_[i]->Color(clr);
			float factor = (50.0f + this->AppTime() * 0.6f) / particle_lights_.size();
			particle_lights_[i]->Position(float3(6.0f * sin(factor * i),
				5.0f + 10.0f / particle_lights_.size() * i, 6.0f * cos(factor * i) + 1));
		}
	}

	return deferred_rendering_->Update(pass);
}
