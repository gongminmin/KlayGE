#include <KlayGE/KlayGE.hpp>
#include <KlayGE/ResLoader.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/FrameBuffer.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/InputFactory.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/RenderEffect.hpp>
#include <KlayGE/SceneObject.hpp>
#include <KlayGE/SceneObjectHelper.hpp>
#include <KlayGE/Camera.hpp>
#include <KlayGE/Mesh.hpp>
#include <KlayGE/PostProcess.hpp>
#include <KlayGE/Light.hpp>
#include <KlayGE/DeferredRenderingLayer.hpp>
#include <KlayGE/UI.hpp>
#include <KlayGE/RenderableHelper.hpp>
#include <KlayGE/SceneManager.hpp>

#include <KlayGE/SSRPostProcess.hpp>

#include <sstream>
#include <boost/bind.hpp>
#include <boost/foreach.hpp>
#include <boost/typeof/typeof.hpp>

#include "AtmosphericScattering.hpp"

using namespace KlayGE;

namespace
{
	class AtmosphericScatteringMesh : public StaticMesh
	{
	public:
		AtmosphericScatteringMesh(RenderModelPtr const & model, std::wstring const & name)
			: StaticMesh(model, name)
		{
			RenderFactory & rf = Context::Instance().RenderFactoryInstance();
			RenderEffectPtr effect = rf.LoadEffect("AtmosphericScattering.fxml");
			technique_ = effect->TechniqueByName("AtmosphericScatteringTech");
		}

		void BuildMeshInfo()
		{
			aabb_.Min() *= 1.2f;
			aabb_.Max() *= 1.2f;
		}

		void AtmosphereTop(float top)
		{
			*(technique_->Effect().ParameterByName("atmosphere_top")) = top;
		}

		void Density(float density)
		{
			*(technique_->Effect().ParameterByName("density")) = density;
		}
		
		void OnRenderBegin()
		{
			App3DFramework const & app = Context::Instance().AppInstance();

			float4x4 const & view = app.ActiveCamera().ViewMatrix();
			float4x4 const & proj = app.ActiveCamera().ProjMatrix();

			*(technique_->Effect().ParameterByName("mvp")) = model_mat_ * view * proj;

			float4x4 inv_mv = MathLib::inverse(model_mat_ * view);
			*(technique_->Effect().ParameterByName("eye_pos")) = MathLib::transform_coord(float3(0, 0, 0), inv_mv);
			*(technique_->Effect().ParameterByName("look_at_vec")) = MathLib::transform_normal(float3(0, 0, 1), inv_mv);
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
	cfg.graphics_cfg.hdr = true;
	cfg.deferred_rendering = true;
	Context::Instance().Config(cfg);

	AtmosphericScatteringApp app;
	app.Create();
	app.Run();
	
	return 0;
}

AtmosphericScatteringApp::AtmosphericScatteringApp()
	: App3DFramework("Atmospheric Scattering")
{
	ResLoader::Instance().AddPath("../../Samples/media/AtmosphericScattering");
}

bool AtmosphericScatteringApp::ConfirmDevice() const
{
	RenderDeviceCaps const & caps = Context::Instance().RenderFactoryInstance().RenderEngineInstance().DeviceCaps();
	if (caps.max_shader_model < 2)
	{
		return false;
	}
	return true;
}

void AtmosphericScatteringApp::InitObjects()
{
	font_ = Context::Instance().RenderFactoryInstance().MakeFont("gkai00mp.kfont");

	this->LookAt(float3(0, 0, -4.0f), float3(0, 0, 0), float3(0, 1, 0));
	this->Proj(0.01f, 500.0f);

	tb_controller_.AttachCamera(this->ActiveCamera());
	tb_controller_.Scalers(0.003f, 0.003f);

	RenderModelPtr model_sphere = SyncLoadModel("geosphere.7z//geosphere.meshml", EAH_GPU_Read,
		CreateModelFactory<RenderModel>(), CreateMeshFactory<AtmosphericScatteringMesh>());
	sphere_ = MakeSharedPtr<SceneObjectHelper>(model_sphere->Mesh(0), SceneObjectHelper::SOA_Cullable);
	sphere_->AddToSceneManager();

	UIManager::Instance().Load(ResLoader::Instance().Open("AtmosphericScattering.uiml"));
	dialog_param_ = UIManager::Instance().GetDialog("AtmosphericScattering");
	id_atmosphere_top_ = dialog_param_->IDFromName("atmosphere_top");
	id_density_ = dialog_param_->IDFromName("density");

	dialog_param_->Control<UISlider>(id_atmosphere_top_)->OnValueChangedEvent().connect(boost::bind(&AtmosphericScatteringApp::AtmosphereTopHandler, this, _1));
	this->AtmosphereTopHandler(*(dialog_param_->Control<UISlider>(id_atmosphere_top_)));

	dialog_param_->Control<UISlider>(id_density_)->OnValueChangedEvent().connect(boost::bind(&AtmosphericScatteringApp::DensityHandler, this, _1));
	this->DensityHandler(*(dialog_param_->Control<UISlider>(id_density_)));

	sun_light_ = MakeSharedPtr<DirectionalLightSource>();
	sun_light_->Attrib(0);
	sun_light_->Color(float3(1, 1, 1));
	sun_light_->Direction(float3(-1, 0, 0));
	sun_light_->AddToSceneManager();

	sun_light_src_ = MakeSharedPtr<SceneObjectLightSourceProxy>(sun_light_);
	checked_pointer_cast<SceneObjectLightSourceProxy>(sun_light_src_)->Scaling(0.1f, 0.1f, 0.1f);
	sun_light_src_->AddToSceneManager();

	InputEngine& inputEngine(Context::Instance().InputFactoryInstance().InputEngineInstance());
	InputActionMap actionMap;
	actionMap.AddActions(actions, actions + sizeof(actions) / sizeof(actions[0]));

	action_handler_t input_handler = MakeSharedPtr<input_signal>();
	input_handler->connect(boost::bind(&AtmosphericScatteringApp::InputHandler, this, _1, _2));
	inputEngine.ActionMap(actionMap, input_handler, true);
}

void AtmosphericScatteringApp::OnResize(KlayGE::uint32_t width, KlayGE::uint32_t height)
{
	App3DFramework::OnResize(width, height);

	UIManager::Instance().SettleCtrls(width, height);
}

void AtmosphericScatteringApp::InputHandler(KlayGE::InputEngine const & /*sender*/, KlayGE::InputAction const & action)
{
	switch (action.first)
	{
	case Exit:
		this->Quit();
		break;
	}
}

void AtmosphericScatteringApp::AtmosphereTopHandler(KlayGE::UISlider const & sender)
{
	float value = 1 + sender.GetValue() / 1000.0f;
	checked_pointer_cast<AtmosphericScatteringMesh>(sphere_->GetRenderable())->AtmosphereTop(value);
}

void AtmosphericScatteringApp::DensityHandler(KlayGE::UISlider const & sender)
{
	float value = sender.GetValue() / 100000.0f;
	checked_pointer_cast<AtmosphericScatteringMesh>(sphere_->GetRenderable())->Density(value);
}

void AtmosphericScatteringApp::DoUpdateOverlay()
{
	UIManager::Instance().Render();

	font_->RenderText(0, 0, Color(1, 1, 0, 1), L"Atmospheric Scattering", 16);

	std::wostringstream stream;
	stream.precision(2);
	stream << std::fixed << this->FPS() << " FPS";
	font_->RenderText(0, 18, Color(1, 1, 0, 1), stream.str(), 16);
}

uint32_t AtmosphericScatteringApp::DoUpdate(KlayGE::uint32_t /*pass*/)
{
	RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();

	re.BindFrameBuffer(FrameBufferPtr());
	Color clear_clr(0.0f, 0.0f, 0.0f, 1);

	re.CurFrameBuffer()->Clear(FrameBuffer::CBM_Color | FrameBuffer::CBM_Depth, clear_clr, 1, 0);
	return App3DFramework::URV_Need_Flush | App3DFramework::URV_Finished;
}
