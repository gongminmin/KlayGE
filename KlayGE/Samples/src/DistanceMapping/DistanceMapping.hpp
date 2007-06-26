#ifndef _DISTANCEMAPPING_HPP
#define _DISTANCEMAPPING_HPP

#include <KlayGE/App3D.hpp>
#include <KlayGE/Font.hpp>
#include <KlayGE/CameraController.hpp>

class DistanceMapping : public KlayGE::App3DFramework
{
public:
	DistanceMapping(std::string const & name, KlayGE::RenderSettings const & settings);

private:
	void InitObjects();
	void DoUpdate(KlayGE::uint32_t pass);

	void InputHandler(KlayGE::InputEngine const & sender, KlayGE::InputAction const & action);

	KlayGE::FontPtr font_;
	KlayGE::SceneObjectPtr polygon_;

	KlayGE::FirstPersonCameraController fpcController_;
};

#endif		// _DISTANCEMAPPING_HPP
