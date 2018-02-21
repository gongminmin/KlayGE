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
	class RenderQuad : public RenderableHelper
	{
	public:
		RenderQuad()
			: RenderableHelper(L"RasterizationOrder")
		{
			RenderFactory& rf = Context::Instance().RenderFactoryInstance();

			effect_ = SyncLoadRenderEffect("RasterizationOrder.fxml");
			ras_order_tech_[0] = effect_->TechniqueByName("RasterizationOrder");
			ras_order_tech_[1] = effect_->TechniqueByName("RasterizationOrderColorMap");
			technique_ = ras_order_tech_[0];

			float2 pos[] =
			{
				float2(-1, +1),
				float2(+3, +1),
				float2(-1, -3)
			};

			rl_ = rf.MakeRenderLayout();
			rl_->TopologyType(RenderLayout::TT_TriangleList);

			GraphicsBufferPtr pos_vb = rf.MakeVertexBuffer(BU_Static, EAH_GPU_Read | EAH_Immutable, sizeof(pos), pos);
			rl_->BindVertexStream(pos_vb, VertexElement(VEU_Position, 0, EF_GR32F));

			// From https://github.com/BIDS/colormap/blob/master/parula.py
			uint32_t const color_map[] =
			{
				Color(0.2081f, 0.1663f, 0.5292f, 1).ABGR(),
				Color(0.2116238095f, 0.1897809524f, 0.5776761905f, 1).ABGR(),
				Color(0.212252381f, 0.2137714286f, 0.6269714286f, 1).ABGR(),
				Color(0.2081f, 0.2386f, 0.6770857143f, 1).ABGR(),
				Color(0.1959047619f, 0.2644571429f, 0.7279f, 1).ABGR(),
				Color(0.1707285714f, 0.2919380952f, 0.779247619f, 1).ABGR(),
				Color(0.1252714286f, 0.3242428571f, 0.8302714286f, 1).ABGR(),
				Color(0.0591333333f, 0.3598333333f, 0.8683333333f, 1).ABGR(),
				Color(0.0116952381f, 0.3875095238f, 0.8819571429f, 1).ABGR(),
				Color(0.0059571429f, 0.4086142857f, 0.8828428571f, 1).ABGR(),
				Color(0.0165142857f, 0.4266f, 0.8786333333f, 1).ABGR(),
				Color(0.032852381f, 0.4430428571f, 0.8719571429f, 1).ABGR(),
				Color(0.0498142857f, 0.4585714286f, 0.8640571429f, 1).ABGR(),
				Color(0.0629333333f, 0.4736904762f, 0.8554380952f, 1).ABGR(),
				Color(0.0722666667f, 0.4886666667f, 0.8467f, 1).ABGR(),
				Color(0.0779428571f, 0.5039857143f, 0.8383714286f, 1).ABGR(),
				Color(0.079347619f, 0.5200238095f, 0.8311809524f, 1).ABGR(),
				Color(0.0749428571f, 0.5375428571f, 0.8262714286f, 1).ABGR(),
				Color(0.0640571429f, 0.5569857143f, 0.8239571429f, 1).ABGR(),
				Color(0.0487714286f, 0.5772238095f, 0.8228285714f, 1).ABGR(),
				Color(0.0343428571f, 0.5965809524f, 0.819852381f, 1).ABGR(),
				Color(0.0265f, 0.6137f, 0.8135f, 1).ABGR(),
				Color(0.0238904762f, 0.6286619048f, 0.8037619048f, 1).ABGR(),
				Color(0.0230904762f, 0.6417857143f, 0.7912666667f, 1).ABGR(),
				Color(0.0227714286f, 0.6534857143f, 0.7767571429f, 1).ABGR(),
				Color(0.0266619048f, 0.6641952381f, 0.7607190476f, 1).ABGR(),
				Color(0.0383714286f, 0.6742714286f, 0.743552381f, 1).ABGR(),
				Color(0.0589714286f, 0.6837571429f, 0.7253857143f, 1).ABGR(),
				Color(0.0843f, 0.6928333333f, 0.7061666667f, 1).ABGR(),
				Color(0.1132952381f, 0.7015f, 0.6858571429f, 1).ABGR(),
				Color(0.1452714286f, 0.7097571429f, 0.6646285714f, 1).ABGR(),
				Color(0.1801333333f, 0.7176571429f, 0.6424333333f, 1).ABGR(),
				Color(0.2178285714f, 0.7250428571f, 0.6192619048f, 1).ABGR(),
				Color(0.2586428571f, 0.7317142857f, 0.5954285714f, 1).ABGR(),
				Color(0.3021714286f, 0.7376047619f, 0.5711857143f, 1).ABGR(),
				Color(0.3481666667f, 0.7424333333f, 0.5472666667f, 1).ABGR(),
				Color(0.3952571429f, 0.7459f, 0.5244428571f, 1).ABGR(),
				Color(0.4420095238f, 0.7480809524f, 0.5033142857f, 1).ABGR(),
				Color(0.4871238095f, 0.7490619048f, 0.4839761905f, 1).ABGR(),
				Color(0.5300285714f, 0.7491142857f, 0.4661142857f, 1).ABGR(),
				Color(0.5708571429f, 0.7485190476f, 0.4493904762f, 1).ABGR(),
				Color(0.609852381f, 0.7473142857f, 0.4336857143f, 1).ABGR(),
				Color(0.6473f, 0.7456f, 0.4188f, 1).ABGR(),
				Color(0.6834190476f, 0.7434761905f, 0.4044333333f, 1).ABGR(),
				Color(0.7184095238f, 0.7411333333f, 0.3904761905f, 1).ABGR(),
				Color(0.7524857143f, 0.7384f, 0.3768142857f, 1).ABGR(),
				Color(0.7858428571f, 0.7355666667f, 0.3632714286f, 1).ABGR(),
				Color(0.8185047619f, 0.7327333333f, 0.3497904762f, 1).ABGR(),
				Color(0.8506571429f, 0.7299f, 0.3360285714f, 1).ABGR(),
				Color(0.8824333333f, 0.7274333333f, 0.3217f, 1).ABGR(),
				Color(0.9139333333f, 0.7257857143f, 0.3062761905f, 1).ABGR(),
				Color(0.9449571429f, 0.7261142857f, 0.2886428571f, 1).ABGR(),
				Color(0.9738952381f, 0.7313952381f, 0.266647619f, 1).ABGR(),
				Color(0.9937714286f, 0.7454571429f, 0.240347619f, 1).ABGR(),
				Color(0.9990428571f, 0.7653142857f, 0.2164142857f, 1).ABGR(),
				Color(0.9955333333f, 0.7860571429f, 0.196652381f, 1).ABGR(),
				Color(0.988f, 0.8066f, 0.1793666667f, 1).ABGR(),
				Color(0.9788571429f, 0.8271428571f, 0.1633142857f, 1).ABGR(),
				Color(0.9697f, 0.8481380952f, 0.147452381f, 1).ABGR(),
				Color(0.9625857143f, 0.8705142857f, 0.1309f, 1).ABGR(),
				Color(0.9588714286f, 0.8949f, 0.1132428571f, 1).ABGR(),
				Color(0.9598238095f, 0.9218333333f, 0.0948380952f, 1).ABGR(),
				Color(0.9661f, 0.9514428571f, 0.0755333333f, 1).ABGR(),
				Color(0.9763f, 0.9831f, 0.0538f, 1).ABGR()
			};

			ElementInitData init_data;
			init_data.data = color_map;
			init_data.row_pitch = sizeof(color_map);
			init_data.slice_pitch = init_data.row_pitch * 1;
			TexturePtr color_map_tex = rf.MakeTexture2D(static_cast<uint32_t>(std::size(color_map)), 1, 1, 1, EF_ABGR8,
				1, 0, EAH_GPU_Read | EAH_Immutable, init_data);
			*(effect_->ParameterByName("color_map")) = color_map_tex;
		}

		void ColorMapOn(bool on)
		{
			technique_ = ras_order_tech_[on];
		}

		void BindRasOrderBuffer(GraphicsBufferPtr const & ras_order_buff)
		{
			*(effect_->ParameterByName("ras_order_buff")) = ras_order_buff;
		}

	private:
		RenderTechnique* ras_order_tech_[2];
	};

	enum
	{
		Capture,
		Exit
	};

	InputActionDefine actions[] =
	{
		InputActionDefine(Capture, KS_Space),
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
	RenderDeviceCaps const & caps = Context::Instance().RenderFactoryInstance().RenderEngineInstance().DeviceCaps();
	if (caps.max_shader_model < ShaderModel(5, 0))
	{
		return false;
	}
	return true;
}

void RasterizationOrderApp::OnCreate()
{
	font_ = SyncLoadFont("gkai00mp.kfont");

	render_quad_ = MakeSharedPtr<RenderQuad>();

	RenderFactory& rf = Context::Instance().RenderFactoryInstance();

	ras_order_fb_ = rf.MakeFrameBuffer();

	copy_pp_ = SyncLoadPostProcess("Copy.ppml", "bilinear_copy");

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

	UIManager::Instance().Load(ResLoader::Instance().Open("RasterizationOrder.uiml"));
	dialog_params_ = UIManager::Instance().GetDialog("Parameters");
	id_color_map_ = dialog_params_->IDFromName("ColorMap");
	id_capture_ = dialog_params_->IDFromName("Capture");

	dialog_params_->Control<UICheckBox>(id_color_map_)->OnChangedEvent().connect(
		[this](UICheckBox const & sender)
		{
			this->ColorMapHandler(sender);
		});
	this->ColorMapHandler(*dialog_params_->Control<UICheckBox>(id_color_map_));
	dialog_params_->Control<UIButton>(id_capture_)->OnClickedEvent().connect(
		[this](UIButton const & sender)
		{
			this->CaptureHandler(sender);
		});
}

void RasterizationOrderApp::OnResize(uint32_t width, uint32_t height)
{
	RenderFactory& rf = Context::Instance().RenderFactoryInstance();
	ras_order_buff_ = rf.MakeVertexBuffer(BU_Dynamic,
		EAH_GPU_Read | EAH_GPU_Write | EAH_GPU_Unordered | EAH_Raw,
		width * height * sizeof(uint32_t), nullptr, EF_R32UI);
	ras_order_uav_ = rf.MakeGraphicsBufferUnorderedAccessView(*ras_order_buff_, EF_R32UI);
	ras_order_fb_->AttachUAV(0, ras_order_uav_);

	checked_pointer_cast<RenderQuad>(render_quad_)->BindRasOrderBuffer(ras_order_buff_);

	ras_order_tex_ = rf.MakeTexture2D(width, height, 1, 1, EF_ABGR8, 1, 0, EAH_GPU_Read | EAH_GPU_Write);
	ras_order_fb_->Attach(FrameBuffer::ATT_Color0, rf.Make2DRenderView(*ras_order_tex_, 0, 1, 0));

	copy_pp_->InputPin(0, ras_order_tex_);

	App3DFramework::OnResize(width, height);
	UIManager::Instance().SettleCtrls();
}

void RasterizationOrderApp::InputHandler(InputEngine const & /*sender*/, InputAction const & action)
{
	switch (action.first)
	{
	case Capture:
		CaptureFrame();
		break;

	case Exit:
		this->Quit();
		break;
	}
}

void RasterizationOrderApp::ColorMapHandler(UICheckBox const & sender)
{
	checked_pointer_cast<RenderQuad>(render_quad_)->ColorMapOn(sender.GetChecked());
}

void RasterizationOrderApp::CaptureHandler(KlayGE::UIButton const & sender)
{
	KFL_UNUSED(sender);

	CaptureFrame();
}

void RasterizationOrderApp::DoUpdateOverlay()
{
	UIManager::Instance().Render();

	std::wostringstream stream;
	stream.precision(2);
	stream << std::fixed << this->FPS() << " FPS";

	font_->RenderText(0, 0, Color(1, 1, 0, 1), L"GPU Rasterization Order", 16);
	font_->RenderText(0, 18, Color(1, 1, 0, 1), stream.str(), 16);
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
		re.CurFrameBuffer()->Clear(FrameBuffer::CBM_Depth, Color(0, 0, 0, 1), 1.0f, 0);
		copy_pp_->Apply();
		return App3DFramework::URV_NeedFlush | App3DFramework::URV_Finished;
	}
}

void RasterizationOrderApp::CaptureFrame()
{
	std::string file_name = "ras_order";
	if (dialog_params_->Control<UICheckBox>(id_color_map_)->GetChecked())
	{
		file_name += "_cm";
	}
	file_name += ".dds";

	SaveTexture(ras_order_tex_, file_name);
}
