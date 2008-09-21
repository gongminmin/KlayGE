#include <KlayGE/KlayGE.hpp>
#include <KlayGE/ThrowErr.hpp>
#include <KlayGE/Util.hpp>
#include <KlayGE/Math.hpp>
#include <KlayGE/Font.hpp>
#include <KlayGE/Renderable.hpp>
#include <KlayGE/RenderableHelper.hpp>
#include <KlayGE/FrameBuffer.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/RenderEffect.hpp>
#include <KlayGE/SceneManager.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/ResLoader.hpp>
#include <KlayGE/RenderSettings.hpp>
#include <KlayGE/KMesh.hpp>
#include <KlayGE/SceneObjectHelper.hpp>
#include <KlayGE/PostProcess.hpp>
#include <KlayGE/SATPostProcess.hpp>
#include <KlayGE/Util.hpp>

#include <KlayGE/D3D9/D3D9RenderFactory.hpp>
#include <KlayGE/OpenGL/OGLRenderFactory.hpp>

#include <KlayGE/OCTree/OCTree.hpp>

#include <KlayGE/Input.hpp>
#include <KlayGE/DInput/DInputFactory.hpp>

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

#include "DepthOfField.hpp"

using namespace std;
using namespace KlayGE;

namespace
{
	int const NUM_INSTANCE = 400;

	class Teapot : public SceneObjectHelper
	{
	private:
		struct InstData
		{
			float4 col[3];
			Color clr;
		};

	public:
		Teapot()
			: SceneObjectHelper(SOA_Cullable)
		{
			instance_format_.push_back(vertex_element(VEU_TextureCoord, 1, EF_ABGR32F));
			instance_format_.push_back(vertex_element(VEU_TextureCoord, 2, EF_ABGR32F));
			instance_format_.push_back(vertex_element(VEU_TextureCoord, 3, EF_ABGR32F));
			instance_format_.push_back(vertex_element(VEU_Diffuse, 0, EF_ABGR32F));
		}

		void Instance(float4x4 const & mat, Color const & clr)
		{
			float4x4 matT = MathLib::transpose(mat);

			inst_.col[0] = matT.Row(0);
			inst_.col[1] = matT.Row(1);
			inst_.col[2] = matT.Row(2);
			inst_.clr = clr;
		}

		void const * InstanceData() const
		{
			return &inst_;
		}

		void SetRenderable(RenderablePtr ra)
		{
			renderable_ = ra;
		}

	private:
		InstData inst_;
	};

	class RenderInstance : public KMesh
	{
	public:
		RenderInstance(RenderModelPtr model, std::wstring const & /*name*/)
			: KMesh(model, L"Instance")
		{
			RenderFactory& rf = Context::Instance().RenderFactoryInstance();

			technique_ = rf.LoadEffect("DepthOfField.kfx")->TechniqueByName("ColorDepth");
		}

		void BuildMeshInfo()
		{
		}

		void OnRenderBegin()
		{
			App3DFramework const & app = Context::Instance().AppInstance();

			float4x4 const & view = app.ActiveCamera().ViewMatrix();
			float4x4 const & proj = app.ActiveCamera().ProjMatrix();

			*(technique_->Effect().ParameterByName("view")) = view;
			*(technique_->Effect().ParameterByName("view_proj")) = view * proj;

			*(technique_->Effect().ParameterByName("light_in_world")) = float3(2, 2, -3);

			*(technique_->Effect().ParameterByName("depth_min")) = app.ActiveCamera().NearPlane();
			*(technique_->Effect().ParameterByName("inv_depth_range")) = 1 / (app.ActiveCamera().FarPlane() - app.ActiveCamera().NearPlane());
		}
	};

	class ClearFloatPostProcess : public PostProcess
	{
	public:
		ClearFloatPostProcess()
			: PostProcess(Context::Instance().RenderFactoryInstance().LoadEffect("DepthOfField.kfx")->TechniqueByName("ClearFloat"))
		{
		}

		void ClearColor(float4 const & clr)
		{
			*(technique_->Effect().ParameterByName("clear_clr")) = clr;
		}

		void OnRenderBegin()
		{
		}
	};

	class DepthOfField : public PostProcess
	{
	public:
		DepthOfField()
			: PostProcess(Context::Instance().RenderFactoryInstance().LoadEffect("DepthOfField.kfx")->TechniqueByName("DepthOfField"))
		{
		}

		void FocusPlane(float focus_plane)
		{
			focus_plane_ = focus_plane;
		}
		float FocusPlane() const
		{
			return focus_plane_;
		}

		void FocusRange(float focus_range)
		{
			focus_range_ = focus_range;
		}
		float FocusRange() const
		{
			return focus_range_;
		}

		void ShowBlurFactor(bool show)
		{
			show_blur_factor_ = show;
			*(technique_->Effect().ParameterByName("show_blur_factor")) = show_blur_factor_;
		}
		bool ShowBlurFactor() const
		{
			return show_blur_factor_;
		}

		void Source(TexturePtr const & tex, bool flipping)
		{
			PostProcess::Source(tex, flipping);

			uint32_t const width = tex->Width(0);
			uint32_t const height = tex->Height(0);

			sat_.Source(tex, flipping);

			*(technique_->Effect().ParameterByName("sat_size")) = float4(static_cast<float>(width),
				static_cast<float>(height), 1.0f / width, 1.0f / height);
		}

		void Apply()
		{
			if (!show_blur_factor_)
			{
				sat_.Apply();
				*(technique_->Effect().ParameterByName("sat_sampler")) = sat_.SATTexture();
			}

			PostProcess::Apply();
		}

		void OnRenderBegin()
		{
			PostProcess::OnRenderBegin();

			App3DFramework const & app = Context::Instance().AppInstance();
			float const depth_range = app.ActiveCamera().FarPlane() - app.ActiveCamera().NearPlane();

			*(technique_->Effect().ParameterByName("focus_plane")) = (focus_plane_ - app.ActiveCamera().NearPlane()) / depth_range;
			*(technique_->Effect().ParameterByName("inv_focus_range")) = depth_range / focus_range_;
		}

	private:
		SummedAreaTablePostProcess sat_;

		float focus_plane_;
		float focus_range_;
		bool show_blur_factor_;
	};


	enum
	{
		FocusPlaneSlider,
		FocusPlaneStatic,
		FocusRangeSlider,
		FocusRangeStatic,
		BlurFactor,
		CtrlCamera,
	};

	enum
	{
		Exit,
	};

	InputActionDefine actions[] =
	{
		InputActionDefine(CtrlCamera, KS_LeftCtrl),
		InputActionDefine(CtrlCamera, KS_RightCtrl),
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
	ResLoader::Instance().AddPath("../../media/DepthOfField");

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

	DepthOfFieldApp app("Depth of field", settings);
	app.Create();
	app.Run();

	return 0;
}

DepthOfFieldApp::DepthOfFieldApp(std::string const & name, RenderSettings const & settings)
					: App3DFramework(name, settings)
{
}

void DepthOfFieldApp::InitObjects()
{
	font_ = Context::Instance().RenderFactoryInstance().MakeFont("gkai00mp.kfont", 16);

	boost::shared_ptr<KlayGE::Renderable> renderInstance = LoadKModel("teapot.kmodel", EAH_CPU_Write | EAH_GPU_Read, CreateKModelFactory<RenderModel>(), CreateKMeshFactory<RenderInstance>())->Mesh(0);
	for (int i = 0; i < 10; ++ i)
	{
		for (int j = 0; j < NUM_INSTANCE / 10; ++ j)
		{
			float const s = sin(2 * PI * j / (NUM_INSTANCE / 10));
			float const c = cos(2 * PI * j / (NUM_INSTANCE / 10));

			SceneObjectPtr so(new Teapot);
			checked_pointer_cast<Teapot>(so)->Instance(
				MathLib::translation(s, i / 10.0f, c), Color(s, c, 0, 1));

			checked_pointer_cast<Teapot>(so)->SetRenderable(renderInstance);
			so->AddToSceneManager();
		}
	}

	this->LookAt(float3(-1.8f, 1.9f, -1.8f), float3(0, 0, 0));
	this->Proj(0.1f, 100);

	RenderEngine& renderEngine(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
	clr_depth_buffer_ = Context::Instance().RenderFactoryInstance().MakeFrameBuffer();
	clr_depth_buffer_->GetViewport().camera = renderEngine.CurFrameBuffer()->GetViewport().camera;

	fpcController_.Scalers(0.05f, 0.5f);

	InputEngine& inputEngine(Context::Instance().InputFactoryInstance().InputEngineInstance());
	InputActionMap actionMap;
	actionMap.AddActions(actions, actions + sizeof(actions) / sizeof(actions[0]));

	action_handler_t input_handler(new input_signal);
	input_handler->connect(boost::bind(&DepthOfFieldApp::InputHandler, this, _1, _2));
	inputEngine.ActionMap(actionMap, input_handler, true);

	depth_of_field_.reset(new DepthOfField);
	depth_of_field_->Destinate(FrameBufferPtr());

	clear_float_.reset(new ClearFloatPostProcess);
	checked_pointer_cast<ClearFloatPostProcess>(clear_float_)->ClearColor(float4(0.2f - 0.5f, 0.4f - 0.5f, 0.6f - 0.5f, 1 - 0.5f));

	dialog_ = UIManager::Instance().MakeDialog();

	dialog_->AddControl(UIControlPtr(new UIStatic(dialog_, FocusPlaneStatic, L"Focus plane:", 60, 200, 100, 24, false)));
	dialog_->AddControl(UIControlPtr(new UISlider(dialog_, FocusPlaneSlider, 60, 220, 100, 24, 0, 200, 100, false)));
	dialog_->Control<UISlider>(FocusPlaneSlider)->OnValueChangedEvent().connect(boost::bind(&DepthOfFieldApp::FocusPlaneChangedHandler, this, _1));

	dialog_->AddControl(UIControlPtr(new UIStatic(dialog_, FocusRangeStatic, L"Focus range:", 60, 268, 100, 24, false)));
	dialog_->AddControl(UIControlPtr(new UISlider(dialog_, FocusRangeSlider, 60, 288, 100, 24, 0, 200, 100, false)));
	dialog_->Control<UISlider>(FocusRangeSlider)->OnValueChangedEvent().connect(boost::bind(&DepthOfFieldApp::FocusRangeChangedHandler, this, _1));

	dialog_->AddControl(UIControlPtr(new UICheckBox(dialog_, BlurFactor, L"Blur factor",
                            60, 356, 350, 24, false, 0, false)));
	dialog_->Control<UICheckBox>(BlurFactor)->SetChecked(false);
	dialog_->Control<UICheckBox>(BlurFactor)->OnChangedEvent().connect(boost::bind(&DepthOfFieldApp::BlurFactorHandler, this, _1));

	dialog_->AddControl(UIControlPtr(new UICheckBox(dialog_, CtrlCamera, L"Control camera",
                            60, 424, 350, 24, false, 0, false)));
	dialog_->Control<UICheckBox>(CtrlCamera)->SetChecked(false);
	dialog_->Control<UICheckBox>(CtrlCamera)->OnChangedEvent().connect(boost::bind(&DepthOfFieldApp::CtrlCameraHandler, this, _1));

	this->FocusPlaneChangedHandler(*dialog_->Control<UISlider>(FocusPlaneSlider));
	this->FocusRangeChangedHandler(*dialog_->Control<UISlider>(FocusRangeSlider));
	this->BlurFactorHandler(*dialog_->Control<UICheckBox>(BlurFactor));
}

void DepthOfFieldApp::OnResize(uint32_t width, uint32_t height)
{
	RenderFactory& rf = Context::Instance().RenderFactoryInstance();
	clr_depth_tex_ = rf.MakeTexture2D(width, height, 1, EF_ABGR16F, EAH_GPU_Read | EAH_GPU_Write);
	clr_depth_buffer_->Attach(FrameBuffer::ATT_Color0, rf.Make2DRenderView(*clr_depth_tex_, 0));
	clr_depth_buffer_->Attach(FrameBuffer::ATT_DepthStencil, rf.MakeDepthStencilRenderView(width, height, EF_D16, 0));

	depth_of_field_->Source(clr_depth_tex_, clr_depth_buffer_->RequiresFlipping());
	depth_of_field_->Destinate(FrameBufferPtr());

	clear_float_->Destinate(clr_depth_buffer_);

	dialog_->GetControl(FocusPlaneStatic)->SetLocation(width - 120, 200);
	dialog_->GetControl(FocusPlaneSlider)->SetLocation(width - 120, 220);
	dialog_->GetControl(FocusRangeStatic)->SetLocation(width - 120, 268);
	dialog_->GetControl(FocusRangeSlider)->SetLocation(width - 120, 288);
	dialog_->GetControl(BlurFactor)->SetLocation(width - 120, 356);
	dialog_->GetControl(CtrlCamera)->SetLocation(width - 120, 424);
}

void DepthOfFieldApp::InputHandler(InputEngine const & /*sender*/, InputAction const & action)
{
	switch (action.first)
	{
	case Exit:
		this->Quit();
		break;
	}
}

void DepthOfFieldApp::FocusPlaneChangedHandler(KlayGE::UISlider const & sender)
{
	checked_pointer_cast<DepthOfField>(depth_of_field_)->FocusPlane(sender.GetValue() / 50.0f);
}

void DepthOfFieldApp::FocusRangeChangedHandler(KlayGE::UISlider const & sender)
{
	checked_pointer_cast<DepthOfField>(depth_of_field_)->FocusRange(sender.GetValue() / 50.0f);
}

void DepthOfFieldApp::BlurFactorHandler(KlayGE::UICheckBox const & sender)
{
	checked_pointer_cast<DepthOfField>(depth_of_field_)->ShowBlurFactor(sender.GetChecked());
}

void DepthOfFieldApp::CtrlCameraHandler(KlayGE::UICheckBox const & sender)
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

uint32_t DepthOfFieldApp::DoUpdate(uint32_t pass)
{
	RenderEngine& renderEngine(Context::Instance().RenderFactoryInstance().RenderEngineInstance());

	switch (pass)
	{
	case 0:
		UIManager::Instance().HandleInput();

		renderEngine.BindFrameBuffer(clr_depth_buffer_);
		clear_float_->Apply();
		renderEngine.CurFrameBuffer()->Attached(FrameBuffer::ATT_DepthStencil)->Clear(1.0f);
		return App3DFramework::URV_Need_Flush;

	default:
		depth_of_field_->Apply();

		renderEngine.BindFrameBuffer(FrameBufferPtr());
		renderEngine.CurFrameBuffer()->Attached(FrameBuffer::ATT_DepthStencil)->Clear(1.0f);

		UIManager::Instance().Render();

		FrameBuffer& rw = *renderEngine.CurFrameBuffer();

		font_->RenderText(0, 0, Color(1, 1, 0, 1), L"Depth of field");
		font_->RenderText(0, 18, Color(1, 1, 0, 1), rw.Description());

		std::wostringstream stream;
		stream << this->FPS() << " FPS";
		font_->RenderText(0, 36, Color(1, 1, 0, 1), stream.str());
		return App3DFramework::URV_Only_New_Objs | App3DFramework::URV_Need_Flush | App3DFramework::URV_Finished;
	}
}
