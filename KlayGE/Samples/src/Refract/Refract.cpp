#include <KlayGE/KlayGE.hpp>
#include <KlayGE/ThrowErr.hpp>
#include <KlayGE/Util.hpp>
#include <KlayGE/Math.hpp>
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
#include <KlayGE/KMesh.hpp>
#include <KlayGE/SceneObjectHelper.hpp>
#include <KlayGE/HDRPostProcess.hpp>

#include <KlayGE/D3D9/D3D9RenderFactory.hpp>
#include <KlayGE/OpenGL/OGLRenderFactory.hpp>

#include <KlayGE/OCTree/OCTree.hpp>

#include <KlayGE/Input.hpp>
#include <KlayGE/DInput/DInputFactory.hpp>

#include <vector>
#include <sstream>
#include <fstream>
#include <ctime>
#include <boost/bind.hpp>
#ifdef KLAYGE_COMPILER_MSVC
#pragma warning(push)
#pragma warning(disable: 4251 4275 4512 4702)
#endif
#include <boost/program_options.hpp>
#ifdef KLAYGE_COMPILER_MSVC
#pragma warning(pop)
#endif

#include "Refract.hpp"

using namespace std;
using namespace KlayGE;

namespace
{
	class HDRSkyBox : public RenderableSkyBox
	{
	public:
		HDRSkyBox()
		{
			RenderFactory& rf = Context::Instance().RenderFactoryInstance();
			technique_ = rf.LoadEffect("HDRSkyBox.kfx")->TechniqueByName("HDRSkyBoxTec");

			skybox_cubeMapSampler_ep_ = technique_->Effect().ParameterByName("skybox_cubeMapSampler");
			skybox_CcubeMapSampler_ep_ = technique_->Effect().ParameterByName("skybox_CcubeMapSampler");
			inv_mvp_ep_ = technique_->Effect().ParameterByName("inv_mvp");
		}

		void CompressedCubeMap(TexturePtr const & y_cube, TexturePtr const & c_cube)
		{
			y_tex_ = y_cube;
			c_tex_ = c_cube;
		}

		void OnRenderBegin()
		{
			RenderableSkyBox::OnRenderBegin();

			*skybox_cubeMapSampler_ep_ = y_tex_;
			*skybox_CcubeMapSampler_ep_ = c_tex_;
		}

	private:
		TexturePtr y_tex_;
		TexturePtr c_tex_;

		RenderEffectParameterPtr skybox_CcubeMapSampler_ep_;
	};

	class HDRSceneObjectSkyBox : public SceneObjectSkyBox
	{
	public:
		HDRSceneObjectSkyBox()
		{
			renderable_.reset(new HDRSkyBox);
		}

		void CompressedCubeMap(TexturePtr const & y_cube, TexturePtr const & c_cube)
		{
			checked_pointer_cast<HDRSkyBox>(renderable_)->CompressedCubeMap(y_cube, c_cube);
		}
	};

	class RefractorRenderable : public KMesh
	{
	public:
		RefractorRenderable(RenderModelPtr model, std::wstring const & /*name*/)
			: KMesh(model, L"Refractor")
		{
			RenderFactory& rf = Context::Instance().RenderFactoryInstance();

			RenderEffectPtr effect = rf.LoadEffect("Refract.kfx");
			front_face_tech_ = effect->TechniqueByName("Refract");
			back_face_tech_ = effect->TechniqueByName("RefractBackFace");

			technique_ = back_face_tech_;
			*(technique_->Effect().ParameterByName("eta_ratio")) = float3(1 / 1.1f, 1 / 1.11f, 1 / 1.12f);
		}

		void BuildMeshInfo()
		{
		}

		void Pass(int pass)
		{
			switch (pass)
			{
			case 0:
				technique_ = back_face_tech_;
				break;

			case 1:
				technique_ = front_face_tech_;
				break;

			default:
				BOOST_ASSERT(false);
				break;
			}
		}

		void BackFaceTexture(TexturePtr const & bf_tex)
		{
			*(technique_->Effect().ParameterByName("BackFace_Sampler")) = bf_tex;
		}

		void CompressedCubeMap(TexturePtr const & y_cube, TexturePtr const & c_cube)
		{
			*(technique_->Effect().ParameterByName("skybox_YcubeMapSampler")) = y_cube;
			*(technique_->Effect().ParameterByName("skybox_CcubeMapSampler")) = c_cube;
		}

		void OnRenderBegin()
		{
			App3DFramework const & app = Context::Instance().AppInstance();

			float4x4 const & model = float4x4::Identity();
			float4x4 const & view = app.ActiveCamera().ViewMatrix();
			float4x4 const & proj = app.ActiveCamera().ProjMatrix();

			float4x4 const mv = model * view;
			float4x4 const mvp = mv * proj;

			*(technique_->Effect().ParameterByName("model")) = model;
			*(technique_->Effect().ParameterByName("modelit")) = MathLib::transpose(MathLib::inverse(model));
			*(technique_->Effect().ParameterByName("mvp")) = mvp;
			*(technique_->Effect().ParameterByName("mv")) = mv;
			*(technique_->Effect().ParameterByName("eye_pos")) = app.ActiveCamera().EyePos();

			*(technique_->Effect().ParameterByName("inv_vp")) = MathLib::inverse(view * proj);
		}

	private:
		RenderTechniquePtr back_face_tech_;
		RenderTechniquePtr front_face_tech_;
	};

	class RefractorObject : public SceneObjectHelper
	{
	public:
		RefractorObject(TexturePtr const & y_cube, TexturePtr const & c_cube)
			: SceneObjectHelper(SOA_Cullable)
		{
			renderable_ = LoadKModel("teapot.kmodel", EAH_CPU_Write | EAH_GPU_Read, CreateKModelFactory<RenderModel>(), CreateKMeshFactory<RefractorRenderable>())->Mesh(0);
			checked_pointer_cast<RefractorRenderable>(renderable_)->CompressedCubeMap(y_cube, c_cube);
		}

		void Pass(int pass)
		{
			checked_pointer_cast<RefractorRenderable>(renderable_)->Pass(pass);
		}

		void BackFaceTexture(TexturePtr const & bf_tex)
		{
			checked_pointer_cast<RefractorRenderable>(renderable_)->BackFaceTexture(bf_tex);
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
		RenderFactory& rf = Context::Instance().RenderFactoryInstance();
		RenderEngine& re = rf.RenderEngineInstance();
		RenderDeviceCaps const & caps = re.DeviceCaps();
		if (caps.max_shader_model < 2)
		{
			return false;
		}

		try
		{
			TexturePtr temp_tex = rf.MakeTexture2D(800, 600, 1, EF_ABGR16F, EAH_GPU_Read | EAH_GPU_Write);
			rf.Make2DRenderView(*temp_tex, 0);
			rf.MakeDepthStencilRenderView(800, 600, EF_D16, 0);
		}
		catch (...)
		{
			return false;
		}

		return true;
	}
}

int main()
{
	ResLoader::Instance().AddPath("../../media/Common");
	ResLoader::Instance().AddPath("../../media/Refract");

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

	Refract app("Refract", settings);
	app.Create();
	app.Run();

	return 0;
}

Refract::Refract(std::string const & name, RenderSettings const & settings)
			: App3DFramework(name, settings)
{
}

void Refract::InitObjects()
{
	// 建立字体
	font_ = Context::Instance().RenderFactoryInstance().MakeFont("gkai00mp.kfont", 16);

	y_cube_map_ = LoadTexture("uffizi_cross_y.dds", EAH_CPU_Write | EAH_GPU_Read);
	c_cube_map_ = LoadTexture("uffizi_cross_c.dds", EAH_CPU_Write | EAH_GPU_Read);

	refractor_.reset(new RefractorObject(y_cube_map_, c_cube_map_));
	refractor_->AddToSceneManager();

	sky_box_.reset(new HDRSceneObjectSkyBox);
	checked_pointer_cast<HDRSceneObjectSkyBox>(sky_box_)->CompressedCubeMap(y_cube_map_, c_cube_map_);
	sky_box_->AddToSceneManager();

	RenderFactory& rf = Context::Instance().RenderFactoryInstance();
	RenderEngine& re = rf.RenderEngineInstance();

	this->LookAt(float3(0.36f, 0.11f, -0.39f), float3(0, 0.11f, 0));
	this->Proj(0.05f, 100);

	fpcController_.AttachCamera(this->ActiveCamera());
	fpcController_.Scalers(0.05f, 0.05f);

	InputEngine& inputEngine(Context::Instance().InputFactoryInstance().InputEngineInstance());
	InputActionMap actionMap;
	actionMap.AddActions(actions, actions + sizeof(actions) / sizeof(actions[0]));

	action_handler_t input_handler(new input_signal);
	input_handler->connect(boost::bind(&Refract::InputHandler, this, _1, _2));
	inputEngine.ActionMap(actionMap, input_handler, true);

	render_buffer_ = rf.MakeFrameBuffer();
	hdr_buffer_ = rf.MakeFrameBuffer();
	FrameBufferPtr screen_buffer = re.CurFrameBuffer();
	render_buffer_->GetViewport().camera = hdr_buffer_->GetViewport().camera = screen_buffer->GetViewport().camera;

	hdr_.reset(new HDRPostProcess);
}

void Refract::OnResize(uint32_t width, uint32_t height)
{
	App3DFramework::OnResize(width, height);

	RenderFactory& rf = Context::Instance().RenderFactoryInstance();

	render_tex_ = rf.MakeTexture2D(width, height, 1, EF_ABGR16F, EAH_GPU_Read | EAH_GPU_Write);
	render_buffer_->Attach(FrameBuffer::ATT_Color0, rf.Make2DRenderView(*render_tex_, 0));
	render_buffer_->Attach(FrameBuffer::ATT_DepthStencil, rf.MakeDepthStencilRenderView(width, height, EF_D16, 0));

	hdr_tex_ = rf.MakeTexture2D(width, height, 1, EF_ABGR16F, EAH_GPU_Read | EAH_GPU_Write);
	hdr_buffer_->Attach(FrameBuffer::ATT_Color0, rf.Make2DRenderView(*hdr_tex_, 0));
	hdr_buffer_->Attach(FrameBuffer::ATT_DepthStencil, rf.RenderEngineInstance().CurFrameBuffer()->Attached(FrameBuffer::ATT_DepthStencil));

	hdr_->Source(hdr_tex_, hdr_buffer_->RequiresFlipping());
	hdr_->Destinate(FrameBufferPtr());
}

void Refract::InputHandler(InputEngine const & /*sender*/, InputAction const & action)
{
	switch (action.first)
	{
	case Exit:
		this->Quit();
		break;
	}
}

uint32_t Refract::DoUpdate(uint32_t pass)
{
	RenderEngine& re(Context::Instance().RenderFactoryInstance().RenderEngineInstance());

	switch (pass)
	{
	case 0:
		// 第一遍，渲染背面
		re.BindFrameBuffer(render_buffer_);
		re.CurFrameBuffer()->Clear(FrameBuffer::CBM_Color | FrameBuffer::CBM_Depth, Color(0, 0, 0, 1), 0.0f, 0);

		checked_pointer_cast<RefractorObject>(refractor_)->Pass(0);
		sky_box_->Visible(false);
		return App3DFramework::URV_Need_Flush;

	case 1:
		// 第二遍，渲染正面
		re.BindFrameBuffer(hdr_buffer_);
		re.CurFrameBuffer()->Clear(FrameBuffer::CBM_Color | FrameBuffer::CBM_Depth, Color(0.2f, 0.4f, 0.6f, 1), 1.0f, 0);

		checked_pointer_cast<RefractorObject>(refractor_)->Pass(1);
		checked_pointer_cast<RefractorObject>(refractor_)->BackFaceTexture(render_tex_);

		sky_box_->Visible(true);
		return App3DFramework::URV_Need_Flush;

	default:
		hdr_->Apply();

		re.BindFrameBuffer(FrameBufferPtr());
		re.CurFrameBuffer()->Attached(FrameBuffer::ATT_DepthStencil)->Clear(1.0f);

		std::wostringstream stream;
		stream << this->FPS() << " FPS";

		font_->RenderText(0, 0, Color(1, 1, 0, 1), L"HDR Refract");
		font_->RenderText(0, 18, Color(1, 1, 0, 1), stream.str());
		return App3DFramework::URV_Only_New_Objs | App3DFramework::URV_Need_Flush | App3DFramework::URV_Finished;
	}
}
