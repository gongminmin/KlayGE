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
	void Update();

	KlayGE::FontPtr font_;

	KlayGE::FirstPersonCameraController fpcController_;
};

#endif		// _CARTOON_HPP
