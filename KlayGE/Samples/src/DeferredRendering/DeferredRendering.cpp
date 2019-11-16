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
#include <KlayGE/ParticleSystem.hpp>
#include <KlayGE/PerfProfiler.hpp>

#include <iterator>
#include <sstream>

#include "SampleCommon.hpp"
#include "DeferredRendering.hpp"

using namespace std;
using namespace KlayGE;

namespace
{
	class SpotLightNodeUpdate
	{
	public:
		explicit SpotLightNodeUpdate(float3 const& init_pos)
			: init_pos_(init_pos), random_dis_(0, 1000)
		{
		}

		void operator()(SceneNode& node, float app_time, float elapsed_time)
		{
			KFL_UNUSED(app_time);
			KFL_UNUSED(elapsed_time);

			float3 dir(MathLib::clamp(random_dis_(gen_) * 0.0001f, 0.0f, 0.1f), 1, MathLib::clamp(random_dis_(gen_) * 0.0001f, 0.0f, 0.1f));
			node.TransformToParent(MathLib::inverse(MathLib::look_at_lh(init_pos_, init_pos_ + dir, float3(0, 0, 1))));
		}

	private:
		float3 init_pos_;

		ranlux24_base gen_;
		uniform_int_distribution<> random_dis_;
	};

	class SpotLightSourceUpdate
	{
	public:
		explicit SpotLightSourceUpdate(float3 const & clr)
			: random_dis_(0, 1000),
				color_(clr)
		{
		}

		void operator()(SceneComponent& component, float app_time, float elapsed_time)
		{
			KFL_UNUSED(app_time);
			KFL_UNUSED(elapsed_time);

			checked_cast<LightSource&>(component).Color(color_ * (0.85f + random_dis_(gen_) * 0.0003f));
		}

	private:
		ranlux24_base gen_;
		uniform_int_distribution<> random_dis_;
		float3 color_;
	};

	class GISpotLightNodeUpdate
	{
	public:
		explicit GISpotLightNodeUpdate(float3 const& init_pos)
			: init_pos_(init_pos)
		{
		}

		void operator()(SceneNode& node, float app_time, float elapsed_time)
		{
			KFL_UNUSED(elapsed_time);

			float3 dir(sin(app_time) * 0.3f, -1, 0.1f);
			node.TransformToParent(MathLib::inverse(MathLib::look_at_lh(init_pos_, init_pos_ + dir)));
		}

	private:
		float3 init_pos_;
	};

	class PointLightNodeUpdate
	{
	public:
		PointLightNodeUpdate(uint32_t index, uint32_t num_particle_lights)
			: index_(index), num_particle_lights_(num_particle_lights)
		{
		}

		void operator()(SceneNode& node, float app_time, float elapsed_time)
		{
			KFL_UNUSED(elapsed_time);

			float const factor = (50.0f + app_time * 0.6f) / num_particle_lights_;
			node.TransformToParent(MathLib::translation(
				6.0f * sin(factor * index_), 5.0f + 10.0f / num_particle_lights_ * index_, 6.0f * cos(factor * index_) + 1));
		}

	private:
		uint32_t index_;
		uint32_t num_particle_lights_;
	};

	class PointLightSourceUpdate
	{
	public:
		explicit PointLightSourceUpdate(uint32_t index)
			: index_(index)
		{
		}

		void operator()(SceneComponent& component, float app_time, float elapsed_time)
		{
			KFL_UNUSED(elapsed_time);

			float3 const clr = MathLib::normalize(float3(sin(app_time * 0.3f + index_ * 10.0f),
				cos(app_time * 0.2f + 0.5f + index_ * 20.0f), sin(app_time * 0.1f + 1.0f + index_ * 30.0f))) * 0.3f + 0.1f;
			checked_cast<LightSource&>(component).Color(clr);
		}

	private:
		uint32_t index_;
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
	auto scene_model = ASyncLoadModel("Sponza/sponza.glb", EAH_GPU_Read | EAH_Immutable,
		SceneNode::SOA_Cullable, AddToSceneRootHelper);

	font_ = SyncLoadFont("gkai00mp.kfont");

	deferred_rendering_ = Context::Instance().DeferredRenderingLayerInstance();
	deferred_rendering_->DepthFocus(10, 75);
	deferred_rendering_->BokehLuminanceThreshold(2.5f);

	auto& root_node = Context::Instance().SceneManagerInstance().SceneRootNode();

	AmbientLightSourcePtr ambient_light = MakeSharedPtr<AmbientLightSource>();
	ambient_light->SkylightTex(y_cube, c_cube);
	ambient_light->Color(float3(0.1f, 0.1f, 0.1f));
	root_node.AddComponent(ambient_light);

	float3 const torch_pos[2] = {{+14.6f, 3.7f, -4.3f}, {-18.6f, 3.7f, +6.5f}};

	{
		auto spot_light = MakeSharedPtr<SpotLightSource>();
		spot_light->Attrib(0);
		spot_light->Color(float3(1.0f, 0.17f, 0.05f) * 10.0f);
		spot_light->Falloff(float3(1, 0.5f, 0));
		spot_light->OuterAngle(PI / 2.5f);
		spot_light->InnerAngle(PI / 4);
		spot_light->OnMainThreadUpdate().Connect(SpotLightSourceUpdate(spot_light->Color()));

		auto spot_light_node = MakeSharedPtr<SceneNode>(SceneNode::SOA_Cullable | SceneNode::SOA_Moveable);
		spot_light_node->TransformToParent(MathLib::translation(torch_pos[0]));
		spot_light_node->OnMainThreadUpdate().Connect(SpotLightNodeUpdate(torch_pos[0]));
		spot_light_node->AddComponent(spot_light);
		root_node.AddChild(spot_light_node);
	}
	{
		auto spot_light = MakeSharedPtr<SpotLightSource>();
		spot_light->Attrib(0);
		spot_light->Color(float3(1.0f, 0.17f, 0.05f) * 10.0f);
		spot_light->Falloff(float3(1, 0.5f, 0));
		spot_light->OuterAngle(PI / 2.5f);
		spot_light->InnerAngle(PI / 4);
		spot_light->OnMainThreadUpdate().Connect(SpotLightSourceUpdate(spot_light->Color()));

		auto spot_light_node = MakeSharedPtr<SceneNode>(SceneNode::SOA_Cullable | SceneNode::SOA_Moveable);
		spot_light_node->TransformToParent(MathLib::translation(torch_pos[1]));
		spot_light_node->OnMainThreadUpdate().Connect(SpotLightNodeUpdate(torch_pos[1]));
		spot_light_node->AddComponent(spot_light);
		root_node.AddChild(spot_light_node);
	}
	{
		auto spot_light = MakeSharedPtr<SpotLightSource>();
		spot_light->Attrib(LightSource::LSA_IndirectLighting);
		spot_light->Color(float3(6.0f, 5.88f, 4.38f) * 10.0f);
		spot_light->Falloff(float3(1, 0.1f, 0));
		spot_light->OuterAngle(PI / 8);
		spot_light->InnerAngle(PI / 12);

		auto spot_light_node = MakeSharedPtr<SceneNode>(SceneNode::SOA_Cullable | SceneNode::SOA_Moveable);
		float3 const pos(0.0f, 43.2f, -5.9f);
		spot_light_node->TransformToParent(MathLib::translation(pos));
		spot_light_node->OnMainThreadUpdate().Connect(GISpotLightNodeUpdate(pos));
		spot_light_node->AddComponent(spot_light);
		root_node.AddChild(spot_light_node);

		auto spot_light_proxy = LoadLightSourceProxyModel(spot_light);
		spot_light_node->AddChild(spot_light_proxy->RootNode());
	}

	fpcController_.Scalers(0.05f, 0.5f);

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

	UIManager::Instance().Load(*ResLoader::Instance().Open("DeferredRendering.uiml"));
	dialog_ = UIManager::Instance().GetDialogs()[0];

	id_buffer_combo_ = dialog_->IDFromName("BufferCombo");
	id_illum_combo_ = dialog_->IDFromName("IllumCombo");
	id_il_scale_static_ = dialog_->IDFromName("ILScaleStatic");
	id_il_scale_slider_ = dialog_->IDFromName("ILScaleSlider");
	id_ssvo_ = dialog_->IDFromName("SSVO");
	id_hdr_ = dialog_->IDFromName("HDR");
	id_aa_ = dialog_->IDFromName("AA");
	id_dof_ = dialog_->IDFromName("DoF");
	id_bokeh_ = dialog_->IDFromName("Bokeh");
	id_motion_blur_ = dialog_->IDFromName("MotionBlur");
	id_num_lights_static_ = dialog_->IDFromName("NumLightsStatic");
	id_num_lights_slider_ = dialog_->IDFromName("NumLightsSlider");
	id_ctrl_camera_ = dialog_->IDFromName("CtrlCamera");

#if DEFAULT_DEFERRED != TRIDITIONAL_DEFERRED
	dialog_->Control<UIComboBox>(id_buffer_combo_)->RemoveItem(10);
	dialog_->Control<UIComboBox>(id_buffer_combo_)->RemoveItem(9);
#endif

	dialog_->Control<UIComboBox>(id_buffer_combo_)->OnSelectionChangedEvent().Connect(
		[this](UIComboBox const & sender)
		{
			this->BufferChangedHandler(sender);
		});
	this->BufferChangedHandler(*dialog_->Control<UIComboBox>(id_buffer_combo_));

	dialog_->Control<UIComboBox>(id_illum_combo_)->OnSelectionChangedEvent().Connect(
		[this](UIComboBox const & sender)
		{
			this->IllumChangedHandler(sender);
		});
	this->IllumChangedHandler(*dialog_->Control<UIComboBox>(id_illum_combo_));

	dialog_->Control<UISlider>(id_il_scale_slider_)->SetValue(static_cast<int>(il_scale_ * 10));
	dialog_->Control<UISlider>(id_il_scale_slider_)->OnValueChangedEvent().Connect(
		[this](UISlider const & sender)
		{
			this->ILScaleChangedHandler(sender);
		});
	this->ILScaleChangedHandler(*dialog_->Control<UISlider>(id_il_scale_slider_));

	dialog_->Control<UICheckBox>(id_ssvo_)->OnChangedEvent().Connect(
		[this](UICheckBox const & sender)
		{
			this->SSVOHandler(sender);
		});
	this->SSVOHandler(*dialog_->Control<UICheckBox>(id_ssvo_));
	dialog_->Control<UICheckBox>(id_hdr_)->OnChangedEvent().Connect(
		[this](UICheckBox const & sender)
		{
			this->HDRHandler(sender);
		});
	this->HDRHandler(*dialog_->Control<UICheckBox>(id_hdr_));
	dialog_->Control<UICheckBox>(id_aa_)->OnChangedEvent().Connect(
		[this](UICheckBox const & sender)
		{
			this->AntiAliasHandler(sender);
		});
	this->AntiAliasHandler(*dialog_->Control<UICheckBox>(id_aa_));
	dialog_->Control<UICheckBox>(id_dof_)->OnChangedEvent().Connect(
		[this](UICheckBox const& sender) { this->DepthOfFieldHandler(sender); });
	this->DepthOfFieldHandler(*dialog_->Control<UICheckBox>(id_dof_));
	dialog_->Control<UICheckBox>(id_bokeh_)->OnChangedEvent().Connect([this](UICheckBox const& sender) { this->BokehHandler(sender); });
	this->BokehHandler(*dialog_->Control<UICheckBox>(id_bokeh_));
	dialog_->Control<UICheckBox>(id_motion_blur_)->OnChangedEvent().Connect(
		[this](UICheckBox const& sender) { this->MotionBlurHandler(sender); });
	this->MotionBlurHandler(*dialog_->Control<UICheckBox>(id_motion_blur_));
	dialog_->Control<UISlider>(id_num_lights_slider_)->OnValueChangedEvent().Connect(
		[this](UISlider const & sender)
		{
			this->NumLightsChangedHandler(sender);
		});
	this->NumLightsChangedHandler(*dialog_->Control<UISlider>(id_num_lights_slider_));

	dialog_->Control<UICheckBox>(id_ctrl_camera_)->OnChangedEvent().Connect(
		[this](UICheckBox const & sender)
		{
			this->CtrlCameraHandler(sender);
		});
	this->CtrlCameraHandler(*dialog_->Control<UICheckBox>(id_ctrl_camera_));

	auto skybox = MakeSharedPtr<RenderableSkyBox>();
	skybox->CompressedCubeMap(y_cube, c_cube);
	root_node.AddChild(MakeSharedPtr<SceneNode>(MakeSharedPtr<RenderableComponent>(skybox), SceneNode::SOA_NotCastShadow));

	ps_ = SyncLoadParticleSystem("Fire.psml");
	ps_->Gravity(0.5f);
	ps_->MediaDensity(0.5f);
	root_node.AddChild(ps_->RootNode());

	float const SCALE = 3;
	ps_->RootNode()->TransformToParent(MathLib::scaling(SCALE, SCALE, SCALE));

	ParticleEmitterPtr emitter0 = ps_->Emitter(0);
	emitter0->ModelMatrix(MathLib::translation(torch_pos[0] / SCALE));

	ParticleEmitterPtr emitter1 = emitter0->Clone();
	emitter1->ModelMatrix(MathLib::translation(torch_pos[1] / SCALE));
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

void DeferredRenderingApp::DepthOfFieldHandler(UICheckBox const& sender)
{
	if (DeferredRenderingLayer::DT_Final == buffer_type_)
	{
		dof_enabled_ = sender.GetChecked();
		deferred_rendering_->DepthOfFieldEnabled(dof_enabled_, bokeh_enabled_);
	}
}

void DeferredRenderingApp::BokehHandler(UICheckBox const& sender)
{
	if (DeferredRenderingLayer::DT_Final == buffer_type_)
	{
		bokeh_enabled_ = sender.GetChecked();
		deferred_rendering_->DepthOfFieldEnabled(dof_enabled_, bokeh_enabled_);
	}
}

void DeferredRenderingApp::MotionBlurHandler(UICheckBox const& sender)
{
	if (DeferredRenderingLayer::DT_Final == buffer_type_)
	{
		motion_blur_enabled_ = sender.GetChecked();
		deferred_rendering_->MotionBlurEnabled(motion_blur_enabled_);
	}
}

void DeferredRenderingApp::NumLightsChangedHandler(KlayGE::UISlider const & sender)
{
	int const num_lights = sender.GetValue();

	auto& scene_mgr = Context::Instance().SceneManagerInstance();

	std::lock_guard<std::mutex> lock(scene_mgr.MutexForUpdate());

	for (uint32_t i = 0; i < particle_light_node_update_connections_.size(); ++i)
	{
		particle_light_node_update_connections_[i].Disconnect();
		particle_light_update_connections_[i].Disconnect();
	}

	auto& root_node = scene_mgr.SceneRootNode();

	for (size_t i = num_lights; i < particle_light_nodes_.size(); ++ i)
	{
		root_node.RemoveChild(particle_light_nodes_[i]);
	}

	size_t const old_size = particle_light_nodes_.size();

	particle_light_nodes_.resize(num_lights);
	for (size_t i = old_size; i < particle_light_nodes_.size(); ++i)
	{
		auto particle_light = MakeSharedPtr<PointLightSource>();
		particle_light->Attrib(LightSource::LSA_NoShadow);
		particle_light->Falloff(float3(1, 0, 1));

		particle_light_nodes_[i] = MakeSharedPtr<SceneNode>(SceneNode::SOA_Cullable | SceneNode::SOA_Moveable);
		particle_light_nodes_[i]->AddComponent(particle_light);

		auto particle_light_proxy = LoadLightSourceProxyModel(particle_light);
		particle_light_proxy->RootNode()->TransformToParent(
			MathLib::scaling(0.1f, 0.1f, 0.1f) * particle_light_proxy->RootNode()->TransformToParent());
		particle_light_nodes_[i]->AddChild(particle_light_proxy->RootNode());

		root_node.AddChild(particle_light_nodes_[i]);
	}

	particle_light_node_update_connections_.resize(num_lights);
	particle_light_update_connections_.resize(num_lights);
	for (uint32_t i = 0; i < particle_light_nodes_.size(); ++i)
	{
		particle_light_node_update_connections_[i] = particle_light_nodes_[i]->OnMainThreadUpdate().Connect(
			PointLightNodeUpdate(i, static_cast<uint32_t>(particle_light_nodes_.size())));

		auto light = particle_light_nodes_[i]->FirstComponentOfType<LightSource>();
		particle_light_update_connections_[i] = light->OnMainThreadUpdate().Connect(PointLightSourceUpdate(i));
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

	uint32_t const num_loading_res = ResLoader::Instance().NumLoadingResources();
	if (num_loading_res > 0)
	{
		stream.str(L"");
		stream << "Loading " << num_loading_res << " resources...";
		font_->RenderText(100, 300, Color(1, 0, 0, 1), stream.str(), 48);
	}
}

uint32_t DeferredRenderingApp::DoUpdate(uint32_t pass)
{
	if (pass == 0)
	{
		// Workaround for MotionBlur
		Camera& camera = this->ActiveCamera();
		camera.MainThreadUpdate(this->AppTime(), this->FrameTime());
	}

	return deferred_rendering_->Update(pass);
}
