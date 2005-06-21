#ifndef _DISTANCEMAPPING_HPP
#define _DISTANCEMAPPING_HPP

#include <KlayGE/App3D.hpp>
#include <KlayGE/Font.hpp>
#include <KlayGE/CameraController.hpp>

class DistanceMapping : public KlayGE::App3DFramework
{
public:
	DistanceMapping();

private:
	void InitObjects();

	void Update();

	KlayGE::FontPtr font_;
	boost::shared_ptr<KlayGE::Renderable> renderPolygon_;

	KlayGE::FirstPersonCameraController fpcController_;

	KlayGE::uint32_t action_map_id_;
};

#endif		// _DISTANCEMAPPING_HPP
