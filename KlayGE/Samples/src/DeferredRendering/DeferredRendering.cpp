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
#include "DeferredRendering.hpp"

using namespace std;
using namespace KlayGE;

namespace
{
	class SpotLightSourceUpdate
	{
	public:
		SpotLightSourceUpdate(float cone_radius, float cone_height, float org_angle, float rot_speed, float3 const & pos)
			: rot_speed_(rot_speed), pos_(pos)
		{
			model_org_ = MathLib::scaling(cone_radius, cone_radius, cone_height) * MathLib::rotation_x(org_angle);
		}

		void operator()(LightSource& light, float app_time, float /*elapsed_time*/)
		{
			light.ModelMatrix(model_org_ * MathLib::rotation_y(app_time * 1000 * rot_speed_)
				* MathLib::translation(pos_));
		}

	private:
		float4x4 model_org_;
		float rot_speed_;
		float3 pos_;
	};

	class GISpotLightSourceUpdate
	{
	public:
		GISpotLightSourceUpdate(float cone_radius, float cone_height, float org_angle, float rot_speed, float3 const & pos)
			: rot_speed_(rot_speed), pos_(pos)
		{
			model_org_ = MathLib::scaling(cone_radius, cone_radius, cone_height) * MathLib::rotation_x(org_angle);
		}

		void operator()(LightSource& light, float app_time, float /*elapsed_time*/)
		{
			light.ModelMatrix(model_org_ * MathLib::rotation_y(sin(app_time * 1000 * rot_speed_) * PI / 6)
				* MathLib::translation(pos_));
		}

	private:
		float4x4 model_org_;
		float rot_speed_;
		float3 pos_;
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

			this->Technique(Context::Instance().RenderFactoryInstance().LoadEffect("DeferredRenderingDebug.fxml")->TechniqueByName("ShowPosition"));
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
	model_ml_ = ASyncLoadModel("sponza_crytek.7z//sponza_crytek.meshml", EAH_GPU_Read | EAH_Immutable);
	y_cube_tl_ = ASyncLoadTexture("Lake_CraterLake03_y.dds", EAH_GPU_Read | EAH_Immutable);
	c_cube_tl_ = ASyncLoadTexture("Lake_CraterLake03_c.dds", EAH_GPU_Read | EAH_Immutable);

	font_ = Context::Instance().RenderFactoryInstance().MakeFont("gkai00mp.kfont");

	deferred_rendering_ = Context::Instance().DeferredRenderingLayerInstance();
	
	point_light_ = MakeSharedPtr<PointLightSource>();
	point_light_->Attrib(0);
	point_light_->Color(float3(0.8f, 0.96f, 1.0f));
	point_light_->Position(float3(0, 0, 0));
	point_light_->Falloff(float3(1, 0.5f, 0));
	point_light_->BindUpdateFunc(PointLightSourceUpdate(1 / 1000.0f, float3(2, 10, 0)));
	point_light_->AddToSceneManager();

	spot_light_[0] = MakeSharedPtr<SpotLightSource>();
	spot_light_[0]->Attrib(0);
	spot_light_[0]->Color(float3(2, 0, 0));
	spot_light_[0]->Falloff(float3(1, 0.5f, 0));
	spot_light_[0]->OuterAngle(PI / 6);
	spot_light_[0]->InnerAngle(PI / 8);
	spot_light_[0]->BindUpdateFunc(SpotLightSourceUpdate(sqrt(3.0f) / 3, 1.0f, PI, 1 / 1400.0f, float3(0.0f, 4.0f, 0.0f)));
	spot_light_[0]->AddToSceneManager();

	spot_light_[1] = MakeSharedPtr<SpotLightSource>();
	spot_light_[1]->Attrib(0);
	spot_light_[1]->Color(float3(0, 2, 0));
	spot_light_[1]->Falloff(float3(1, 0.5f, 0));
	spot_light_[1]->OuterAngle(PI / 4);
	spot_light_[1]->InnerAngle(PI / 6);
	spot_light_[1]->BindUpdateFunc(SpotLightSourceUpdate(1.0f, 1.0f, 0.0f, -1 / 700.0f, float3(0.0f, 3.4f, 0.0f)));
	spot_light_[1]->AddToSceneManager();

	spot_light_[2] = MakeSharedPtr<SpotLightSource>();
	spot_light_[2]->Attrib(LSA_IndirectLighting);
	spot_light_[2]->Color(float3(6.0f, 5.88f, 4.38f));
	spot_light_[2]->Falloff(float3(1, 0.1f, 0));
	spot_light_[2]->OuterAngle(PI / 6);
	spot_light_[2]->InnerAngle(PI / 8);
	spot_light_[2]->BindUpdateFunc(GISpotLightSourceUpdate(sqrt(3.0f) / 3, 1.0f, PI * 0.13f, 1 / 2800.0f, float3(0.0f, 16.0f, -4.8f)));
	spot_light_[2]->AddToSceneManager();

	point_light_src_ = MakeSharedPtr<SceneObjectLightSourceProxy>(point_light_);
	checked_pointer_cast<SceneObjectLightSourceProxy>(point_light_src_)->Scaling(0.1f, 0.1f, 0.1f);
	spot_light_src_[0] = MakeSharedPtr<SceneObjectLightSourceProxy>(spot_light_[0]);
	checked_pointer_cast<SceneObjectLightSourceProxy>(spot_light_src_[0])->Scaling(0.1f, 0.1f, 0.1f);
	spot_light_src_[1] = MakeSharedPtr<SceneObjectLightSourceProxy>(spot_light_[1]);
	checked_pointer_cast<SceneObjectLightSourceProxy>(spot_light_src_[1])->Scaling(0.1f, 0.1f, 0.1f);
	spot_light_src_[2] = MakeSharedPtr<SceneObjectLightSourceProxy>(spot_light_[2]);
	checked_pointer_cast<SceneObjectLightSourceProxy>(spot_light_src_[2])->Scaling(0.1f, 0.1f, 0.1f);
	point_light_src_->AddToSceneManager();
	spot_light_src_[0]->AddToSceneManager();
	spot_light_src_[1]->AddToSceneManager();
	spot_light_src_[2]->AddToSceneManager();

	fpcController_.Scalers(0.05f, 0.5f);

	InputEngine& inputEngine(Context::Instance().InputFactoryInstance().InputEngineInstance());
	InputActionMap actionMap;
	actionMap.AddActions(actions, actions + sizeof(actions) / sizeof(actions[0]));

	action_handler_t input_handler = MakeSharedPtr<input_signal>();
	input_handler->connect(KlayGE::bind(&DeferredRenderingApp::InputHandler, this, KlayGE::placeholders::_1, KlayGE::placeholders::_2));
	inputEngine.ActionMap(actionMap, input_handler, true);

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
}

void DeferredRenderingApp::OnResize(uint32_t width, uint32_t height)
{
	App3DFramework::OnResize(width, height);

	RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
	deferred_rendering_->SetupViewport(0, re.CurFrameBuffer(), 0);

	debug_pp_->InputPin(0, deferred_rendering_->OpaqueGBufferRT0Tex(0));
	debug_pp_->InputPin(1, deferred_rendering_->OpaqueDepthTex(0));
	debug_pp_->InputPin(2, deferred_rendering_->OpaqueLightingTex(0));
	debug_pp_->InputPin(3, deferred_rendering_->SmallSSVOTex(0));

	UIManager::Instance().SettleCtrls(width, height);
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
			else
			{
				scene_model_ = model_ml_();
				if (scene_model_)
				{
					scene_objs_.resize(scene_model_->NumMeshes());
					for (size_t i = 0; i < scene_model_->NumMeshes(); ++ i)
					{
						scene_objs_[i] = MakeSharedPtr<SceneObjectHelper>(scene_model_->Mesh(i), SceneObject::SOA_Cullable);
						scene_objs_[i]->AddToSceneManager();
					}

					loading_percentage_ = 100;
				}
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
