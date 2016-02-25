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
#include <KlayGE/RenderableHelper.hpp>
#include <KlayGE/Light.hpp>
#include <KlayGE/SceneObjectHelper.hpp>
#include <KlayGE/Show.hpp>
#include <KlayGE/UI.hpp>
#include <KlayGE/Camera.hpp>

#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/InputFactory.hpp>
#include <KlayGE/ShowFactory.hpp>

#include <vector>
#include <sstream>

#include "SampleCommon.hpp"
#include "VideoTexture.hpp"

using namespace std;
using namespace KlayGE;

namespace
{
	class RenderTeapot : public StaticMesh
	{
	public:
		RenderTeapot(RenderModelPtr model, std::wstring const & /*name*/)
			: StaticMesh(model, L"Teapot")
		{
			technique_ = SyncLoadRenderEffect("VideoTexture.fxml")->TechniqueByName("Object");
		}

		virtual void DoBuildMeshInfo() override
		{
			AABBox const & pos_bb = this->PosBound();
			*(technique_->Effect().ParameterByName("pos_center")) = pos_bb.Center();
			*(technique_->Effect().ParameterByName("pos_extent")) = pos_bb.HalfSize();

			AABBox const & tc_bb = this->TexcoordBound();
			*(technique_->Effect().ParameterByName("tc_center")) = float2(tc_bb.Center().x(), tc_bb.Center().y());
			*(technique_->Effect().ParameterByName("tc_extent")) = float2(tc_bb.HalfSize().x(), tc_bb.HalfSize().y());
		}

		void OnRenderBegin()
		{
			App3DFramework const & app = Context::Instance().AppInstance();
			Camera const & camera = app.ActiveCamera();

			*(technique_->Effect().ParameterByName("mvp")) = camera.ViewProjMatrix();
			*(technique_->Effect().ParameterByName("mv")) = camera.ViewMatrix();
			*(technique_->Effect().ParameterByName("eye_pos")) = camera.EyePos();
		}

		void VideoTexture(TexturePtr const & video_tex)
		{
			*(technique_->Effect().ParameterByName("video_tex")) = video_tex;
		}

		void LightPos(float3 const & light_pos)
		{
			*(technique_->Effect().ParameterByName("light_pos")) = light_pos;
		}

		void LightColor(float3 const & light_color)
		{
			*(technique_->Effect().ParameterByName("light_color")) = light_color;
		}

		void LightFalloff(float3 const & light_falloff)
		{
			*(technique_->Effect().ParameterByName("light_falloff")) = light_falloff;
		}
	};

	class TeapotObject : public SceneObjectHelper
	{
	public:
		TeapotObject()
			: SceneObjectHelper(SOA_Cullable)
		{
			renderable_ = SyncLoadModel("teapot.meshml", EAH_GPU_Read | EAH_Immutable, CreateModelFactory<RenderModel>(), CreateMeshFactory<RenderTeapot>())->Subrenderable(0);
		}

		void VideoTexture(TexturePtr const & video_tex)
		{
			checked_pointer_cast<RenderTeapot>(renderable_)->VideoTexture(video_tex);
		}

		void LightPos(float3 const & light_pos)
		{
			checked_pointer_cast<RenderTeapot>(renderable_)->LightPos(light_pos);
		}

		void LightColor(float3 const & light_color)
		{
			checked_pointer_cast<RenderTeapot>(renderable_)->LightColor(light_color);
		}

		void LightFalloff(float3 const & light_falloff)
		{
			checked_pointer_cast<RenderTeapot>(renderable_)->LightFalloff(light_falloff);
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
}


int SampleMain()
{
	VideoTextureApp app;
	app.Create();
	app.Run();

	return 0;
}

VideoTextureApp::VideoTextureApp()
					: App3DFramework("Video Texture")
{
	ResLoader::Instance().AddPath("../../Samples/media/VideoTexture");
}

bool VideoTextureApp::ConfirmDevice() const
{
	return true;
}

void VideoTextureApp::OnCreate()
{
	font_ = SyncLoadFont("gkai00mp.kfont");

	this->LookAt(float3(-0.18f, 0.24f, -0.18f), float3(0, 0.05f, 0));
	this->Proj(0.01f, 100);

	tb_controller_.AttachCamera(this->ActiveCamera());
	tb_controller_.Scalers(0.003f, 0.0001f);

	light_ = MakeSharedPtr<PointLightSource>();
	light_->Attrib(0);
	light_->Color(float3(2, 2, 2));
	light_->Falloff(float3(1, 0, 1.0f));
	light_->Position(float3(0.25f, 0.5f, -1.0f));
	light_->AddToSceneManager();

	light_proxy_ = MakeSharedPtr<SceneObjectLightSourceProxy>(light_);
	checked_pointer_cast<SceneObjectLightSourceProxy>(light_proxy_)->Scaling(0.01f, 0.01f, 0.01f);
	light_proxy_->AddToSceneManager();

	InputEngine& inputEngine(Context::Instance().InputFactoryInstance().InputEngineInstance());
	InputActionMap actionMap;
	actionMap.AddActions(actions, actions + sizeof(actions) / sizeof(actions[0]));

	action_handler_t input_handler = MakeSharedPtr<input_signal>();
	input_handler->connect(std::bind(&VideoTextureApp::InputHandler, this, std::placeholders::_1, std::placeholders::_2));
	inputEngine.ActionMap(actionMap, input_handler);

	ground_ = MakeSharedPtr<TeapotObject>();
	ground_->AddToSceneManager();

#ifdef KLAYGE_PLATFORM_WINDOWS_DESKTOP
	ShowEngine& se = Context::Instance().ShowFactoryInstance().ShowEngineInstance();
	se.Load(ResLoader::Instance().Locate("big_buck_bunny.avi"));
	se.Play();
#else
	RenderFactory& rf = Context::Instance().RenderFactoryInstance();

	ElementFormat fmt;
	uint32_t data = 0xFF000000;
	if (rf.RenderEngineInstance().DeviceCaps().texture_format_support(EF_ABGR8))
	{
		fmt = EF_ABGR8;
	}
	else
	{
		BOOST_ASSERT(rf.RenderEngineInstance().DeviceCaps().texture_format_support(EF_ARGB8));
		fmt = EF_ARGB8;
	}

	ElementInitData init_data;
	init_data.data = &data;
	init_data.slice_pitch = init_data.row_pitch = sizeof(data);

	TexturePtr dummy_tex = rf.MakeTexture2D(1, 1, 1, 1, fmt, 1, 0, EAH_GPU_Read | EAH_Immutable, &init_data);
	checked_pointer_cast<TeapotObject>(ground_)->VideoTexture(dummy_tex);
#endif

	UIManager::Instance().Load(ResLoader::Instance().Open("VideoTexture.uiml"));
}

void VideoTextureApp::OnResize(uint32_t width, uint32_t height)
{
	App3DFramework::OnResize(width, height);

	UIManager::Instance().SettleCtrls();
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

void VideoTextureApp::DoUpdateOverlay()
{
	UIManager::Instance().Render();

	std::wostringstream stream;
	stream.precision(2);
	stream << std::fixed << this->FPS() << " FPS";

	font_->RenderText(0, 0, Color(1, 1, 0, 1), L"Video Texture", 16);
	font_->RenderText(0, 18, Color(1, 1, 0, 1), stream.str(), 16);
}

uint32_t VideoTextureApp::DoUpdate(uint32_t /*pass*/)
{
	RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
#ifdef KLAYGE_PLATFORM_WINDOWS_DESKTOP
	ShowEngine& se = Context::Instance().ShowFactoryInstance().ShowEngineInstance();
#endif

	Color clear_clr(0.2f, 0.4f, 0.6f, 1);
	if (Context::Instance().Config().graphics_cfg.gamma)
	{
		clear_clr.r() = 0.029f;
		clear_clr.g() = 0.133f;
		clear_clr.b() = 0.325f;
	}		
	re.CurFrameBuffer()->Clear(FrameBuffer::CBM_Color | FrameBuffer::CBM_Depth, clear_clr, 1.0f, 0);

#ifdef KLAYGE_PLATFORM_WINDOWS_DESKTOP
	checked_pointer_cast<TeapotObject>(ground_)->VideoTexture(se.PresentTexture());
#endif
	checked_pointer_cast<TeapotObject>(ground_)->LightPos(light_->Position());
	checked_pointer_cast<TeapotObject>(ground_)->LightColor(light_->Color());
	checked_pointer_cast<TeapotObject>(ground_)->LightFalloff(light_->Falloff());

	return App3DFramework::URV_NeedFlush | App3DFramework::URV_Finished;
}
