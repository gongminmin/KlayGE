#include <KlayGE/KlayGE.hpp>
#include <KFL/ThrowErr.hpp>
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
#include <boost/bind.hpp>

#include "Fractal.hpp"

using namespace std;
using namespace KlayGE;

#ifdef KLAYGE_COMPILER_MSVC
extern "C"
{
	_declspec(dllexport) uint32_t NvOptimusEnablement = 0x00000001;
}
#endif

namespace
{
	class RenderFractal : public RenderableHelper
	{
	public:
		RenderFractal()
			: RenderableHelper(L"Fractal")
		{
			RenderFactory& rf = Context::Instance().RenderFactoryInstance();

			technique_ = rf.LoadEffect("Fractal.fxml")->TechniqueByName("Mandelbrot");

			float2 pos[] =
			{
				float2(-1, +1),
				float2(+1, +1),
				float2(-1, -1),
				float2(+1, -1),
			};

			rl_ = rf.MakeRenderLayout();
			rl_->TopologyType(RenderLayout::TT_TriangleStrip);

			ElementInitData init_data;
			init_data.row_pitch = sizeof(pos);
			init_data.slice_pitch = 0;
			init_data.data = pos;
			GraphicsBufferPtr pos_vb = rf.MakeVertexBuffer(BU_Static, EAH_GPU_Read | EAH_Immutable, &init_data);

			rl_->BindVertexStream(pos_vb, make_tuple(vertex_element(VEU_Position, 0, EF_GR32F)));

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

			*(technique_->Effect().ParameterByName("clr0")) = clr0;
			*(technique_->Effect().ParameterByName("clr1")) = clr1;
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

int main()
{
	ResLoader::Instance().AddPath("../../Samples/media/Common");

	Context::Instance().LoadCfg("KlayGE.cfg");

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

bool Fractal::ConfirmDevice() const
{
	RenderFactory& rf = Context::Instance().RenderFactoryInstance();
	RenderEngine& re = rf.RenderEngineInstance();
	RenderDeviceCaps const & caps = re.DeviceCaps();
	if (caps.max_shader_model < 2)
	{
		return false;
	}

	return true;
}

void Fractal::InitObjects()
{
	font_ = Context::Instance().RenderFactoryInstance().MakeFont("gkai00mp.kfont");

	renderFractal_ = MakeSharedPtr<RenderFractal>();

	InputEngine& inputEngine(Context::Instance().InputFactoryInstance().InputEngineInstance());
	InputActionMap actionMap;
	actionMap.AddActions(actions, actions + sizeof(actions) / sizeof(actions[0]));

	action_handler_t input_handler = MakeSharedPtr<input_signal>();
	input_handler->connect(boost::bind(&Fractal::InputHandler, this, _1, _2));
	inputEngine.ActionMap(actionMap, input_handler, true);

	UIManager::Instance().Load(ResLoader::Instance().Open("Fractal.uiml"));
}

void Fractal::OnResize(uint32_t width, uint32_t height)
{
	App3DFramework::OnResize(width, height);
	UIManager::Instance().SettleCtrls(width, height);
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
	std::wostringstream stream;
	stream.precision(2);
	stream << std::fixed << this->FPS() << " FPS";

	font_->RenderText(0, 0, Color(1, 1, 0, 1), L"GPU Fractal", 16);
	font_->RenderText(0, 18, Color(1, 1, 0, 1), stream.str(), 16);

	UIManager::Instance().Render();
}

uint32_t Fractal::DoUpdate(uint32_t /*pass*/)
{
	RenderEngine& renderEngine(Context::Instance().RenderFactoryInstance().RenderEngineInstance());

	renderEngine.BindFrameBuffer(FrameBufferPtr());
	renderEngine.CurFrameBuffer()->Clear(FrameBuffer::CBM_Color | FrameBuffer::CBM_Depth, Color(0, 0, 0, 1), 1.0f, 0);
	renderFractal_->AddToRenderQueue();

	return App3DFramework::URV_Need_Flush | App3DFramework::URV_Finished;
}
