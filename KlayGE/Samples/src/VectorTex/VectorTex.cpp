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
#include "VectorTex.hpp"

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
			effect_ = SyncLoadRenderEffect("VectorTex.fxml");
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

		void VectorTexture(TexturePtr const & vector_tex)
		{
			*(effect_->ParameterByName("vector_tex")) = vector_tex;
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
	VectorTexApp app;
	app.Create();
	app.Run();

	return 0;
}

VectorTexApp::VectorTexApp()
	: App3DFramework("Vector Texture")
{
	Context::Instance().ResLoaderInstance().AddPath("../../Samples/media/VectorTex");
}

void VectorTexApp::OnCreate()
{
	font_ = SyncLoadFont("gkai00mp.kfont");

	this->LookAt(float3(-0.18f, 0.24f, -0.18f), float3(0, 0.05f, 0));
	this->Proj(0.01f, 100);

	tb_controller_.AttachCamera(this->ActiveCamera());
	tb_controller_.Scalers(0.003f, 0.0001f);

	auto& context = Context::Instance();
	InputEngine& inputEngine(context.InputFactoryInstance().InputEngineInstance());
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
	context.SceneManagerInstance().SceneRootNode().AddChild(object_);

	checked_cast<RenderTeapot&>(*model_->Mesh(0)).VectorTexture(ASyncLoadTexture("Drawing.dds", EAH_GPU_Read | EAH_Immutable));

	context.UIManagerInstance().Load(*context.ResLoaderInstance().Open("VectorTex.uiml"));
}

void VectorTexApp::OnResize(uint32_t width, uint32_t height)
{
	App3DFramework::OnResize(width, height);

	Context::Instance().UIManagerInstance().SettleCtrls();
}

void VectorTexApp::InputHandler(InputEngine const & /*sender*/, InputAction const & action)
{
	switch (action.first)
	{
	case Exit:
		this->Quit();
		break;
	}
}

void VectorTexApp::DoUpdateOverlay()
{
	Context::Instance().UIManagerInstance().Render();

	std::wostringstream stream;
	stream.precision(2);
	stream << std::fixed << this->FPS() << " FPS";

	font_->RenderText(0, 0, Color(1, 1, 0, 1), L"Video Texture", 16);
	font_->RenderText(0, 18, Color(1, 1, 0, 1), stream.str(), 16);
}

uint32_t VectorTexApp::DoUpdate(uint32_t /*pass*/)
{
	RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();

	Color clear_clr(0.2f, 0.4f, 0.6f, 1);
	if (Context::Instance().Config().graphics_cfg.gamma)
	{
		clear_clr.r() = 0.029f;
		clear_clr.g() = 0.133f;
		clear_clr.b() = 0.325f;
	}
	re.CurFrameBuffer()->Clear(FrameBuffer::CBM_Color | FrameBuffer::CBM_Depth, clear_clr, 1.0f, 0);

	return App3DFramework::URV_NeedFlush | App3DFramework::URV_Finished;
}
