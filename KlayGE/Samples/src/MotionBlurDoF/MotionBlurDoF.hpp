#ifndef _MOTIONBLURDOF_HPP
#define _MOTIONBLURDOF_HPP

#include <KlayGE/App3D.hpp>
#include <KlayGE/Font.hpp>
#include <KlayGE/CameraController.hpp>
#include <KlayGE/UI.hpp>

class MotionBlurDoFApp : public KlayGE::App3DFramework
{
public:
	MotionBlurDoFApp();

private:
	void OnCreate();
	void OnResize(KlayGE::uint32_t width, KlayGE::uint32_t height);

	void DoUpdateOverlay();
	KlayGE::uint32_t DoUpdate(KlayGE::uint32_t pass);

	void InputHandler(KlayGE::InputEngine const & sender, KlayGE::InputAction const & action);
	void DoFOnHandler(KlayGE::UICheckBox const & sender);
	void BokehOnHandler(KlayGE::UICheckBox const & sender);
	void FocusPlaneChangedHandler(KlayGE::UISlider const & sender);
	void FocusRangeChangedHandler(KlayGE::UISlider const & sender);
	void BlurFactorHandler(KlayGE::UICheckBox const & sender);
	void MBOnHandler(KlayGE::UICheckBox const & sender);
	void MotionVecHandler(KlayGE::UICheckBox const & sender);
	void UseInstancingHandler(KlayGE::UICheckBox const & sender);
	void CtrlCameraHandler(KlayGE::UICheckBox const & sender);

	KlayGE::FontPtr font_;
	KlayGE::RenderablePtr renderInstance_;
	KlayGE::RenderablePtr renderMesh_;

	std::vector<KlayGE::SceneObjectPtr> scene_objs_;

	bool use_instance_;

	KlayGE::FirstPersonCameraController fpcController_;

	bool depth_texture_support_;
	KlayGE::PostProcessPtr depth_to_linear_pp_;

	KlayGE::FrameBufferPtr clr_depth_fb_;
	KlayGE::FrameBufferPtr motion_vec_fb_;
	KlayGE::TexturePtr color_tex_;
	KlayGE::TexturePtr ds_tex_;
	KlayGE::TexturePtr depth_tex_;
	KlayGE::TexturePtr motion_vec_tex_;
	KlayGE::TexturePtr dof_tex_;

	KlayGE::PostProcessPtr depth_of_field_;
	KlayGE::PostProcessPtr bokeh_filter_;
	KlayGE::PostProcessPtr motion_blur_;
	KlayGE::PostProcessPtr depth_of_field_copy_pp_;
	KlayGE::PostProcessPtr motion_blur_copy_pp_;

	KlayGE::ScriptModulePtr script_module_;
	KlayGE::RenderModelPtr model_instance_;
	KlayGE::RenderModelPtr model_mesh_;
	KlayGE::uint32_t loading_percentage_;

	bool dof_on_;
	bool bokeh_on_;
	bool mb_on_;

	KlayGE::UIDialogPtr dof_dialog_;
	KlayGE::UIDialogPtr mb_dialog_;
	KlayGE::UIDialogPtr app_dialog_;
	int id_dof_on_;
	int id_bokeh_on_;
	int id_focus_plane_static_;
	int id_focus_plane_slider_;
	int id_focus_range_static_;
	int id_focus_range_slider_;
	int id_blur_factor_;
	int id_mb_on_;
	int id_motion_vec_;
	int id_use_instancing_;
	int id_ctrl_camera_;

	size_t num_objs_rendered_;
	size_t num_renderables_rendered_;
	size_t num_primitives_rendered_;
	size_t num_vertices_rendered_;
};

#endif		// _MOTIONBLURDOF_HPP
