#include <KlayGE/KlayGE.hpp>
#include <KFL/CXX17/iterator.hpp>
#include <KlayGE/ResLoader.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/FrameBuffer.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/InputFactory.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/RenderEffect.hpp>
#include <KlayGE/SceneNode.hpp>
#include <KlayGE/Camera.hpp>
#include <KlayGE/Mesh.hpp>
#include <KlayGE/PostProcess.hpp>
#include <KlayGE/Light.hpp>
#include <KlayGE/DeferredRenderingLayer.hpp>
#include <KlayGE/UI.hpp>
#include <KlayGE/Renderable.hpp>
#include <KlayGE/SceneManager.hpp>
#include <KlayGE/Window.hpp>

#include <KlayGE/SSRPostProcess.hpp>

#include <sstream>

#include "SampleCommon.hpp"
#include "AtmosphericScattering.hpp"

using namespace KlayGE;

namespace
{
	class PlanetMesh : public StaticMesh
	{
	public:
		explicit PlanetMesh(std::wstring_view name)
			: StaticMesh(name)
		{
			effect_ = SyncLoadRenderEffect("AtmosphericScattering.fxml");
			technique_ = effect_->TechniqueByName("PlanetTech");
		}

		void DoBuildMeshInfo(RenderModel const & model) override
		{
			KFL_UNUSED(model);
		}

		void LightDir(float3 const & dir)
		{
			*(effect_->ParameterByName("light_dir")) = dir;
		}

		void Density(float density)
		{
			*(effect_->ParameterByName("density")) = density;
		}

		void Beta(Color const & clr)
		{
			*(effect_->ParameterByName("beta")) = float3(clr.r(), clr.g(), clr.b());
		}

		void Absorb(Color const & clr)
		{
			*(effect_->ParameterByName("absorb")) = float3(clr.r(), clr.g(), clr.b());
		}
	};

	class AtmosphereMesh : public StaticMesh
	{
	public:
		explicit AtmosphereMesh(std::wstring_view name)
			: StaticMesh(name)
		{
			effect_ = SyncLoadRenderEffect("AtmosphericScattering.fxml");
			technique_ = effect_->TechniqueByName("AtmosphereTech");
		}

		virtual void DoBuildMeshInfo(RenderModel const & model) override
		{
			KFL_UNUSED(model);

			pos_aabb_.Min() *= 1.2f;
			pos_aabb_.Max() *= 1.2f;
		}

		void AtmosphereTop(float top)
		{
			*(effect_->ParameterByName("atmosphere_top")) = top;
		}

		void LightDir(float3 const & dir)
		{
			*(effect_->ParameterByName("light_dir")) = dir;
		}

		void Density(float density)
		{
			*(effect_->ParameterByName("density")) = density;
		}

		void Beta(Color const & clr)
		{
			*(effect_->ParameterByName("beta")) = float3(clr.r(), clr.g(), clr.b());
		}

		void Absorb(Color const & clr)
		{
			*(effect_->ParameterByName("absorb")) = float3(clr.r(), clr.g(), clr.b());
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
	AtmosphericScatteringApp app;
	app.Create();
	app.Run();
	
	return 0;
}

AtmosphericScatteringApp::AtmosphericScatteringApp()
	: App3DFramework("Atmospheric Scattering"),
		obj_controller_(true, MB_Left, MB_Middle, 0),
		light_controller_(true, MB_Right, 0, 0)
{
	ResLoader::Instance().AddPath("../../Samples/media/AtmosphericScattering");
}

void AtmosphericScatteringApp::OnCreate()
{
	font_ = SyncLoadFont("gkai00mp.kfont");

	this->LookAt(float3(0, 0, -4.0f), float3(0, 0, 0), float3(0, 1, 0));
	this->Proj(0.01f, 500.0f);

	obj_controller_.AttachCamera(this->ActiveCamera());
	obj_controller_.Scalers(0.003f, 0.003f);

	auto& root_node = Context::Instance().SceneManagerInstance().SceneRootNode();

	auto light_ctrl_camera_node =
		MakeSharedPtr<SceneNode>(SceneNode::SOA_Cullable | SceneNode::SOA_Moveable | SceneNode::SOA_NotCastShadow);
	light_ctrl_camera_ = MakeSharedPtr<Camera>();
	light_ctrl_camera_node->AddComponent(light_ctrl_camera_);
	light_ctrl_camera_->LookAtDist(0.01f);
	light_ctrl_camera_node->TransformToParent(
		MathLib::inverse(MathLib::look_at_lh(float3(-0.01f, 0, 0), float3(0, 0, 0), float3(0, -1, 0))));
	root_node.AddChild(light_ctrl_camera_node);
	light_controller_.AttachCamera(*light_ctrl_camera_);
	light_controller_.Scalers(0.003f, 0.003f);

	planet_model_ = SyncLoadModel("geosphere.glb", EAH_GPU_Read | EAH_Immutable,
		SceneNode::SOA_Cullable, AddToSceneRootHelper,
		CreateModelFactory<RenderModel>, CreateMeshFactory<PlanetMesh>);

	atmosphere_model_ = SyncLoadModel("geosphere.glb", EAH_GPU_Read | EAH_Immutable,
		SceneNode::SOA_Cullable, AddToSceneRootHelper,
		CreateModelFactory<RenderModel>, CreateMeshFactory<AtmosphereMesh>);

	UIManager::Instance().Load(ResLoader::Instance().Open("AtmosphericScattering.uiml"));
	dialog_param_ = UIManager::Instance().GetDialog("AtmosphericScattering");
	id_atmosphere_top_ = dialog_param_->IDFromName("atmosphere_top");
	id_density_ = dialog_param_->IDFromName("density");
	id_beta_button_ = dialog_param_->IDFromName("beta_button");
	id_absorb_button_ = dialog_param_->IDFromName("absorb_button");

	dialog_param_->Control<UISlider>(id_atmosphere_top_)->OnValueChangedEvent().Connect(
		[this](UISlider const & sender)
		{
			this->AtmosphereTopHandler(sender);
		});
	this->AtmosphereTopHandler(*(dialog_param_->Control<UISlider>(id_atmosphere_top_)));

	dialog_param_->Control<UISlider>(id_density_)->OnValueChangedEvent().Connect(
		[this](UISlider const & sender)
		{
			this->DensityHandler(sender);
		});
	this->DensityHandler(*(dialog_param_->Control<UISlider>(id_density_)));

	dialog_param_->Control<UITexButton>(id_beta_button_)->OnClickedEvent().Connect(
		[this](UITexButton const & sender)
		{
			this->ChangeBetaHandler(sender);
		});
	dialog_param_->Control<UITexButton>(id_absorb_button_)->OnClickedEvent().Connect(
		[this](UITexButton const & sender)
		{
			this->ChangeAbsorbHandler(sender);
		});

	this->LoadBeta(Color(38.05f, 82.36f, 214.65f, 1));
	this->LoadAbsorb(Color(0.75f, 0.85f, 1, 1));

	auto sun_light = MakeSharedPtr<DirectionalLightSource>();
	sun_light->Attrib(LightSource::LSA_NoShadow);
	sun_light->Color(float3(1, 1, 1));
	auto sun_light_node = MakeSharedPtr<SceneNode>(SceneNode::SOA_Cullable);
	auto sun_light_proxy = LoadLightSourceProxyModel(sun_light);
	sun_light_proxy->RootNode()->TransformToParent(MathLib::scaling(0.1f, 0.1f, 0.1f) * sun_light_proxy->RootNode()->TransformToParent());
	sun_light_node->AddChild(sun_light_proxy->RootNode());
	sun_light_node->OnMainThreadUpdate().Connect([this](SceneNode& node, float app_time, float elapsed_time) {
		KFL_UNUSED(app_time);
		KFL_UNUSED(elapsed_time);

		node.TransformToParent(light_ctrl_camera_->InverseViewMatrix());
	});
	sun_light_node->AddComponent(sun_light);
	root_node.AddChild(sun_light_node);

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
}

void AtmosphericScatteringApp::OnResize(KlayGE::uint32_t width, KlayGE::uint32_t height)
{
	App3DFramework::OnResize(width, height);

	UIManager::Instance().SettleCtrls();
}

void AtmosphericScatteringApp::LoadBeta(Color const & clr)
{
	RenderFactory& rf = Context::Instance().RenderFactoryInstance();

	planet_model_->ForEachMesh([clr](Renderable& renderable)
		{
			checked_cast<PlanetMesh&>(renderable).Beta(clr);
		});
	atmosphere_model_->ForEachMesh([clr](Renderable& renderable)
		{
			checked_cast<AtmosphereMesh&>(renderable).Beta(clr);
		});

	Color f4_clr = clr / 250.0f;

	auto const fmt = rf.RenderEngineInstance().DeviceCaps().BestMatchTextureFormat(MakeSpan({EF_ABGR8, EF_ARGB8}));
	BOOST_ASSERT(fmt != EF_Unknown);
	uint32_t data = 0xFF000000 | ((fmt == EF_ABGR8) ? f4_clr.ABGR() : f4_clr.ARGB());

	ElementInitData init_data;
	init_data.data = &data;
	init_data.row_pitch = 4;
	TexturePtr tex_for_button = rf.MakeTexture2D(1, 1, 1, 1, fmt, 1, 0, EAH_GPU_Read | EAH_Immutable, MakeSpan<1>(init_data));
	dialog_param_->Control<UITexButton>(id_beta_button_)->SetTexture(tex_for_button);
}

void AtmosphericScatteringApp::LoadAbsorb(Color const & clr)
{
	RenderFactory& rf = Context::Instance().RenderFactoryInstance();

	planet_model_->ForEachMesh([clr](Renderable& renderable)
		{
			checked_cast<PlanetMesh&>(renderable).Absorb(clr);
		});
	atmosphere_model_->ForEachMesh([clr](Renderable& renderable)
		{
			checked_cast<AtmosphereMesh&>(renderable).Absorb(clr);
		});

	auto const fmt = rf.RenderEngineInstance().DeviceCaps().BestMatchTextureFormat(MakeSpan({EF_ABGR8, EF_ARGB8}));
	BOOST_ASSERT(fmt != EF_Unknown);
	uint32_t data = 0xFF000000 | ((fmt == EF_ABGR8) ? clr.ABGR() : clr.ARGB());

	ElementInitData init_data;
	init_data.data = &data;
	init_data.row_pitch = 4;
	TexturePtr tex_for_button = rf.MakeTexture2D(1, 1, 1, 1, fmt, 1, 0, EAH_GPU_Read | EAH_Immutable, MakeSpan<1>(init_data));
	dialog_param_->Control<UITexButton>(id_absorb_button_)->SetTexture(tex_for_button);
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
	atmosphere_model_->ForEachMesh([value](Renderable& renderable)
		{
			checked_cast<AtmosphereMesh&>(renderable).AtmosphereTop(value);
		});
}

void AtmosphericScatteringApp::DensityHandler(KlayGE::UISlider const & sender)
{
	float value = sender.GetValue() / 100000.0f;
	planet_model_->ForEachMesh([value](Renderable& renderable)
		{
			checked_cast<PlanetMesh&>(renderable).Density(value);
		});
	atmosphere_model_->ForEachMesh([value](Renderable& renderable)
		{
			checked_cast<AtmosphereMesh&>(renderable).Density(value);
		});
}

void AtmosphericScatteringApp::ChangeBetaHandler(KlayGE::UITexButton const & /*sender*/)
{
#if defined KLAYGE_PLATFORM_WINDOWS_DESKTOP
	CHOOSECOLORA occ;
	HWND hwnd = this->MainWnd()->HWnd();

	static COLORREF cust_clrs[16] = { RGB(0xFF, 0xFF, 0xFF), RGB(0xFF, 0xFF, 0xFF), RGB(0xFF, 0xFF, 0xFF), RGB(0xFF, 0xFF, 0xFF),
		RGB(0xFF, 0xFF, 0xFF), RGB(0xFF, 0xFF, 0xFF), RGB(0xFF, 0xFF, 0xFF), RGB(0xFF, 0xFF, 0xFF),
		RGB(0xFF, 0xFF, 0xFF), RGB(0xFF, 0xFF, 0xFF), RGB(0xFF, 0xFF, 0xFF), RGB(0xFF, 0xFF, 0xFF),
		RGB(0xFF, 0xFF, 0xFF), RGB(0xFF, 0xFF, 0xFF), RGB(0xFF, 0xFF, 0xFF), RGB(0xFF, 0xFF, 0xFF) };

	Color f4_clr = beta_ / 250.0f;

	ZeroMemory(&occ, sizeof(occ));
	occ.lStructSize = sizeof(occ);
	occ.hwndOwner = hwnd;
	occ.hInstance = nullptr;
	occ.rgbResult = f4_clr.ABGR();
	occ.lpCustColors = cust_clrs;
	occ.Flags = CC_ANYCOLOR | CC_FULLOPEN | CC_RGBINIT;
	occ.lCustData = 0;
	occ.lpfnHook = nullptr;
	occ.lpTemplateName = nullptr;

	if (ChooseColorA(&occ))
	{
		beta_ = Color(occ.rgbResult) * 250.0f;
		std::swap(beta_.r(), beta_.b());
		this->LoadBeta(beta_);
	}
#endif
}

void AtmosphericScatteringApp::ChangeAbsorbHandler(KlayGE::UITexButton const & /*sender*/)
{
#if defined KLAYGE_PLATFORM_WINDOWS_DESKTOP
	CHOOSECOLORA occ;
	HWND hwnd = this->MainWnd()->HWnd();

	static COLORREF cust_clrs[16] = { RGB(0xFF, 0xFF, 0xFF), RGB(0xFF, 0xFF, 0xFF), RGB(0xFF, 0xFF, 0xFF), RGB(0xFF, 0xFF, 0xFF),
		RGB(0xFF, 0xFF, 0xFF), RGB(0xFF, 0xFF, 0xFF), RGB(0xFF, 0xFF, 0xFF), RGB(0xFF, 0xFF, 0xFF),
		RGB(0xFF, 0xFF, 0xFF), RGB(0xFF, 0xFF, 0xFF), RGB(0xFF, 0xFF, 0xFF), RGB(0xFF, 0xFF, 0xFF),
		RGB(0xFF, 0xFF, 0xFF), RGB(0xFF, 0xFF, 0xFF), RGB(0xFF, 0xFF, 0xFF), RGB(0xFF, 0xFF, 0xFF) };

	ZeroMemory(&occ, sizeof(occ));
	occ.lStructSize = sizeof(occ);
	occ.hwndOwner = hwnd;
	occ.hInstance = nullptr;
	occ.rgbResult = absorb_.ABGR();
	occ.lpCustColors = cust_clrs;
	occ.Flags = CC_ANYCOLOR | CC_FULLOPEN | CC_RGBINIT;
	occ.lCustData = 0;
	occ.lpfnHook = nullptr;
	occ.lpTemplateName = nullptr;

	if (ChooseColorA(&occ))
	{
		absorb_ = Color(occ.rgbResult);
		std::swap(absorb_.r(), absorb_.b());
		this->LoadAbsorb(absorb_);
	}
#endif
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

	float3 const& dir = light_ctrl_camera_->ForwardVec();
	planet_model_->ForEachMesh([&dir](Renderable& renderable)
		{
			checked_cast<PlanetMesh&>(renderable).LightDir(-dir);
		});
	atmosphere_model_->ForEachMesh([&dir](Renderable& renderable)
		{
			checked_cast<AtmosphereMesh&>(renderable).LightDir(-dir);
		});

	re.BindFrameBuffer(FrameBufferPtr());
	Color clear_clr(0.0f, 0.0f, 0.0f, 1);

	re.CurFrameBuffer()->Clear(FrameBuffer::CBM_Color | FrameBuffer::CBM_Depth | FrameBuffer::CBM_Stencil, clear_clr, 1, 0);
	return App3DFramework::URV_NeedFlush | App3DFramework::URV_Finished;
}
