#include <KlayGE/KlayGE.hpp>
#include <KlayGE/ResLoader.hpp>
#include <KlayGE/RenderSettings.hpp>
#include <KlayGE/FrameBuffer.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/RenderEngine.hpp>

#include "EmptyApp.hpp"

using namespace std;
using namespace KlayGE;

int main()
{
	Context::Instance().LoadCfg("KlayGE.cfg");

	EmptyApp app;
	app.Create();
	app.Run();

	return 0;
}

EmptyApp::EmptyApp()
			: App3DFramework("EmptyApp")
{
}

bool EmptyApp::ConfirmDevice() const
{
	return true;
}

void EmptyApp::DoUpdateOverlay()
{
}

uint32_t EmptyApp::DoUpdate(uint32_t /*pass*/)
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

	return App3DFramework::URV_Need_Flush | App3DFramework::URV_Finished;
}
