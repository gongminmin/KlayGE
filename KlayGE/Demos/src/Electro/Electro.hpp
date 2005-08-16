#ifndef _ELECTRO_HPP
#define _ELECTRO_HPP

#include <KlayGE/App3D.hpp>
#include <KlayGE/Font.hpp>
#include <KlayGE/CameraController.hpp>

class Electro : public KlayGE::App3DFramework
{
public:
	Electro();

private:
	void InitObjects();

	void Update(KlayGE::uint32_t pass);

	KlayGE::FontPtr font_;
	boost::shared_ptr<KlayGE::Renderable> renderElectro_;

	KlayGE::uint32_t action_map_id_;
};

#endif		// _ELECTRO_HPP
