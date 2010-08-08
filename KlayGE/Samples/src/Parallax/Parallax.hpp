#ifndef _PARALLAX_HPP
#define _PARALLAX_HPP

#include <KlayGE/App3D.hpp>
#include <KlayGE/Font.hpp>
#include <KlayGE/CameraController.hpp>
#include <KlayGE/UI.hpp>

class Parallax : public KlayGE::App3DFramework
{
public:
	Parallax();

private:
	void InitObjects();
	void OnResize(KlayGE::uint32_t width, KlayGE::uint32_t height);

	void DoUpdateOverlay();
	KlayGE::uint32_t DoUpdate(KlayGE::uint32_t pass);

	void InputHandler(KlayGE::InputEngine const & sender, KlayGE::InputAction const & action);
	void ScaleChangedHandler(KlayGE::UISlider const & sender);
	void BiasChangedHandler(KlayGE::UISlider const & sender);
	void CtrlCameraHandler(KlayGE::UICheckBox const & sender);

	KlayGE::FontPtr font_;
	KlayGE::SceneObjectPtr polygon_;

	KlayGE::FirstPersonCameraController fpcController_;

	KlayGE::UIDialogPtr dialog_;
	float parallax_scale_;
	float parallax_bias_;

	int id_scale_static_;
	int id_scale_slider_;
	int id_bias_static_;
	int id_bias_slider_;
	int id_ctrl_camera_;
};

#endif		// _PARALLAX_HPP
