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
#include <KlayGE/SceneObjectHelper.hpp>
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
	virtual void InitObjects() KLAYGE_OVERRIDE;
	virtual void OnResize(uint32_t width, uint32_t height) KLAYGE_OVERRIDE;
	virtual void DoUpdateOverlay();
	virtual KlayGE::uint32_t DoUpdate(KlayGE::uint32_t pass);

	void InputHandler(KlayGE::InputEngine const & sender, KlayGE::InputAction const & action);
	void SSSHandler(KlayGE::UICheckBox const & sender);
	void SSSStrengthChangedHandler(KlayGE::UISlider const & sender);
	void SSSCorrectionChangedHandler(KlayGE::UISlider const & sender);
	void TranslucencyHandler(KlayGE::UICheckBox const & sender);
	void TranslucencyStrengthChangedHandler(KlayGE::UISlider const & sender);

	KlayGE::FontPtr font_;
	KlayGE::TrackballCameraController obj_controller_;
	KlayGE::TrackballCameraController light_controller_;
	KlayGE::SceneObjectHelperPtr subsurface_obj_;

	KlayGE::FrameBufferPtr depth_ls_fb_;
	KlayGE::FrameBufferPtr color_fb_;
	KlayGE::FrameBufferPtr sss_fb_;
	KlayGE::TexturePtr shadow_tex_, shadow_ds_tex_;
	KlayGE::TexturePtr shading_tex_, normal_tex_, albedo_tex_;
	KlayGE::TexturePtr depth_tex_, ds_tex_;

	KlayGE::LightSourcePtr light_;
	KlayGE::SceneObjectLightSourceProxyPtr light_proxy_;
	KlayGE::PostProcessPtr sss_blur_pp_;
	KlayGE::PostProcessPtr translucency_pp_;

	KlayGE::CameraPtr scene_camera_;
	KlayGE::CameraPtr light_camera_;

	KlayGE::PostProcessPtr depth_to_linear_pp_;
	KlayGE::PostProcessPtr copy_pp_;

	bool sss_on_;
	bool translucency_on_;

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
