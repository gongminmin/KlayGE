#ifndef _VERTEXDISPLACEMENT_HPP
#define _VERTEXDISPLACEMENT_HPP

#include <KlayGE/App3D.hpp>
#include <KlayGE/Font.hpp>
#include <KlayGE/CameraController.hpp>

class VertexDisplacement : public KlayGE::App3DFramework
{
public:
	VertexDisplacement(std::string const & name, KlayGE::RenderSettings const & settings);

private:
	void InitObjects();
	void DoUpdate(KlayGE::uint32_t pass);

	void InputHandler(KlayGE::InputEngine const & sender, KlayGE::InputAction const & action);

	KlayGE::FontPtr font_;
	KlayGE::SceneObjectPtr flag_;

	KlayGE::FirstPersonCameraController fpcController_;
};

#endif		// _VERTEXDISPLACEMENT_HPP