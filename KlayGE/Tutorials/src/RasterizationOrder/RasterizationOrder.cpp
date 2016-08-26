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
#include <KlayGE/PostProcess.hpp>

#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/InputFactory.hpp>

#include <vector>
#include <sstream>

#include "SampleCommon.hpp"
#include "RasterizationOrder.hpp"

using namespace std;
using namespace KlayGE;

namespace
{
	class RenderFractal : public RenderableHelper
	{
	public:
		RenderFractal()
			: RenderableHelper(L"RasterizationOrder")
		{
			RenderFactory& rf = Context::Instance().RenderFactoryInstance();

			effect_ = SyncLoadRenderEffect("RasterizationOrder.fxml");
			technique_ = effect_->TechniqueByName("RasterizationOrder");

			float2 pos[] =
			{
				float2(-1, +1),
				float2(+3, +1),
				float2(-1, -3)
			};

			rl_ = rf.MakeRenderLayout();
			rl_->TopologyType(RenderLayout::TT_TriangleList);

			GraphicsBufferPtr pos_vb = rf.MakeVertexBuffer(BU_Static, EAH_GPU_Read | EAH_Immutable, sizeof(pos), pos);
			rl_->BindVertexStream(pos_vb, std::make_tuple(vertex_element(VEU_Position, 0, EF_GR32F)));
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
	ContextCfg cfg = Context::Instance().Config();
	cfg.graphics_cfg.hdr = false;
	cfg.graphics_cfg.gamma = false;
	cfg.graphics_cfg.ppaa = false;
	cfg.graphics_cfg.color_grading = false;
	Context::Instance().Config(cfg);

	RasterizationOrderApp app;
	app.Create();
	app.Run();

	return 0;
}

RasterizationOrderApp::RasterizationOrderApp()
			: App3DFramework("RasterizationOrder")
{
	ResLoader::Instance().AddPath("../../Tutorials/media/RasterizationOrder");
}

bool RasterizationOrderApp::ConfirmDevice() const
{
	return true;
}

void RasterizationOrderApp::OnCreate()
{
	font_ = SyncLoadFont("gkai00mp.kfont");

	render_quad_ = MakeSharedPtr<RenderFractal>();

	RenderFactory& rf = Context::Instance().RenderFactoryInstance();

	ras_order_fb_ = rf.MakeFrameBuffer();

	copy_pp_ = SyncLoadPostProcess("Copy.ppml", "bilinear_copy");

	InputEngine& inputEngine(Context::Instance().InputFactoryInstance().InputEngineInstance());
	InputActionMap actionMap;
	actionMap.AddActions(actions, actions + sizeof(actions) / sizeof(actions[0]));

	action_handler_t input_handler = MakeSharedPtr<input_signal>();
	input_handler->connect(std::bind(&RasterizationOrderApp::InputHandler, this, std::placeholders::_1, std::placeholders::_2));
	inputEngine.ActionMap(actionMap, input_handler);
}

void RasterizationOrderApp::OnResize(uint32_t width, uint32_t height)
{
	RenderFactory& rf = Context::Instance().RenderFactoryInstance();
	ras_order_buff_ = rf.MakeVertexBuffer(BU_Dynamic,
		EAH_GPU_Read | EAH_GPU_Write | EAH_GPU_Unordered | EAH_Raw,
		width * height * sizeof(uint32_t), nullptr, EF_R32UI);
	ras_order_uav_ = rf.MakeGraphicsBufferUnorderedAccessView(*ras_order_buff_, EF_R32UI);
	ras_order_fb_->AttachUAV(0, ras_order_uav_);

	ras_order_tex_ = rf.MakeTexture2D(width, height, 1, 1, EF_ABGR8, 1, 0, EAH_GPU_Read | EAH_GPU_Write, nullptr);
	ras_order_fb_->Attach(FrameBuffer::ATT_Color0, rf.Make2DRenderView(*ras_order_tex_, 0, 1, 0));

	copy_pp_->InputPin(0, ras_order_tex_);

	App3DFramework::OnResize(width, height);
}

void RasterizationOrderApp::InputHandler(InputEngine const & /*sender*/, InputAction const & action)
{
	switch (action.first)
	{
	case Exit:
		this->Quit();
		break;
	}
}

void RasterizationOrderApp::DoUpdateOverlay()
{
}

uint32_t RasterizationOrderApp::DoUpdate(uint32_t pass)
{
	RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();

	switch (pass)
	{
	case 0:
		ras_order_uav_->Clear(uint4(0, 0, 0, 0));
		re.BindFrameBuffer(ras_order_fb_);
		render_quad_->AddToRenderQueue();
		return App3DFramework::URV_NeedFlush;

	case 1:
	default:
		re.BindFrameBuffer(FrameBufferPtr());
		copy_pp_->Apply();
		return App3DFramework::URV_NeedFlush | App3DFramework::URV_Finished;
	}
}
