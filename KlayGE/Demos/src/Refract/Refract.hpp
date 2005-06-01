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

	void Update();

	KlayGE::FontPtr font_;

	KlayGE::FirstPersonCameraController fpcController_;

	KlayGE::uint32_t action_map_id_;
};

#endif		// _REFRACT_HPP
