#ifndef _SHADOWCUBEMAP_HPP
#define _SHADOWCUBEMAP_HPP

#include <KlayGE/App3D.hpp>
#include <KlayGE/Font.hpp>
#include <KlayGE/CameraController.hpp>
#include <KlayGE/UI.hpp>

class ShadowCubeMap : public KlayGE::App3DFramework
{
public:
	ShadowCubeMap();

	bool ConfirmDevice() const;

private:
	void InitObjects();
	void OnResize(KlayGE::uint32_t width, KlayGE::uint32_t height);

	void DoUpdateOverlay();
	KlayGE::uint32_t DoUpdate(KlayGE::uint32_t pass);

	void InputHandler(KlayGE::InputEngine const & sender, KlayGE::InputAction const & action);
	void MinVarianceChangedHandler(KlayGE::UISlider const & sender);
	void BleedingReduceChangedHandler(KlayGE::UISlider const & sender);
	void CtrlCameraHandler(KlayGE::UICheckBox const & sender);

	KlayGE::FontPtr font_;

	KlayGE::SceneObjectPtr ground_;
	KlayGE::SceneObjectPtr mesh_;

	KlayGE::FirstPersonCameraController fpcController_;

	KlayGE::FrameBufferPtr shadow_buffer_;
	KlayGE::TexturePtr shadow_tex_;
	KlayGE::TexturePtr shadow_cube_tex_;
	KlayGE::PostProcessPtr sm_filter_pps_[6];

	KlayGE::TexturePtr lamp_tex_;

	KlayGE::SceneObjectPtr light_proxy_;
	KlayGE::PointLightSourcePtr light_;

	KlayGE::UIDialogPtr dialog_;
	int id_min_variance_static_;
	int id_min_variance_slider_;
	int id_bleeding_reduce_static_;
	int id_bleeding_reduce_slider_;
	int id_ctrl_camera_;
};

#endif		// _SHADOWCUBEMAP_HPP
