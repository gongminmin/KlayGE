#include <KlayGE/KlayGE.hpp>
#include <KlayGE/App3D.hpp>
#include <KlayGE/ResLoader.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/Font.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/FrameBuffer.hpp>
#include <KlayGE/UI.hpp>

#include <vector>
#include <sstream>

#include "SampleCommon.hpp"

class TutorFramework : public KlayGE::App3DFramework
{

public:
	TutorFramework();

protected:
	virtual void OnCreate();

private:
	virtual void DoUpdateOverlay();
	virtual KlayGE::uint32_t DoUpdate(KlayGE::uint32_t pass);

private:
	KlayGE::FontPtr font_;
};

int SampleMain()
{
	TutorFramework app;
	app.Create();
	app.Run();

	return 0;
}

#ifdef KLAYGE_PLATFORM_IOS
App3DFramework* SampleApp()
{
	return new TutorFramework();
}
#endif

TutorFramework::TutorFramework()
	: App3DFramework("Tutor1")
{
}

void TutorFramework::OnCreate()
{
	font_ = KlayGE::SyncLoadFont("gkai00mp.kfont");
}

void TutorFramework::DoUpdateOverlay()
{
	std::wostringstream stream;
	stream.precision(2);
	stream << std::fixed << this->FPS() << " FPS";

	font_->RenderText(0, 0, KlayGE::Color(1, 1, 0, 1), L"Tutorial 1", 16);
	font_->RenderText(0, 18, KlayGE::Color(1, 1, 0, 1), stream.str(), 16);
}

KlayGE::uint32_t TutorFramework::DoUpdate(KlayGE::uint32_t /*pass*/)
{
	KlayGE::RenderEngine& re = KlayGE::Context::Instance().RenderFactoryInstance().RenderEngineInstance();

	KlayGE::Color clear_clr(0.2f, 0.4f, 0.6f, 1);
	if (KlayGE::Context::Instance().Config().graphics_cfg.gamma)
	{
		clear_clr.r() = 0.029f;
		clear_clr.g() = 0.133f;
		clear_clr.b() = 0.325f;
	}
	re.CurFrameBuffer()->Clear(KlayGE::FrameBuffer::CBM_Color | KlayGE::FrameBuffer::CBM_Depth,
		clear_clr, 1.0f, 0);

	return KlayGE::App3DFramework::URV_NeedFlush | KlayGE::App3DFramework::URV_Finished;
}
