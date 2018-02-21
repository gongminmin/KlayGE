#include <KlayGE/KlayGE.hpp>
#include <KFL/CXX17/iterator.hpp>
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
#include <KlayGE/AudioFactory.hpp>
#include <KlayGE/Audio.hpp>
#include <KlayGE/AudioDataSource.hpp>

#include <vector>
#include <sstream>
#include <fstream>

#include "SampleCommon.hpp"
#include "Sound.hpp"

using namespace std;
using namespace KlayGE;

namespace
{
	enum
	{
		Exit
	};

	InputActionDefine actions[] =
	{
		InputActionDefine(Exit, KS_Escape),
	};
}


int SampleMain()
{
	SoundApp app;
	app.Create();
	app.Run();

	return 0;
}

SoundApp::SoundApp()
			: App3DFramework("Sound")
{
	ResLoader::Instance().AddPath("../../Samples/media/Sound");
}

void SoundApp::OnCreate()
{
	font_ = SyncLoadFont("gkai00mp.kfont");

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

	AudioDataSourceFactory& adsf = Context::Instance().AudioDataSourceFactoryInstance();
	music_1_ = adsf.MakeAudioDataSource();
	music_1_->Open(ResLoader::Instance().Open("Carl_Douglas_-_Kung_Fu_Fighting.ogg"));
	music_2_ = adsf.MakeAudioDataSource();
	music_2_->Open(ResLoader::Instance().Open("Metallica_-_Enter_Sandman.ogg"));
	sound_ = adsf.MakeAudioDataSource();
	sound_->Open(ResLoader::Instance().Open("Cash_register.ogg"));

	AudioFactory& af = Context::Instance().AudioFactoryInstance();
	AudioEngine& ae = af.AudioEngineInstance();
	ae.AddBuffer(1, af.MakeMusicBuffer(music_1_, 3));
	ae.AddBuffer(2, af.MakeMusicBuffer(music_2_, 3));
	ae.AddBuffer(3, af.MakeSoundBuffer(sound_));

	UIManager::Instance().Load(ResLoader::Instance().Open("Sound.uiml"));
	dialog_ = UIManager::Instance().GetDialogs()[0];

	id_music_1_ = dialog_->IDFromName("Music_1");
	id_music_2_ = dialog_->IDFromName("Music_2");
	id_sound_ = dialog_->IDFromName("Sound");
	id_volume_static_ = dialog_->IDFromName("VolumeStatic");
	id_volume_slider_ = dialog_->IDFromName("VolumeSlider");

	dialog_->Control<UICheckBox>(id_music_1_)->OnChangedEvent().connect(
		[this](UICheckBox const & sender)
		{
			this->Music1Handler(sender);
		});
	this->Music1Handler(*dialog_->Control<UICheckBox>(id_music_1_));

	dialog_->Control<UICheckBox>(id_music_2_)->OnChangedEvent().connect(
		[this](UICheckBox const & sender)
		{
			this->Music2Handler(sender);
		});
	this->Music2Handler(*dialog_->Control<UICheckBox>(id_music_2_));

	dialog_->Control<UIButton>(id_sound_)->OnClickedEvent().connect(
		[this](UIButton const & sender)
		{
			this->SoundHandler(sender);
		});

	dialog_->Control<UISlider>(id_volume_slider_)->OnValueChangedEvent().connect(
		[this](UISlider const & sender)
		{
			this->VolumeChangedHandler(sender);
		});
	this->VolumeChangedHandler(*dialog_->Control<UISlider>(id_volume_slider_));
}

void SoundApp::OnResize(uint32_t width, uint32_t height)
{
	App3DFramework::OnResize(width, height);

	UIManager::Instance().SettleCtrls();
}

void SoundApp::InputHandler(InputEngine const & /*sender*/, InputAction const & action)
{
	switch (action.first)
	{
	case Exit:
		this->Quit();
		break;
	}
}

void SoundApp::Music1Handler(UICheckBox const & sender)
{
	AudioFactory& af = Context::Instance().AudioFactoryInstance();
	AudioEngine& ae = af.AudioEngineInstance();
	if (sender.GetChecked())
	{
		ae.Play(1, true);
	}
	else
	{
		ae.Stop(1);
	}
}

void SoundApp::Music2Handler(UICheckBox const & sender)
{
	AudioFactory& af = Context::Instance().AudioFactoryInstance();
	AudioEngine& ae = af.AudioEngineInstance();
	if (sender.GetChecked())
	{
		ae.Play(2, true);
	}
	else
	{
		ae.Stop(2);
	}
}

void SoundApp::SoundHandler(UIButton const & sender)
{
	KFL_UNUSED(sender);

	AudioFactory& af = Context::Instance().AudioFactoryInstance();
	AudioEngine& ae = af.AudioEngineInstance();
	ae.Play(3, false);
}

void SoundApp::VolumeChangedHandler(UISlider const & sender)
{
	volume_ = sender.GetValue() * 0.01f;

	AudioFactory& af = Context::Instance().AudioFactoryInstance();
	AudioEngine& ae = af.AudioEngineInstance();
	ae.SoundVolume(volume_);
	ae.MusicVolume(volume_);	

	std::wostringstream stream;
	stream << L"Volume: " << volume_;
	dialog_->Control<UIStatic>(id_volume_static_)->SetText(stream.str());
}

void SoundApp::DoUpdateOverlay()
{
	UIManager::Instance().Render();

	RenderEngine& renderEngine(Context::Instance().RenderFactoryInstance().RenderEngineInstance());

	std::wostringstream stream;
	stream.precision(2);
	stream << std::fixed << this->FPS() << " FPS";

	font_->RenderText(0, 0, Color(1, 1, 0, 1), L"Sound", 16);
	font_->RenderText(0, 18, Color(1, 1, 0, 1), renderEngine.ScreenFrameBuffer()->Description(), 16);
	font_->RenderText(0, 36, Color(1, 1, 0, 1), stream.str(), 16);
}

uint32_t SoundApp::DoUpdate(uint32_t /*pass*/)
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
