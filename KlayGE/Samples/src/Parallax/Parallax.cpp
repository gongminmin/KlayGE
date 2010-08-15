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

#include "Parallax.hpp"

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

			technique_ = rf.LoadEffect("parallax.fxml")->TechniqueByName("Parallax");

			*(technique_->Effect().ParameterByName("diffuse_tex")) = LoadTexture("diffuse.dds", EAH_GPU_Read)();
			*(technique_->Effect().ParameterByName("normal_tex")) = LoadTexture("normal.dds", EAH_GPU_Read)();
			*(technique_->Effect().ParameterByName("height_tex")) = LoadTexture("height.dds", EAH_GPU_Read)();
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
		}

		void LightPos(float3 const & light_pos)
		{
			*(technique_->Effect().ParameterByName("light_pos")) = light_pos;
		}

		void ParallaxScale(float scale)
		{
			*(technique_->Effect().ParameterByName("parallax_scale")) = scale;
		}

		void ParallaxBias(float bias)
		{
			*(technique_->Effect().ParameterByName("parallax_bias")) = bias;
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

		void ParallaxScale(float scale)
		{
			RenderModelPtr model = checked_pointer_cast<RenderModel>(renderable_);
			for (uint32_t i = 0; i < model->NumMeshes(); ++ i)
			{
				checked_pointer_cast<RenderPolygon>(model->Mesh(i))->ParallaxScale(scale);
			}
		}

		void ParallaxBias(float bias)
		{
			RenderModelPtr model = checked_pointer_cast<RenderModel>(renderable_);
			for (uint32_t i = 0; i < model->NumMeshes(); ++ i)
			{
				checked_pointer_cast<RenderPolygon>(model->Mesh(i))->ParallaxBias(bias);
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


int main()
{
	ResLoader::Instance().AddPath("../Samples/media/Common");

	Context::Instance().LoadCfg("KlayGE.cfg");

	Parallax app;
	app.Create();
	app.Run();

	return 0;
}

Parallax::Parallax()
			: App3DFramework("Parallax"),
				parallax_scale_(0.06f), parallax_bias_(0.02f)
{
	ResLoader::Instance().AddPath("../Samples/media/Parallax");
}

bool Parallax::ConfirmDevice() const
{
	RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
	RenderDeviceCaps const & caps = re.DeviceCaps();
	if (caps.max_shader_model < 2)
	{
		return false;
	}
	return true;
}

void Parallax::InitObjects()
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
	input_handler->connect(boost::bind(&Parallax::InputHandler, this, _1, _2));
	inputEngine.ActionMap(actionMap, input_handler, true);

	UIManager::Instance().Load(ResLoader::Instance().Load("Parallax.uiml"));
	dialog_ = UIManager::Instance().GetDialogs()[0];

	id_scale_static_ = dialog_->IDFromName("ScaleStatic");
	id_scale_slider_ = dialog_->IDFromName("ScaleSlider");
	id_bias_static_ = dialog_->IDFromName("BiasStatic");
	id_bias_slider_ = dialog_->IDFromName("BiasSlider");
	id_ctrl_camera_ = dialog_->IDFromName("CtrlCamera");

	dialog_->Control<UISlider>(id_scale_slider_)->SetValue(static_cast<int>(parallax_scale_ * 100));
	dialog_->Control<UISlider>(id_scale_slider_)->OnValueChangedEvent().connect(boost::bind(&Parallax::ScaleChangedHandler, this, _1));
	this->ScaleChangedHandler(*dialog_->Control<UISlider>(id_scale_slider_));

	dialog_->Control<UISlider>(id_bias_slider_)->SetValue(static_cast<int>(parallax_bias_ * 100));
	dialog_->Control<UISlider>(id_bias_slider_)->OnValueChangedEvent().connect(boost::bind(&Parallax::BiasChangedHandler, this, _1));
	this->BiasChangedHandler(*dialog_->Control<UISlider>(id_bias_slider_));

	dialog_->Control<UICheckBox>(id_ctrl_camera_)->OnChangedEvent().connect(boost::bind(&Parallax::CtrlCameraHandler, this, _1));
}

void Parallax::OnResize(uint32_t width, uint32_t height)
{
	App3DFramework::OnResize(width, height);

	UIManager::Instance().SettleCtrls(width, height);
}

void Parallax::InputHandler(InputEngine const & /*sender*/, InputAction const & action)
{
	switch (action.first)
	{
	case Exit:
		this->Quit();
		break;
	}
}

void Parallax::ScaleChangedHandler(KlayGE::UISlider const & sender)
{
	parallax_scale_ = sender.GetValue() / 100.0f;
	checked_pointer_cast<PolygonObject>(polygon_)->ParallaxScale(parallax_scale_);

	std::wostringstream stream;
	stream << L"Scale: " << parallax_scale_;
	dialog_->Control<UIStatic>(id_scale_static_)->SetText(stream.str());
}

void Parallax::BiasChangedHandler(KlayGE::UISlider const & sender)
{
	parallax_bias_ = sender.GetValue() / 100.0f;
	checked_pointer_cast<PolygonObject>(polygon_)->ParallaxBias(parallax_bias_);

	std::wostringstream stream;
	stream << L"Bias: " << parallax_bias_;
	dialog_->Control<UIStatic>(id_bias_static_)->SetText(stream.str());
}

void Parallax::CtrlCameraHandler(KlayGE::UICheckBox const & sender)
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

void Parallax::DoUpdateOverlay()
{
	UIManager::Instance().Render();

	std::wostringstream stream;
	stream << this->FPS();

	font_->RenderText(0, 0, Color(1, 1, 0, 1), L"Parallax Mapping", 16);
	font_->RenderText(0, 18, Color(1, 1, 0, 1), stream.str(), 16);

	SceneManager& sceneMgr(Context::Instance().SceneManagerInstance());
	stream.str(L"");
	stream << sceneMgr.NumRenderablesRendered() << " Renderables "
		<< sceneMgr.NumPrimitivesRendered() << " Primitives "
		<< sceneMgr.NumVerticesRendered() << " Vertices";
	font_->RenderText(0, 36, Color(1, 1, 1, 1), stream.str(), 16);
}

uint32_t Parallax::DoUpdate(uint32_t /*pass*/)
{
	RenderEngine& renderEngine(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
	renderEngine.CurFrameBuffer()->Clear(FrameBuffer::CBM_Color | FrameBuffer::CBM_Depth, Color(0.2f, 0.4f, 0.6f, 1), 1.0f, 0);

	/*float degree(std::clock() / 700.0f);
	float3 lightPos(2, 0, 1);
	float4x4 matRot(MathLib::rotation_y(degree));
	lightPos = MathLib::transform_coord(lightPos, matRot);*/
	float3 lightPos(0.5f, 1, -2);
	checked_pointer_cast<PolygonObject>(polygon_)->LightPos(lightPos);

	return App3DFramework::URV_Need_Flush | App3DFramework::URV_Finished;
}
