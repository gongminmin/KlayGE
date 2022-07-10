#include <KlayGE/KlayGE.hpp>
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
#include <KlayGE/SceneNode.hpp>
#include <KlayGE/UI.hpp>
#include <KlayGE/Camera.hpp>

#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/InputFactory.hpp>

#include <iterator>
#include <sstream>
#include <vector>

#include "SampleCommon.hpp"
#include "VertexDisplacement.hpp"

using namespace std;
using namespace KlayGE;

namespace
{
	int const LENGTH = 4;
	int const WIDTH = 3;

	class FlagRenderable : public RenderablePlane
	{
	public:
		FlagRenderable(int length_segs, int width_segs)
			: RenderablePlane(static_cast<float>(LENGTH), static_cast<float>(WIDTH), length_segs, width_segs, true, false)
		{
			effect_ = SyncLoadRenderEffect("VertexDisplacement.fxml");
			technique_ = effect_->TechniqueByName("VertexDisplacement");

			*(effect_->ParameterByName("flag_tex")) = ASyncLoadTexture("powered_by_klayge.dds", EAH_GPU_Read | EAH_Immutable);
			*(effect_->ParameterByName("half_length")) = LENGTH / 2.0f;
			*(effect_->ParameterByName("half_width")) = WIDTH / 2.0f;
			*(effect_->ParameterByName("lightDir")) = float3(1, 0, -1);
		}

		void SetAngle(float angle)
		{
			*(effect_->ParameterByName("currentAngle")) = angle;
		}

		void OnRenderBegin()
		{
			RenderablePlane::OnRenderBegin();

			App3DFramework const & app = Context::Instance().AppInstance();
			Camera const & camera = app.ActiveCamera();

			*(effect_->ParameterByName("modelview")) = camera.ViewMatrix();
			*(effect_->ParameterByName("proj")) = camera.ProjMatrix();
			*(effect_->ParameterByName("mvp")) = camera.ViewProjMatrix();
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
	VertexDisplacement app;
	app.Create();
	app.Run();

	return 0;
}

VertexDisplacement::VertexDisplacement()
						: App3DFramework("VertexDisplacement")
{
	ResLoader::Instance().AddPath("../../Tutorials/media/VertexDisplacement");
}

void VertexDisplacement::OnCreate()
{
	font_ = SyncLoadFont("gkai00mp.kfont");

	flag_ = MakeSharedPtr<SceneNode>(MakeSharedPtr<RenderableComponent>(MakeSharedPtr<FlagRenderable>(8, 6)), SceneNode::SOA_Cullable);
	flag_->OnMainThreadUpdate().Connect([](SceneNode& node, float app_time, float elapsed_time)
		{
			KFL_UNUSED(elapsed_time);

			node.FirstComponentOfType<RenderableComponent>()->BoundRenderableOfType<FlagRenderable>().SetAngle(app_time / 0.4f);
		});
	Context::Instance().SceneManagerInstance().SceneRootNode().AddChild(flag_);

	this->LookAt(float3(0, 0, -10), float3(0, 0, 0));
	this->Proj(0.1f, 20.0f);

	fpcController_.AttachCamera(this->ActiveCamera());
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

	UIManager::Instance().Load(*ResLoader::Instance().Open("VertexDisplacement.uiml"));
}

void VertexDisplacement::OnResize(uint32_t width, uint32_t height)
{
	App3DFramework::OnResize(width, height);

	UIManager::Instance().SettleCtrls();
}

void VertexDisplacement::InputHandler(InputEngine const & /*sender*/, InputAction const & action)
{
	switch (action.first)
	{
	case Exit:
		this->Quit();
		break;
	}
}

void VertexDisplacement::DoUpdateOverlay()
{
	UIManager::Instance().Render();

	RenderEngine& renderEngine(Context::Instance().RenderFactoryInstance().RenderEngineInstance());

	std::wostringstream stream;
	stream.precision(2);
	stream << std::fixed << this->FPS() << " FPS";

	font_->RenderText(0, 0, Color(1, 1, 0, 1), L"Vertex displacement", 16);
	font_->RenderText(0, 18, Color(1, 1, 0, 1), stream.str(), 16);

	SceneManager& sceneMgr(Context::Instance().SceneManagerInstance());
	stream.str(L"");
	stream << sceneMgr.NumRenderablesRendered() << " Renderables "
		<< sceneMgr.NumPrimitivesRendered() << " Primitives "
		<< sceneMgr.NumVerticesRendered() << " Vertices";
	font_->RenderText(0, 36, Color(1, 1, 1, 1), stream.str(), 16);
	font_->RenderText(0, 54, Color(1, 1, 0, 1), renderEngine.Name(), 16);
}

uint32_t VertexDisplacement::DoUpdate(uint32_t /*pass*/)
{
	RenderEngine& renderEngine(Context::Instance().RenderFactoryInstance().RenderEngineInstance());

	Color clear_clr(0.2f, 0.4f, 0.6f, 1);
	if (Context::Instance().Config().graphics_cfg.gamma)
	{
		clear_clr.r() = 0.029f;
		clear_clr.g() = 0.133f;
		clear_clr.b() = 0.325f;
	}
	renderEngine.CurFrameBuffer()->Clear(FrameBuffer::CBM_Color | FrameBuffer::CBM_Depth, clear_clr, 1.0f, 0);

	return App3DFramework::URV_NeedFlush | App3DFramework::URV_Finished;
}
