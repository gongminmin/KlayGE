#include <KlayGE/KlayGE.hpp>
#include <KlayGE/ThrowErr.hpp>
#include <KlayGE/Util.hpp>
#include <KlayGE/Math.hpp>
#include <KlayGE/Font.hpp>
#include <KlayGE/Renderable.hpp>
#include <KlayGE/RenderableHelper.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/Sampler.hpp>
#include <KlayGE/RenderEffect.hpp>
#include <KlayGE/FrameBuffer.hpp>
#include <KlayGE/SceneManager.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/ResLoader.hpp>
#include <KlayGE/RenderSettings.hpp>
#include <KlayGE/KMesh.hpp>
#include <KlayGE/RenderableHelper.hpp>
#include <KlayGE/SceneObjectHelper.hpp>

#include <KlayGE/D3D9/D3D9RenderFactory.hpp>
#include <KlayGE/OpenGL/OGLRenderFactory.hpp>

#include <KlayGE/OCTree/OCTree.hpp>

#include <KlayGE/Input.hpp>
#include <KlayGE/DInput/DInputFactory.hpp>

#include <KlayGE/Show.hpp>
#include <KlayGE/DShow/DShowFactory.hpp>

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

#include "VideoTexture.hpp"

using namespace std;
using namespace KlayGE;

namespace
{
	class RenderTeapot : public KMesh
	{
	public:
		RenderTeapot(RenderModelPtr model, std::wstring const & /*name*/)
			: KMesh(model, L"Teapot")
		{
			RenderFactory& rf = Context::Instance().RenderFactoryInstance();

			technique_ = rf.LoadEffect("VideoTexture.kfx")->TechniqueByName("Object");
		}

		void BuildMeshInfo()
		{
		}

		void OnRenderBegin()
		{
			App3DFramework const & app = Context::Instance().AppInstance();

			float4x4 const & view = app.ActiveCamera().ViewMatrix();
			float4x4 const & proj = app.ActiveCamera().ProjMatrix();

			*(technique_->Effect().ParameterByName("mvp")) = view * proj;
		}

		void VideoTexture(TexturePtr video_tex)
		{
			*(technique_->Effect().ParameterByName("video_sampler")) = video_tex;
		}
	};

	class TeapotObject : public SceneObjectHelper
	{
	public:
		TeapotObject()
			: SceneObjectHelper(SOA_Cullable)
		{
			renderable_ = LoadKModel("teapot.kmodel", EAH_CPU_Write | EAH_GPU_Read, CreateKModelFactory<RenderModel>(), CreateKMeshFactory<RenderTeapot>())->Mesh(0);
		}

		void VideoTexture(TexturePtr video_tex)
		{
			checked_pointer_cast<RenderTeapot>(renderable_)->VideoTexture(video_tex);
		}
	};

	enum
	{
		Exit
	};

	InputActionDefine actions[] =
	{
		InputActionDefine(Exit, KS_Escape)
	};

	bool ConfirmDevice()
	{
		RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
		RenderDeviceCaps const & caps = re.DeviceCaps();
		if (caps.max_shader_model < 1)
		{
			return false;
		}
		return true;
	}
}


int main()
{
	ResLoader::Instance().AddPath("../../media/Common");
	ResLoader::Instance().AddPath("../../media/VideoTexture");

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
			("context.show_factory", boost::program_options::value<std::string>(), "Show Factory")
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

			if (vm.count("context.show_factory"))
			{
				std::string sf_name = vm["context.show_factory"].as<std::string>();
				if ("DShow" == sf_name)
				{
					Context::Instance().ShowFactoryInstance(DShowFactoryInstance());
				}
			}
			else
			{
				Context::Instance().ShowFactoryInstance(DShowFactoryInstance());
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
			Context::Instance().ShowFactoryInstance(DShowFactoryInstance());
		}

		settings.width = width;
		settings.height = height;
		settings.color_fmt = static_cast<ElementFormat>(color_fmt);
		settings.full_screen = full_screen;
		settings.ConfirmDevice = ConfirmDevice;
	}

	VideoTextureApp app("Video Texture", settings);
	app.Create();
	app.Run();

	return 0;
}

VideoTextureApp::VideoTextureApp(std::string const & name, RenderSettings const & settings)
					: App3DFramework(name, settings)
{
}

void VideoTextureApp::InitObjects()
{
	// ½¨Á¢×ÖÌå
	font_ = Context::Instance().RenderFactoryInstance().MakeFont("gkai00mp.kfont", 16);

	this->LookAt(float3(-0.2f, 0.3f, -0.2f), float3(0, 0, 0));
	this->Proj(0.01f, 100);

	fpcController_.AttachCamera(this->ActiveCamera());
	fpcController_.Scalers(0.05f, 0.1f);

	InputEngine& inputEngine(Context::Instance().InputFactoryInstance().InputEngineInstance());
	InputActionMap actionMap;
	actionMap.AddActions(actions, actions + sizeof(actions) / sizeof(actions[0]));

	action_handler_t input_handler(new input_signal);
	input_handler->connect(boost::bind(&VideoTextureApp::InputHandler, this, _1, _2));
	inputEngine.ActionMap(actionMap, input_handler, true);

	ground_.reset(new TeapotObject);
	ground_->AddToSceneManager();

	ShowEngine& se = Context::Instance().ShowFactoryInstance().ShowEngineInstance();
	se.Load(ResLoader::Instance().Locate("planete.avi"));
	se.Play();
}

void VideoTextureApp::InputHandler(InputEngine const & /*sender*/, InputAction const & action)
{
	switch (action.first)
	{
	case Exit:
		this->Quit();
		break;
	}
}

uint32_t VideoTextureApp::DoUpdate(uint32_t /*pass*/)
{
	RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
	ShowEngine& se = Context::Instance().ShowFactoryInstance().ShowEngineInstance();

	re.CurFrameBuffer()->Clear(FrameBuffer::CBM_Color | FrameBuffer::CBM_Depth, Color(0.2f, 0.4f, 0.6f, 1), 1.0f, 0);

	checked_pointer_cast<TeapotObject>(ground_)->VideoTexture(se.PresentTexture());

	std::wostringstream stream;
	stream << this->FPS();

	font_->RenderText(0, 0, Color(1, 1, 0, 1), L"Video Texture");
	font_->RenderText(0, 18, Color(1, 1, 0, 1), stream.str());

	return App3DFramework::URV_Need_Flush | App3DFramework::URV_Finished;
}
