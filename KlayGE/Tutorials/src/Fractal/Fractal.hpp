#ifndef _FRACTAL_HPP
#define _FRACTAL_HPP

#include <KlayGE/App3D.hpp>
#include <KlayGE/Font.hpp>
#include <KlayGE/CameraController.hpp>

class Fractal : public KlayGE::App3DFramework
{
public:
	Fractal();

private:
	void OnCreate();
	void OnResize(KlayGE::uint32_t width, KlayGE::uint32_t height);

	void InputHandler(KlayGE::InputEngine const & sender, KlayGE::InputAction const & action);

	void DoUpdateOverlay();
	KlayGE::uint32_t DoUpdate(KlayGE::uint32_t pass);

	KlayGE::FontPtr font_;
	KlayGE::RenderablePtr renderFractal_;
};

#endif		// _FRACTAL_HPP
