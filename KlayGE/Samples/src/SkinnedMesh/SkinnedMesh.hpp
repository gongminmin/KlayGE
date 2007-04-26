#ifndef _SKINNEDMESH_HPP
#define _SKINNEDMESH_HPP

#include <KlayGE/App3D.hpp>
#include <KlayGE/Font.hpp>
#include <KlayGE/CameraController.hpp>
#include "Model.hpp"

class SkinnedMeshApp : public KlayGE::App3DFramework
{
public:
	SkinnedMeshApp();

private:
	void InitObjects();
	void DoUpdate(KlayGE::uint32_t pass);

	void InputHandler(KlayGE::InputEngine const & sender, KlayGE::InputAction const & action);

	KlayGE::FontPtr font_;

	boost::shared_ptr<MD5SkinnedModel> model_;
	boost::shared_ptr<KlayGE::KeyFramesType> anim_;

	KlayGE::FirstPersonCameraController fpsController_;
};

#endif		// _SKINNEDMESH_HPP
