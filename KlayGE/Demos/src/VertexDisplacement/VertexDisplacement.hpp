#ifndef _VERTEXDISPLACEMENT_HPP
#define _VERTEXDISPLACEMENT_HPP

#include <KlayGE/App3D.hpp>
#include <KlayGE/Font.hpp>
#include <KlayGE/CameraController.hpp>

class VertexDisplacement : public KlayGE::App3DFramework
{
public:
	VertexDisplacement();

private:
	void InitObjects();
	void DoUpdate(KlayGE::uint32_t pass);

	void InputHandler(KlayGE::InputEngine const & sender, KlayGE::InputAction const & action);

	KlayGE::FontPtr font_;
	boost::shared_ptr<KlayGE::Renderable> flag_;

	KlayGE::FirstPersonCameraController fpcController_;
};

#endif		// _VERTEXDISPLACEMENT_HPP