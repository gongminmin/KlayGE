#ifndef _ATMOSPHERICSCATTERING_HPP
#define _ATMOSPHERICSCATTERING_HPP

#include <KlayGE/App3D.hpp>
#include <KlayGE/Font.hpp>
#include <KlayGE/CameraController.hpp>
#include <KlayGE/UI.hpp>

class AtmosphericScatteringApp : public KlayGE::App3DFramework
{
public:
	AtmosphericScatteringApp();

private:
	void OnCreate();
	void OnResize(KlayGE::uint32_t width, KlayGE::uint32_t height);
	void DoUpdateOverlay();
	KlayGE::uint32_t DoUpdate(KlayGE::uint32_t pass);

	void LoadBeta(KlayGE::Color const & clr);
	void LoadAbsorb(KlayGE::Color const & clr);

	void InputHandler(KlayGE::InputEngine const & sender, KlayGE::InputAction const & action);

	void AtmosphereTopHandler(KlayGE::UISlider const & sender);
	void DensityHandler(KlayGE::UISlider const & sender);
	void ChangeBetaHandler(KlayGE::UITexButton const & sender);
	void ChangeAbsorbHandler(KlayGE::UITexButton const & sender);

private:
	KlayGE::FontPtr font_;
	KlayGE::SceneNodePtr planet_;
	KlayGE::SceneNodePtr atmosphere_;

	KlayGE::TrackballCameraController obj_controller_;
	KlayGE::TrackballCameraController light_controller_;
	KlayGE::Camera light_ctrl_camera_;

	KlayGE::SceneNodePtr sun_light_src_;
	KlayGE::DirectionalLightSourcePtr sun_light_;

	KlayGE::Color beta_;
	KlayGE::Color absorb_;

	KlayGE::UIDialogPtr dialog_param_;
	int id_atmosphere_top_;
	int id_density_;
	int id_beta_button_;
	int id_absorb_button_;
};

#endif		// _ATMOSPHERICSCATTERING_HPP
