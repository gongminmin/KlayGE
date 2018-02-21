#include <KlayGE/KlayGE.hpp>
#include <KFL/CXX17/iterator.hpp>
#include <KFL/Util.hpp>
#include <KFL/Math.hpp>
#include <KlayGE/Font.hpp>
#include <KlayGE/GraphicsBuffer.hpp>
#include <KlayGE/Renderable.hpp>
#include <KlayGE/RenderableHelper.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/RenderEffect.hpp>
#include <KlayGE/FrameBuffer.hpp>
#include <KlayGE/SceneManager.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/ResLoader.hpp>
#include <KlayGE/RenderSettings.hpp>
#include <KlayGE/SceneObjectHelper.hpp>
#include <KlayGE/UI.hpp>
#include <KlayGE/Light.hpp>
#include <KlayGE/Camera.hpp>

#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/InputFactory.hpp>

#include <vector>
#include <sstream>

#include "SampleCommon.hpp"
#include "Tessellation.hpp"

using namespace std;
using namespace KlayGE;

namespace
{
	class RenderTriangle : public RenderableHelper
	{
	public:
		RenderTriangle()
			: RenderableHelper(L"Triangle")
		{
			RenderFactory& rf = Context::Instance().RenderFactoryInstance();

			effect_ = SyncLoadRenderEffect("TessellationApp.fxml");

			technique_ = effect_->TechniqueByName("NoTessellation");
			tess_factors_param_ = effect_->ParameterByName("tess_factors");

			float3 xyzs[] =
			{
				float3(0, +0.8f, 0),
				float3(+0.7f, -0.8f, 0),
				float3(-0.7f, -0.8f, 0)
			};

			rl_ = rf.MakeRenderLayout();

			GraphicsBufferPtr pos_vb = rf.MakeVertexBuffer(BU_Static, EAH_GPU_Read | EAH_Immutable, sizeof(xyzs), xyzs);
			rl_->BindVertexStream(pos_vb, VertexElement(VEU_Position, 0, EF_BGR32F));

			pos_aabb_ = MathLib::compute_aabbox(&xyzs[0], &xyzs[0] + std::size(xyzs));
			tc_aabb_ = AABBox(float3(0, 0, 0), float3(0, 0, 0));
		}

		void TessEnabled(bool enabled)
		{
			if (enabled)
			{
				rl_->TopologyType(RenderLayout::TT_3_Ctrl_Pt_PatchList);
				technique_ = effect_->TechniqueByName("Tessellation");
			}
			else
			{
				rl_->TopologyType(RenderLayout::TT_TriangleList);
				technique_ = effect_->TechniqueByName("NoTessellation");
			}
		}

		void TessFactors(float4 const & factor)
		{
			*tess_factors_param_ = factor;
		}

	private:
		RenderEffectParameter* tess_factors_param_;
	};

	class TriangleObject : public SceneObjectHelper
	{
	public:
		TriangleObject()
			: SceneObjectHelper(MakeSharedPtr<RenderTriangle>(), SOA_Cullable)
		{
		}

		void TessEnabled(bool enabled)
		{
			checked_pointer_cast<RenderTriangle>(renderable_)->TessEnabled(enabled);
		}

		void TessFactors(float4 const & factor)
		{
			checked_pointer_cast<RenderTriangle>(renderable_)->TessFactors(factor);
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
	ContextCfg cfg = Context::Instance().Config();
	cfg.graphics_cfg.hdr = false;
	cfg.graphics_cfg.ppaa = false;
	Context::Instance().Config(cfg);

	TessellationApp app;
	app.Create();
	app.Run();

	return 0;
}

TessellationApp::TessellationApp()
					: App3DFramework("Tessellation"),
						tess_factor_(0, 0, 0, 0)
{
	ResLoader::Instance().AddPath("../../Tutorials/media/Tessellation");
}

void TessellationApp::OnCreate()
{
	font_ = SyncLoadFont("gkai00mp.kfont");

	polygon_ = MakeSharedPtr<TriangleObject>();
	polygon_->ModelMatrix(MathLib::rotation_x(-0.5f));
	polygon_->AddToSceneManager();

	this->LookAt(float3(2, 0, -2), float3(0, 0, 0));
	this->Proj(0.1f, 100);

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

	UIManager::Instance().Load(ResLoader::Instance().Open("Tessellation.uiml"));
	dialog_ = UIManager::Instance().GetDialogs()[0];

	id_tess_enabled_ = dialog_->IDFromName("Tessellation");
	id_edge0_static_ = dialog_->IDFromName("Edge0Static");
	id_edge0_slider_ = dialog_->IDFromName("Edge0Slider");
	id_edge1_static_ = dialog_->IDFromName("Edge1Static");
	id_edge1_slider_ = dialog_->IDFromName("Edge1Slider");
	id_edge2_static_ = dialog_->IDFromName("Edge2Static");
	id_edge2_slider_ = dialog_->IDFromName("Edge2Slider");
	id_inside_static_ = dialog_->IDFromName("InsideStatic");
	id_inside_slider_ = dialog_->IDFromName("InsideSlider");

	dialog_->Control<UICheckBox>(id_tess_enabled_)->OnChangedEvent().connect(
		[this](UICheckBox const & sender)
		{
			this->TessellationOnHandler(sender);
		});
	this->TessellationOnHandler(*dialog_->Control<UICheckBox>(id_tess_enabled_));
	dialog_->Control<UISlider>(id_edge0_slider_)->OnValueChangedEvent().connect(
		[this](UISlider const & sender)
		{
			this->Edge0ChangedHandler(sender);
		});
	this->Edge0ChangedHandler(*dialog_->Control<UISlider>(id_edge0_slider_));
	dialog_->Control<UISlider>(id_edge1_slider_)->OnValueChangedEvent().connect(
		[this](UISlider const & sender)
		{
			this->Edge1ChangedHandler(sender);
		});
	this->Edge1ChangedHandler(*dialog_->Control<UISlider>(id_edge1_slider_));
	dialog_->Control<UISlider>(id_edge2_slider_)->OnValueChangedEvent().connect(
		[this](UISlider const & sender)
		{
			this->Edge2ChangedHandler(sender);
		});
	this->Edge2ChangedHandler(*dialog_->Control<UISlider>(id_edge2_slider_));
	dialog_->Control<UISlider>(id_inside_slider_)->OnValueChangedEvent().connect(
		[this](UISlider const & sender)
		{
			this->InsideChangedHandler(sender);
		});
	this->InsideChangedHandler(*dialog_->Control<UISlider>(id_inside_slider_));

	RenderDeviceCaps const & caps = Context::Instance().RenderFactoryInstance().RenderEngineInstance().DeviceCaps();
	if (!caps.ds_support)
	{
		dialog_->Control<UICheckBox>(id_tess_enabled_)->SetEnabled(false);
	}
}

void TessellationApp::OnResize(uint32_t width, uint32_t height)
{
	App3DFramework::OnResize(width, height);

	UIManager::Instance().SettleCtrls();
}

void TessellationApp::InputHandler(InputEngine const & /*sender*/, InputAction const & action)
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

void TessellationApp::TessellationOnHandler(UICheckBox const & sender)
{
	bool enabled = sender.GetChecked();

	checked_pointer_cast<TriangleObject>(polygon_)->TessEnabled(enabled);

	dialog_->Control<UIStatic>(id_edge0_static_)->SetEnabled(enabled);
	dialog_->Control<UISlider>(id_edge0_slider_)->SetEnabled(enabled);
	dialog_->Control<UIStatic>(id_edge1_static_)->SetEnabled(enabled);
	dialog_->Control<UISlider>(id_edge1_slider_)->SetEnabled(enabled);
	dialog_->Control<UIStatic>(id_edge2_static_)->SetEnabled(enabled);
	dialog_->Control<UISlider>(id_edge2_slider_)->SetEnabled(enabled);
	dialog_->Control<UIStatic>(id_inside_static_)->SetEnabled(enabled);
	dialog_->Control<UISlider>(id_inside_slider_)->SetEnabled(enabled);
}

void TessellationApp::Edge0ChangedHandler(KlayGE::UISlider const & sender)
{
	tess_factor_.x() = sender.GetValue() / 10.0f;
	checked_pointer_cast<TriangleObject>(polygon_)->TessFactors(tess_factor_);

	std::wostringstream stream;
	stream << L"Edge 0: " << tess_factor_.x();
	dialog_->Control<UIStatic>(id_edge0_static_)->SetText(stream.str());
}

void TessellationApp::Edge1ChangedHandler(KlayGE::UISlider const & sender)
{
	tess_factor_.y() = sender.GetValue() / 10.0f;
	checked_pointer_cast<TriangleObject>(polygon_)->TessFactors(tess_factor_);

	std::wostringstream stream;
	stream << L"Edge 1: " << tess_factor_.y();
	dialog_->Control<UIStatic>(id_edge1_static_)->SetText(stream.str());
}

void TessellationApp::Edge2ChangedHandler(KlayGE::UISlider const & sender)
{
	tess_factor_.z() = sender.GetValue() / 10.0f;
	checked_pointer_cast<TriangleObject>(polygon_)->TessFactors(tess_factor_);

	std::wostringstream stream;
	stream << L"Edge 2: " << tess_factor_.z();
	dialog_->Control<UIStatic>(id_edge2_static_)->SetText(stream.str());
}

void TessellationApp::InsideChangedHandler(KlayGE::UISlider const & sender)
{
	tess_factor_.w() = sender.GetValue() / 10.0f;
	checked_pointer_cast<TriangleObject>(polygon_)->TessFactors(tess_factor_);

	std::wostringstream stream;
	stream << L"Inside: " << tess_factor_.w();
	dialog_->Control<UIStatic>(id_inside_static_)->SetText(stream.str());
}

void TessellationApp::DoUpdateOverlay()
{
	UIManager::Instance().Render();

	RenderEngine& renderEngine(Context::Instance().RenderFactoryInstance().RenderEngineInstance());

	std::wostringstream stream;
	stream.precision(2);
	stream << std::fixed << this->FPS() << " FPS";

	font_->RenderText(0, 0, Color(1, 1, 0, 1), L"Tessellation", 16);
	font_->RenderText(0, 18, Color(1, 1, 0, 1), renderEngine.Name(), 16);
	font_->RenderText(0, 36, Color(1, 1, 0, 1), stream.str(), 16);
}

uint32_t TessellationApp::DoUpdate(uint32_t /*pass*/)
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
