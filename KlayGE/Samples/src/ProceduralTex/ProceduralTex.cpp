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
			RenderFactory& rf = Context::Instance().RenderFactoryInstance();

			technique_ = rf.LoadEffect("ProceduralTex.kfx")->TechniqueByName("ProceduralMarbleTex");

			uint8_t const permutation[] =
			{
				151, 160, 137, 91, 90, 15,
				131, 13, 201, 95, 96, 53, 194, 233, 7, 225, 140, 36, 103, 30, 69, 142, 8, 99, 37, 240, 21, 10, 23,
				190, 6, 148, 247, 120, 234, 75, 0, 26, 197, 62, 94, 252, 219, 203, 117, 35, 11, 32, 57, 177, 33,
				88, 237, 149, 56, 87, 174, 20, 125, 136, 171, 168, 68, 175, 74, 165, 71, 134, 139, 48, 27, 166,
				77, 146, 158, 231, 83, 111, 229, 122, 60, 211, 133, 230, 220, 105, 92, 41, 55, 46, 245, 40, 244,
				102, 143, 54, 65, 25, 63, 161, 1, 216, 80, 73, 209, 76, 132, 187, 208, 89, 18, 169, 200, 196,
				135, 130, 116, 188, 159, 86, 164, 100, 109, 198, 173, 186, 3, 64, 52, 217, 226, 250, 124, 123,
				5, 202, 38, 147, 118, 126, 255, 82, 85, 212, 207, 206, 59, 227, 47, 16, 58, 17, 182, 189, 28, 42,
				223, 183, 170, 213, 119, 248, 152, 2, 44, 154, 163, 70, 221, 153, 101, 155, 167, 43, 172, 9,
				129, 22, 39, 253, 19, 98, 108, 110, 79, 113, 224, 232, 178, 185, 112, 104, 218, 246, 97, 228,
				251, 34, 242, 193, 238, 210, 144, 12, 191, 179, 162, 241, 81, 51, 145, 235, 249, 14, 239, 107,
				49, 192, 214, 31, 181, 199, 106, 157, 184, 84, 204, 176, 115, 121, 50, 45, 127, 4, 150, 254,
				138, 236, 205, 93, 222, 114, 67, 29, 24, 72, 243, 141, 128, 195, 78, 66, 215, 61, 156, 180
			};

			ElementInitData init_data;
			init_data.data = permutation;
			init_data.slice_pitch = init_data.row_pitch = sizeof(permutation);
			permutation_tex_ = rf.MakeTexture2D(256, 1, 1, EF_L8, 1, 0, EAH_GPU_Read, &init_data);

			int8_t const grad[] = 
			{
				127, 127, 0, 0,
				-128, 127, 0, 0,
				127, -128, 0, 0,
				-128, -128, 0, 0,
				127, 0, 127, 0,
				-128, 0, 127, 0,
				127, 0, -128, 0,
				-128, 0, -128, 0,
				0, 127, 127, 0,
				0, -128, 127, 0,
				0, 127, -128, 0,
				0, -128, -128, 0,
				127, 127, 0, 0,
				0, -128, 127, 0,
				-128, 127, 0, 0,
				0, -128, -128, 0
			};

			init_data.data = grad;
			init_data.slice_pitch = init_data.row_pitch = sizeof(grad);
			grad_tex_ = rf.MakeTexture2D(16, 1, 1, EF_SIGNED_ABGR8, 1, 0, EAH_GPU_Read, &init_data);

			*(technique_->Effect().ParameterByName("permutation_tex")) = permutation_tex_;
			*(technique_->Effect().ParameterByName("grad_tex")) = grad_tex_;
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
		}

		void ProceduralType(int type)
		{
			technique_ = technique_->Effect().TechniqueByIndex(type);
		}

		void ProceduralFreq(float freq)
		{
			*(technique_->Effect().ParameterByName("freq")) = freq;
		}

	private:
		TexturePtr permutation_tex_;
		TexturePtr grad_tex_;
	};

	class PolygonObject : public SceneObjectHelper
	{
	public:
		PolygonObject()
			: SceneObjectHelper(SOA_Cullable)
		{
			renderable_ = LoadKModel("teapot.kmodel", EAH_GPU_Read, CreateKModelFactory<RenderModel>(), CreateKMeshFactory<RenderPolygon>());
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
	ResLoader::Instance().AddPath("../Samples/media/ProceduralTex");

	RenderSettings settings = Context::Instance().LoadCfg("KlayGE.cfg");
	settings.ConfirmDevice = ConfirmDevice;

	ProceduralTexApp app("ProceduralTex", settings);
	app.Create();
	app.Run();

	return 0;
}

ProceduralTexApp::ProceduralTexApp(std::string const & name, RenderSettings const & settings)
			: App3DFramework(name, settings),
				procedural_type_(0), procedural_freq_(10)
{
}

void ProceduralTexApp::InitObjects()
{
	// ½¨Á¢×ÖÌå
	font_ = Context::Instance().RenderFactoryInstance().MakeFont("gkai00mp.kfont");

	polygon_.reset(new PolygonObject);
	polygon_->AddToSceneManager();

	this->LookAt(float3(-0.3f, 0.4f, -0.3f), float3(0, 0, 0));
	this->Proj(0.01f, 100);

	fpcController_.Scalers(0.05f, 0.01f);

	InputEngine& inputEngine(Context::Instance().InputFactoryInstance().InputEngineInstance());
	InputActionMap actionMap;
	actionMap.AddActions(actions, actions + sizeof(actions) / sizeof(actions[0]));

	action_handler_t input_handler(new input_signal);
	input_handler->connect(boost::bind(&ProceduralTexApp::InputHandler, this, _1, _2));
	inputEngine.ActionMap(actionMap, input_handler, true);

	UIManager::Instance().Load(ResLoader::Instance().Load("ProceduralTex.kui"));
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

uint32_t ProceduralTexApp::DoUpdate(uint32_t /*pass*/)
{
	UIManager::Instance().HandleInput();

	RenderEngine& renderEngine(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
	renderEngine.CurFrameBuffer()->Clear(FrameBuffer::CBM_Color | FrameBuffer::CBM_Depth, Color(0.2f, 0.4f, 0.6f, 1), 1.0f, 0);

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

	return App3DFramework::URV_Need_Flush | App3DFramework::URV_Finished;
}
