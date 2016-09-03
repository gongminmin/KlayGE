#ifndef _GLOBALILLUMINATION_HPP
#define _GLOBALILLUMINATION_HPP

#include <KlayGE/App3D.hpp>
#include <KlayGE/Font.hpp>
#include <KlayGE/CameraController.hpp>
#include <KlayGE/UI.hpp>
#include <KlayGE/PostProcess.hpp>

class GlobalIlluminationApp : public KlayGE::App3DFramework
{
public:
	GlobalIlluminationApp();

private:
	void OnCreate();
	void OnResize(KlayGE::uint32_t width, KlayGE::uint32_t height);

	void DoUpdateOverlay();
	KlayGE::uint32_t DoUpdate(KlayGE::uint32_t pass);

	void InputHandler(KlayGE::InputEngine const & sender, KlayGE::InputAction const & action);
	void IllumChangedHandler(KlayGE::UIComboBox const & sender);
	void ILScaleChangedHandler(KlayGE::UISlider const & sender);
	void SSGIHandler(KlayGE::UICheckBox const & sender);
	void SSVOHandler(KlayGE::UICheckBox const & sender);
	void HDRHandler(KlayGE::UICheckBox const & sender);
	void AAHandler(KlayGE::UICheckBox const & sender);
	void ColorGradingHandler(KlayGE::UICheckBox const & sender);
	void CtrlCameraHandler(KlayGE::UICheckBox const & sender);

	KlayGE::FontPtr font_;
	KlayGE::SceneObjectPtr sky_box_;
	KlayGE::SceneObjectPtr spot_light_src_;

	KlayGE::SpotLightSourcePtr spot_light_;

	KlayGE::FirstPersonCameraController fpcController_;

	KlayGE::DeferredRenderingLayer* deferred_rendering_;

	KlayGE::UIDialogPtr dialog_;

	float il_scale_;

	int id_illum_combo_;
	int id_il_scale_static_;
	int id_il_scale_slider_;
	int id_ssgi_;
	int id_ssvo_;
	int id_hdr_;
	int id_aa_;
	int id_cg_;
	int id_ctrl_camera_;
};

#endif		// _GLOBALILLUMINATION_HPP
