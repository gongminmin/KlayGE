#ifndef _METALNESS_HPP
#define _METALNESS_HPP

#include <KlayGE/App3D.hpp>
#include <KlayGE/Font.hpp>
#include <KlayGE/CameraController.hpp>

class MetalnessApp : public KlayGE::App3DFramework
{
public:
	MetalnessApp();

private:
	void OnCreate();
	void DoUpdateOverlay();
	KlayGE::uint32_t DoUpdate(KlayGE::uint32_t pass);
	void OnResize(KlayGE::uint32_t width, KlayGE::uint32_t height);

	void InputHandler(KlayGE::InputEngine const & sender, KlayGE::InputAction const & action);
	void SingleObjectHandler(KlayGE::UICheckBox const & sender);
	void GlossinessChangedHandler(KlayGE::UISlider const & sender);
	void MetalnessChangedHandler(KlayGE::UISlider const & sender);

	KlayGE::FontPtr font_;
	std::vector<KlayGE::SceneObjectPtr> spheres_;
	KlayGE::SceneObjectPtr single_object_;
	KlayGE::SceneObjectPtr sky_box_;

	KlayGE::float3 albedo_;
	float glossiness_;
	float metalness_;

	KlayGE::TrackballCameraController obj_controller_;

	KlayGE::UIDialogPtr dialog_;
	int id_single_object_;
	int id_glossiness_static_;
	int id_glossiness_;
	int id_metalness_static_;
	int id_metalness_;
};

#endif		// _ENVLIGHTING_HPP
