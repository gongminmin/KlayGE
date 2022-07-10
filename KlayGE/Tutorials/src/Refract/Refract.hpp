#ifndef _REFRACT_HPP
#define _REFRACT_HPP

#include <KlayGE/App3D.hpp>
#include <KlayGE/Font.hpp>
#include <KlayGE/CameraController.hpp>

class Refract : public KlayGE::App3DFramework
{
public:
	Refract();

private:
	void OnCreate();
	void DoUpdateOverlay();
	KlayGE::uint32_t DoUpdate(KlayGE::uint32_t pass);
	void OnResize(KlayGE::uint32_t width, KlayGE::uint32_t height);

	void InputHandler(KlayGE::InputEngine const & sender, KlayGE::InputAction const & action);

	KlayGE::FontPtr font_;
	KlayGE::RenderModelPtr refractor_model_;
	KlayGE::SceneNodePtr refractor_;
	KlayGE::SceneNodePtr skybox_;

	KlayGE::TrackballCameraController tb_controller_;

	bool depth_texture_support_;
	KlayGE::FrameBufferPtr backface_buffer_;
	KlayGE::FrameBufferPtr backface_depth_buffer_;
	KlayGE::TexturePtr backface_tex_;
	KlayGE::TexturePtr backface_ds_tex_;
	KlayGE::TexturePtr backface_depth_tex_;
	KlayGE::PostProcessPtr depth_to_linear_pp_;
};

#endif		// _REFRACT_HPP
