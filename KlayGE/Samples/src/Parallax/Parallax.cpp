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

#include <KlayGE/D3D9/D3D9RenderFactory.hpp>
#include <KlayGE/OpenGL/OGLRenderFactory.hpp>

#include <KlayGE/OCTree/OCTree.hpp>

#include <KlayGE/Input.hpp>
#include <KlayGE/DInput/DInputFactory.hpp>

#include <vector>
#include <sstream>
#include <fstream>
#include <ctime>
#include <boost/tuple/tuple.hpp>
#include <boost/bind.hpp>
#ifdef KLAYGE_COMPILER_MSVC
#pragma warning(push)
#pragma warning(disable: 4251 4275 4512 4702)
#endif
#include <boost/program_options.hpp>
#ifdef KLAYGE_COMPILER_MSVC
#pragma warning(pop)
#endif

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

			technique_ = rf.LoadEffect("parallax.kfx")->TechniqueByName("Parallax");

			*(technique_->Effect().ParameterByName("diffuseMapSampler")) = LoadTexture("diffuse.dds", EAH_CPU_Write | EAH_GPU_Read);
			*(technique_->Effect().ParameterByName("normalMapSampler")) = LoadTexture("normal.dds", EAH_CPU_Write | EAH_GPU_Read);
			*(technique_->Effect().ParameterByName("heightMapSampler")) = LoadTexture("height.dds", EAH_CPU_Write | EAH_GPU_Read);
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
			renderable_ = LoadKModel("teapot.kmodel", EAH_CPU_Write | EAH_GPU_Read, CreateKModelFactory<RenderModel>(), CreateKMeshFactory<RenderPolygon>());
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
		ScaleStatic,
		ScaleSlider,
		BiasStatic,
		BiasSlider,
		CtrlCamera
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
	ResLoader::Instance().AddPath("../../media/Common");
	ResLoader::Instance().AddPath("../../media/Parallax");

	RenderSettings settings;
	SceneManagerPtr sm;

	{
		int octree_depth = 3;
		int width = 800;
		int height = 600;
		int color_fmt = 13; // EF_ARGB8
		bool full_screen = false;

		boost::program_options::options_description desc("Configuration");
		desc.add_options()
			("context.render_factory", boost::program_options::value<std::string>(), "Render Factory")
			("context.input_factory", boost::program_options::value<std::string>(), "Input Factory")
			("context.scene_manager", boost::program_options::value<std::string>(), "Scene Manager")
			("octree.depth", boost::program_options::value<int>(&octree_depth)->default_value(3), "Octree depth")
			("screen.width", boost::program_options::value<int>(&width)->default_value(800), "Screen Width")
			("screen.height", boost::program_options::value<int>(&height)->default_value(600), "Screen Height")
			("screen.color_fmt", boost::program_options::value<int>(&color_fmt)->default_value(13), "Screen Color Format")
			("screen.fullscreen", boost::program_options::value<bool>(&full_screen)->default_value(false), "Full Screen");

		std::ifstream cfg_fs(ResLoader::Instance().Locate("KlayGE.cfg").c_str());
		if (cfg_fs)
		{
			boost::program_options::variables_map vm;
			boost::program_options::store(boost::program_options::parse_config_file(cfg_fs, desc), vm);
			boost::program_options::notify(vm);

			if (vm.count("context.render_factory"))
			{
				std::string rf_name = vm["context.render_factory"].as<std::string>();
				if ("D3D9" == rf_name)
				{
					Context::Instance().RenderFactoryInstance(D3D9RenderFactoryInstance());
				}
				if ("OpenGL" == rf_name)
				{
					Context::Instance().RenderFactoryInstance(OGLRenderFactoryInstance());
				}
			}
			else
			{
				Context::Instance().RenderFactoryInstance(D3D9RenderFactoryInstance());
			}

			if (vm.count("context.input_factory"))
			{
				std::string if_name = vm["context.input_factory"].as<std::string>();
				if ("DInput" == if_name)
				{
					Context::Instance().InputFactoryInstance(DInputFactoryInstance());
				}
			}
			else
			{
				Context::Instance().InputFactoryInstance(DInputFactoryInstance());
			}

			if (vm.count("context.scene_manager"))
			{
				std::string sm_name = vm["context.scene_manager"].as<std::string>();
				if ("Octree" == sm_name)
				{
					sm.reset(new OCTree(octree_depth));
					Context::Instance().SceneManagerInstance(*sm);
				}
			}
		}
		else
		{
			Context::Instance().RenderFactoryInstance(D3D9RenderFactoryInstance());
			Context::Instance().InputFactoryInstance(DInputFactoryInstance());
		}

		settings.width = width;
		settings.height = height;
		settings.color_fmt = static_cast<ElementFormat>(color_fmt);
		settings.full_screen = full_screen;
		settings.ConfirmDevice = ConfirmDevice;
	}

	Parallax app("Parallax", settings);
	app.Create();
	app.Run();

	return 0;
}

Parallax::Parallax(std::string const & name, RenderSettings const & settings)
			: App3DFramework(name, settings),
				parallax_scale_(0.06f), parallax_bias_(0.02f)
{
}

void Parallax::InitObjects()
{
	// ½¨Á¢×ÖÌå
	font_ = Context::Instance().RenderFactoryInstance().MakeFont("gkai00mp.kfont", 16);

	polygon_.reset(new PolygonObject);
	polygon_->AddToSceneManager();

	this->LookAt(float3(-0.3f, 0.4f, -0.3f), float3(0, 0, 0));
	this->Proj(0.01f, 100);

	fpcController_.Scalers(0.05f, 0.01f);

	InputEngine& inputEngine(Context::Instance().InputFactoryInstance().InputEngineInstance());
	InputActionMap actionMap;
	actionMap.AddActions(actions, actions + sizeof(actions) / sizeof(actions[0]));

	action_handler_t input_handler(new input_signal);
	input_handler->connect(boost::bind(&Parallax::InputHandler, this, _1, _2));
	inputEngine.ActionMap(actionMap, input_handler, true);

	dialog_ = UIManager::Instance().MakeDialog();

	dialog_->AddControl(UIControlPtr(new UIStatic(dialog_, ScaleStatic, L"Scale:", 60, 280, 100, 24, false)));
	dialog_->AddControl(UIControlPtr(new UISlider(dialog_, ScaleSlider, 60, 300, 100, 24, 0, 20,
		static_cast<int>(parallax_scale_ * 100), false)));
	dialog_->Control<UISlider>(ScaleSlider)->OnValueChangedEvent().connect(boost::bind(&Parallax::ScaleChangedHandler, this, _1));
	this->ScaleChangedHandler(*dialog_->Control<UISlider>(ScaleSlider));

	dialog_->AddControl(UIControlPtr(new UIStatic(dialog_, BiasStatic, L"Bias:", 60, 348, 100, 24, false)));
	dialog_->AddControl(UIControlPtr(new UISlider(dialog_, BiasSlider, 60, 368, 100, 24, 0, 10,
		static_cast<int>(parallax_bias_ * 100), false)));
	dialog_->Control<UISlider>(BiasSlider)->OnValueChangedEvent().connect(boost::bind(&Parallax::BiasChangedHandler, this, _1));
	this->BiasChangedHandler(*dialog_->Control<UISlider>(BiasSlider));

	dialog_->AddControl(UIControlPtr(new UICheckBox(dialog_, CtrlCamera, L"Control camera",
                            60, 550, 350, 24, false, 0, false)));
	dialog_->Control<UICheckBox>(CtrlCamera)->SetChecked(false);
	dialog_->Control<UICheckBox>(CtrlCamera)->OnChangedEvent().connect(boost::bind(&Parallax::CtrlCameraHandler, this, _1));
}

void Parallax::OnResize(uint32_t width, uint32_t /*height*/)
{
	dialog_->GetControl(ScaleStatic)->SetLocation(width - 120, 280);
	dialog_->GetControl(ScaleSlider)->SetLocation(width - 120, 300);
	dialog_->GetControl(BiasStatic)->SetLocation(width - 120, 348);
	dialog_->GetControl(BiasSlider)->SetLocation(width - 120, 368);
	dialog_->GetControl(CtrlCamera)->SetLocation(width - 120, 416);
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
	dialog_->Control<UIStatic>(ScaleStatic)->SetText(stream.str());
}

void Parallax::BiasChangedHandler(KlayGE::UISlider const & sender)
{
	parallax_bias_ = sender.GetValue() / 100.0f;
	checked_pointer_cast<PolygonObject>(polygon_)->ParallaxBias(parallax_bias_);

	std::wostringstream stream;
	stream << L"Bias: " << parallax_bias_;
	dialog_->Control<UIStatic>(BiasStatic)->SetText(stream.str());
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

uint32_t Parallax::DoUpdate(uint32_t /*pass*/)
{
	UIManager::Instance().HandleInput();

	RenderEngine& renderEngine(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
	renderEngine.CurFrameBuffer()->Clear(FrameBuffer::CBM_Color | FrameBuffer::CBM_Depth, Color(0.2f, 0.4f, 0.6f, 1), 1.0f, 0);

	float degree(std::clock() / 700.0f);
	float3 lightPos(2, 0, 1);
	float4x4 matRot(MathLib::rotation_y(degree));
	lightPos = MathLib::transform_coord(lightPos, matRot);
	checked_pointer_cast<PolygonObject>(polygon_)->LightPos(lightPos);

	UIManager::Instance().Render();

	std::wostringstream stream;
	stream << this->FPS();

	font_->RenderText(0, 0, Color(1, 1, 0, 1), L"Parallax Mapping");
	font_->RenderText(0, 18, Color(1, 1, 0, 1), stream.str());

	SceneManager& sceneMgr(Context::Instance().SceneManagerInstance());
	stream.str(L"");
	stream << sceneMgr.NumRenderablesRendered() << " Renderables "
		<< sceneMgr.NumPrimitivesRendered() << " Primitives "
		<< sceneMgr.NumVerticesRendered() << " Vertices";
	font_->RenderText(0, 36, Color(1, 1, 1, 1), stream.str());

	return App3DFramework::URV_Need_Flush | App3DFramework::URV_Finished;
}
