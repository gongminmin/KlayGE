#ifndef _CARTOON_HPP
#define _CARTOON_HPP

#include <KlayGE/App3D.hpp>
#include <KlayGE/Font.hpp>
#include <KlayGE/CameraController.hpp>
#include <KlayGE/UI.hpp>

class Cartoon : public KlayGE::App3DFramework
{
public:
	Cartoon(std::string const & name, KlayGE::RenderSettings const & settings);

private:
	void InitObjects();

	KlayGE::uint32_t DoUpdate(KlayGE::uint32_t pass);

	void OnResize(KlayGE::uint32_t width, KlayGE::uint32_t height);

	void InputHandler(KlayGE::InputEngine const & sender, KlayGE::InputAction const & action);
	void CheckBoxHandler(KlayGE::UICheckBox const & sender);

	KlayGE::FontPtr font_;
	KlayGE::SceneObjectPtr torus_;

	KlayGE::FirstPersonCameraController fpcController_;

	KlayGE::FrameBufferPtr g_buffer_;
	KlayGE::TexturePtr normal_depth_tex_;
	KlayGE::TexturePtr color_tex_;
	KlayGE::PostProcessPtr cartoon_;

	bool cartoon_style_;

	KlayGE::UIDialogPtr dialog_;
};

#endif		// _CARTOON_HPP
