#include <KlayGE/KlayGE.hpp>
#include <KlayGE/ThrowErr.hpp>
#include <KlayGE/Util.hpp>
#include <KlayGE/Math.hpp>
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
#include <KlayGE/KMesh.hpp>
#include <KlayGE/GraphicsBuffer.hpp>
#include <KlayGE/SceneObjectHelper.hpp>
#include <KlayGE/UI.hpp>

#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/InputFactory.hpp>

#include <vector>
#include <sstream>
#include <fstream>
#include <boost/bind.hpp>

#include "Text.hpp"

using namespace std;
using namespace KlayGE;

namespace
{
	enum
	{
		Exit,
		Scale_Text
	};

	InputActionDefine actions[] =
	{
		InputActionDefine(Exit, KS_Escape),
		InputActionDefine(Scale_Text, MS_Z),
	};
}


int main()
{
	ResLoader::Instance().AddPath("../Samples/media/Common");

	Context::Instance().LoadCfg("KlayGE.cfg");

	TextApp app;
	app.Create();
	app.Run();

	return 0;
}

TextApp::TextApp()
			: App3DFramework("Text"),
				scale_(1)
{
	ResLoader::Instance().AddPath("../Samples/media/Text");
}

bool TextApp::ConfirmDevice() const
{
	RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
	RenderDeviceCaps const & caps = re.DeviceCaps();
	if (caps.max_shader_model < 2)
	{
		return false;
	}
	return true;
}

void TextApp::InitObjects()
{
	// ½¨Á¢×ÖÌå
	font_ = Context::Instance().RenderFactoryInstance().MakeFont("gkai00mp.kfont");

	{
		ifstream ifs(ResLoader::Instance().Locate("text.txt").c_str());
		std::string str;
		std::wstring wstr;
		while (ifs)
		{
			std::getline(ifs, str);
			Convert(wstr, str);
			text_ += wstr + L'\n';
		}
	}

	this->LookAt(float3(-0.3f, 0.4f, -0.3f), float3(0, 0, 0));
	this->Proj(0.01f, 100);

	InputEngine& inputEngine(Context::Instance().InputFactoryInstance().InputEngineInstance());
	InputActionMap actionMap;
	actionMap.AddActions(actions, actions + sizeof(actions) / sizeof(actions[0]));

	action_handler_t input_handler = MakeSharedPtr<input_signal>();
	input_handler->connect(boost::bind(&TextApp::InputHandler, this, _1, _2));
	inputEngine.ActionMap(actionMap, input_handler, true);

	UIManager::Instance().Load(ResLoader::Instance().Load("Text.uiml"));
}

void TextApp::OnResize(uint32_t width, uint32_t height)
{
	App3DFramework::OnResize(width, height);

	UIManager::Instance().SettleCtrls(width, height);
}

void TextApp::InputHandler(InputEngine const & /*sender*/, InputAction const & action)
{
	switch (action.first)
	{
	case Scale_Text:
		scale_ += action.second / 720.0f;
		break;

	case Exit:
		this->Quit();
		break;
	}
}

void TextApp::DoUpdateOverlay()
{
	RenderEngine& renderEngine(Context::Instance().RenderFactoryInstance().RenderEngineInstance());

	std::wostringstream stream;
	stream.precision(2);
	stream << fixed << this->FPS() << L" FPS";

	font_->RenderText(0, 0, Color(1, 1, 0, 1), L"Text", 16);
	font_->RenderText(0, 18, Color(1, 1, 0, 1), renderEngine.CurFrameBuffer()->Description(), 16);
	font_->RenderText(0, 36, Color(1, 1, 0, 1), stream.str(), 16);
	font_->RenderText(0, 56, 0.5f, 1, 1, Color(1, 1, 1, 1), text_, static_cast<uint32_t>(32 * scale_));

	UIManager::Instance().Render();
}

uint32_t TextApp::DoUpdate(uint32_t /*pass*/)
{
	RenderEngine& renderEngine(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
	renderEngine.CurFrameBuffer()->Clear(FrameBuffer::CBM_Color | FrameBuffer::CBM_Depth, Color(0.2f, 0.4f, 0.6f, 1), 1.0f, 0);

	return App3DFramework::URV_Need_Flush | App3DFramework::URV_Finished;
}
