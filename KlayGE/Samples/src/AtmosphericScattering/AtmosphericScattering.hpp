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

	bool ConfirmDevice() const;

private:
	void InitObjects();
	void OnResize(KlayGE::uint32_t width, KlayGE::uint32_t height);
	void DoUpdateOverlay();
	KlayGE::uint32_t DoUpdate(KlayGE::uint32_t pass);

	void InputHandler(KlayGE::InputEngine const & sender, KlayGE::InputAction const & action);

	void AtmosphereTopHandler(KlayGE::UISlider const & sender);
	void DensityHandler(KlayGE::UISlider const & sender);

private:
	KlayGE::FontPtr font_;
	KlayGE::SceneObjectPtr sphere_;

	KlayGE::TrackballCameraController tb_controller_;

	KlayGE::SceneObjectPtr sun_light_src_;
	KlayGE::DirectionalLightSourcePtr sun_light_;

	KlayGE::UIDialogPtr dialog_param_;
	int id_atmosphere_top_;
	int id_density_;
};

#endif		// _ATMOSPHERICSCATTERING_HPP
