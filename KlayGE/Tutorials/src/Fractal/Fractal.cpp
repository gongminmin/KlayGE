#include <KlayGE/KlayGE.hpp>
#include <KFL/CXX17/iterator.hpp>
#include <KFL/Util.hpp>
#include <KlayGE/GraphicsBuffer.hpp>
#include <KFL/Math.hpp>
#include <KlayGE/Font.hpp>
#include <KlayGE/Renderable.hpp>
#include <KlayGE/RenderableHelper.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/RenderEffect.hpp>
#include <KlayGE/FrameBuffer.hpp>
#include <KlayGE/RenderLayout.hpp>
#include <KlayGE/SceneManager.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/ResLoader.hpp>
#include <KlayGE/Texture.hpp>
#include <KlayGE/RenderSettings.hpp>
#include <KlayGE/UI.hpp>

#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/InputFactory.hpp>

#include <vector>
#include <sstream>

#include "SampleCommon.hpp"
#include "Fractal.hpp"

using namespace std;
using namespace KlayGE;

namespace
{
	class RenderFractal : public RenderableHelper
	{
	public:
		RenderFractal()
			: RenderableHelper(L"Fractal")
		{
			RenderFactory& rf = Context::Instance().RenderFactoryInstance();

			effect_ = SyncLoadRenderEffect("Fractal.fxml");
			technique_ = effect_->TechniqueByName("Mandelbrot");

			float2 pos[] =
			{
				float2(-1, +1),
				float2(+1, +1),
				float2(-1, -1),
				float2(+1, -1),
			};

			rl_ = rf.MakeRenderLayout();
			rl_->TopologyType(RenderLayout::TT_TriangleStrip);

			GraphicsBufferPtr pos_vb = rf.MakeVertexBuffer(BU_Static, EAH_GPU_Read | EAH_Immutable, sizeof(pos), pos);

			rl_->BindVertexStream(pos_vb, VertexElement(VEU_Position, 0, EF_GR32F));

			float3 clr0(0, 0.2f, 0.6f);
			float3 clr1(0.2f, 1, 0);
			if (Context::Instance().Config().graphics_cfg.gamma)
			{
				clr0.x() = MathLib::srgb_to_linear(clr0.x());
				clr0.y() = MathLib::srgb_to_linear(clr0.y());
				clr0.z() = MathLib::srgb_to_linear(clr0.z());

				clr1.x() = MathLib::srgb_to_linear(clr1.x());
				clr1.y() = MathLib::srgb_to_linear(clr1.y());
				clr1.z() = MathLib::srgb_to_linear(clr1.z());
			}

			*(effect_->ParameterByName("clr0")) = clr0;
			*(effect_->ParameterByName("clr1")) = clr1;
		}
	};

	enum
	{
		Exit,
		FullScreen,
	};

	InputActionDefine actions[] =
	{
		InputActionDefine(Exit, KS_Escape),
		InputActionDefine(FullScreen, KS_Enter),
	};
}

int SampleMain()
{
	Fractal app;
	app.Create();
	app.Run();

	return 0;
}

Fractal::Fractal()
			: App3DFramework("Fractal")
{
	ResLoader::Instance().AddPath("../../Tutorials/media/Fractal");
}

void Fractal::OnCreate()
{
	font_ = SyncLoadFont("gkai00mp.kfont");

	renderFractal_ = MakeSharedPtr<RenderFractal>();

	InputEngine& inputEngine(Context::Instance().InputFactoryInstance().InputEngineInstance());
	InputActionMap actionMap;
	actionMap.AddActions(actions, actions + std::size(actions));

	action_handler_t input_handler = MakeSharedPtr<input_signal>();
	input_handler->connect(
		[this](InputEngine const & sender, InputAction const & action)
		{
			this->InputHandler(sender, action);
		});
	inputEngine.ActionMap(actionMap, input_handler);

	UIManager::Instance().Load(ResLoader::Instance().Open("Fractal.uiml"));
}

void Fractal::OnResize(uint32_t width, uint32_t height)
{
	App3DFramework::OnResize(width, height);
	UIManager::Instance().SettleCtrls();
}

void Fractal::InputHandler(InputEngine const & /*sender*/, InputAction const & action)
{
	switch (action.first)
	{
	case FullScreen:
		{
			RenderEngine& renderEngine(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
			renderEngine.EndFrame();
			ContextCfg const & cfg = Context::Instance().Config();
			renderEngine.Resize(cfg.graphics_cfg.width, cfg.graphics_cfg.height);
			renderEngine.FullScreen(!renderEngine.FullScreen());
			renderEngine.BeginFrame();
		}
		break;

	case Exit:
		this->Quit();
		break;
	}
}

void Fractal::DoUpdateOverlay()
{
	UIManager::Instance().Render();

	std::wostringstream stream;
	stream.precision(2);
	stream << std::fixed << this->FPS() << " FPS";

	font_->RenderText(0, 0, Color(1, 1, 0, 1), L"GPU Fractal", 16);
	font_->RenderText(0, 18, Color(1, 1, 0, 1), stream.str(), 16);
}

uint32_t Fractal::DoUpdate(uint32_t /*pass*/)
{
	RenderEngine& renderEngine(Context::Instance().RenderFactoryInstance().RenderEngineInstance());

	renderEngine.BindFrameBuffer(FrameBufferPtr());
	renderEngine.CurFrameBuffer()->Clear(FrameBuffer::CBM_Color | FrameBuffer::CBM_Depth, Color(0, 0, 0, 1), 1.0f, 0);
	renderFractal_->AddToRenderQueue();

	return App3DFramework::URV_NeedFlush | App3DFramework::URV_Finished;
}
