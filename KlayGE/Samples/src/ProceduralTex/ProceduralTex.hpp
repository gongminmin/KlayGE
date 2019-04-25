#ifndef _PARALLAX_HPP
#define _PARALLAX_HPP

#include <KlayGE/App3D.hpp>
#include <KlayGE/Font.hpp>
#include <KlayGE/CameraController.hpp>
#include <KlayGE/UI.hpp>

class ProceduralTexApp : public KlayGE::App3DFramework
{
public:
	ProceduralTexApp();

private:
	void OnCreate();
	void OnResize(KlayGE::uint32_t width, KlayGE::uint32_t height);

	void DoUpdateOverlay();
	KlayGE::uint32_t DoUpdate(KlayGE::uint32_t pass);

	void InputHandler(KlayGE::InputEngine const & sender, KlayGE::InputAction const & action);
	void TypeChangedHandler(KlayGE::UIComboBox const & sender);
	void FreqChangedHandler(KlayGE::UISlider const & sender);

	KlayGE::FontPtr font_;
	KlayGE::RenderModelPtr polygon_model_;

	KlayGE::TrackballCameraController tb_controller_;

	KlayGE::LightSourcePtr light_;

	KlayGE::UIDialogPtr dialog_;
	int procedural_type_;
	float procedural_freq_;

	int id_type_static_;
	int id_type_combo_;
	int id_freq_static_;
	int id_freq_slider_;

	KlayGE::uint32_t loading_percentage_;
};

#endif		// _PARALLAX_HPP
