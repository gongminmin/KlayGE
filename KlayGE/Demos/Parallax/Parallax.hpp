#ifndef _PARALLAX_HPP
#define _PARALLAX_HPP

#include <KlayGE/App3D.hpp>
#include <KlayGE/Font.hpp>

class Parallax : public KlayGE::App3DFramework
{
public:
	Parallax();

private:
	void InitObjects();

	void Update();

	KlayGE::FontPtr font_;
};

#endif		// _PARALLAX_HPP
