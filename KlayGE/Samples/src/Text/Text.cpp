#include <KlayGE/KlayGE.hpp>
#include <KFL/CXX17/iterator.hpp>
#include <KFL/ErrorHandling.hpp>
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
#include <KlayGE/GraphicsBuffer.hpp>
#include <KlayGE/SceneObjectHelper.hpp>
#include <KlayGE/UI.hpp>
#include <KlayGE/Input.hpp>

#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/InputFactory.hpp>

#include <vector>
#include <sstream>
#include <fstream>

#include "SampleCommon.hpp"
#include "Text.hpp"

using namespace std;
using namespace KlayGE;

namespace
{
	enum
	{
		Exit,
		Move,
		Scale
	};

	InputActionDefine actions[] =
	{
		InputActionDefine(Exit, KS_Escape),

		InputActionDefine(Move, TS_Pan),
		InputActionDefine(Move, MS_X),
		InputActionDefine(Move, MS_Y),
		InputActionDefine(Scale, TS_Zoom),
		InputActionDefine(Scale, TS_Wheel),
		InputActionDefine(Scale, MS_Z),
	};
}


int SampleMain()
{
	TextApp app;
	app.Create();
	app.Run();

	return 0;
}

TextApp::TextApp()
			: App3DFramework("Text"),
				last_mouse_pt_(-1, -1), position_(0, 0), scale_(1)
{
	ResLoader::Instance().AddPath("../../Samples/media/Text");
}

void TextApp::OnCreate()
{
	font_ = SyncLoadFont("gkai00mp.kfont");

	{
		// text.txt is in UCS2 little endian
		ResIdentifierPtr text_input = ResLoader::Instance().Open("text.txt");
		text_input->seekg(0, std::ios_base::end);
		uint32_t size = static_cast<uint32_t>(text_input->tellg());
		std::vector<uint16_t> ucs2_text(size / 2, '\0');
		text_input->seekg(0, std::ios_base::beg);
		text_input->read(&ucs2_text[0], size);

		text_.resize(ucs2_text.size(), L'\0');
		for (size_t i = 0; i < ucs2_text.size(); ++ i)
		{
			ucs2_text[i] = LE2Native(ucs2_text[i]);
			text_[i] = ucs2_text[i];
		}
	}

	this->LookAt(float3(-0.3f, 0.4f, -0.3f), float3(0, 0, 0));
	this->Proj(0.01f, 100);

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

	UIManager::Instance().Load(ResLoader::Instance().Open("Text.uiml"));
}

void TextApp::OnResize(uint32_t width, uint32_t height)
{
	App3DFramework::OnResize(width, height);

	UIManager::Instance().SettleCtrls();
}

void TextApp::InputHandler(InputEngine const & /*sender*/, InputAction const & action)
{
	switch (action.first)
	{
	case Move:
		switch (action.second->type)
		{
		case InputEngine::IDT_Mouse:
			{
				InputMouseActionParamPtr param = checked_pointer_cast<InputMouseActionParam>(action.second);
				int2 this_mouse_pt(param->abs_coord);
				if (param->buttons_state & MB_Left)
				{
					if ((last_mouse_pt_.x() != -1) || (last_mouse_pt_.y() != -1))
					{
						position_ += float2(this_mouse_pt - last_mouse_pt_) / scale_;
					}
				}

				last_mouse_pt_ = this_mouse_pt;
			}
			break;

		case InputEngine::IDT_Touch:
			{
				InputTouchActionParamPtr param = checked_pointer_cast<InputTouchActionParam>(action.second);
				position_ += float2(param->move_vec) / scale_;
			}
			break;

		default:
			KFL_UNREACHABLE("Invalid input type");
		}
		break;

	case Scale:
		switch (action.second->type)
		{
		case InputEngine::IDT_Mouse:
			{
				InputMouseActionParamPtr param = checked_pointer_cast<InputMouseActionParam>(action.second);
				float f = 1.0f + MathLib::clamp(param->wheel_delta / 1200.0f, -0.5f, 0.5f);
				float2 p = float2(-param->abs_coord) / scale_ + position_;
				float2 new_position = (position_ - p * (1 - f)) / f;
				float new_scale = scale_ * f;
				if ((new_scale > 0.25f) && (new_scale < 32))
				{
					position_ = new_position;
					scale_ = new_scale;
				}
			}
			break;

		case InputEngine::IDT_Touch:
			{
				InputTouchActionParamPtr param = checked_pointer_cast<InputTouchActionParam>(action.second);
				float f;
				if (TS_Zoom == param->gesture)
				{
					f = param->zoom;
				}
				else
				{
					f = 1.0f + MathLib::clamp(param->wheel_delta / 1200.0f, -0.5f, 0.5f);
				}
				float2 p = float2(-param->center) / scale_ + position_;
				float2 new_position = (position_ - p * (1 - f)) / f;
				float new_scale = scale_ * f;
				if ((new_scale > 0.25f) && (new_scale < 32))
				{
					position_ = new_position;
					scale_ = new_scale;
				}
			}
			break;

		default:
			KFL_UNREACHABLE("Invalid input type");
		}
		break;

	case Exit:
		this->Quit();
		break;
	}
}

void TextApp::DoUpdateOverlay()
{
	RenderEngine& renderEngine(Context::Instance().RenderFactoryInstance().RenderEngineInstance());

	float2 fxy = position_ * scale_;
	font_->RenderText(fxy.x(), fxy.y(), 0.5f, 1, 1, Color(1, 1, 1, 1), text_, 32 * scale_);

	std::wostringstream stream;
	stream.precision(2);
	stream << std::fixed << this->FPS() << " FPS";

	font_->RenderText(0, 0, Color(1, 1, 0, 1), L"Text", 16);
	font_->RenderText(0, 18, Color(1, 1, 0, 1), renderEngine.ScreenFrameBuffer()->Description(), 16);
	font_->RenderText(0, 36, Color(1, 1, 0, 1), stream.str(), 16);

	UIManager::Instance().Render();
}

uint32_t TextApp::DoUpdate(uint32_t /*pass*/)
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
