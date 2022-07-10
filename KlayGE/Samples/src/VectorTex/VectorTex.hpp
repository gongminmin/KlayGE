#ifndef _VECTOR_TEX_HPP
#define _VECTOR_TEX_HPP

#include <KlayGE/App3D.hpp>
#include <KlayGE/Font.hpp>
#include <KlayGE/CameraController.hpp>

class VectorTexApp : public KlayGE::App3DFramework
{
public:
	VectorTexApp();

private:
	void OnCreate();
	void OnResize(KlayGE::uint32_t width, KlayGE::uint32_t height);

	void DoUpdateOverlay();
	KlayGE::uint32_t DoUpdate(KlayGE::uint32_t pass);

	void InputHandler(KlayGE::InputEngine const & sender, KlayGE::InputAction const & action);

	KlayGE::FontPtr font_;

	KlayGE::SceneNodePtr object_;
	KlayGE::RenderModelPtr model_;

	KlayGE::TrackballCameraController tb_controller_;
};

#endif		// _VECTOR_TEX_HPP
