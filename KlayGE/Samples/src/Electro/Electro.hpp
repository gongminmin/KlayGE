#ifndef _ELECTRO_HPP
#define _ELECTRO_HPP

#include <KlayGE/App3D.hpp>
#include <KlayGE/Font.hpp>
#include <KlayGE/CameraController.hpp>

class Electro : public KlayGE::App3DFramework
{
public:
	Electro(std::string const & name, KlayGE::RenderSettings const & settings);

private:
	void InitObjects();
	KlayGE::uint32_t DoUpdate(KlayGE::uint32_t pass);

	KlayGE::FontPtr font_;
	boost::shared_ptr<KlayGE::Renderable> renderElectro_;
};

#endif		// _ELECTRO_HPP
