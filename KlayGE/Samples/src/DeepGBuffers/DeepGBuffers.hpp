#ifndef KLAYGE_SAMPLES_DEEP_G_BUFFERS_HPP
#define KLAYGE_SAMPLES_DEEP_G_BUFFERS_HPP

#include <KlayGE/App3D.hpp>
#include <KlayGE/Font.hpp>
#include <KlayGE/CameraController.hpp>
#include <KlayGE/UI.hpp>

class DeepGBuffersApp : public KlayGE::App3DFramework
{
public:
	DeepGBuffersApp();

private:
	void OnCreate();

	void DoUpdateOverlay();
	KlayGE::uint32_t DoUpdate(KlayGE::uint32_t pass);

	void OnResize(KlayGE::uint32_t width, KlayGE::uint32_t height);

	void InputHandler(KlayGE::InputEngine const & sender, KlayGE::InputAction const & action);
	void ReceivesLightingHandler(KlayGE::UICheckBox const & sender);
	void TransparencyChangedHandler(KlayGE::UISlider const & sender);
	void SimpleForwardHandler(KlayGE::UICheckBox const & sender);
	void CtrlCameraHandler(KlayGE::UICheckBox const & sender);

	KlayGE::FontPtr font_;

	KlayGE::RenderModelPtr scene_model_;

	KlayGE::FirstPersonCameraController fpcController_;

	KlayGE::DeferredRenderingLayer* deferred_rendering_;

	KlayGE::UIDialogPtr dialog_;
	int id_receives_lighting_;
	int id_transparency_static_;
	int id_transparency_slider_;
	int id_simple_forward_;
	int id_ctrl_camera_;

	float transparency_;
};

#endif		// KLAYGE_SAMPLES_DEEP_G_BUFFERS_HPP
