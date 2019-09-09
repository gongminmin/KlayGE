#include <KlayGE/KlayGE.hpp>
#include <KFL/Util.hpp>
#include <KFL/Math.hpp>
#include <KlayGE/Font.hpp>
#include <KlayGE/Renderable.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/RenderEffect.hpp>
#include <KlayGE/FrameBuffer.hpp>
#include <KlayGE/SceneManager.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/ResLoader.hpp>
#include <KlayGE/RenderSettings.hpp>
#include <KlayGE/Mesh.hpp>
#include <KlayGE/Light.hpp>
#include <KlayGE/SceneNode.hpp>
#include <KlayGE/Show.hpp>
#include <KlayGE/UI.hpp>
#include <KlayGE/Camera.hpp>

#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/InputFactory.hpp>
#include <KlayGE/ShowFactory.hpp>

#include <iterator>
#include <sstream>
#include <vector>

#include "SampleCommon.hpp"
#include "VideoTexture.hpp"

using namespace std;
using namespace KlayGE;

namespace
{
	class RenderTeapot : public StaticMesh
	{
	public:
		explicit RenderTeapot(std::wstring_view name)
			: StaticMesh(name)
		{
			effect_ = SyncLoadRenderEffect("VideoTexture.fxml");
			technique_ = effect_->TechniqueByName("Object");
		}

		void OnRenderBegin()
		{
			StaticMesh::OnRenderBegin();

			App3DFramework const & app = Context::Instance().AppInstance();
			Camera const & camera = app.ActiveCamera();

			*(effect_->ParameterByName("mvp")) = camera.ViewProjMatrix();
			*(effect_->ParameterByName("mv")) = camera.ViewMatrix();
			*(effect_->ParameterByName("eye_pos")) = camera.EyePos();
		}

		void VideoTexture(TexturePtr const & video_tex)
		{
			*(effect_->ParameterByName("video_tex")) = video_tex;
		}

		void LightPos(float3 const & light_pos)
		{
			*(effect_->ParameterByName("light_pos")) = light_pos;
		}

		void LightColor(float3 const & light_color)
		{
			*(effect_->ParameterByName("light_color")) = light_color;
		}

		void LightFalloff(float3 const & light_falloff)
		{
			*(effect_->ParameterByName("light_falloff")) = light_falloff;
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

void VideoTextureApp::OnCreate()
{
	font_ = SyncLoadFont("gkai00mp.kfont");

	this->LookAt(float3(-0.18f, 0.24f, -0.18f), float3(0, 0.05f, 0));
	this->Proj(0.01f, 100);

	tb_controller_.AttachCamera(this->ActiveCamera());
	tb_controller_.Scalers(0.003f, 0.0001f);

	auto& root_node = Context::Instance().SceneManagerInstance().SceneRootNode();

	light_ = MakeSharedPtr<PointLightSource>();
	light_->Attrib(0);
	light_->Color(float3(2, 2, 2));
	light_->Falloff(float3(1, 0, 1.0f));

	auto light_proxy = LoadLightSourceProxyModel(light_);
	light_proxy->RootNode()->TransformToParent(MathLib::scaling(0.01f, 0.01f, 0.01f) * light_proxy->RootNode()->TransformToParent());

	auto light_node = MakeSharedPtr<SceneNode>(SceneNode::SOA_Cullable);
	light_node->TransformToParent(MathLib::translation(0.25f, 0.5f, -1.0f));
	light_node->AddComponent(light_);
	light_node->AddChild(light_proxy->RootNode());
	root_node.AddChild(light_node);

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

	model_ = SyncLoadModel("teapot.glb", EAH_GPU_Read | EAH_Immutable,
		SceneNode::SOA_Cullable, nullptr,
		CreateModelFactory<RenderModel>, CreateMeshFactory<RenderTeapot>);
	object_ = MakeSharedPtr<SceneNode>(MakeSharedPtr<RenderableComponent>(model_->Mesh(0)), SceneNode::SOA_Cullable);
	Context::Instance().SceneManagerInstance().SceneRootNode().AddChild(object_);

	ShowEngine& se = Context::Instance().ShowFactoryInstance().ShowEngineInstance();
	se.Load(ResLoader::Instance().Locate("big_buck_bunny.avi"));
	se.Play();

	UIManager::Instance().Load(*ResLoader::Instance().Open("VideoTexture.uiml"));
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
	ShowEngine& se = Context::Instance().ShowFactoryInstance().ShowEngineInstance();

	Color clear_clr(0.2f, 0.4f, 0.6f, 1);
	if (Context::Instance().Config().graphics_cfg.gamma)
	{
		clear_clr.r() = 0.029f;
		clear_clr.g() = 0.133f;
		clear_clr.b() = 0.325f;
	}		
	re.CurFrameBuffer()->Clear(FrameBuffer::CBM_Color | FrameBuffer::CBM_Depth, clear_clr, 1.0f, 0);

	auto& teapot_renderable = object_->FirstComponentOfType<RenderableComponent>()->BoundRenderableOfType<RenderTeapot>();
	teapot_renderable.VideoTexture(se.PresentTexture());
	teapot_renderable.LightPos(light_->Position());
	teapot_renderable.LightColor(light_->Color());
	teapot_renderable.LightFalloff(light_->Falloff());

	return App3DFramework::URV_NeedFlush | App3DFramework::URV_Finished;
}
