#ifndef _PERPIXELLIGHTING_HPP
#define _PERPIXELLIGHTING_HPP

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

#endif		// _PERPIXELLIGHTING_HPP
