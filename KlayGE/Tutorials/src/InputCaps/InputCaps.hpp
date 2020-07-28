#ifndef _DISTANCEMAPPING_HPP
#define _DISTANCEMAPPING_HPP

#include <KlayGE/App3D.hpp>
#include <KlayGE/Font.hpp>
#include <KlayGE/CameraController.hpp>

class InputCaps : public KlayGE::App3DFramework
{
public:
	InputCaps();

private:
	void OnCreate();
	void OnResize(KlayGE::uint32_t width, KlayGE::uint32_t height);
	void DoUpdateOverlay();
	KlayGE::uint32_t DoUpdate(KlayGE::uint32_t pass);

	void InputHandler(KlayGE::InputEngine const & sender, KlayGE::InputAction const & action);

	KlayGE::FontPtr font_;

	std::wstring key_str_;
	std::wstring mouse_str_;
	std::wstring joystick_str_;
	std::wstring touch_str_;
	std::wstring sensor_str_;

	KlayGE::InputJoystickPtr joystick_;
};

#endif		// _DISTANCEMAPPING_HPP
