#ifndef _POSTPROCESSING_HPP
#define _POSTPROCESSING_HPP

#include <KlayGE/App3D.hpp>
#include <KlayGE/Font.hpp>
#include <KlayGE/CameraController.hpp>
#include <KlayGE/UI.hpp>

class PostProcessingApp : public KlayGE::App3DFramework
{
public:
	PostProcessingApp();

private:
	void InitObjects();

	void DoUpdateOverlay();
	KlayGE::uint32_t DoUpdate(KlayGE::uint32_t pass);

	void OnResize(KlayGE::uint32_t width, KlayGE::uint32_t height);

	void InputHandler(KlayGE::InputEngine const & sender, KlayGE::InputAction const & action);
	void FPSCameraHandler(KlayGE::UICheckBox const & sender);
	void CopyHandler(KlayGE::UIRadioButton const & sender);
	void AsciiArtsHandler(KlayGE::UIRadioButton const & sender);
	void CartoonHandler(KlayGE::UIRadioButton const & sender);
	void TilingHandler(KlayGE::UIRadioButton const & sender);
	void HDRHandler(KlayGE::UIRadioButton const & sender);
	void NightVisionHandler(KlayGE::UIRadioButton const & sender);
	void OldFashionHandler(KlayGE::UIRadioButton const & sender);

	KlayGE::FontPtr font_;
	KlayGE::SceneObjectPtr torus_;
	KlayGE::SceneObjectPtr sky_box_;

	KlayGE::FirstPersonCameraController fpcController_;

	KlayGE::FrameBufferPtr g_buffer_;
	KlayGE::TexturePtr normal_depth_tex_;
	KlayGE::TexturePtr color_tex_;
	KlayGE::PostProcessPtr active_pp_;
	KlayGE::PostProcessPtr copy_;
	KlayGE::PostProcessPtr ascii_arts_;
	KlayGE::PostProcessPtr cartoon_;
	KlayGE::PostProcessPtr tiling_;
	KlayGE::PostProcessPtr hdr_;
	KlayGE::PostProcessPtr night_vision_;
	KlayGE::PostProcessPtr old_fashion_;

	KlayGE::UIDialogPtr dialog_;
	int id_fps_camera_;
	int id_copy_;
	int id_ascii_arts_;
	int id_cartoon_;
	int id_tiling_;
	int id_hdr_;
	int id_night_vision_;
	int id_old_fashion_;
};

#endif		// _POSTPROCESSING_HPP
