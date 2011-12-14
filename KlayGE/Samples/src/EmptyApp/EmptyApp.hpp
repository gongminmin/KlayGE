#ifndef _EMPTYAPP_HPP
#define _EMPTYAPP_HPP

#include <KlayGE/App3D.hpp>
#include <KlayGE/Font.hpp>
#include <KlayGE/CameraController.hpp>
#include <KlayGE/UI.hpp>

class EmptyApp : public KlayGE::App3DFramework
{
public:
	EmptyApp();

	bool ConfirmDevice() const;

private:
	void DoUpdateOverlay();
	KlayGE::uint32_t DoUpdate(KlayGE::uint32_t pass);
};

#endif		// _EMPTYAPP_HPP
