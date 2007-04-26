#ifndef _CARTOON_HPP
#define _CARTOON_HPP

#include <KlayGE/App3D.hpp>
#include <KlayGE/Font.hpp>
#include <KlayGE/CameraController.hpp>

class Cartoon : public KlayGE::App3DFramework
{
public:
	Cartoon();

private:
	void InitObjects();

	KlayGE::uint32_t NumPasses() const;
	void DoUpdate(KlayGE::uint32_t pass);

	void OnResize(KlayGE::uint32_t width, KlayGE::uint32_t height);

	void InputHandler(KlayGE::InputEngine const & sender, KlayGE::InputAction const & action);

	KlayGE::FontPtr font_;
	KlayGE::SceneObjectPtr torus_;

	KlayGE::FirstPersonCameraController fpcController_;

	KlayGE::FrameBufferPtr normal_depth_buffer_;
	KlayGE::TexturePtr normal_depth_tex_;
};

#endif		// _CARTOON_HPP
