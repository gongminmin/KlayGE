#ifndef _FOLIAGE_HPP
#define _FOLIAGE_HPP

#include <KlayGE/App3D.hpp>
#include <KlayGE/Font.hpp>
#include <KlayGE/CameraController.hpp>
#include <KlayGE/UI.hpp>

class FoliageApp : public KlayGE::App3DFramework
{
public:
	FoliageApp();

private:
	void OnCreate();
	void OnResize(KlayGE::uint32_t width, KlayGE::uint32_t height);
	void DoUpdateOverlay();
	KlayGE::uint32_t DoUpdate(KlayGE::uint32_t pass);

	void InputHandler(KlayGE::InputEngine const & sender, KlayGE::InputAction const & action);

	void LightShaftHandler(KlayGE::UICheckBox const & sender);
	void FPSCameraHandler(KlayGE::UICheckBox const & sender);

	KlayGE::FontPtr font_;
	KlayGE::SceneNodePtr terrain_;
	KlayGE::SceneNodePtr sky_box_;
	KlayGE::SceneNodePtr sun_flare_;
	KlayGE::FirstPersonCameraController fpcController_;

	KlayGE::DeferredRenderingLayer* deferred_rendering_;

	KlayGE::PostProcessPtr fog_pp_;

	bool light_shaft_on_;
	KlayGE::PostProcessPtr light_shaft_pp_;

	KlayGE::UIDialogPtr dialog_params_;
	int id_light_shaft_;
	int id_fps_camera_;

	KlayGE::LightSourcePtr sun_light_;
};

#endif		// _FOLIAGE_HPP
