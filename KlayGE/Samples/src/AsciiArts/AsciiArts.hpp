#ifndef _ASCIIARTS_HPP
#define _ASCIIARTS_HPP

#include <KlayGE/App3D.hpp>
#include <KlayGE/Font.hpp>
#include <KlayGE/CameraController.hpp>
#include <KlayGE/UI.hpp>

class AsciiArtsApp : public KlayGE::App3DFramework
{
public:
	AsciiArtsApp(std::string const & name, KlayGE::RenderSettings const & settings);

private:
	void InitObjects();
	void OnResize(KlayGE::uint32_t width, KlayGE::uint32_t height);

	KlayGE::uint32_t DoUpdate(KlayGE::uint32_t pass);

	void BuildAsciiLumsTex();

	void RenderInfos();

	void InputHandler(KlayGE::InputEngine const & sender, KlayGE::InputAction const & action);
	void CheckBoxHandler(KlayGE::UICheckBox const & sender);

private:
	KlayGE::FontPtr font_;

	KlayGE::FirstPersonCameraController fpcController_;
	KlayGE::SceneObjectPtr obj_;

	KlayGE::TexturePtr ascii_lums_tex_;

	KlayGE::FrameBufferPtr render_buffer_;
	KlayGE::TexturePtr rendered_tex_;

	KlayGE::TexturePtr downsample_tex_;

	KlayGE::PostProcessPtr downsampler_;
	KlayGE::PostProcessPtr ascii_arts_;

	bool show_ascii_;

	KlayGE::UIDialogPtr dialog_;
	int id_switch_ascii_;
};

#endif		// _ASCIIARTS_HPP