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
#include <KlayGE/Camera.hpp>
#include <KlayGE/DeferredRenderingLayer.hpp>
#include <KlayGE/ParticleSystem.hpp>
#include <KlayGE/PerfProfiler.hpp>

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
		SwitchableMesh(RenderModelPtr const & model, std::wstring const & name)
			: StaticMesh(model, name),
				lighting_(true)
		{
			gbuffers_effect_ = SyncLoadRenderEffect("GBuffer.fxml");
			no_lighting_gbuffers_effect_ = SyncLoadRenderEffect("DeepGBuffers.fxml");

			this->BindDeferredEffect(gbuffers_effect_);
			technique_ = special_shading_tech_;
		}

		void ReceivesLighting(bool lighting)
		{
			lighting_ = lighting;
			this->BindDeferredEffect(lighting ? gbuffers_effect_ : no_lighting_gbuffers_effect_);

			this->UpdateTechniques();
		}

		void Transparency(float alpha)
		{
			mtl_->albedo.w() = alpha;

			effect_attrs_ &= ~(EA_TransparencyFront | EA_TransparencyBack);
			if (alpha < 1 - 1e-6f)
			{
				effect_attrs_ |= (EA_TransparencyFront | EA_TransparencyBack | EA_SpecialShading);
			}
			this->UpdateTechniques();
		}

	private:
		bool lighting_;

		RenderEffectPtr gbuffers_effect_;
		RenderEffectPtr no_lighting_gbuffers_effect_;
	};

	class SpotLightSourceUpdate
	{
	public:
		explicit SpotLightSourceUpdate(float3 const & pos)
			: init_pos_(pos)
		{
		}

		void operator()(LightSource& light, float app_time, float elapsed_time)
		{
			KFL_UNUSED(elapsed_time);

			float3 new_pos = MathLib::transform_coord(float3(init_pos_.x(), 0, init_pos_.z()), MathLib::rotation_y(app_time));
			new_pos.y() = init_pos_.y();
			light.Position(new_pos);
			light.Direction(-new_pos);
		}

	private:
		float3 init_pos_;
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
	RenderablePtr scene_model = ASyncLoadModel("chipmunk-CE-01.meshml", EAH_GPU_Read | EAH_Immutable,
		CreateModelFactory<RenderModel>(), CreateMeshFactory<SwitchableMesh>());

	font_ = SyncLoadFont("gkai00mp.kfont");

	deferred_rendering_ = Context::Instance().DeferredRenderingLayerInstance();
	deferred_rendering_->SSVOEnabled(0, false);

	AmbientLightSourcePtr ambient_light = MakeSharedPtr<AmbientLightSource>();
	ambient_light->SkylightTex(y_cube, c_cube);
	ambient_light->Color(float3(0.1f, 0.1f, 0.1f));
	ambient_light->AddToSceneManager();
	
	spot_light_[0] = MakeSharedPtr<SpotLightSource>();
	spot_light_[0]->Attrib(LightSource::LSA_NoShadow);
	spot_light_[0]->Color(float3(100, 0, 0));
	spot_light_[0]->Falloff(float3(1, 0, 1));
	spot_light_[0]->Position(float3(10, 5, 10));
	spot_light_[0]->Direction(float3(0, 1, 0));
	spot_light_[0]->OuterAngle(PI / 2.5f);
	spot_light_[0]->InnerAngle(PI / 4);
	spot_light_[0]->BindUpdateFunc(SpotLightSourceUpdate(spot_light_[0]->Position()));
	spot_light_[0]->AddToSceneManager();

	spot_light_[1] = MakeSharedPtr<SpotLightSource>();
	spot_light_[1]->Attrib(LightSource::LSA_NoShadow);
	spot_light_[1]->Color(float3(0, 100, 0));
	spot_light_[1]->Falloff(float3(1, 0, 1));
	spot_light_[1]->Position(float3(-10, 5, -10));
	spot_light_[1]->Direction(float3(0, 1, 0));
	spot_light_[1]->OuterAngle(PI / 2.5f);
	spot_light_[1]->InnerAngle(PI / 4);
	spot_light_[1]->BindUpdateFunc(SpotLightSourceUpdate(spot_light_[1]->Position()));
	spot_light_[1]->AddToSceneManager();

	spot_light_src_[0] = MakeSharedPtr<SceneObjectLightSourceProxy>(spot_light_[0]);
	checked_pointer_cast<SceneObjectLightSourceProxy>(spot_light_src_[0])->Scaling(0.1f, 0.1f, 0.1f);
	spot_light_src_[0]->AddToSceneManager();
	spot_light_src_[1] = MakeSharedPtr<SceneObjectLightSourceProxy>(spot_light_[1]);
	checked_pointer_cast<SceneObjectLightSourceProxy>(spot_light_src_[1])->Scaling(0.1f, 0.1f, 0.1f);
	spot_light_src_[1]->AddToSceneManager();

	scene_obj_ = MakeSharedPtr<SceneObjectHelper>(scene_model, SceneObject::SOA_Cullable);
	scene_obj_->ModelMatrix(MathLib::scaling(3.0f, 3.0f, 3.0f));
	scene_obj_->AddToSceneManager();

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

	UIManager::Instance().Load(ResLoader::Instance().Open("DeepGBuffers.uiml"));
	dialog_ = UIManager::Instance().GetDialogs()[0];
	id_receives_lighting_ = dialog_->IDFromName("Lighting");
	id_transparency_static_ = dialog_->IDFromName("TransparencyStatic");
	id_transparency_slider_ = dialog_->IDFromName("TransparencySlider");
	id_ctrl_camera_ = dialog_->IDFromName("CtrlCamera");

	dialog_->Control<UICheckBox>(id_receives_lighting_)->OnChangedEvent().connect(
		[this](UICheckBox const & sender)
		{
			this->ReceivesLightingHandler(sender);
		});
	this->ReceivesLightingHandler(*dialog_->Control<UICheckBox>(id_receives_lighting_));

	dialog_->Control<UISlider>(id_transparency_slider_)->SetValue(static_cast<int>(transparency_ * 20));
	dialog_->Control<UISlider>(id_transparency_slider_)->OnValueChangedEvent().connect(
		[this](UISlider const & sender)
		{
			this->TransparencyChangedHandler(sender);
		});
	this->TransparencyChangedHandler(*dialog_->Control<UISlider>(id_transparency_slider_));

	dialog_->Control<UICheckBox>(id_ctrl_camera_)->OnChangedEvent().connect(
		[this](UICheckBox const & sender)
		{
			this->CtrlCameraHandler(sender);
		});
	this->CtrlCameraHandler(*dialog_->Control<UICheckBox>(id_ctrl_camera_));

	sky_box_ = MakeSharedPtr<SceneObjectSkyBox>();
	checked_pointer_cast<SceneObjectSkyBox>(sky_box_)->CompressedCubeMap(y_cube, c_cube);
	sky_box_->AddToSceneManager();
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

	RenderablePtr const & model = scene_obj_->GetRenderable();
	for (uint32_t i = 0; i < model->NumSubrenderables(); ++ i)
	{
		checked_pointer_cast<SwitchableMesh>(model->Subrenderable(i))->ReceivesLighting(lighting);
	}
}

void DeepGBuffersApp::TransparencyChangedHandler(UISlider const & sender)
{
	transparency_ = sender.GetValue() / 20.0f;

	RenderablePtr const & model = scene_obj_->GetRenderable();
	for (uint32_t i = 0; i < model->NumSubrenderables(); ++ i)
	{
		checked_pointer_cast<SwitchableMesh>(model->Subrenderable(i))->Transparency(transparency_);
	}

	std::wostringstream stream;
	stream << L"Transparency: " << transparency_;
	dialog_->Control<UIStatic>(id_transparency_static_)->SetText(stream.str());
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
}

uint32_t DeepGBuffersApp::DoUpdate(uint32_t pass)
{
	return deferred_rendering_->Update(pass);
}
