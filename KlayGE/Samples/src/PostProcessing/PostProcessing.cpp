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
#include <KlayGE/SceneObjectHelper.hpp>
#include <KlayGE/PostProcess.hpp>
#include <KlayGE/HDRPostProcess.hpp>

#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/InputFactory.hpp>

#include <sstream>
#include <ctime>
#include <boost/bind.hpp>

#include "AsciiArtsPP.hpp"
#include "CartoonPP.hpp"
#include "TilingPP.hpp"
#include "PostProcessing.hpp"

using namespace std;
using namespace KlayGE;

namespace
{
	class RenderTorus : public KMesh
	{
	public:
		RenderTorus(RenderModelPtr model, std::wstring const & /*name*/)
			: KMesh(model, L"Torus")
		{
			RenderFactory& rf = Context::Instance().RenderFactoryInstance();

			technique_ = rf.LoadEffect("PostProcessing.fxml")->TechniqueByName("GBufferTech");
		}

		void BuildMeshInfo()
		{
		}

		void OnRenderBegin()
		{
			Camera const & camera = Context::Instance().AppInstance().ActiveCamera();

			float4x4 const & view = camera.ViewMatrix();
			float4x4 const & proj = camera.ProjMatrix();

			*(technique_->Effect().ParameterByName("model_view")) = view;
			*(technique_->Effect().ParameterByName("proj")) = proj;

			*(technique_->Effect().ParameterByName("light_in_eye")) = MathLib::transform_coord(float3(2, 2, -3), view);
			*(technique_->Effect().ParameterByName("depth_near_far_invfar")) = float3(camera.NearPlane(), camera.FarPlane(), 1 / camera.FarPlane());
		}
	};

	class TorusObject : public SceneObjectHelper
	{
	public:
		TorusObject()
			: SceneObjectHelper(SOA_Cullable)
		{
			renderable_ = LoadModel("dino50.meshml", EAH_GPU_Read, CreateKModelFactory<RenderModel>(), CreateKMeshFactory<RenderTorus>())->Mesh(0);
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
		if (caps.max_simultaneous_rts < 2)
		{
			return false;
		}

		try
		{
			TexturePtr temp_tex = rf.MakeTexture2D(800, 600, 1, EF_ABGR16F, 1, 0, EAH_GPU_Read | EAH_GPU_Write, NULL);
			rf.Make2DRenderView(*temp_tex, 0);
			rf.MakeDepthStencilRenderView(800, 600, EF_D16, 1, 0);
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
	ResLoader::Instance().AddPath("../Samples/media/Common");
	ResLoader::Instance().AddPath("../Samples/media/PostProcessing");

	RenderSettings settings = Context::Instance().LoadCfg("KlayGE.cfg");
	settings.ConfirmDevice = ConfirmDevice;

	PostProcessingApp app("Post Processing", settings);
	app.Create();
	app.Run();

	return 0;
}

PostProcessingApp::PostProcessingApp(std::string const & name, RenderSettings const & settings)
			: App3DFramework(name, settings)
{
}

void PostProcessingApp::InitObjects()
{
	font_ = Context::Instance().RenderFactoryInstance().MakeFont("gkai00mp.kfont");

	torus_ = MakeSharedPtr<TorusObject>();
	torus_->AddToSceneManager();

	this->LookAt(float3(0, 0, -2), float3(0, 0, 0));
	this->Proj(0.1f, 100.0f);

	TexturePtr y_cube_map = LoadTexture("uffizi_cross_y.dds", EAH_GPU_Read)();
	TexturePtr c_cube_map = LoadTexture("uffizi_cross_c.dds", EAH_GPU_Read)();
	sky_box_ = MakeSharedPtr<SceneObjectHDRSkyBox>();
	checked_pointer_cast<SceneObjectHDRSkyBox>(sky_box_)->CompressedCubeMap(y_cube_map, c_cube_map);
	sky_box_->AddToSceneManager();

	RenderEngine& renderEngine(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
	g_buffer_ = Context::Instance().RenderFactoryInstance().MakeFrameBuffer();
	g_buffer_->GetViewport().camera = renderEngine.CurFrameBuffer()->GetViewport().camera;

	fpcController_.Scalers(0.05f, 0.1f);

	InputEngine& inputEngine(Context::Instance().InputFactoryInstance().InputEngineInstance());
	InputActionMap actionMap;
	actionMap.AddActions(actions, actions + sizeof(actions) / sizeof(actions[0]));

	action_handler_t input_handler = MakeSharedPtr<input_signal>();
	input_handler->connect(boost::bind(&PostProcessingApp::InputHandler, this, _1, _2));
	inputEngine.ActionMap(actionMap, input_handler, true);

	ascii_arts_ = MakeSharedPtr<AsciiArtsPostProcess>();
	cartoon_ = MakeSharedPtr<CartoonPostProcess>();
	tiling_ = MakeSharedPtr<TilingPostProcess>();
	hdr_ = MakeSharedPtr<HDRPostProcess>(false);

	UIManager::Instance().Load(ResLoader::Instance().Load("PostProcessing.uiml"));
	dialog_ = UIManager::Instance().GetDialogs()[0];

	id_fps_camera_ = dialog_->IDFromName("FPSCamera");
	id_ascii_arts_ = dialog_->IDFromName("AsciiArtsPP");
	id_cartoon_ = dialog_->IDFromName("CartoonPP");
	id_tiling_ = dialog_->IDFromName("TilingPP");
	id_hdr_ = dialog_->IDFromName("HDRPP");

	dialog_->Control<UICheckBox>(id_fps_camera_)->OnChangedEvent().connect(boost::bind(&PostProcessingApp::FPSCameraHandler, this, _1));
	dialog_->Control<UIRadioButton>(id_ascii_arts_)->OnChangedEvent().connect(boost::bind(&PostProcessingApp::AsciiArtsHandler, this, _1));
	dialog_->Control<UIRadioButton>(id_cartoon_)->OnChangedEvent().connect(boost::bind(&PostProcessingApp::CartoonHandler, this, _1));
	dialog_->Control<UIRadioButton>(id_tiling_)->OnChangedEvent().connect(boost::bind(&PostProcessingApp::TilingHandler, this, _1));
	dialog_->Control<UIRadioButton>(id_hdr_)->OnChangedEvent().connect(boost::bind(&PostProcessingApp::HDRHandler, this, _1));
	this->CartoonHandler(*dialog_->Control<UIRadioButton>(id_cartoon_));
}

void PostProcessingApp::OnResize(uint32_t width, uint32_t height)
{
	App3DFramework::OnResize(width, height);

	RenderFactory& rf = Context::Instance().RenderFactoryInstance();
	color_tex_ = rf.MakeTexture2D(width, height, 1, EF_ABGR16F, 1, 0, EAH_GPU_Read | EAH_GPU_Write, NULL);
	normal_depth_tex_ = rf.MakeTexture2D(width, height, 1, EF_ABGR16F, 1, 0, EAH_GPU_Read | EAH_GPU_Write, NULL);
	g_buffer_->Attach(FrameBuffer::ATT_Color0, rf.Make2DRenderView(*color_tex_, 0));
	g_buffer_->Attach(FrameBuffer::ATT_Color1, rf.Make2DRenderView(*normal_depth_tex_, 0));
	g_buffer_->Attach(FrameBuffer::ATT_DepthStencil, rf.MakeDepthStencilRenderView(width, height, EF_D16, 1, 0));

	ascii_arts_->Source(color_tex_, g_buffer_->RequiresFlipping());
	ascii_arts_->Destinate(FrameBufferPtr());

	cartoon_->Source(normal_depth_tex_, g_buffer_->RequiresFlipping());
	checked_pointer_cast<CartoonPostProcess>(cartoon_)->ColorTex(color_tex_);
	cartoon_->Destinate(FrameBufferPtr());

	tiling_->Source(color_tex_, g_buffer_->RequiresFlipping());
	tiling_->Destinate(FrameBufferPtr());

	hdr_->Source(color_tex_, g_buffer_->RequiresFlipping());
	hdr_->Destinate(FrameBufferPtr());

	UIManager::Instance().SettleCtrls(width, height);
}

void PostProcessingApp::InputHandler(InputEngine const & /*sender*/, InputAction const & action)
{
	switch (action.first)
	{
	case Exit:
		this->Quit();
		break;
	}
}

void PostProcessingApp::FPSCameraHandler(UICheckBox const & sender)
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

void PostProcessingApp::AsciiArtsHandler(UIRadioButton const & sender)
{
	if (sender.GetChecked())
	{
		active_pp_ = ascii_arts_;
	}
}

void PostProcessingApp::CartoonHandler(UIRadioButton const & sender)
{
	if (sender.GetChecked())
	{
		active_pp_ = cartoon_;
	}
}

void PostProcessingApp::TilingHandler(UIRadioButton const & sender)
{
	if (sender.GetChecked())
	{
		active_pp_ = tiling_;
	}
}

void PostProcessingApp::HDRHandler(UIRadioButton const & sender)
{
	if (sender.GetChecked())
	{
		active_pp_ = hdr_;
	}
}

void PostProcessingApp::DoUpdateOverlay()
{
	RenderEngine& renderEngine(Context::Instance().RenderFactoryInstance().RenderEngineInstance());

	UIManager::Instance().Render();

	FrameBuffer& rw = *checked_pointer_cast<FrameBuffer>(renderEngine.CurFrameBuffer());

	font_->RenderText(0, 0, Color(1, 1, 0, 1), L"Post Processing", 16);
	font_->RenderText(0, 18, Color(1, 1, 0, 1), rw.Description(), 16);

	std::wostringstream stream;
	stream << rw.DepthBits() << " bits depth " << rw.StencilBits() << " bits stencil";
	font_->RenderText(0, 36, Color(1, 1, 1, 1), stream.str(), 16);

	stream.str(L"");
	stream.precision(2);
	stream << fixed << this->FPS() << " FPS";
	font_->RenderText(0, 54, Color(1, 1, 0, 1), stream.str(), 16);
}

uint32_t PostProcessingApp::DoUpdate(uint32_t pass)
{
	RenderEngine& renderEngine(Context::Instance().RenderFactoryInstance().RenderEngineInstance());

	switch (pass)
	{
	case 0:
		renderEngine.BindFrameBuffer(g_buffer_);
		renderEngine.CurFrameBuffer()->Clear(FrameBuffer::CBM_Color | FrameBuffer::CBM_Depth, Color(0, 0, 1, 1), 1.0f, 0);
		return App3DFramework::URV_Need_Flush;

	default:
		renderEngine.BindFrameBuffer(FrameBufferPtr());
		renderEngine.CurFrameBuffer()->Clear(FrameBuffer::CBM_Color | FrameBuffer::CBM_Depth, Color(0.2f, 0.4f, 0.6f, 1), 1.0f, 0);
		active_pp_->Apply();

		return App3DFramework::URV_Finished;
	}
}
