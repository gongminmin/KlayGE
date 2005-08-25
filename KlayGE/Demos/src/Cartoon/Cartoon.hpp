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
	void Update(KlayGE::uint32_t pass);

	KlayGE::FontPtr font_;
	boost::shared_ptr<KlayGE::StaticMesh> renderTorus_;

	KlayGE::FirstPersonCameraController fpcController_;

	KlayGE::uint32_t action_map_id_;
};

#endif		// _CARTOON_HPP
