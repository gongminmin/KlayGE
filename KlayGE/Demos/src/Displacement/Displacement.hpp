#ifndef _DISPLACEMENT_HPP
#define _DISPLACEMENT_HPP

#include <KlayGE/App3D.hpp>
#include <KlayGE/Font.hpp>
#include <KlayGE/CameraController.hpp>

class Displacement : public KlayGE::App3DFramework
{
public:
	Displacement();

private:
	void InitObjects();

	void Update();

	KlayGE::FontPtr font_;

	KlayGE::FirstPersonCameraController fpcController_;
};

#endif		// _DISPLACEMENT_HPP
