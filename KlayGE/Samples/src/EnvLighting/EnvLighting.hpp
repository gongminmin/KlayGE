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
	KlayGE::SceneNodePtr sphere_group_;
	std::vector<KlayGE::RenderModelPtr> sphere_models_;

	KlayGE::TrackballCameraController obj_controller_;

	KlayGE::TexturePtr integrated_brdf_tex_;

	KlayGE::UIDialogPtr dialog_;
	int rendering_type_;
	int id_type_combo_;

	float distance_ = 0.8f;
};

#endif		// _ENVLIGHTING_HPP
