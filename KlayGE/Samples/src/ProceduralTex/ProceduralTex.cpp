#include <KlayGE/KlayGE.hpp>
#include <KFL/CXX17/iterator.hpp>
#include <KFL/Util.hpp>
#include <KFL/Math.hpp>
#include <KlayGE/Font.hpp>
#include <KlayGE/Renderable.hpp>
#include <KlayGE/RenderableHelper.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/RenderEffect.hpp>
#include <KlayGE/FrameBuffer.hpp>
#include <KlayGE/SceneManager.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/ResLoader.hpp>
#include <KlayGE/RenderSettings.hpp>
#include <KlayGE/Mesh.hpp>
#include <KlayGE/GraphicsBuffer.hpp>
#include <KlayGE/Light.hpp>
#include <KlayGE/SceneObjectHelper.hpp>
#include <KlayGE/Camera.hpp>

#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/InputFactory.hpp>

#include <vector>
#include <sstream>

#include "SampleCommon.hpp"
#include "ProceduralTex.hpp"

using namespace std;
using namespace KlayGE;

namespace
{
	class RenderPolygon : public StaticMesh
	{
	public:
		RenderPolygon(RenderModelPtr const & model, std::wstring const & name)
			: StaticMesh(model, name)
		{
			effect_ = SyncLoadRenderEffect("ProceduralTex.fxml");
			technique_ = effect_->TechniqueByName("ProceduralMarbleTex");
		}

		virtual void DoBuildMeshInfo() override
		{
			AABBox const & pos_bb = this->PosBound();
			*(effect_->ParameterByName("pos_center")) = pos_bb.Center();
			*(effect_->ParameterByName("pos_extent")) = pos_bb.HalfSize();
		}

		void AppTime(float app_time)
		{
			*(effect_->ParameterByName("t")) = app_time / 2.0f;
		}

		void OnRenderBegin()
		{
			App3DFramework const & app = Context::Instance().AppInstance();
			Camera const & camera = app.ActiveCamera();

			float4x4 const & model = float4x4::Identity();

			*(effect_->ParameterByName("mvp")) = model * camera.ViewProjMatrix();
			*(effect_->ParameterByName("eye_pos")) = camera.EyePos();
		}

		void LightPos(float3 const & light_pos)
		{
			*(effect_->ParameterByName("light_pos")) = light_pos;
		}

		void LightColor(float3 const & light_color)
		{
			*(effect_->ParameterByName("light_color")) = light_color;
		}

		void LightFalloff(float3 const & light_falloff)
		{
			*(effect_->ParameterByName("light_falloff")) = light_falloff;
		}

		void ProceduralType(int type)
		{
			technique_ = effect_->TechniqueByIndex(type);
		}

		void ProceduralFreq(float freq)
		{
			*(effect_->ParameterByName("freq")) = freq;
		}
	};

	class PolygonObject : public SceneObjectHelper
	{
	public:
		PolygonObject()
			: SceneObjectHelper(SOA_Cullable)
		{
			renderable_ = SyncLoadModel("teapot.meshml", EAH_GPU_Read | EAH_Immutable, CreateModelFactory<RenderModel>(), CreateMeshFactory<RenderPolygon>());
		}

		void LightPos(float3 const & light_pos)
		{
			RenderModelPtr model = checked_pointer_cast<RenderModel>(renderable_);
			for (uint32_t i = 0; i < model->NumSubrenderables(); ++ i)
			{
				checked_pointer_cast<RenderPolygon>(model->Subrenderable(i))->LightPos(light_pos);
			}
		}

		void LightColor(float3 const & light_color)
		{
			RenderModelPtr model = checked_pointer_cast<RenderModel>(renderable_);
			for (uint32_t i = 0; i < model->NumSubrenderables(); ++ i)
			{
				checked_pointer_cast<RenderPolygon>(model->Subrenderable(i))->LightColor(light_color);
			}
		}

		void LightFalloff(float3 const & light_falloff)
		{
			RenderModelPtr model = checked_pointer_cast<RenderModel>(renderable_);
			for (uint32_t i = 0; i < model->NumSubrenderables(); ++ i)
			{
				checked_pointer_cast<RenderPolygon>(model->Subrenderable(i))->LightFalloff(light_falloff);
			}
		}

		void ProceduralType(int type)
		{
			RenderModelPtr model = checked_pointer_cast<RenderModel>(renderable_);
			for (uint32_t i = 0; i < model->NumSubrenderables(); ++ i)
			{
				checked_pointer_cast<RenderPolygon>(model->Subrenderable(i))->ProceduralType(type);
			}
		}

		void ProceduralFreq(float freq)
		{
			RenderModelPtr model = checked_pointer_cast<RenderModel>(renderable_);
			for (uint32_t i = 0; i < model->NumSubrenderables(); ++ i)
			{
				checked_pointer_cast<RenderPolygon>(model->Subrenderable(i))->ProceduralFreq(freq);
			}
		}

		virtual void SubThreadUpdate(float app_time, float /*elapsed_time*/) override
		{
			RenderModelPtr model = checked_pointer_cast<RenderModel>(renderable_);
			for (uint32_t i = 0; i < model->NumSubrenderables(); ++ i)
			{
				checked_pointer_cast<RenderPolygon>(model->Subrenderable(i))->AppTime(app_time);
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
	ProceduralTexApp app;
	app.Create();
	app.Run();

	return 0;
}

ProceduralTexApp::ProceduralTexApp()
			: App3DFramework("ProceduralTex"),
				procedural_type_(0), procedural_freq_(10),
				loading_percentage_(0)
{
	ResLoader::Instance().AddPath("../../Samples/media/ProceduralTex");
}

void ProceduralTexApp::OnCreate()
{
	font_ = SyncLoadFont("gkai00mp.kfont");
	UIManager::Instance().Load(ResLoader::Instance().Open("ProceduralTex.uiml"));
}

void ProceduralTexApp::OnResize(uint32_t width, uint32_t height)
{
	App3DFramework::OnResize(width, height);

	UIManager::Instance().SettleCtrls();
}

void ProceduralTexApp::InputHandler(InputEngine const & /*sender*/, InputAction const & action)
{
	switch (action.first)
	{
	case Exit:
		this->Quit();
		break;
	}
}

void ProceduralTexApp::TypeChangedHandler(KlayGE::UIComboBox const & sender)
{
	procedural_type_ = sender.GetSelectedIndex();
	checked_pointer_cast<PolygonObject>(polygon_)->ProceduralType(procedural_type_);
}

void ProceduralTexApp::FreqChangedHandler(KlayGE::UISlider const & sender)
{
	procedural_freq_ = static_cast<float>(sender.GetValue());
	checked_pointer_cast<PolygonObject>(polygon_)->ProceduralFreq(procedural_freq_);

	std::wostringstream stream;
	stream << L"Freq: " << procedural_freq_;
	dialog_->Control<UIStatic>(id_freq_static_)->SetText(stream.str());
}

void ProceduralTexApp::DoUpdateOverlay()
{
	UIManager::Instance().Render();

	std::wostringstream stream;
	stream.precision(2);
	stream << std::fixed << this->FPS() << " FPS";

	font_->RenderText(0, 0, Color(1, 1, 0, 1), L"Procedural Texture", 16);
	font_->RenderText(0, 18, Color(1, 1, 0, 1), stream.str(), 16);

	SceneManager& sceneMgr(Context::Instance().SceneManagerInstance());
	stream.str(L"");
	stream << sceneMgr.NumRenderablesRendered() << " Renderables "
		<< sceneMgr.NumPrimitivesRendered() << " Primitives "
		<< sceneMgr.NumVerticesRendered() << " Vertices";
	font_->RenderText(0, 36, Color(1, 1, 1, 1), stream.str(), 16);
}

uint32_t ProceduralTexApp::DoUpdate(uint32_t /*pass*/)
{
	RenderEngine& renderEngine(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
	if (loading_percentage_ < 100)
	{
		UIDialogPtr const & dialog_loading = UIManager::Instance().GetDialog("Loading");

		UIStaticPtr const & msg = dialog_loading->Control<UIStatic>(dialog_loading->IDFromName("Msg"));
		UIProgressBarPtr const & progress_bar = dialog_loading->Control<UIProgressBar>(dialog_loading->IDFromName("Progress"));			

		if (loading_percentage_ < 20)
		{
			dialog_ = UIManager::Instance().GetDialog("ProceduralTex");
			dialog_->SetVisible(false);
			
			dialog_loading->SetVisible(true);

			loading_percentage_ = 20;
			progress_bar->SetValue(loading_percentage_);
			msg->SetText(L"Loading Geometry");
		}
		else if (loading_percentage_ < 60)
		{
			polygon_ = MakeSharedPtr<PolygonObject>();
			polygon_->AddToSceneManager();

			this->LookAt(float3(-0.18f, 0.24f, -0.18f), float3(0, 0.05f, 0));
			this->Proj(0.01f, 100);

			tb_controller_.AttachCamera(this->ActiveCamera());
			tb_controller_.Scalers(0.01f, 0.003f);

			loading_percentage_ = 60;
			progress_bar->SetValue(loading_percentage_);
			msg->SetText(L"Loading Light");
		}
		else if (loading_percentage_ < 80)
		{
			light_ = MakeSharedPtr<PointLightSource>();
			light_->Attrib(0);
			light_->Color(float3(2, 2, 2));
			light_->Falloff(float3(1, 0, 1.0f));
			light_->Position(float3(0.25f, 0.5f, -1.0f));
			light_->AddToSceneManager();

			light_proxy_ = MakeSharedPtr<SceneObjectLightSourceProxy>(light_);
			checked_pointer_cast<SceneObjectLightSourceProxy>(light_proxy_)->Scaling(0.01f, 0.01f, 0.01f);
			light_proxy_->AddToSceneManager();

			loading_percentage_ = 80;
			progress_bar->SetValue(loading_percentage_);
			msg->SetText(L"Initalizing Input System");
		}
		else if (loading_percentage_ < 90)
		{
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

			loading_percentage_ = 90;
			progress_bar->SetValue(loading_percentage_);
			msg->SetText(L"Initalizing UI");
		}
		else
		{
			id_type_static_ = dialog_->IDFromName("TypeStatic");
			id_type_combo_ = dialog_->IDFromName("TypeCombo");
			id_freq_static_ = dialog_->IDFromName("FreqStatic");
			id_freq_slider_ = dialog_->IDFromName("FreqSlider");

			dialog_->Control<UIComboBox>(id_type_combo_)->OnSelectionChangedEvent().connect(
				[this](UIComboBox const & sender)
				{
					this->TypeChangedHandler(sender);
				});
			this->TypeChangedHandler(*dialog_->Control<UIComboBox>(id_type_combo_));

			dialog_->Control<UISlider>(id_freq_slider_)->SetValue(static_cast<int>(procedural_freq_));
			dialog_->Control<UISlider>(id_freq_slider_)->OnValueChangedEvent().connect(
				[this](UISlider const & sender)
				{
					this->FreqChangedHandler(sender);
				});
			this->FreqChangedHandler(*dialog_->Control<UISlider>(id_freq_slider_));

			loading_percentage_ = 100;
			progress_bar->SetValue(loading_percentage_);
			msg->SetText(L"DONE");

			dialog_->SetVisible(true);
			dialog_loading->SetVisible(false);
		}

		renderEngine.CurFrameBuffer()->Clear(FrameBuffer::CBM_Color | FrameBuffer::CBM_Depth, Color(0.0f, 0.0f, 0.0f, 1), 1.0f, 0);
		return App3DFramework::URV_SkipPostProcess | App3DFramework::URV_Finished;
	}
	else
	{
		Color clear_clr(0.2f, 0.4f, 0.6f, 1);
		if (Context::Instance().Config().graphics_cfg.gamma)
		{
			clear_clr.r() = 0.029f;
			clear_clr.g() = 0.133f;
			clear_clr.b() = 0.325f;
		}
		renderEngine.CurFrameBuffer()->Clear(FrameBuffer::CBM_Color | FrameBuffer::CBM_Depth, clear_clr, 1.0f, 0);

		float3 light_pos(0.25f, 0.5f, -1.0f);
		light_pos = MathLib::transform_coord(light_pos, this->ActiveCamera().InverseViewMatrix());
		light_pos = MathLib::normalize(light_pos) * 1.2f;
		light_->Position(light_pos);

		checked_pointer_cast<PolygonObject>(polygon_)->LightPos(light_->Position());
		checked_pointer_cast<PolygonObject>(polygon_)->LightColor(light_->Color());
		checked_pointer_cast<PolygonObject>(polygon_)->LightFalloff(light_->Falloff());

		return App3DFramework::URV_NeedFlush | App3DFramework::URV_Finished;
	}
}
