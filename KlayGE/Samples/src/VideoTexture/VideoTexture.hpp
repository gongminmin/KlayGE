#ifndef _VIDEOTEXTURE_HPP
#define _VIDEOTEXTURE_HPP

#include <KlayGE/App3D.hpp>
#include <KlayGE/Font.hpp>
#include <KlayGE/CameraController.hpp>

class VideoTextureApp : public KlayGE::App3DFramework
{
public:
	VideoTextureApp();

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

	KlayGE::LightSourcePtr light_;
};

#endif		// _VIDEOTEXTURE_HPP
