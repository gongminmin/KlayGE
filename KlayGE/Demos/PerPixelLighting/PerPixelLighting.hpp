#ifndef _TESTAPP3D_HPP
#define _TESTAPP3D_HPP

#include <KlayGE/App3D.hpp>
#include <KlayGE/Font.hpp>

class PerPixelLighting : public KlayGE::App3DFramework
{
public:
	PerPixelLighting();

private:
	void InitObjects();

	void Update();

	KlayGE::FontPtr font_;
};

#endif		// _TESTAPP3D_HPP
