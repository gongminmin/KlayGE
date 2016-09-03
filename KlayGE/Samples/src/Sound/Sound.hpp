#ifndef _SOUND_HPP
#define _SOUND_HPP

#include <KlayGE/App3D.hpp>
#include <KlayGE/Font.hpp>
#include <KlayGE/CameraController.hpp>
#include <KlayGE/UI.hpp>

class SoundApp : public KlayGE::App3DFramework
{
public:
	SoundApp();

private:
	void OnCreate();
	void OnResize(KlayGE::uint32_t width, KlayGE::uint32_t height);

	void DoUpdateOverlay();
	KlayGE::uint32_t DoUpdate(KlayGE::uint32_t pass);

	void InputHandler(KlayGE::InputEngine const & sender, KlayGE::InputAction const & action);
	void Music1Handler(KlayGE::UICheckBox const & sender);
	void Music2Handler(KlayGE::UICheckBox const & sender);
	void SoundHandler(KlayGE::UIButton const & sender);
	void VolumeChangedHandler(KlayGE::UISlider const & sender);

	KlayGE::FontPtr font_;
	KlayGE::AudioDataSourcePtr music_1_;
	KlayGE::AudioDataSourcePtr music_2_;
	KlayGE::AudioDataSourcePtr sound_;

	float volume_;

	KlayGE::UIDialogPtr dialog_;

	int id_music_1_;
	int id_music_2_;
	int id_sound_;
	int id_volume_static_;
	int id_volume_slider_;
};

#endif		// _SOUND_HPP
