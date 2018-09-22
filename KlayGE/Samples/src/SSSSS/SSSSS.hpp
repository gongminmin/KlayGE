#pragma once

#include <KlayGE/KlayGE.hpp>
#include <KlayGE/App3D.hpp>
#include <KlayGE/ResLoader.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/CameraController.hpp>
#include <KlayGE/Font.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/RenderEffect.hpp>
#include <KlayGE/SceneNodeHelper.hpp>
#include <KlayGE/Mesh.hpp>
#include <KlayGE/Camera.hpp>

#include <KlayGE/FrameBuffer.hpp>

#include <vector>
#include <sstream>

class SSSSSApp : public KlayGE::App3DFramework
{
public:
	SSSSSApp();

private:
	virtual void OnCreate() override;
	virtual void OnResize(KlayGE::uint32_t width, KlayGE::uint32_t height) override;
	virtual void DoUpdateOverlay();
	virtual KlayGE::uint32_t DoUpdate(KlayGE::uint32_t pass);

	void InputHandler(KlayGE::InputEngine const & sender, KlayGE::InputAction const & action);
	void SSSHandler(KlayGE::UICheckBox const & sender);
	void SSSStrengthChangedHandler(KlayGE::UISlider const & sender);
	void SSSCorrectionChangedHandler(KlayGE::UISlider const & sender);
	void TranslucencyHandler(KlayGE::UICheckBox const & sender);
	void TranslucencyStrengthChangedHandler(KlayGE::UISlider const & sender);

	KlayGE::DeferredRenderingLayer* deferred_rendering_;

	KlayGE::FontPtr font_;
	KlayGE::TrackballCameraController obj_controller_;
	KlayGE::TrackballCameraController light_controller_;

	KlayGE::LightSourcePtr light_;
	KlayGE::SceneNodePtr light_proxy_;

	KlayGE::CameraPtr scene_camera_;
	KlayGE::CameraPtr light_camera_;

	KlayGE::UIDialogPtr dialog_params_;
	int id_sss_;
	int id_sss_strength_static_;
	int id_sss_strength_slider_;
	int id_sss_correction_static_;
	int id_sss_correction_slider_;
	int id_translucency_;
	int id_translucency_strength_static_;
	int id_translucency_strength_slider_;
};
