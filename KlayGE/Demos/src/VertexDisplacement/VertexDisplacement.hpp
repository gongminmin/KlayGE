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
	void Update();

	KlayGE::FontPtr font_;

	KlayGE::FirstPersonCameraController fpcController_;
};

#endif		// _VERTEXDISPLACEMENT_HPP