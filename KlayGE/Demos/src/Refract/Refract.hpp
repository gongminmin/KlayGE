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

	void Update(KlayGE::uint32_t pass);

	KlayGE::FontPtr font_;
	boost::shared_ptr<KlayGE::StaticMesh> refractor_;
	boost::shared_ptr<KlayGE::Renderable> renderSkyBox_;

	KlayGE::FirstPersonCameraController fpcController_;

	KlayGE::uint32_t action_map_id_;

	KlayGE::TexturePtr cube_map_;
};

#endif		// _REFRACT_HPP
