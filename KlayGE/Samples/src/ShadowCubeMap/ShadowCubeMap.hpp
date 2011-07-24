#ifndef _SHADOWCUBEMAP_HPP
#define _SHADOWCUBEMAP_HPP

#include <KlayGE/App3D.hpp>
#include <KlayGE/Font.hpp>
#include <KlayGE/CameraController.hpp>
#include <KlayGE/UI.hpp>

enum SM_TYPE
{
	SMT_DP,
	SMT_Cube,
	SMT_CubeOne
};
	
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
	void SMTypeChangedHandler(KlayGE::UIComboBox const & sender);
	void CtrlCameraHandler(KlayGE::UICheckBox const & sender);

	KlayGE::FontPtr font_;

	std::vector<KlayGE::SceneObjectPtr> scene_objs_;

	KlayGE::FirstPersonCameraController fpcController_;

	KlayGE::FrameBufferPtr shadow_cube_buffer_;
	KlayGE::TexturePtr shadow_tex_;
	KlayGE::TexturePtr shadow_cube_tex_;
	KlayGE::PostProcessPtr sm_filter_pps_[6];

	KlayGE::FrameBufferPtr shadow_cube_one_buffer_;
	KlayGE::TexturePtr shadow_cube_one_tex_;

	KlayGE::FrameBufferPtr shadow_dual_buffers_[2];
	KlayGE::TexturePtr shadow_dual_texs_[2];
	KlayGE::RenderViewPtr shadow_dual_view_[2];
	KlayGE::TexturePtr shadow_dual_tex_;

	KlayGE::TexturePtr lamp_tex_;

	KlayGE::SceneObjectPtr light_proxy_;
	KlayGE::LightSourcePtr light_;

	SM_TYPE sm_type_;

	KlayGE::UIDialogPtr dialog_;
	int id_min_variance_static_;
	int id_min_variance_slider_;
	int id_bleeding_reduce_static_;
	int id_bleeding_reduce_slider_;
	int id_sm_type_static_;
	int id_sm_type_combo_;
	int id_ctrl_camera_;
};

#endif		// _SHADOWCUBEMAP_HPP
