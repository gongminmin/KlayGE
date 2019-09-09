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
#include <KlayGE/Camera.hpp>
#include <KlayGE/DeferredRenderingLayer.hpp>
#include <KlayGE/ParticleSystem.hpp>
#include <KlayGE/PerfProfiler.hpp>

#include <iterator>
#include <sstream>

#include "SampleCommon.hpp"
#include "DeepGBuffers.hpp"

using namespace std;
using namespace KlayGE;

namespace
{
	class SwitchableMesh : public StaticMesh
	{
	public:
		explicit SwitchableMesh(std::wstring_view name)
			: StaticMesh(name)
		{
			gbuffers_effect_ = SyncLoadRenderEffect("GBuffer.fxml");
			no_lighting_gbuffers_effect_ = SyncLoadRenderEffect("DeepGBuffers.fxml");

			this->BindDeferredEffect(gbuffers_effect_);
			technique_ = special_shading_tech_;

			simple_forward_tech_ = no_lighting_gbuffers_effect_->TechniqueByName("NoLightingSimpleForwardTech");
		}

		void ReceivesLighting(bool lighting)
		{
			if (!simple_forward_)
			{
				lighting_ = lighting;
				this->BindDeferredEffect(lighting ? gbuffers_effect_ : no_lighting_gbuffers_effect_);

				this->UpdateTechniques();
			}
		}

		void Transparency(float alpha)
		{
			float4 const& albedo = mtl_->Albedo();
			mtl_->Albedo(float4(albedo.x(), albedo.y(), albedo.z(), alpha));

			if (!simple_forward_)
			{
				effect_attrs_ &= ~(EA_TransparencyFront | EA_TransparencyBack);
				if (alpha < 1 - 1e-6f)
				{
					effect_attrs_ |= (EA_TransparencyFront | EA_TransparencyBack | EA_SpecialShading);
				}
				this->UpdateTechniques();
			}
		}

		void SimpleForwardMode(bool simple_forward)
		{
			simple_forward_ = simple_forward;

			if (simple_forward_)
			{
				this->BindDeferredEffect(no_lighting_gbuffers_effect_);
				effect_attrs_ |= EA_SimpleForward;
			}
			else
			{
				effect_attrs_ &= ~EA_SimpleForward;

				this->ReceivesLighting(lighting_);
				this->Transparency(mtl_->Albedo().w());
			}
		}

	private:
		bool lighting_ = true;
		bool simple_forward_ = false;

		RenderEffectPtr gbuffers_effect_;
		RenderEffectPtr no_lighting_gbuffers_effect_;
	};


	enum
	{
		Exit,
		Dump,
		Profile
	};

	InputActionDefine actions[] =
	{
		InputActionDefine(Exit, KS_Escape)
	};
}

int SampleMain()
{
	ContextCfg cfg = Context::Instance().Config();
	cfg.deferred_rendering = true;
	Context::Instance().Config(cfg);

	DeepGBuffersApp app;
	app.Create();
	app.Run();

	return 0;
}

DeepGBuffersApp::DeepGBuffersApp()
			: App3DFramework("DeepGBuffers"),
				transparency_(1.0f)
{
	ResLoader::Instance().AddPath("../../Samples/media/DeepGBuffers");
}

void DeepGBuffersApp::OnCreate()
{
	this->LookAt(float3(0, 2.44f, -5.72f), float3(0.61f, 0.8f, 2.28f));
	this->Proj(0.1f, 500.0f);

	TexturePtr c_cube = ASyncLoadTexture("Lake_CraterLake03_filtered_c.dds", EAH_GPU_Read | EAH_Immutable);
	TexturePtr y_cube = ASyncLoadTexture("Lake_CraterLake03_filtered_y.dds", EAH_GPU_Read | EAH_Immutable);
	scene_model_ = ASyncLoadModel("chipmunk-CE-01.glb", EAH_GPU_Read | EAH_Immutable,
		SceneNode::SOA_Cullable,
		[](RenderModel& model)
		{
			model.RootNode()->TransformToParent(MathLib::scaling(3.0f, 3.0f, 3.0f));
			AddToSceneRootHelper(model);
		},
		CreateModelFactory<RenderModel>, CreateMeshFactory<SwitchableMesh>);

	font_ = SyncLoadFont("gkai00mp.kfont");

	deferred_rendering_ = Context::Instance().DeferredRenderingLayerInstance();
	deferred_rendering_->SSVOEnabled(0, false);

	auto& root_node = Context::Instance().SceneManagerInstance().SceneRootNode();

	AmbientLightSourcePtr ambient_light = MakeSharedPtr<AmbientLightSource>();
	ambient_light->SkylightTex(y_cube, c_cube);
	ambient_light->Color(float3(0.1f, 0.1f, 0.1f));
	root_node.AddComponent(ambient_light);

	float3 const light_colors[] = {{100, 0, 0}, {0, 100, 0}};
	float3 const light_positions[] = {{10, 5, 10}, {-10, 5, -10}};
	for (uint32_t i = 0; i < 2; ++i)
	{
		auto spot_light = MakeSharedPtr<SpotLightSource>();
		spot_light->Attrib(LightSource::LSA_NoShadow);
		spot_light->Color(light_colors[i]);
		spot_light->Falloff(float3(1, 0, 1));
		spot_light->OuterAngle(PI / 2.5f);
		spot_light->InnerAngle(PI / 4);
		float3 const light_pos = light_positions[i];
		auto spot_light_node = MakeSharedPtr<SceneNode>(SceneNode::SOA_Cullable | SceneNode::SOA_Moveable);
		spot_light_node->OnMainThreadUpdate().Connect([light_pos](SceneNode& node, float app_time, float elapsed_time)
		{
			KFL_UNUSED(elapsed_time);

			float3 new_pos = MathLib::transform_coord(float3(light_pos.x(), 0, light_pos.z()), MathLib::rotation_y(app_time));
			new_pos.y() = light_pos.y();
			node.TransformToParent(MathLib::inverse(MathLib::look_at_lh(new_pos, float3(0, 0, 0))));
		});
		spot_light_node->AddComponent(spot_light);
		root_node.AddChild(spot_light_node);

		auto spot_light_proxy = LoadLightSourceProxyModel(spot_light);
		spot_light_proxy->RootNode()->TransformToParent(
			MathLib::scaling(0.1f, 0.1f, 0.1f) * spot_light_proxy->RootNode()->TransformToParent());
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

	UIManager::Instance().Load(*ResLoader::Instance().Open("DeepGBuffers.uiml"));
	dialog_ = UIManager::Instance().GetDialogs()[0];
	id_receives_lighting_ = dialog_->IDFromName("Lighting");
	id_transparency_static_ = dialog_->IDFromName("TransparencyStatic");
	id_transparency_slider_ = dialog_->IDFromName("TransparencySlider");
	id_simple_forward_ = dialog_->IDFromName("SimpleForward");
	id_ctrl_camera_ = dialog_->IDFromName("CtrlCamera");

	dialog_->Control<UICheckBox>(id_receives_lighting_)->OnChangedEvent().Connect(
		[this](UICheckBox const & sender)
		{
			this->ReceivesLightingHandler(sender);
		});
	this->ReceivesLightingHandler(*dialog_->Control<UICheckBox>(id_receives_lighting_));

	dialog_->Control<UISlider>(id_transparency_slider_)->SetValue(static_cast<int>(transparency_ * 20));
	dialog_->Control<UISlider>(id_transparency_slider_)->OnValueChangedEvent().Connect(
		[this](UISlider const & sender)
		{
			this->TransparencyChangedHandler(sender);
		});
	this->TransparencyChangedHandler(*dialog_->Control<UISlider>(id_transparency_slider_));

	dialog_->Control<UICheckBox>(id_simple_forward_)->OnChangedEvent().Connect(
		[this](UICheckBox const & sender)
		{
			this->SimpleForwardHandler(sender);
		});
	this->SimpleForwardHandler(*dialog_->Control<UICheckBox>(id_simple_forward_));

	dialog_->Control<UICheckBox>(id_ctrl_camera_)->OnChangedEvent().Connect(
		[this](UICheckBox const & sender)
		{
			this->CtrlCameraHandler(sender);
		});
	this->CtrlCameraHandler(*dialog_->Control<UICheckBox>(id_ctrl_camera_));

	auto skybox = MakeSharedPtr<RenderableSkyBox>();
	skybox->CompressedCubeMap(y_cube, c_cube);
	root_node.AddChild(MakeSharedPtr<SceneNode>(MakeSharedPtr<RenderableComponent>(skybox), SceneNode::SOA_NotCastShadow));
}

void DeepGBuffersApp::OnResize(uint32_t width, uint32_t height)
{
	App3DFramework::OnResize(width, height);

	RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
	deferred_rendering_->SetupViewport(0, re.CurFrameBuffer(), 0);

	UIManager::Instance().SettleCtrls();
}

void DeepGBuffersApp::InputHandler(InputEngine const & /*sender*/, InputAction const & action)
{
	switch (action.first)
	{
	case Exit:
		this->Quit();
		break;
	}
}

void DeepGBuffersApp::ReceivesLightingHandler(UICheckBox const & sender)
{
	bool lighting = sender.GetChecked();

	for (uint32_t i = 0; i < scene_model_->NumMeshes(); ++ i)
	{
		checked_pointer_cast<SwitchableMesh>(scene_model_->Mesh(i))->ReceivesLighting(lighting);
	}
}

void DeepGBuffersApp::TransparencyChangedHandler(UISlider const & sender)
{
	transparency_ = sender.GetValue() / 20.0f;

	for (uint32_t i = 0; i < scene_model_->NumMeshes(); ++ i)
	{
		checked_pointer_cast<SwitchableMesh>(scene_model_->Mesh(i))->Transparency(transparency_);
	}

	std::wostringstream stream;
	stream << L"Transparency: " << transparency_;
	dialog_->Control<UIStatic>(id_transparency_static_)->SetText(stream.str());
}

void DeepGBuffersApp::SimpleForwardHandler(UICheckBox const & sender)
{
	bool const simple_forward = sender.GetChecked();

	auto control = dialog_->Control<UICheckBox>(id_receives_lighting_);
	control->SetEnabled(!simple_forward);

	for (uint32_t i = 0; i < scene_model_->NumMeshes(); ++ i)
	{
		checked_pointer_cast<SwitchableMesh>(scene_model_->Mesh(i))->SimpleForwardMode(simple_forward);
	}
}

void DeepGBuffersApp::CtrlCameraHandler(UICheckBox const & sender)
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

void DeepGBuffersApp::DoUpdateOverlay()
{
	RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();

	UIManager::Instance().Render();

	font_->RenderText(0, 0, Color(1, 1, 0, 1), L"Deferred Rendering", 16);
	font_->RenderText(0, 18, Color(1, 1, 0, 1), re.ScreenFrameBuffer()->Description(), 16);

	std::wostringstream stream;
	stream.precision(2);
	stream << std::fixed << this->FPS() << " FPS";
	font_->RenderText(0, 36, Color(1, 1, 0, 1), stream.str(), 16);

	uint32_t const num_loading_res = ResLoader::Instance().NumLoadingResources();
	if (num_loading_res > 0)
	{
		stream.str(L"");
		stream << "Loading " << num_loading_res << " resources...";
		font_->RenderText(100, 300, Color(1, 0, 0, 1), stream.str(), 48);
	}
}

uint32_t DeepGBuffersApp::DoUpdate(uint32_t pass)
{
	return deferred_rendering_->Update(pass);
}
