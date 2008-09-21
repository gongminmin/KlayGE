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
#include <KlayGE/OcclusionQuery.hpp>
#include <KlayGE/PostProcess.hpp>

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

#include "DepthPeeling.hpp"

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

			technique_ = rf.LoadEffect("DepthPeeling.kfx")->TechniqueByName("DepthPeeling1st");
		}

		void BuildMeshInfo()
		{
		}

		void DepthPeelingEnabled(bool dp)
		{
			if (!dp)
			{
				technique_ = technique_->Effect().TechniqueByName("NoDepthPeeling");
			}
		}

		void FirstPass(bool fp)
		{
			if (fp)
			{
				technique_ = technique_->Effect().TechniqueByName("DepthPeeling1st");
			}
			else
			{
				technique_ = technique_->Effect().TechniqueByName("DepthPeelingNth");
			}
		}

		void LastDepth(TexturePtr depth_tex)
		{
			*(technique_->Effect().ParameterByName("last_depth_sampler")) = depth_tex;
		}

		void OnRenderBegin()
		{
			App3DFramework const & app = Context::Instance().AppInstance();

			float4x4 const & model = float4x4::Identity();
			float4x4 const & view = app.ActiveCamera().ViewMatrix();
			float4x4 const & proj = app.ActiveCamera().ProjMatrix();

			*(technique_->Effect().ParameterByName("mvp")) = model * view * proj;

			RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
			float4 const & texel_to_pixel = re.TexelToPixelOffset() * 2;
			float const x_offset = texel_to_pixel.x() / re.CurFrameBuffer()->Width();
			float const y_offset = texel_to_pixel.y() / re.CurFrameBuffer()->Height();
			*(technique_->Effect().ParameterByName("offset")) = float2(x_offset, y_offset);
		}

		void LightPos(float3 const & light_pos)
		{
			*(technique_->Effect().ParameterByName("light_pos")) = light_pos;
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

		void DepthPeelingEnabled(bool dp)
		{
			RenderModelPtr model = checked_pointer_cast<RenderModel>(renderable_);
			for (uint32_t i = 0; i < model->NumMeshes(); ++ i)
			{
				checked_pointer_cast<RenderPolygon>(model->Mesh(i))->DepthPeelingEnabled(dp);
			}
		}

		void FirstPass(bool fp)
		{
			RenderModelPtr model = checked_pointer_cast<RenderModel>(renderable_);
			for (uint32_t i = 0; i < model->NumMeshes(); ++ i)
			{
				checked_pointer_cast<RenderPolygon>(model->Mesh(i))->FirstPass(fp);
			}
		}

		void LastDepth(TexturePtr depth_tex)
		{
			RenderModelPtr model = checked_pointer_cast<RenderModel>(renderable_);
			for (uint32_t i = 0; i < model->NumMeshes(); ++ i)
			{
				checked_pointer_cast<RenderPolygon>(model->Mesh(i))->LastDepth(depth_tex);
			}
		}
	};

	class BlendPostProcess : public PostProcess
	{
	public:
		BlendPostProcess()
			: PostProcess(Context::Instance().RenderFactoryInstance().LoadEffect("DepthPeeling.kfx")->TechniqueByName("Blend"))
		{
		}
	};


	enum
	{
		UseDepthPeeling
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
		if (caps.max_simultaneous_rts < 2)
		{
			return false;
		}

		try
		{
			TexturePtr temp_tex = rf.MakeTexture2D(800, 600, 1, EF_R32F, EAH_GPU_Read | EAH_GPU_Write);
			rf.Make2DRenderView(*temp_tex, 0);
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
	ResLoader::Instance().AddPath("../../media/DepthPeeling");

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

	DepthPeelingApp app("DepthPeeling", settings);
	app.Create();
	app.Run();

	return 0;
}

DepthPeelingApp::DepthPeelingApp(std::string const & name, RenderSettings const & settings)
			: App3DFramework(name, settings)
{
}

void DepthPeelingApp::InitObjects()
{
	// ½¨Á¢×ÖÌå
	font_ = Context::Instance().RenderFactoryInstance().MakeFont("gkai00mp.kfont", 16);

	polygon_.reset(new PolygonObject);
	checked_pointer_cast<PolygonObject>(polygon_)->LightPos(float3(0, 2, -1));
	polygon_->AddToSceneManager();

	this->LookAt(float3(-0.3f, 0.4f, -0.3f), float3(0, 0, 0));
	this->Proj(0.01f, 100);

	RenderFactory& rf = Context::Instance().RenderFactoryInstance();
	RenderEngine& re = rf.RenderEngineInstance();

	peeling_fbs_.resize(9);
	for (size_t i = 0; i < peeling_fbs_.size(); ++ i)
	{
		peeling_fbs_[i] = rf.MakeFrameBuffer();
		peeling_fbs_[i]->GetViewport().camera = re.CurFrameBuffer()->GetViewport().camera;
	}
	peeled_texs_.resize(peeling_fbs_.size());
	peeled_views_.resize(peeled_texs_.size());

	for (size_t i = 0; i < oc_queries_.size(); ++ i)
	{
		oc_queries_[i] = rf.MakeOcclusionQuery();
	}

	fpcController_.AttachCamera(this->ActiveCamera());
	fpcController_.Scalers(0.05f, 0.01f);

	InputEngine& inputEngine(Context::Instance().InputFactoryInstance().InputEngineInstance());
	InputActionMap actionMap;
	actionMap.AddActions(actions, actions + sizeof(actions) / sizeof(actions[0]));

	action_handler_t input_handler(new input_signal);
	input_handler->connect(boost::bind(&DepthPeelingApp::InputHandler, this, _1, _2));
	inputEngine.ActionMap(actionMap, input_handler, true);

	blend_pp_.reset(new BlendPostProcess);

	dialog_ = UIManager::Instance().MakeDialog();
	dialog_->AddControl(UIControlPtr(new UICheckBox(dialog_, UseDepthPeeling, L"Depth peeling",
                            60, 550, 350, 24, false, 0, false)));
	dialog_->Control<UICheckBox>(UseDepthPeeling)->SetChecked(true);
	dialog_->Control<UICheckBox>(UseDepthPeeling)->OnChangedEvent().connect(boost::bind(&DepthPeelingApp::CheckBoxHandler, this, _1));
}

void DepthPeelingApp::OnResize(uint32_t width, uint32_t height)
{
	App3DFramework::OnResize(width, height);

	RenderFactory& rf = Context::Instance().RenderFactoryInstance();
	RenderEngine& re = rf.RenderEngineInstance();

	depth_texs_[0] = rf.MakeTexture2D(width, height, 1, EF_R32F, EAH_GPU_Read | EAH_GPU_Write);
	depth_texs_[1] = rf.MakeTexture2D(width, height, 1, EF_R32F, EAH_GPU_Read | EAH_GPU_Write);
	depth_view_ = rf.Make2DRenderView(*depth_texs_[0], 0);

	default_depth_view_ = re.CurFrameBuffer()->Attached(FrameBuffer::ATT_DepthStencil);

	for (size_t i = 0; i < peeling_fbs_.size(); ++ i)
	{
		peeled_texs_[i] = rf.MakeTexture2D(width, height, 1, EF_ARGB8, EAH_GPU_Read | EAH_GPU_Write);
		peeled_views_[i] = rf.Make2DRenderView(*peeled_texs_[i], 0);

		peeling_fbs_[i]->Attach(FrameBuffer::ATT_Color0, peeled_views_[i]);
		peeling_fbs_[i]->Attach(FrameBuffer::ATT_Color1, depth_view_);
		peeling_fbs_[i]->Attach(FrameBuffer::ATT_DepthStencil, default_depth_view_);
	}

	blend_pp_->Destinate(FrameBufferPtr());

	dialog_->GetControl(UseDepthPeeling)->SetLocation(60, height - 50);
}

void DepthPeelingApp::InputHandler(InputEngine const & /*sender*/, InputAction const & action)
{
	switch (action.first)
	{
	case Exit:
		this->Quit();
		break;
	}
}

void DepthPeelingApp::CheckBoxHandler(UICheckBox const & /*sender*/)
{
	use_depth_peeling_ = dialog_->Control<UICheckBox>(UseDepthPeeling)->GetChecked();
}

uint32_t DepthPeelingApp::DoUpdate(uint32_t pass)
{
	SceneManager& sceneMgr(Context::Instance().SceneManagerInstance());

	RenderEngine& re(Context::Instance().RenderFactoryInstance().RenderEngineInstance());

	switch (pass)
	{
	case 0:
		UIManager::Instance().HandleInput();

		if (use_depth_peeling_)
		{
			num_layers_ = 1;

			checked_pointer_cast<PolygonObject>(polygon_)->FirstPass(true);
			re.BindFrameBuffer(peeling_fbs_[0]);
			peeled_views_[0]->Clear(Color(0, 0, 0, 0));
			depth_view_->Clear(Color(0, 0, 0, 0));
			default_depth_view_->Clear(1.0f);
			return App3DFramework::URV_Need_Flush;
		}
		else
		{
			checked_pointer_cast<PolygonObject>(polygon_)->DepthPeelingEnabled(false);

			re.BindFrameBuffer(FrameBufferPtr());
			re.CurFrameBuffer()->Attached(FrameBuffer::ATT_Color0)->Clear(Color(0.2f, 0.4f, 0.6f, 1));
			re.CurFrameBuffer()->Attached(FrameBuffer::ATT_DepthStencil)->Clear(1.0f);

			UIManager::Instance().Render();

			std::wostringstream stream;
			stream << this->FPS();

			font_->RenderText(0, 0, Color(1, 1, 0, 1), L"Depth Peeling");
			font_->RenderText(0, 18, Color(1, 1, 0, 1), stream.str());

			stream.str(L"");
			stream << sceneMgr.NumRenderablesRendered() << " Renderables "
				<< sceneMgr.NumPrimitivesRendered() << " Primitives "
				<< sceneMgr.NumVerticesRendered() << " Vertices";
			font_->RenderText(0, 36, Color(1, 1, 1, 1), stream.str());

			return App3DFramework::URV_Need_Flush | App3DFramework::URV_Finished;
		}

	case 1:
		checked_pointer_cast<PolygonObject>(polygon_)->FirstPass(false);
		checked_pointer_cast<PolygonObject>(polygon_)->LastDepth(depth_texs_[1]);
		for (size_t i = 1; i < peeled_texs_.size(); i += oc_queries_.size())
		{
			for (size_t j = 0; j < oc_queries_.size(); ++ j)
			{
				re.BindFrameBuffer(peeling_fbs_[i + j]);
				peeled_views_[i + j]->Clear(Color(0, 0, 0, 0));
				default_depth_view_->Clear(1.0f);
				depth_texs_[0]->CopyToTexture(*depth_texs_[1]);

				oc_queries_[j]->Begin();
				sceneMgr.Flush();
				oc_queries_[j]->End();
			}

			for (size_t j = 0; j < oc_queries_.size(); ++ j)
			{
				if (checked_pointer_cast<OcclusionQuery>(oc_queries_[j])->SamplesPassed() < 1)
				{
					return App3DFramework::URV_Flushed;
				}
				else
				{
					++ num_layers_;
				}
			}
		}
		return App3DFramework::URV_Flushed;

	default:
		re.BindFrameBuffer(FrameBufferPtr());
		re.CurFrameBuffer()->Attached(FrameBuffer::ATT_Color0)->Clear(Color(0.2f, 0.4f, 0.6f, 1));
		re.CurFrameBuffer()->Attached(FrameBuffer::ATT_DepthStencil)->Clear(1.0f);
		for (size_t i = 0; i < num_layers_; ++ i)
		{
			blend_pp_->Source(peeled_texs_[num_layers_ - 1 - i],
				peeling_fbs_[num_layers_ - 1 - i]->RequiresFlipping());
			blend_pp_->Apply();
		}

		UIManager::Instance().Render();

		std::wostringstream stream;
		stream << this->FPS();

		font_->RenderText(0, 0, Color(1, 1, 0, 1), L"Depth Peeling");
		font_->RenderText(0, 18, Color(1, 1, 0, 1), stream.str());

		stream.str(L"");
		stream << sceneMgr.NumRenderablesRendered() << " Renderables "
			<< sceneMgr.NumPrimitivesRendered() << " Primitives "
			<< sceneMgr.NumVerticesRendered() << " Vertices";
		font_->RenderText(0, 36, Color(1, 1, 1, 1), stream.str());

		return App3DFramework::URV_Only_New_Objs | App3DFramework::URV_Need_Flush | App3DFramework::URV_Finished;
	}
}
