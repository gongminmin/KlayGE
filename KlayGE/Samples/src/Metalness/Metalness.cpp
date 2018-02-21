#include <KlayGE/KlayGE.hpp>
#include <KFL/CXX17/iterator.hpp>
#include <KFL/Util.hpp>
#include <KFL/Math.hpp>
#include <KlayGE/Font.hpp>
#include <KlayGE/Renderable.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/RenderEffect.hpp>
#include <KlayGE/FrameBuffer.hpp>
#include <KlayGE/SceneManager.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/ResLoader.hpp>
#include <KlayGE/Texture.hpp>
#include <KlayGE/RenderableHelper.hpp>
#include <KlayGE/RenderSettings.hpp>
#include <KlayGE/Mesh.hpp>
#include <KlayGE/SceneObjectHelper.hpp>
#include <KlayGE/Camera.hpp>
#include <KlayGE/UI.hpp>
#include <KlayGE/PostProcess.hpp>
#include <KlayGE/Light.hpp>
#include <KlayGE/TexCompressionBC.hpp>

#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/InputFactory.hpp>

#include <vector>
#include <sstream>

#include "SampleCommon.hpp"
#include "Metalness.hpp"

using namespace std;
using namespace KlayGE;

namespace
{
	class MetalRenderable : public StaticMesh
	{
	public:
		MetalRenderable(RenderModelPtr const & model, std::wstring const & /*name*/)
			: StaticMesh(model, L"Metal")
		{
			effect_ = SyncLoadRenderEffect("Metalness.fxml");
			technique_ = effect_->TechniqueByName("PBFittingPrefiltered");

			SceneManager& sm = Context::Instance().SceneManagerInstance();
			for (uint32_t i = 0; i < sm.NumLights(); ++ i)
			{
				LightSourcePtr const & light = sm.GetLight(i);
				if (LightSource::LT_Ambient == light->Type())
				{
					*(effect_->ParameterByName("skybox_Ycube_tex")) = light->SkylightTexY();
					*(effect_->ParameterByName("skybox_Ccube_tex")) = light->SkylightTexC();

					uint32_t const mip = light->SkylightTexY()->NumMipMaps();
					*(effect_->ParameterByName("diff_spec_mip")) = int2(mip - 1, mip - 2);
					break;
				}
			}
		}

		virtual void DoBuildMeshInfo() override
		{
			AABBox const & pos_bb = this->PosBound();
			*(effect_->ParameterByName("pos_center")) = pos_bb.Center();
			*(effect_->ParameterByName("pos_extent")) = pos_bb.HalfSize();

			AABBox const & tc_bb = this->TexcoordBound();
			*(effect_->ParameterByName("tc_center")) = float2(tc_bb.Center().x(), tc_bb.Center().y());
			*(effect_->ParameterByName("tc_extent")) = float2(tc_bb.HalfSize().x(), tc_bb.HalfSize().y());
		}

		void Material(float3 const & albedo, float metalness, float glossiness)
		{
			*(effect_->ParameterByName("albedo")) = albedo;
			*(effect_->ParameterByName("metalness")) = metalness;
			*(effect_->ParameterByName("glossiness")) = glossiness;
		}

		void OnRenderBegin()
		{
			App3DFramework const & app = Context::Instance().AppInstance();
			Camera const & camera = app.ActiveCamera();

			float4x4 const mvp = model_mat_ * camera.ViewProjMatrix();

			*(effect_->ParameterByName("model")) = model_mat_;
			*(effect_->ParameterByName("mvp")) = mvp;
			*(effect_->ParameterByName("eye_pos")) = camera.EyePos();
		}
	};

	class MetalObject : public SceneObjectHelper
	{
	public:
		explicit MetalObject(std::string const & model_name)
			: SceneObjectHelper(SOA_Cullable)
		{
			renderable_ = SyncLoadModel(model_name, EAH_GPU_Read | EAH_Immutable,
				CreateModelFactory<RenderModel>(), CreateMeshFactory<MetalRenderable>());
		}

		void Material(float3 const & albedo, float metalness, float glossiness)
		{
			for (uint32_t i = 0; i < renderable_->NumSubrenderables(); ++ i)
			{
				checked_pointer_cast<MetalRenderable>(renderable_->Subrenderable(i))->Material(albedo, metalness, glossiness);
			}
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

	AmbientLightSourcePtr ambient_light = MakeSharedPtr<AmbientLightSource>();
	ambient_light->SkylightTex(y_cube_map, c_cube_map);
	ambient_light->AddToSceneManager();

	uint32_t spheres_row = 10;
	uint32_t spheres_column = 10;
	spheres_.resize(spheres_row * spheres_column);
	for (uint32_t i = 0; i < spheres_row; ++ i)
	{
		for (uint32_t j = 0; j < spheres_column; ++ j)
		{
			spheres_[i * spheres_column + j] = MakeSharedPtr<MetalObject>("sphere_high.meshml");
			checked_pointer_cast<MetalObject>(spheres_[i * spheres_column + j])->Material(albedo_,
				(i + 1.0f) / spheres_row, j / (spheres_column - 1.0f));
			spheres_[i * spheres_column + j]->ModelMatrix(MathLib::scaling(1.3f, 1.3f, 1.3f)
				* MathLib::translation((-static_cast<float>(spheres_row / 2) + i + 0.5f) * 0.08f, 0.0f,
					(-static_cast<float>(spheres_column / 2) + j + 0.5f) * 0.08f));
			spheres_[i * spheres_column + j]->AddToSceneManager();
		}
	}

	single_object_ = MakeSharedPtr<MetalObject>("helmet_armet_2.meshml");
	single_object_->ModelMatrix(MathLib::scaling(2.0f, 2.0f, 2.0f));
	single_object_->AddToSceneManager();

	sky_box_ = MakeSharedPtr<SceneObjectSkyBox>(0);
	checked_pointer_cast<SceneObjectSkyBox>(sky_box_)->CompressedCubeMap(y_cube_map, c_cube_map);
	sky_box_->AddToSceneManager();

	this->LookAt(float3(0.0f, 0.3f, -0.9f), float3(0, 0, 0));
	this->Proj(0.05f, 100);

	obj_controller_.AttachCamera(this->ActiveCamera());
	obj_controller_.Scalers(0.003f, 0.003f);

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

	UIManager::Instance().Load(ResLoader::Instance().Open("Metalness.uiml"));

	dialog_ = UIManager::Instance().GetDialog("Parameters");
	id_single_object_ = dialog_->IDFromName("SingleObject");
	id_glossiness_static_ = dialog_->IDFromName("GlossinessStatic");
	id_glossiness_ = dialog_->IDFromName("GlossinessSlider");
	id_metalness_static_ = dialog_->IDFromName("MetalnessStatic");
	id_metalness_ = dialog_->IDFromName("MetalnessSlider");

	dialog_->Control<UICheckBox>(id_single_object_)->OnChangedEvent().connect(
		[this](UICheckBox const & sender)
		{
			this->SingleObjectHandler(sender);
		});
	this->SingleObjectHandler(*dialog_->Control<UICheckBox>(id_single_object_));
	dialog_->Control<UISlider>(id_glossiness_)->OnValueChangedEvent().connect(
		[this](UISlider const & sender)
		{
			this->GlossinessChangedHandler(sender);
		});
	this->GlossinessChangedHandler(*dialog_->Control<UISlider>(id_glossiness_));
	dialog_->Control<UISlider>(id_metalness_)->OnValueChangedEvent().connect(
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

	for (size_t i = 0; i < spheres_.size(); ++ i)
	{
		spheres_[i]->Visible(!single_sphere_mode);
	}
	single_object_->Visible(single_sphere_mode);
}

void MetalnessApp::GlossinessChangedHandler(UISlider const & sender)
{
	glossiness_ = sender.GetValue() * 0.01f;
	
	std::wostringstream stream;
	stream << L"Glossiness: " << glossiness_;
	dialog_->Control<UIStatic>(id_glossiness_static_)->SetText(stream.str());

	checked_pointer_cast<MetalObject>(single_object_)->Material(albedo_, metalness_, glossiness_);
}

void MetalnessApp::MetalnessChangedHandler(UISlider const & sender)
{
	metalness_ = sender.GetValue() * 0.01f;

	std::wostringstream stream;
	stream << L"Metalness: " << metalness_;
	dialog_->Control<UIStatic>(id_metalness_static_)->SetText(stream.str());

	checked_pointer_cast<MetalObject>(single_object_)->Material(albedo_, metalness_, glossiness_);
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
	re.CurFrameBuffer()->Attached(FrameBuffer::ATT_DepthStencil)->ClearDepthStencil(1.0f, 0);

	return App3DFramework::URV_NeedFlush | App3DFramework::URV_Finished;
}
