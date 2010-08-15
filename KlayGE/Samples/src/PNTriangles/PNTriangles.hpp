#ifndef _PNTRIANGLES_HPP
#define _PNTRIANGLES_HPP

#include <KlayGE/App3D.hpp>
#include <KlayGE/Font.hpp>
#include <KlayGE/CameraController.hpp>

class PNTrianglesApp : public KlayGE::App3DFramework
{
public:
	PNTrianglesApp();

	bool ConfirmDevice() const;

private:
	void InitObjects();
	void OnResize(KlayGE::uint32_t width, KlayGE::uint32_t height);
	void DoUpdateOverlay();
	KlayGE::uint32_t DoUpdate(KlayGE::uint32_t pass);

	void InputHandler(KlayGE::InputEngine const & sender, KlayGE::InputAction const & action);

	void TessChangedHandler(KlayGE::UISlider const & sender);
	void LineModeHandler(KlayGE::UICheckBox const & sender);
	void AdaptiveTessHandler(KlayGE::UICheckBox const & sender);
	void EnablePNTrianglesHandler(KlayGE::UICheckBox const & sender);
	void AnimationHandler(KlayGE::UICheckBox const & sender);
	void FPSCameraHandler(KlayGE::UICheckBox const & sender);

	int tess_factor_;
	bool animation_;
	float last_time_;
	int frame_;

	KlayGE::FontPtr font_;
	KlayGE::SceneObjectPtr polygon_;

	KlayGE::FirstPersonCameraController fpcController_;

	KlayGE::UIDialogPtr dialog_params_;
	int id_warning_static_;
	int id_tess_static_;
	int id_tess_slider_;
	int id_line_mode_;
	int id_adaptive_tess_;
	int id_enable_pn_triangles_;
	int id_animation_;
	int id_fps_camera_;
};

#endif		// _PNTRIANGLES_HPP
