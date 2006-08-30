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
	void InitObjects();
	KlayGE::uint32_t Refract::NumPasses() const;
	void DoUpdate(KlayGE::uint32_t pass);
	void OnResize(KlayGE::uint32_t width, KlayGE::uint32_t height);

	void InputHandler(KlayGE::InputEngine const & sender, KlayGE::InputAction const & action);

	KlayGE::FontPtr font_;
	KlayGE::SceneObjectPtr refractor_;
	KlayGE::SceneObjectPtr sky_box_;

	KlayGE::FirstPersonCameraController fpcController_;

	KlayGE::TexturePtr y_cube_map_;
	KlayGE::TexturePtr c_cube_map_;

	KlayGE::FrameBufferPtr render_buffer_;
	KlayGE::TexturePtr rendered_tex_;

	KlayGE::HDRPostProcessPtr hdr_;

	KlayGE::FrameBufferPtr back_face_buffer_;
	KlayGE::TexturePtr back_face_tex_;
};

#endif		// _REFRACT_HPP
