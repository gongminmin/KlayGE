#include <KlayGE/KlayGE.hpp>
#include <KlayGE/ThrowErr.hpp>
#include <KlayGE/Util.hpp>
#include <KlayGE/Math.hpp>
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
#include <KlayGE/KMesh.hpp>
#include <KlayGE/GraphicsBuffer.hpp>
#include <KlayGE/SceneObjectHelper.hpp>

#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/InputFactory.hpp>

#include <vector>
#include <sstream>
#include <ctime>
#include <boost/bind.hpp>

#include "ProceduralTex.hpp"

using namespace std;
using namespace KlayGE;

namespace
{
	class RenderPolygon : public KMesh
	{
	public:
		RenderPolygon(RenderModelPtr model, std::wstring const & name)
			: KMesh(model, name)
		{
			technique_ = Context::Instance().RenderFactoryInstance().LoadEffect("ProceduralTex.fxml")->TechniqueByName("ProceduralMarbleTex");
		}

		void BuildMeshInfo()
		{
		}

		void OnRenderBegin()
		{
			App3DFramework const & app = Context::Instance().AppInstance();

			float4x4 const & model = float4x4::Identity();
			float4x4 const & view = app.ActiveCamera().ViewMatrix();
			float4x4 const & proj = app.ActiveCamera().ProjMatrix();

			*(technique_->Effect().ParameterByName("mvp")) = model * view * proj;
			*(technique_->Effect().ParameterByName("eye_pos")) = app.ActiveCamera().EyePos();
			*(technique_->Effect().ParameterByName("t")) = clock() / 2000.0f;
		}

		void LightPos(float3 const & light_pos)
		{
			*(technique_->Effect().ParameterByName("light_pos")) = light_pos;
		}

		void ProceduralType(int type)
		{
			technique_ = technique_->Effect().TechniqueByIndex(type);
		}

		void ProceduralFreq(float freq)
		{
			*(technique_->Effect().ParameterByName("freq")) = freq;
		}
	};

	class PolygonObject : public SceneObjectHelper
	{
	public:
		PolygonObject()
			: SceneObjectHelper(SOA_Cullable)
		{
			renderable_ = LoadModel("teapot.meshml", EAH_GPU_Read, CreateKModelFactory<RenderModel>(), CreateKMeshFactory<RenderPolygon>())();
		}

		void LightPos(float3 const & light_pos)
		{
			RenderModelPtr model = checked_pointer_cast<RenderModel>(renderable_);
			for (uint32_t i = 0; i < model->NumMeshes(); ++ i)
			{
				checked_pointer_cast<RenderPolygon>(model->Mesh(i))->LightPos(light_pos);
			}
		}

		void ProceduralType(int type)
		{
			RenderModelPtr model = checked_pointer_cast<RenderModel>(renderable_);
			for (uint32_t i = 0; i < model->NumMeshes(); ++ i)
			{
				checked_pointer_cast<RenderPolygon>(model->Mesh(i))->ProceduralType(type);
			}
		}

		void ProceduralFreq(float freq)
		{
			RenderModelPtr model = checked_pointer_cast<RenderModel>(renderable_);
			for (uint32_t i = 0; i < model->NumMeshes(); ++ i)
			{
				checked_pointer_cast<RenderPolygon>(model->Mesh(i))->ProceduralFreq(freq);
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

	bool ConfirmDevice()
	{
		RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
		RenderDeviceCaps const & caps = re.DeviceCaps();
		if (caps.max_shader_model < 2)
		{
			return false;
		}
		return true;
	}
}


int main()
{
	ResLoader::Instance().AddPath("../Samples/media/Common");

	ContextCfg context_cfg = Context::Instance().LoadCfg("KlayGE.cfg");
	context_cfg.graphics_cfg.ConfirmDevice = ConfirmDevice;

	Context::Instance().Config(context_cfg);

	ProceduralTexApp app;
	app.Create();
	app.Run();

	return 0;
}

ProceduralTexApp::ProceduralTexApp()
			: App3DFramework("ProceduralTex"),
				procedural_type_(0), procedural_freq_(10)
{
	ResLoader::Instance().AddPath("../Samples/media/ProceduralTex");
}

void ProceduralTexApp::InitObjects()
{
	// ½¨Á¢×ÖÌå
	font_ = Context::Instance().RenderFactoryInstance().MakeFont("gkai00mp.kfont");

	polygon_ = MakeSharedPtr<PolygonObject>();
	polygon_->AddToSceneManager();

	this->LookAt(float3(-0.3f, 0.4f, -0.3f), float3(0, 0, 0));
	this->Proj(0.01f, 100);

	fpcController_.Scalers(0.05f, 0.01f);

	InputEngine& inputEngine(Context::Instance().InputFactoryInstance().InputEngineInstance());
	InputActionMap actionMap;
	actionMap.AddActions(actions, actions + sizeof(actions) / sizeof(actions[0]));

	action_handler_t input_handler = MakeSharedPtr<input_signal>();
	input_handler->connect(boost::bind(&ProceduralTexApp::InputHandler, this, _1, _2));
	inputEngine.ActionMap(actionMap, input_handler, true);

	UIManager::Instance().Load(ResLoader::Instance().Load("ProceduralTex.uiml"));
	dialog_ = UIManager::Instance().GetDialogs()[0];

	id_type_static_ = dialog_->IDFromName("TypeStatic");
	id_type_combo_ = dialog_->IDFromName("TypeCombo");
	id_freq_static_ = dialog_->IDFromName("FreqStatic");
	id_freq_slider_ = dialog_->IDFromName("FreqSlider");
	id_ctrl_camera_ = dialog_->IDFromName("CtrlCamera");

	dialog_->Control<UIComboBox>(id_type_combo_)->OnSelectionChangedEvent().connect(boost::bind(&ProceduralTexApp::TypeChangedHandler, this, _1));
	this->TypeChangedHandler(*dialog_->Control<UIComboBox>(id_type_combo_));

	dialog_->Control<UISlider>(id_freq_slider_)->SetValue(static_cast<int>(procedural_freq_));
	dialog_->Control<UISlider>(id_freq_slider_)->OnValueChangedEvent().connect(boost::bind(&ProceduralTexApp::FreqChangedHandler, this, _1));
	this->FreqChangedHandler(*dialog_->Control<UISlider>(id_freq_slider_));

	dialog_->Control<UICheckBox>(id_ctrl_camera_)->OnChangedEvent().connect(boost::bind(&ProceduralTexApp::CtrlCameraHandler, this, _1));
}

void ProceduralTexApp::OnResize(uint32_t width, uint32_t height)
{
	App3DFramework::OnResize(width, height);

	UIManager::Instance().SettleCtrls(width, height);
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

void ProceduralTexApp::CtrlCameraHandler(KlayGE::UICheckBox const & sender)
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

void ProceduralTexApp::DoUpdateOverlay()
{
	UIManager::Instance().Render();

	std::wostringstream stream;
	stream.precision(2);
	stream << fixed << this->FPS() << " FPS";

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
	renderEngine.CurFrameBuffer()->Clear(FrameBuffer::CBM_Color | FrameBuffer::CBM_Depth, Color(0.2f, 0.4f, 0.6f, 1), 1.0f, 0);

	float3 lightPos(0.5f, 1, -2);
	checked_pointer_cast<PolygonObject>(polygon_)->LightPos(lightPos);

	return App3DFramework::URV_Need_Flush | App3DFramework::URV_Finished;
}
