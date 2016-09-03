#ifndef _TEXT_HPP
#define _TEXT_HPP

#include <KlayGE/App3D.hpp>
#include <KlayGE/Font.hpp>
#include <KlayGE/CameraController.hpp>
#include <KlayGE/UI.hpp>

class TextApp : public KlayGE::App3DFramework
{
public:
	TextApp();

private:
	void OnCreate();
	void OnResize(KlayGE::uint32_t width, KlayGE::uint32_t height);

	void DoUpdateOverlay();
	KlayGE::uint32_t DoUpdate(KlayGE::uint32_t pass);

	void InputHandler(KlayGE::InputEngine const & sender, KlayGE::InputAction const & action);

	KlayGE::FontPtr font_;
	std::wstring text_;

	KlayGE::int2 last_mouse_pt_;
	KlayGE::float2 position_;
	float scale_;
};

#endif		// _TEXT_HPP
