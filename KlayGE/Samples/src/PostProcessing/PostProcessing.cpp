#include <KlayGE/KlayGE.hpp>
#include <KFL/ThrowErr.hpp>
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
#include <KlayGE/SceneObjectHelper.hpp>
#include <KlayGE/PostProcess.hpp>
#include <KlayGE/HDRPostProcess.hpp>
#include <KlayGE/Camera.hpp>
#include <KlayGE/DeferredRenderingLayer.hpp>

#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/InputFactory.hpp>

#include <sstream>

#include "SampleCommon.hpp"
#include "AsciiArtsPP.hpp"
#include "CartoonPP.hpp"
#include "TilingPP.hpp"
#include "PostProcessing.hpp"
#include "NightVisionPP.hpp"

using namespace std;
using namespace KlayGE;

namespace
{
	class ObjectUpdate
	{
	public:
		void operator()(SceneObject& obj, float app_time, float /*elapsed_time*/)
		{
			obj.ModelMatrix(MathLib::rotation_y(-app_time / 1.5f));
		}
	};

	class PointLightSourceUpdate
	{
	public:
		void operator()(LightSource& light, float /*app_time*/, float /*elapsed_time*/)
		{
			float4x4 inv_view = Context::Instance().AppInstance().ActiveCamera().InverseViewMatrix();
			light.Position(MathLib::transform_coord(float3(2, 2, -3), inv_view));
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
	ContextCfg cfg = Context::Instance().Config();
	cfg.deferred_rendering = true;
	Context::Instance().Config(cfg);

	PostProcessingApp app;
	app.Create();
	app.Run();

	return 0;
}

PostProcessingApp::PostProcessingApp()
			: App3DFramework("Post Processing")
{
	ResLoader::Instance().AddPath("../../Samples/media/PostProcessing");
}

bool PostProcessingApp::ConfirmDevice() const
{
	RenderDeviceCaps const & caps = Context::Instance().RenderFactoryInstance().RenderEngineInstance().DeviceCaps();
	if (caps.max_shader_model < 2)
	{
		return false;
	}
	return true;
}

void PostProcessingApp::InitObjects()
{
	this->LookAt(float3(0, 0.5f, -2), float3(0, 0, 0));
	this->Proj(0.1f, 150.0f);

	loading_percentage_ = 0;
	model_ml_ = ASyncLoadModel("dino50.7z//dino50.meshml", EAH_GPU_Read | EAH_Immutable);
	y_cube_tl_ = ASyncLoadTexture("rnl_cross_y.dds", EAH_GPU_Read | EAH_Immutable);
	c_cube_tl_ = ASyncLoadTexture("rnl_cross_c.dds", EAH_GPU_Read | EAH_Immutable);

	RenderFactory& rf = Context::Instance().RenderFactoryInstance();
	RenderEngine& re = rf.RenderEngineInstance();

	font_ = SyncLoadFont("gkai00mp.kfont");

	deferred_rendering_ = Context::Instance().DeferredRenderingLayerInstance();
	deferred_rendering_->SSVOEnabled(0, false);
	re.HDREnabled(false);
	re.PPAAEnabled(0);
	re.ColorGradingEnabled(false);

	point_light_ = MakeSharedPtr<PointLightSource>();
	point_light_->Attrib(LSA_NoShadow);
	point_light_->Color(float3(18, 18, 18));
	point_light_->Position(float3(0, 0, 0));
	point_light_->Falloff(float3(1, 0, 1));
	point_light_->BindUpdateFunc(PointLightSourceUpdate());
	point_light_->AddToSceneManager();

	fpcController_.Scalers(0.05f, 0.1f);

	InputEngine& inputEngine(Context::Instance().InputFactoryInstance().InputEngineInstance());
	InputActionMap actionMap;
	actionMap.AddActions(actions, actions + sizeof(actions) / sizeof(actions[0]));

	action_handler_t input_handler = MakeSharedPtr<input_signal>();
	input_handler->connect(KlayGE::bind(&PostProcessingApp::InputHandler, this, KlayGE::placeholders::_1, KlayGE::placeholders::_2));
	inputEngine.ActionMap(actionMap, input_handler, true);

	copy_ = LoadPostProcess(ResLoader::Instance().Open("Copy.ppml"), "copy");
	ascii_arts_ = MakeSharedPtr<AsciiArtsPostProcess>();
	cartoon_ = MakeSharedPtr<CartoonPostProcess>();
	tiling_ = MakeSharedPtr<TilingPostProcess>();
	hdr_ = MakeSharedPtr<HDRPostProcess>(false);
	night_vision_ = MakeSharedPtr<NightVisionPostProcess>();
	old_fashion_ = LoadPostProcess(ResLoader::Instance().Open("OldFashion.ppml"), "old_fashion");

	UIManager::Instance().Load(ResLoader::Instance().Open("PostProcessing.uiml"));
	dialog_ = UIManager::Instance().GetDialogs()[0];

	id_fps_camera_ = dialog_->IDFromName("FPSCamera");
	id_copy_ = dialog_->IDFromName("CopyPP");
	id_ascii_arts_ = dialog_->IDFromName("AsciiArtsPP");
	id_cartoon_ = dialog_->IDFromName("CartoonPP");
	id_tiling_ = dialog_->IDFromName("TilingPP");
	id_hdr_ = dialog_->IDFromName("HDRPP");
	id_night_vision_ = dialog_->IDFromName("NightVisionPP");
	id_old_fashion_ = dialog_->IDFromName("OldFashionPP");

	dialog_->Control<UICheckBox>(id_fps_camera_)->OnChangedEvent().connect(KlayGE::bind(&PostProcessingApp::FPSCameraHandler, this, KlayGE::placeholders::_1));
	dialog_->Control<UIRadioButton>(id_copy_)->OnChangedEvent().connect(KlayGE::bind(&PostProcessingApp::CopyHandler, this, KlayGE::placeholders::_1));
	dialog_->Control<UIRadioButton>(id_ascii_arts_)->OnChangedEvent().connect(KlayGE::bind(&PostProcessingApp::AsciiArtsHandler, this, KlayGE::placeholders::_1));
	dialog_->Control<UIRadioButton>(id_cartoon_)->OnChangedEvent().connect(KlayGE::bind(&PostProcessingApp::CartoonHandler, this, KlayGE::placeholders::_1));
	dialog_->Control<UIRadioButton>(id_tiling_)->OnChangedEvent().connect(KlayGE::bind(&PostProcessingApp::TilingHandler, this, KlayGE::placeholders::_1));
	dialog_->Control<UIRadioButton>(id_hdr_)->OnChangedEvent().connect(KlayGE::bind(&PostProcessingApp::HDRHandler, this, KlayGE::placeholders::_1));
	dialog_->Control<UIRadioButton>(id_night_vision_)->OnChangedEvent().connect(KlayGE::bind(&PostProcessingApp::NightVisionHandler, this, KlayGE::placeholders::_1));
	dialog_->Control<UIRadioButton>(id_old_fashion_)->OnChangedEvent().connect(KlayGE::bind(&PostProcessingApp::OldFashionHandler, this, KlayGE::placeholders::_1));
	this->CartoonHandler(*dialog_->Control<UIRadioButton>(id_cartoon_));
	
	sky_box_ = MakeSharedPtr<SceneObjectSkyBox>();
	sky_box_->AddToSceneManager();

	color_fb_ = rf.MakeFrameBuffer();
	color_fb_->GetViewport()->camera = re.CurFrameBuffer()->GetViewport()->camera;
}

void PostProcessingApp::OnResize(uint32_t width, uint32_t height)
{
	App3DFramework::OnResize(width, height);

	RenderFactory& rf = Context::Instance().RenderFactoryInstance();
	ElementFormat fmt;
	if (rf.RenderEngineInstance().DeviceCaps().texture_format_support(EF_B10G11R11F))
	{
		fmt = EF_B10G11R11F;
	}
	else if (rf.RenderEngineInstance().DeviceCaps().texture_format_support(EF_ABGR8))
	{
		fmt = EF_ABGR8;
	}
	else
	{
		BOOST_ASSERT(rf.RenderEngineInstance().DeviceCaps().texture_format_support(EF_ARGB8));

		fmt = EF_ARGB8;
	}
	color_tex_ = rf.MakeTexture2D(width, height, 4, 1, fmt, 1, 0, EAH_GPU_Read | EAH_GPU_Write | EAH_Generate_Mips, nullptr);
	color_fb_->Attach(FrameBuffer::ATT_Color0, rf.Make2DRenderView(*color_tex_, 0, 1, 0));

	deferred_rendering_->SetupViewport(0, color_fb_, 0);

	copy_->InputPin(0, color_tex_);

	ascii_arts_->InputPin(0, color_tex_);

	cartoon_->InputPin(0, deferred_rendering_->OpaqueGBufferRT0Tex(0));
	cartoon_->InputPin(1, deferred_rendering_->OpaqueDepthTex(0));
	cartoon_->InputPin(2, color_tex_);

	tiling_->InputPin(0, color_tex_);

	hdr_->InputPin(0, color_tex_);

	night_vision_->InputPin(0, color_tex_);

	old_fashion_->InputPin(0, color_tex_);

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

void PostProcessingApp::CopyHandler(UIRadioButton const & sender)
{
	if (sender.GetChecked())
	{
		active_pp_ = copy_;
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

void PostProcessingApp::NightVisionHandler(UIRadioButton const & sender)
{
	if (sender.GetChecked())
	{
		active_pp_ = night_vision_;
	}
}

void PostProcessingApp::OldFashionHandler(UIRadioButton const & sender)
{
	if (sender.GetChecked())
	{
		active_pp_ = old_fashion_;
	}
}

void PostProcessingApp::DoUpdateOverlay()
{
	RenderEngine& renderEngine(Context::Instance().RenderFactoryInstance().RenderEngineInstance());

	UIManager::Instance().Render();

	font_->RenderText(0, 0, Color(1, 1, 0, 1), L"Post Processing", 16);
	font_->RenderText(0, 18, Color(1, 1, 0, 1), renderEngine.ScreenFrameBuffer()->Description(), 16);

	std::wostringstream stream;
	stream.precision(2);
	stream << std::fixed << this->FPS() << " FPS";
	font_->RenderText(0, 36, Color(1, 1, 0, 1), stream.str(), 16);
}

uint32_t PostProcessingApp::DoUpdate(uint32_t pass)
{
	if (0 == pass)
	{
		if (loading_percentage_ < 100)
		{
			if (loading_percentage_ < 10)
			{
				TexturePtr y_cube_tex = y_cube_tl_();
				TexturePtr c_cube_tex = c_cube_tl_();
				checked_pointer_cast<SceneObjectSkyBox>(sky_box_)->CompressedCubeMap(y_cube_tex, c_cube_tex);
				if (!!y_cube_tex && !!c_cube_tex)
				{
					loading_percentage_ = 10;
				}
			}
			else
			{
				RenderModelPtr scene_model = model_ml_();
				if (scene_model)
				{
					scene_obj_ = MakeSharedPtr<SceneObjectHelper>(scene_model->Mesh(0), SceneObject::SOA_Cullable | SceneObject::SOA_Moveable);
					scene_obj_->BindUpdateFunc(ObjectUpdate());
					scene_obj_->AddToSceneManager();
					loading_percentage_ = 100;
				}
			}
		}
	}

	RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
		
	uint32_t ret = deferred_rendering_->Update(pass);
	if (ret & App3DFramework::URV_Finished)
	{
		color_tex_->BuildMipSubLevels();
		re.BindFrameBuffer(FrameBufferPtr());
		re.CurFrameBuffer()->Attached(FrameBuffer::ATT_DepthStencil)->ClearDepth(1.0f);
		active_pp_->Apply();

		return App3DFramework::URV_Skip_Postprocess | App3DFramework::URV_Finished;
	}
	else
	{
		return ret;
	}
}
