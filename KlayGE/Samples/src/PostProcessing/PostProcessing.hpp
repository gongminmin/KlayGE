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
	void OnCreate();

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
	void SepiaHandler(KlayGE::UIRadioButton const & sender);
	void CrossStitchingHandler(KlayGE::UIRadioButton const & sender);
	void FrostedGlassHandler(KlayGE::UIRadioButton const & sender);
	void BlackHoleHandler(KlayGE::UIRadioButton const & sender);

	KlayGE::FontPtr font_;
	KlayGE::SceneNodePtr sky_box_;

	KlayGE::FirstPersonCameraController fpcController_;

	KlayGE::DeferredRenderingLayer* deferred_rendering_;

	KlayGE::TexturePtr color_tex_;
	KlayGE::FrameBufferPtr color_fb_;
	KlayGE::PostProcessPtr active_pp_;
	KlayGE::PostProcessPtr copy_;
	KlayGE::PostProcessPtr ascii_arts_;
	KlayGE::PostProcessPtr cartoon_;
	KlayGE::PostProcessPtr tiling_;
	KlayGE::PostProcessPtr hdr_;
	KlayGE::PostProcessPtr night_vision_;
	KlayGE::PostProcessPtr sepia_;
	KlayGE::PostProcessPtr cross_stitching_;
	KlayGE::PostProcessPtr frosted_glass_;
	KlayGE::PostProcessPtr black_hole_;

	KlayGE::UIDialogPtr dialog_;
	int id_fps_camera_;
	int id_copy_;
	int id_ascii_arts_;
	int id_cartoon_;
	int id_tiling_;
	int id_hdr_;
	int id_night_vision_;
	int id_old_fashion_;
	int id_cross_stitching_;
	int id_frosted_glass_;
	int id_black_hole_;

	KlayGE::PointLightSourcePtr point_light_;
};

#endif		// _POSTPROCESSING_HPP
