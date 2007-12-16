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

	KlayGE::uint32_t DoUpdate(KlayGE::uint32_t pass);

	void InputHandler(KlayGE::InputEngine const & sender, KlayGE::InputAction const & action);
	void FocusPlaneChangedHandler(KlayGE::UISlider const & sender);
	void FocusRangeChangedHandler(KlayGE::UISlider const & sender);
	void BlurFactorHandler(KlayGE::UICheckBox const & sender);
	void CtrlCameraHandler(KlayGE::UICheckBox const & sender);

	KlayGE::FontPtr font_;

	KlayGE::FirstPersonCameraController fpcController_;

	KlayGE::FrameBufferPtr clr_depth_buffer_;
	KlayGE::TexturePtr clr_depth_tex_;

	KlayGE::PostProcessPtr clear_float_;
	KlayGE::PostProcessPtr depth_of_field_;

	KlayGE::UIDialogPtr dialog_;
};

#endif		// _INFTERRAIN_HPP
