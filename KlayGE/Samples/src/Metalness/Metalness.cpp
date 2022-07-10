#include <KlayGE/KlayGE.hpp>
#include <KFL/Util.hpp>
#include <KFL/Math.hpp>
#include <KlayGE/Font.hpp>
#include <KlayGE/Renderable.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/RenderEffect.hpp>
#include <KlayGE/RenderView.hpp>
#include <KlayGE/FrameBuffer.hpp>
#include <KlayGE/SceneManager.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/ResLoader.hpp>
#include <KlayGE/Texture.hpp>
#include <KlayGE/RenderSettings.hpp>
#include <KlayGE/Mesh.hpp>
#include <KlayGE/SceneNode.hpp>
#include <KlayGE/SkyBox.hpp>
#include <KlayGE/Camera.hpp>
#include <KlayGE/UI.hpp>
#include <KlayGE/PostProcess.hpp>
#include <KlayGE/Light.hpp>
#include <KlayGE/TexCompressionBC.hpp>

#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/InputFactory.hpp>

#include <iterator>
#include <sstream>
#include <vector>

#include "SampleCommon.hpp"
#include "Metalness.hpp"

using namespace std;
using namespace KlayGE;

namespace
{
	class MetalRenderable : public StaticMesh
	{
	public:
		explicit MetalRenderable(std::wstring_view name)
			: StaticMesh(name)
		{
			effect_ = SyncLoadRenderEffect("Metalness.fxml");
			technique_ = effect_->TechniqueByName("PBFittingPrefiltered");

			{
				auto* ambient_light = Context::Instance().SceneManagerInstance().SceneRootNode().FirstComponentOfType<AmbientLightSource>();

				*(effect_->ParameterByName("skybox_Ycube_tex")) = ambient_light->SkylightTexY();
				*(effect_->ParameterByName("skybox_Ccube_tex")) = ambient_light->SkylightTexC();

				uint32_t const mip = ambient_light->SkylightTexY()->NumMipMaps();
				*(effect_->ParameterByName("diff_spec_mip")) = int2(mip - 1, mip - 2);
			}
		}

		using StaticMesh::Material;

		void Material(float3 const & albedo, float metalness, float glossiness)
		{
			*(effect_->ParameterByName("albedo")) = albedo;
			*(effect_->ParameterByName("metalness")) = metalness;
			*(effect_->ParameterByName("glossiness")) = glossiness;
		}

		void OnRenderBegin()
		{
			StaticMesh::OnRenderBegin();

			App3DFramework const & app = Context::Instance().AppInstance();
			Camera const & camera = app.ActiveCamera();

			float4x4 const mvp = model_mat_ * camera.ViewProjMatrix();

			*(effect_->ParameterByName("model")) = model_mat_;
			*(effect_->ParameterByName("mvp")) = mvp;
			*(effect_->ParameterByName("eye_pos")) = camera.EyePos();
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
	MetalnessApp app;
	app.Create();
	app.Run();

	return 0;
}

MetalnessApp::MetalnessApp()
		: App3DFramework("Metalness"),
			obj_controller_(true, MB_Left, MB_Right, MB_Middle)
{
	ResLoader::Instance().AddPath("../../Samples/media/Metalness");

	albedo_ = float3(0.799102738f, 0.496932995f, 0.048171824f) * 0.5f;
}

void MetalnessApp::OnCreate()
{
	font_ = SyncLoadFont("gkai00mp.kfont");

	TexturePtr y_cube_map = ASyncLoadTexture("rnl_cross_filtered_y.dds", EAH_GPU_Read | EAH_Immutable);
	TexturePtr c_cube_map = ASyncLoadTexture("rnl_cross_filtered_c.dds", EAH_GPU_Read | EAH_Immutable);

	auto& root_node = Context::Instance().SceneManagerInstance().SceneRootNode();

	AmbientLightSourcePtr ambient_light = MakeSharedPtr<AmbientLightSource>();
	ambient_light->SkylightTex(y_cube_map, c_cube_map);
	root_node.AddComponent(ambient_light);

	sphere_group_ = MakeSharedPtr<SceneNode>(SceneNode::SOA_Cullable);
	root_node.AddChild(sphere_group_);

	sphere_group_->OnMainThreadUpdate().Connect([](SceneNode& node, float app_time, float elapsed_time)
		{
			KFL_UNUSED(app_time);
			KFL_UNUSED(elapsed_time);

			auto& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
			auto const& camera = *re.CurFrameBuffer()->Viewport()->Camera();

			node.TransformToParent(camera.InverseViewMatrix());
		});

	auto sphere_model_unique = SyncLoadModel("sphere_high.glb", EAH_GPU_Read | EAH_Immutable,
		SceneNode::SOA_Cullable, nullptr,
		CreateModelFactory<RenderModel>, CreateMeshFactory<MetalRenderable>);

	uint32_t const spheres_row = 10;
	uint32_t const spheres_column = 10;
	for (uint32_t i = 0; i < spheres_row; ++ i)
	{
		for (uint32_t j = 0; j < spheres_column; ++ j)
		{
			auto sphere_model = sphere_model_unique->Clone(CreateModelFactory<RenderModel>, CreateMeshFactory<MetalRenderable>);
			this->Material(*sphere_model, albedo_, (i + 1.0f) / spheres_row, j / (spheres_column - 1.0f));

			sphere_model->RootNode()->TransformToParent(MathLib::translation(
				(-static_cast<float>(spheres_column / 2) + j + 0.5f) * 0.06f,
				-(-static_cast<float>(spheres_row / 2) + i + 0.5f) * 0.06f,
				0.8f));
			sphere_group_->AddChild(sphere_model->RootNode());
		}
	}

	single_object_ = MakeSharedPtr<SceneNode>(SceneNode::SOA_Cullable);
	single_object_->TransformToParent(MathLib::scaling(2.0f, 2.0f, 2.0f));
	root_node.AddChild(single_object_);
	single_model_ = ASyncLoadModel("helmet_armet_2.3ds", EAH_GPU_Read | EAH_Immutable,
		SceneNode::SOA_Cullable,
		[this](RenderModel& model)
		{
			this->Material(model, albedo_, metalness_, glossiness_);
			AddToSceneHelper(*single_object_, model);
		},
		CreateModelFactory<RenderModel>, CreateMeshFactory<MetalRenderable>);

	auto skybox = MakeSharedPtr<RenderableSkyBox>();
	skybox->CompressedCubeMap(y_cube_map, c_cube_map);
	root_node.AddChild(MakeSharedPtr<SceneNode>(MakeSharedPtr<RenderableComponent>(skybox), SceneNode::SOA_NotCastShadow));

	this->LookAt(float3(0.0f, 0.3f, -0.9f), float3(0, 0, 0));
	this->Proj(0.05f, 100);

	obj_controller_.AttachCamera(this->ActiveCamera());
	obj_controller_.Scalers(0.003f, 0.003f);

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

	UIManager::Instance().Load(*ResLoader::Instance().Open("Metalness.uiml"));

	dialog_ = UIManager::Instance().GetDialog("Parameters");
	id_single_object_ = dialog_->IDFromName("SingleObject");
	id_glossiness_static_ = dialog_->IDFromName("GlossinessStatic");
	id_glossiness_ = dialog_->IDFromName("GlossinessSlider");
	id_metalness_static_ = dialog_->IDFromName("MetalnessStatic");
	id_metalness_ = dialog_->IDFromName("MetalnessSlider");

	dialog_->Control<UICheckBox>(id_single_object_)->OnChangedEvent().Connect(
		[this](UICheckBox const & sender)
		{
			this->SingleObjectHandler(sender);
		});
	this->SingleObjectHandler(*dialog_->Control<UICheckBox>(id_single_object_));
	dialog_->Control<UISlider>(id_glossiness_)->OnValueChangedEvent().Connect(
		[this](UISlider const & sender)
		{
			this->GlossinessChangedHandler(sender);
		});
	this->GlossinessChangedHandler(*dialog_->Control<UISlider>(id_glossiness_));
	dialog_->Control<UISlider>(id_metalness_)->OnValueChangedEvent().Connect(
		[this](UISlider const & sender)
		{
			this->MetalnessChangedHandler(sender);
		});
	this->MetalnessChangedHandler(*dialog_->Control<UISlider>(id_metalness_));
}

void MetalnessApp::OnResize(uint32_t width, uint32_t height)
{
	App3DFramework::OnResize(width, height);

	UIManager::Instance().SettleCtrls();
}

void MetalnessApp::InputHandler(InputEngine const & /*sender*/, InputAction const & action)
{
	switch (action.first)
	{
	case Exit:
		this->Quit();
		break;
	}
}

void MetalnessApp::SingleObjectHandler(UICheckBox const & sender)
{
	bool single_sphere_mode = sender.GetChecked();
	dialog_->Control<UISlider>(id_glossiness_)->SetEnabled(single_sphere_mode);
	dialog_->Control<UISlider>(id_metalness_)->SetEnabled(single_sphere_mode);

	sphere_group_->Visible(!single_sphere_mode);
	single_object_->Visible(single_sphere_mode);
}

void MetalnessApp::GlossinessChangedHandler(UISlider const & sender)
{
	glossiness_ = sender.GetValue() * 0.01f;
	
	std::wostringstream stream;
	stream << L"Glossiness: " << glossiness_;
	dialog_->Control<UIStatic>(id_glossiness_static_)->SetText(stream.str());

	this->Material(*single_model_, albedo_, metalness_, glossiness_);
}

void MetalnessApp::MetalnessChangedHandler(UISlider const & sender)
{
	metalness_ = sender.GetValue() * 0.01f;

	std::wostringstream stream;
	stream << L"Metalness: " << metalness_;
	dialog_->Control<UIStatic>(id_metalness_static_)->SetText(stream.str());

	this->Material(*single_model_, albedo_, metalness_, glossiness_);
}

void MetalnessApp::Material(RenderModel const & model, float3 const & albedo, float metalness, float glossiness)
{
	for (uint32_t i = 0; i < model.NumMeshes(); ++ i)
	{
		checked_pointer_cast<MetalRenderable>(model.Mesh(i))->Material(albedo, metalness, glossiness);
	}
}

void MetalnessApp::DoUpdateOverlay()
{
	UIManager::Instance().Render();

	std::wostringstream stream;
	stream.precision(2);
	stream << std::fixed << this->FPS() << " FPS";

	font_->RenderText(0, 0, Color(1, 1, 0, 1), L"Metalness", 16);
	font_->RenderText(0, 18, Color(1, 1, 0, 1), stream.str(), 16);
}

uint32_t MetalnessApp::DoUpdate(uint32_t /*pass*/)
{
	RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
	re.CurFrameBuffer()->AttachedDsv()->ClearDepthStencil(1.0f, 0);

	return App3DFramework::URV_NeedFlush | App3DFramework::URV_Finished;
}
