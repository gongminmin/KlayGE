#ifndef _AREALIGHTING_HPP
#define _AREALIGHTING_HPP

#include <KlayGE/App3D.hpp>
#include <KlayGE/Font.hpp>
#include <KlayGE/CameraController.hpp>
#include <KlayGE/UI.hpp>

class AreaLightingApp : public KlayGE::App3DFramework
{
public:
	AreaLightingApp();

private:
	void OnCreate();

	void DoUpdateOverlay();
	KlayGE::uint32_t DoUpdate(KlayGE::uint32_t pass);

	void OnResize(KlayGE::uint32_t width, KlayGE::uint32_t height);

	void InputHandler(KlayGE::InputEngine const & sender, KlayGE::InputAction const & action);
	void LightTypeChangedHandler(KlayGE::UIComboBox const & sender);
	void RadiusChangedHandler(KlayGE::UISlider const & sender);
	void LengthChangedHandler(KlayGE::UISlider const & sender);
	void CtrlCameraHandler(KlayGE::UICheckBox const & sender);

	KlayGE::FontPtr font_;

	KlayGE::SceneNodePtr sky_box_;

	KlayGE::FirstPersonCameraController fpcController_;

	KlayGE::DeferredRenderingLayer* deferred_rendering_;

	KlayGE::LightSourcePtr point_light_;
	KlayGE::SceneNodePtr point_light_src_;
	KlayGE::LightSourcePtr sphere_area_light_;
	KlayGE::SceneNodePtr sphere_area_light_src_;
	KlayGE::LightSourcePtr tube_area_light_;
	KlayGE::SceneNodePtr tube_area_light_src_;

	KlayGE::UIDialogPtr dialog_;

	int id_light_type_combo_;
	int id_radius_static_;
	int id_radius_slider_;
	int id_length_static_;
	int id_length_slider_;
	int id_ctrl_camera_;
};

#endif		// _AREALIGHTING_HPP
