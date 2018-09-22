#ifndef _DEFERREDRENDERING_HPP
#define _DEFERREDRENDERING_HPP

#include <KlayGE/App3D.hpp>
#include <KlayGE/Font.hpp>
#include <KlayGE/CameraController.hpp>
#include <KlayGE/UI.hpp>

class CascadedShadowMapApp : public KlayGE::App3DFramework
{
public:
	CascadedShadowMapApp();

private:
	void OnCreate();

	void DoUpdateOverlay();
	KlayGE::uint32_t DoUpdate(KlayGE::uint32_t pass);

	void OnResize(KlayGE::uint32_t width, KlayGE::uint32_t height);

	void InputHandler(KlayGE::InputEngine const & sender, KlayGE::InputAction const & action);
	void CSMTypeChangedHandler(KlayGE::UIComboBox const & sender);
	void CascadesChangedHandler(KlayGE::UIComboBox const & sender);
	void PSSMFactorChangedHandler(KlayGE::UISlider const & sender);
	void CtrlCameraHandler(KlayGE::UICheckBox const & sender);

	KlayGE::FontPtr font_;

	KlayGE::SceneNodePtr sky_box_;

	KlayGE::FirstPersonCameraController fpcController_;
	KlayGE::TrackballCameraController light_controller_;
	KlayGE::Camera light_ctrl_camera_;

	KlayGE::DeferredRenderingLayer* deferred_rendering_;

	KlayGE::UIDialogPtr dialog_;

	int id_csm_type_combo_;
	int id_cascades_combo_;
	int id_pssm_factor_static_;
	int id_pssm_factor_slider_;
	int id_ctrl_camera_;

	KlayGE::LightSourcePtr sun_light_;

	KlayGE::uint32_t num_cascades_;
	float pssm_factor_;
};

#endif		// _DEFERREDRENDERING_HPP
