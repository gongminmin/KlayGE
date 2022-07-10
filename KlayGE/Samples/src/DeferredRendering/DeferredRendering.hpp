#ifndef _DEFERREDRENDERING_HPP
#define _DEFERREDRENDERING_HPP

#include <KlayGE/App3D.hpp>
#include <KlayGE/Font.hpp>
#include <KlayGE/CameraController.hpp>
#include <KlayGE/UI.hpp>

class DeferredRenderingApp : public KlayGE::App3DFramework
{
public:
	DeferredRenderingApp();

private:
	void OnCreate();

	void DoUpdateOverlay();
	KlayGE::uint32_t DoUpdate(KlayGE::uint32_t pass);

	void OnResize(KlayGE::uint32_t width, KlayGE::uint32_t height);

	void InputHandler(KlayGE::InputEngine const & sender, KlayGE::InputAction const & action);
	void BufferChangedHandler(KlayGE::UIComboBox const & sender);
	void IllumChangedHandler(KlayGE::UIComboBox const & sender);
	void ILScaleChangedHandler(KlayGE::UISlider const & sender);
	void SSVOHandler(KlayGE::UICheckBox const & sender);
	void HDRHandler(KlayGE::UICheckBox const & sender);
	void AntiAliasHandler(KlayGE::UICheckBox const& sender);
	void DepthOfFieldHandler(KlayGE::UICheckBox const& sender);
	void BokehHandler(KlayGE::UICheckBox const& sender);
	void MotionBlurHandler(KlayGE::UICheckBox const& sender);
	void NumLightsChangedHandler(KlayGE::UISlider const & sender);
	void CtrlCameraHandler(KlayGE::UICheckBox const & sender);

	KlayGE::FontPtr font_;

	KlayGE::FirstPersonCameraController fpcController_;

	KlayGE::DeferredRenderingLayer* deferred_rendering_;

	KlayGE::UIDialogPtr dialog_;
	int buffer_type_;
	bool ssvo_enabled_;
	bool hdr_enabled_;
	int anti_alias_enabled_;
	bool dof_enabled_ = false;
	bool bokeh_enabled_ = false;
	bool motion_blur_enabled_ = false;

	float il_scale_;

	int id_buffer_combo_;
	int id_illum_combo_;
	int id_il_scale_static_;
	int id_il_scale_slider_;
	int id_ssvo_;
	int id_hdr_;
	int id_aa_;
	int id_dof_;
	int id_bokeh_;
	int id_motion_blur_;
	int id_num_lights_static_;
	int id_num_lights_slider_;
	int id_ctrl_camera_;

	std::vector<KlayGE::SceneNodePtr> particle_light_nodes_;
	std::vector<KlayGE::Signal::Connection> particle_light_node_update_connections_;
	std::vector<KlayGE::Signal::Connection> particle_light_update_connections_;

	KlayGE::ParticleSystemPtr ps_;
};

#endif		// _DEFERREDRENDERING_HPP
