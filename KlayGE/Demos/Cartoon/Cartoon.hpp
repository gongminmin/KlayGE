#ifndef _TESTAPP3D_HPP
#define _TESTAPP3D_HPP

#include <KlayGE/App3D.hpp>
#include <KlayGE/Font.hpp>

class Cartoon : public KlayGE::App3DFramework
{
public:
	Cartoon();

private:
	void InitObjects();
	void Update();

	KlayGE::FontPtr font_;
	float rotX, rotY;
};

#endif		// _TESTAPP3D_HPP