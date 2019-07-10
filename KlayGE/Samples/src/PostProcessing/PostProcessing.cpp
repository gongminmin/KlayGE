#include <KlayGE/KlayGE.hpp>
#include <KFL/CXX17/iterator.hpp>
#include <KFL/Util.hpp>
#include <KFL/Math.hpp>
#include <KlayGE/Font.hpp>
#include <KlayGE/Renderable.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/RenderEffect.hpp>
#include <KlayGE/RenderView.hpp>
#include <KlayGE/FrameBuffer.hpp>
#include <KlayGE/SceneManager.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/ResLoader.hpp>
#include <KlayGE/RenderSettings.hpp>
#include <KlayGE/Mesh.hpp>
#include <KlayGE/SceneNode.hpp>
#include <KlayGE/SkyBox.hpp>
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
	class PointLightNodeUpdate
	{
	public:
		void operator()(SceneNode& node, float app_time, float elapsed_time)
		{
			KFL_UNUSED(app_time);
			KFL_UNUSED(elapsed_time);

			float4x4 const inv_view = Context::Instance().AppInstance().ActiveCamera().InverseViewMatrix();
			node.TransformToParent(MathLib::translation(MathLib::transform_coord(float3(2, 2, -3), inv_view)));
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

void PostProcessingApp::OnCreate()
{
	this->LookAt(float3(0, 0.5f, -2), float3(0, 0, 0));
	this->Proj(0.1f, 150.0f);

	TexturePtr c_cube = ASyncLoadTexture("rnl_cross_filtered_c.dds", EAH_GPU_Read | EAH_Immutable);
	TexturePtr y_cube = ASyncLoadTexture("rnl_cross_filtered_y.dds", EAH_GPU_Read | EAH_Immutable);
	auto scene_model = ASyncLoadModel("dino50.glb", EAH_GPU_Read | EAH_Immutable,
		SceneNode::SOA_Cullable | SceneNode::SOA_Moveable,
		[](RenderModel& model)
		{
			model.RootNode()->OnMainThreadUpdate().Connect([](SceneNode& node, float app_time, float elapsed_time)
				{
					KFL_UNUSED(elapsed_time);
					node.TransformToParent(MathLib::rotation_y(-app_time / 1.5f));
				});

			AddToSceneRootHelper(model);
		});

	RenderFactory& rf = Context::Instance().RenderFactoryInstance();
	RenderEngine& re = rf.RenderEngineInstance();

	font_ = SyncLoadFont("gkai00mp.kfont");

	deferred_rendering_ = Context::Instance().DeferredRenderingLayerInstance();
	deferred_rendering_->SSVOEnabled(0, false);
	re.HDREnabled(false);
	re.PPAAEnabled(0);
	re.ColorGradingEnabled(false);

	auto& root_node = Context::Instance().SceneManagerInstance().SceneRootNode();

	AmbientLightSourcePtr ambient_light = MakeSharedPtr<AmbientLightSource>();
	ambient_light->SkylightTex(y_cube, c_cube);
	ambient_light->Color(float3(0.1f, 0.1f, 0.1f));
	root_node.AddComponent(ambient_light);

	auto point_light = MakeSharedPtr<PointLightSource>();
	point_light->Attrib(LightSource::LSA_NoShadow);
	point_light->Color(float3(18, 18, 18));
	point_light->Falloff(float3(1, 0, 1));

	auto light_node = MakeSharedPtr<SceneNode>(SceneNode::SOA_Cullable | SceneNode::SOA_Moveable);
	light_node->AddComponent(point_light);
	light_node->OnMainThreadUpdate().Connect(PointLightNodeUpdate());
	root_node.AddChild(light_node);

	fpcController_.Scalers(0.05f, 0.1f);

	InputEngine& inputEngine(Context::Instance().InputFactoryInstance().InputEngineInstance());
	InputActionMap actionMap;
	actionMap.AddActions(actions, actions + std::size(actions));

	action_handler_t input_handler = MakeSharedPtr<input_signal>();
	input_handler->Connect(
		[this](InputEngine const & sender, InputAction const & action)
		{
			this->InputHandler(sender, action);
		});
	inputEngine.ActionMap(actionMap, input_handler);

	copy_ = SyncLoadPostProcess("Copy.ppml", "Copy");
	ascii_arts_ = MakeSharedPtr<AsciiArtsPostProcess>();
	cartoon_ = MakeSharedPtr<CartoonPostProcess>();
	tiling_ = MakeSharedPtr<TilingPostProcess>();
	hdr_ = MakeSharedPtr<HDRPostProcess>(false);
	night_vision_ = MakeSharedPtr<NightVisionPostProcess>();
	sepia_ = SyncLoadPostProcess("Sepia.ppml", "sepia");
	cross_stitching_ = SyncLoadPostProcess("CrossStitching.ppml", "cross_stitching");
	frosted_glass_ = SyncLoadPostProcess("FrostedGlass.ppml", "frosted_glass");
	black_hole_ = SyncLoadPostProcess("BlackHole.ppml", "black_hole");

	UIManager::Instance().Load(ResLoader::Instance().Open("PostProcessing.uiml"));
	dialog_ = UIManager::Instance().GetDialogs()[0];

	id_fps_camera_ = dialog_->IDFromName("FPSCamera");
	id_copy_ = dialog_->IDFromName("CopyPP");
	id_ascii_arts_ = dialog_->IDFromName("AsciiArtsPP");
	id_cartoon_ = dialog_->IDFromName("CartoonPP");
	id_tiling_ = dialog_->IDFromName("TilingPP");
	id_hdr_ = dialog_->IDFromName("HDRPP");
	id_night_vision_ = dialog_->IDFromName("NightVisionPP");
	id_old_fashion_ = dialog_->IDFromName("SepiaPP");
	id_cross_stitching_ = dialog_->IDFromName("CrossStitchingPP");
	id_frosted_glass_ = dialog_->IDFromName("FrostedGlassPP");
	id_black_hole_ = dialog_->IDFromName("BlackHolePP");

	dialog_->Control<UICheckBox>(id_fps_camera_)->OnChangedEvent().Connect(
		[this](UICheckBox const & sender)
		{
			this->FPSCameraHandler(sender);
		});
	dialog_->Control<UIRadioButton>(id_copy_)->OnChangedEvent().Connect(
		[this](UIRadioButton const & sender)
		{
			this->CopyHandler(sender);
		});
	dialog_->Control<UIRadioButton>(id_ascii_arts_)->OnChangedEvent().Connect(
		[this](UIRadioButton const & sender)
		{
			this->AsciiArtsHandler(sender);
		});
	dialog_->Control<UIRadioButton>(id_cartoon_)->OnChangedEvent().Connect(
		[this](UIRadioButton const & sender)
		{
			this->CartoonHandler(sender);
		});
	dialog_->Control<UIRadioButton>(id_tiling_)->OnChangedEvent().Connect(
		[this](UIRadioButton const & sender)
		{
			this->TilingHandler(sender);
		});
	dialog_->Control<UIRadioButton>(id_hdr_)->OnChangedEvent().Connect(
		[this](UIRadioButton const & sender)
		{
			this->HDRHandler(sender);
		});
	dialog_->Control<UIRadioButton>(id_night_vision_)->OnChangedEvent().Connect(
		[this](UIRadioButton const & sender)
		{
			this->NightVisionHandler(sender);
		});
	dialog_->Control<UIRadioButton>(id_old_fashion_)->OnChangedEvent().Connect(
		[this](UIRadioButton const & sender)
		{
			this->SepiaHandler(sender);
		});
	dialog_->Control<UIRadioButton>(id_cross_stitching_)->OnChangedEvent().Connect(
		[this](UIRadioButton const & sender)
		{
			this->CrossStitchingHandler(sender);
		});
	dialog_->Control<UIRadioButton>(id_frosted_glass_)->OnChangedEvent().Connect(
		[this](UIRadioButton const & sender)
		{
			this->FrostedGlassHandler(sender);
		});
	dialog_->Control<UIRadioButton>(id_black_hole_)->OnChangedEvent().Connect(
		[this](UIRadioButton const & sender)
		{
			this->BlackHoleHandler(sender);
		});
	this->CartoonHandler(*dialog_->Control<UIRadioButton>(id_cartoon_));
	
	auto skybox = MakeSharedPtr<RenderableSkyBox>();
	skybox->CompressedCubeMap(y_cube, c_cube);
	root_node.AddChild(MakeSharedPtr<SceneNode>(MakeSharedPtr<RenderableComponent>(skybox), SceneNode::SOA_NotCastShadow));

	color_fb_ = rf.MakeFrameBuffer();
	color_fb_->Viewport()->Camera(re.CurFrameBuffer()->Viewport()->Camera());
}

void PostProcessingApp::OnResize(uint32_t width, uint32_t height)
{
	App3DFramework::OnResize(width, height);

	RenderFactory& rf = Context::Instance().RenderFactoryInstance();
	auto const & caps = rf.RenderEngineInstance().DeviceCaps();
	auto const fmt = caps.BestMatchTextureRenderTargetFormat(MakeSpan({EF_B10G11R11F, EF_ABGR8, EF_ARGB8}), 1, 0);
	BOOST_ASSERT(fmt != EF_Unknown);
	color_tex_ = rf.MakeTexture2D(width, height, 4, 1, fmt, 1, 0, EAH_GPU_Read | EAH_GPU_Write | EAH_Generate_Mips);
	auto color_srv = rf.MakeTextureSrv(color_tex_);
	color_fb_->Attach(FrameBuffer::Attachment::Color0, rf.Make2DRtv(color_tex_, 0, 1, 0));
	color_fb_->Attach(rf.Make2DDsv(width, height, EF_D16, 1, 0));

	deferred_rendering_->SetupViewport(0, color_fb_, 0);

	copy_->InputPin(0, color_srv);

	ascii_arts_->InputPin(0, color_srv);

	cartoon_->InputPin(0, deferred_rendering_->GBufferResolvedRT0Srv(0));
	cartoon_->InputPin(1, deferred_rendering_->ResolvedDepthSrv(0));
	cartoon_->InputPin(2, color_srv);

	tiling_->InputPin(0, color_srv);

	hdr_->InputPin(0, color_srv);

	night_vision_->InputPin(0, color_srv);

	sepia_->InputPin(0, color_srv);

	cross_stitching_->InputPin(0, color_srv);

	frosted_glass_->InputPin(0, color_srv);

	black_hole_->InputPin(0, color_srv);

	UIManager::Instance().SettleCtrls();
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

void PostProcessingApp::SepiaHandler(UIRadioButton const & sender)
{
	if (sender.GetChecked())
	{
		active_pp_ = sepia_;
	}
}

void PostProcessingApp::CrossStitchingHandler(UIRadioButton const & sender)
{
	if (sender.GetChecked())
	{
		active_pp_ = cross_stitching_;
	}
}

void PostProcessingApp::FrostedGlassHandler(UIRadioButton const & sender)
{
	if (sender.GetChecked())
	{
		active_pp_ = frosted_glass_;
	}
}

void PostProcessingApp::BlackHoleHandler(UIRadioButton const & sender)
{
	if (sender.GetChecked())
	{
		active_pp_ = black_hole_;
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

	uint32_t const num_loading_res = ResLoader::Instance().NumLoadingResources();
	if (num_loading_res > 0)
	{
		stream.str(L"");
		stream << "Loading " << num_loading_res << " resources...";
		font_->RenderText(100, 300, Color(1, 0, 0, 1), stream.str(), 48);
	}
}

uint32_t PostProcessingApp::DoUpdate(uint32_t pass)
{
	RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
		
	uint32_t ret = deferred_rendering_->Update(pass);
	if (ret & App3DFramework::URV_Finished)
	{
		if (active_pp_ == black_hole_)
		{
			auto const& camera = *re.CurFrameBuffer()->Viewport()->Camera();

			float3 upper_left = MathLib::transform_coord(float3(-1, +1, 1), camera.InverseViewProjMatrix());
			float3 upper_right = MathLib::transform_coord(float3(+1, +1, 1), camera.InverseViewProjMatrix());
			float3 lower_left = MathLib::transform_coord(float3(-1, -1, 1), camera.InverseViewProjMatrix());

			black_hole_->SetParam(0, camera.ViewProjMatrix());
			black_hole_->SetParam(1, camera.EyePos());
			black_hole_->SetParam(2, upper_left);
			black_hole_->SetParam(3, upper_right - upper_left);
			black_hole_->SetParam(4, lower_left - upper_left);
			black_hole_->SetParam(5, this->AppTime());
		}

		color_tex_->BuildMipSubLevels();
		re.BindFrameBuffer(FrameBufferPtr());
		re.CurFrameBuffer()->AttachedRtv(FrameBuffer::Attachment::Color0)->Discard();
		re.CurFrameBuffer()->AttachedDsv()->ClearDepth(1.0f);
		active_pp_->Apply();

		return App3DFramework::URV_SkipPostProcess | App3DFramework::URV_Finished;
	}
	else
	{
		return ret;
	}
}
