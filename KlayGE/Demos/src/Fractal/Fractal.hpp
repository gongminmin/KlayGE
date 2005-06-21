#ifndef _FRACTAL_HPP
#define _FRACTAL_HPP

#include <KlayGE/App3D.hpp>
#include <KlayGE/Font.hpp>
#include <KlayGE/CameraController.hpp>

class Fractal : public KlayGE::App3DFramework
{
public:
	Fractal();

private:
	void InitObjects();

	void Update();

	KlayGE::FontPtr font_;
	boost::shared_ptr<KlayGE::Renderable> renderFractal_;

	KlayGE::uint32_t action_map_id_;
};

#endif		// _FRACTAL_HPP
