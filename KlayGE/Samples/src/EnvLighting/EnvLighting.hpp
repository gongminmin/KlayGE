#ifndef _ENVLIGHTING_HPP
#define _ENVLIGHTING_HPP

#include <KlayGE/App3D.hpp>
#include <KlayGE/Font.hpp>
#include <KlayGE/CameraController.hpp>

class EnvLightingApp : public KlayGE::App3DFramework
{
public:
	EnvLightingApp();

private:
	void OnCreate();
	void DoUpdateOverlay();
	KlayGE::uint32_t DoUpdate(KlayGE::uint32_t pass);
	void OnResize(KlayGE::uint32_t width, KlayGE::uint32_t height);

	void InputHandler(KlayGE::InputEngine const & sender, KlayGE::InputAction const & action);
	void TypeChangedHandler(KlayGE::UIComboBox const & sender);

	KlayGE::FontPtr font_;
	std::vector<KlayGE::SceneObjectPtr> spheres_;
	KlayGE::SceneObjectPtr sky_box_;

	KlayGE::TrackballCameraController obj_controller_;

	KlayGE::TexturePtr integrate_brdf_tex_;

	KlayGE::UIDialogPtr dialog_;
	int rendering_type_;
	int id_type_combo_;
};

#endif		// _ENVLIGHTING_HPP
