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
#include <KlayGE/ParticleSystem.hpp>

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


	class DeferredRenderingDebug : public PostProcess
	{
	public:
		DeferredRenderingDebug()
			: PostProcess(L"DeferredRenderingDebug")
		{
			input_pins_.push_back(std::make_pair("g_buffer_tex", TexturePtr()));
			input_pins_.push_back(std::make_pair("depth_tex", TexturePtr()));
			input_pins_.push_back(std::make_pair("lighting_tex", TexturePtr()));
			input_pins_.push_back(std::make_pair("ssvo_tex", TexturePtr()));

			this->Technique(SyncLoadRenderEffect("DeferredRenderingDebug.fxml")->TechniqueByName("ShowPosition"));
		}

		void ShowType(int show_type)
		{
			switch (show_type)
			{
			case 0:
				break;

			case 1:
				technique_ = technique_->Effect().TechniqueByName("ShowPosition");
				break;

			case 2:
				technique_ = technique_->Effect().TechniqueByName("ShowNormal");
				break;

			case 3:
				technique_ = technique_->Effect().TechniqueByName("ShowDepth");
				break;

			case 4:
				break;

			case 5:
				technique_ = technique_->Effect().TechniqueByName("ShowSSVO");
				break;

			case 6:
				technique_ = technique_->Effect().TechniqueByName("ShowDiffuseLighting");
				break;

			case 7:
				technique_ = technique_->Effect().TechniqueByName("ShowSpecularLighting");
				break;

			default:
				break;
			}
		}

		void OnRenderBegin()
		{
			PostProcess::OnRenderBegin();

			Camera const & camera = Context::Instance().AppInstance().ActiveCamera();
			*(technique_->Effect().ParameterByName("inv_proj")) = camera.InverseProjMatrix();
			*(technique_->Effect().ParameterByName("depth_near_far_invfar")) = float3(camera.NearPlane(), camera.FarPlane(), 1 / camera.FarPlane());
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
				il_scale_(1.0f),
				num_objs_rendered_(0), num_renderable_rendered_(0),
				num_primitives_rendered_(0), num_vertices_rendered_(0)
{
	ResLoader::Instance().AddPath("../../Samples/media/DeferredRendering");
}

bool DeferredRenderingApp::ConfirmDevice() const
{
	RenderDeviceCaps const & caps = Context::Instance().RenderFactoryInstance().RenderEngineInstance().DeviceCaps();
	if (caps.max_shader_model < 2)
	{
		return false;
	}
	return true;
}

void DeferredRenderingApp::InitObjects()
{
	this->LookAt(float3(-14.5f, 18, -3), float3(-13.6f, 17.55f, -2.8f));
	this->Proj(0.1f, 500.0f);

	loading_percentage_ = 0;
	c_cube_tl_ = ASyncLoadTexture("Lake_CraterLake03_c.dds", EAH_GPU_Read | EAH_Immutable);
	y_cube_tl_ = ASyncLoadTexture("Lake_CraterLake03_y.dds", EAH_GPU_Read | EAH_Immutable);
	model_ml_ = ASyncLoadModel("sponza_crytek.7z//sponza_crytek.meshml", EAH_GPU_Read | EAH_Immutable);

	font_ = SyncLoadFont("gkai00mp.kfont");

	deferred_rendering_ = Context::Instance().DeferredRenderingLayerInstance();
	
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
	spot_light_[2]->Attrib(LSA_IndirectLighting);
	spot_light_[2]->Color(float3(6.0f, 5.88f, 4.38f) * 2.0f);
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

	fpcController_.Scalers(0.05f, 0.5f);

	InputEngine& inputEngine(Context::Instance().InputFactoryInstance().InputEngineInstance());
	InputActionMap actionMap;
	actionMap.AddActions(actions, actions + sizeof(actions) / sizeof(actions[0]));

	action_handler_t input_handler = MakeSharedPtr<input_signal>();
	input_handler->connect(KlayGE::bind(&DeferredRenderingApp::InputHandler, this, KlayGE::placeholders::_1, KlayGE::placeholders::_2));
	inputEngine.ActionMap(actionMap, input_handler);

	debug_pp_ = MakeSharedPtr<DeferredRenderingDebug>();

	UIManager::Instance().Load(ResLoader::Instance().Open("DeferredRendering.uiml"));
	dialog_ = UIManager::Instance().GetDialogs()[0];

	id_buffer_combo_ = dialog_->IDFromName("BufferCombo");
	id_illum_combo_ = dialog_->IDFromName("IllumCombo");
	id_il_scale_static_ = dialog_->IDFromName("ILScaleStatic");
	id_il_scale_slider_ = dialog_->IDFromName("ILScaleSlider");
	id_ssvo_ = dialog_->IDFromName("SSVO");
	id_hdr_ = dialog_->IDFromName("HDR");
	id_aa_ = dialog_->IDFromName("AA");
	id_ctrl_camera_ = dialog_->IDFromName("CtrlCamera");

	dialog_->Control<UIComboBox>(id_buffer_combo_)->OnSelectionChangedEvent().connect(KlayGE::bind(&DeferredRenderingApp::BufferChangedHandler, this, KlayGE::placeholders::_1));
	this->BufferChangedHandler(*dialog_->Control<UIComboBox>(id_buffer_combo_));

	dialog_->Control<UIComboBox>(id_illum_combo_)->OnSelectionChangedEvent().connect(KlayGE::bind(&DeferredRenderingApp::IllumChangedHandler, this, KlayGE::placeholders::_1));
	this->IllumChangedHandler(*dialog_->Control<UIComboBox>(id_illum_combo_));

	dialog_->Control<UISlider>(id_il_scale_slider_)->SetValue(static_cast<int>(il_scale_ * 10));
	dialog_->Control<UISlider>(id_il_scale_slider_)->OnValueChangedEvent().connect(KlayGE::bind(&DeferredRenderingApp::ILScaleChangedHandler, this, KlayGE::placeholders::_1));
	this->ILScaleChangedHandler(*dialog_->Control<UISlider>(id_il_scale_slider_));

	dialog_->Control<UICheckBox>(id_ssvo_)->OnChangedEvent().connect(KlayGE::bind(&DeferredRenderingApp::SSVOHandler, this, KlayGE::placeholders::_1));
	this->SSVOHandler(*dialog_->Control<UICheckBox>(id_ssvo_));
	dialog_->Control<UICheckBox>(id_hdr_)->OnChangedEvent().connect(KlayGE::bind(&DeferredRenderingApp::HDRHandler, this, KlayGE::placeholders::_1));
	this->HDRHandler(*dialog_->Control<UICheckBox>(id_hdr_));
	dialog_->Control<UICheckBox>(id_aa_)->OnChangedEvent().connect(KlayGE::bind(&DeferredRenderingApp::AntiAliasHandler, this, KlayGE::placeholders::_1));
	this->AntiAliasHandler(*dialog_->Control<UICheckBox>(id_aa_));

	dialog_->Control<UICheckBox>(id_ctrl_camera_)->OnChangedEvent().connect(KlayGE::bind(&DeferredRenderingApp::CtrlCameraHandler, this, KlayGE::placeholders::_1));
	this->CtrlCameraHandler(*dialog_->Control<UICheckBox>(id_ctrl_camera_));

	sky_box_ = MakeSharedPtr<SceneObjectSkyBox>();
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

	debug_pp_->InputPin(0, deferred_rendering_->GBufferRT0Tex(0));
	debug_pp_->InputPin(1, deferred_rendering_->DepthTex(0));
	debug_pp_->InputPin(2, deferred_rendering_->LightingTex(0));
	debug_pp_->InputPin(3, deferred_rendering_->SmallSSVOTex(0));

	UIManager::Instance().SettleCtrls();
}

void DeferredRenderingApp::InputHandler(InputEngine const & /*sender*/, InputAction const & action)
{
	switch (action.first)
	{
	case Exit:
		this->Quit();
		break;
	}
}

void DeferredRenderingApp::BufferChangedHandler(UIComboBox const & sender)
{
	buffer_type_ = sender.GetSelectedIndex();
	checked_pointer_cast<DeferredRenderingDebug>(debug_pp_)->ShowType(buffer_type_);

	if (dialog_->Control<UICheckBox>(id_aa_)->GetChecked())
	{
		anti_alias_enabled_ = 1 + (4 == buffer_type_);
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
	if ((0 == buffer_type_) || (5 == buffer_type_))
	{
		ssvo_enabled_ = sender.GetChecked();
		deferred_rendering_->SSVOEnabled(0, ssvo_enabled_);
		RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
		if (5 == buffer_type_)
		{
			re.HDREnabled(false);
		}
		else
		{
			re.HDREnabled(true);
		}
	}
}

void DeferredRenderingApp::HDRHandler(UICheckBox const & sender)
{
	if (0 == buffer_type_)
	{
		hdr_enabled_ = sender.GetChecked();
		RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
		re.HDREnabled(hdr_enabled_);
	}
}

void DeferredRenderingApp::AntiAliasHandler(UICheckBox const & sender)
{
	if (0 == buffer_type_)
	{
		anti_alias_enabled_ = sender.GetChecked();
		RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
		re.PPAAEnabled(anti_alias_enabled_);
	}
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

	UIManager::Instance().Render();

	font_->RenderText(0, 0, Color(1, 1, 0, 1), L"Deferred Rendering", 16);
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

uint32_t DeferredRenderingApp::DoUpdate(uint32_t pass)
{
	SceneManager& sceneMgr(Context::Instance().SceneManagerInstance());
	RenderEngine& renderEngine(Context::Instance().RenderFactoryInstance().RenderEngineInstance());

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
	}
	else if (14 == pass)
	{
		num_objs_rendered_ = sceneMgr.NumObjectsRendered();
		num_renderable_rendered_ = sceneMgr.NumRenderablesRendered();
		num_primitives_rendered_ = sceneMgr.NumPrimitivesRendered();
		num_vertices_rendered_ = sceneMgr.NumVerticesRendered();
	}
	else if (15 == pass)
	{
		if ((1 == buffer_type_) || (2 == buffer_type_) || (3 == buffer_type_))
		{
			renderEngine.BindFrameBuffer(FrameBufferPtr());
			renderEngine.CurFrameBuffer()->Attached(FrameBuffer::ATT_DepthStencil)->ClearDepth(1.0f);
			debug_pp_->Apply();
			return App3DFramework::URV_Skip_Postprocess | App3DFramework::URV_Finished;
		}
	}

	uint32_t ret = deferred_rendering_->Update(pass);
	if (ret & App3DFramework::URV_Finished)
	{
		if ((5 == buffer_type_) || (6 == buffer_type_) || (7 == buffer_type_))
		{
			renderEngine.BindFrameBuffer(FrameBufferPtr());
			renderEngine.CurFrameBuffer()->Attached(FrameBuffer::ATT_DepthStencil)->ClearDepth(1.0f);
			debug_pp_->Apply();
			return App3DFramework::URV_Skip_Postprocess | App3DFramework::URV_Finished;
		}
	}

	return ret;
}
