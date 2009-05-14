#ifndef _INFTERRAIN_HPP
#define _INFTERRAIN_HPP

#include <KlayGE/App3D.hpp>
#include <KlayGE/Font.hpp>
#include <KlayGE/CameraController.hpp>
#include <KlayGE/UI.hpp>

class DepthOfFieldApp : public KlayGE::App3DFramework
{
public:
	DepthOfFieldApp(std::string const & name, KlayGE::RenderSettings const & settings);

private:
	void InitObjects();
	void OnResize(KlayGE::uint32_t width, KlayGE::uint32_t height);

	void DoUpdateOverlay();
	KlayGE::uint32_t DoUpdate(KlayGE::uint32_t pass);

	void InputHandler(KlayGE::InputEngine const & sender, KlayGE::InputAction const & action);
	void FocusPlaneChangedHandler(KlayGE::UISlider const & sender);
	void FocusRangeChangedHandler(KlayGE::UISlider const & sender);
	void BlurFactorHandler(KlayGE::UICheckBox const & sender);
	void UseInstancingHandler(KlayGE::UICheckBox const & sender);
	void CtrlCameraHandler(KlayGE::UICheckBox const & sender);

	KlayGE::FontPtr font_;
	boost::shared_ptr<KlayGE::Renderable> renderInstance_;
	boost::shared_ptr<KlayGE::Renderable> renderMesh_;

	std::vector<KlayGE::SceneObjectPtr> scene_objs_;

	bool use_instance_;

	KlayGE::FirstPersonCameraController fpcController_;

	KlayGE::FrameBufferPtr clr_depth_buffer_;
	KlayGE::TexturePtr clr_depth_tex_;

	KlayGE::PostProcessPtr clear_float_;
	KlayGE::PostProcessPtr depth_of_field_;

	KlayGE::UIDialogPtr dialog_;
	int id_focus_plane_static_;
	int id_focus_plane_slider_;
	int id_focus_range_static_;
	int id_focus_range_slider_;
	int id_blur_factor_;
	int id_use_instancing_;
	int id_ctrl_camera_;

	size_t num_objs_rendered_;
	size_t num_renderable_rendered_;
	size_t num_primitives_rendered_;
	size_t num_vertices_rendered_;
};

#endif		// _INFTERRAIN_HPP
