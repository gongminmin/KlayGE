#ifndef _PARALLAX_HPP
#define _PARALLAX_HPP

#include <KlayGE/App3D.hpp>
#include <KlayGE/Font.hpp>
#include <KlayGE/CameraController.hpp>

class Parallax : public KlayGE::App3DFramework
{
public:
	Parallax();

private:
	void InitObjects();
	void Update(KlayGE::uint32_t pass);

	void InputHandler(KlayGE::InputEngine const & sender, KlayGE::InputAction const & action);

	KlayGE::FontPtr font_;
	boost::shared_ptr<KlayGE::Renderable> renderPolygon_;

	KlayGE::FirstPersonCameraController fpcController_;
};

#endif		// _PARALLAX_HPP
