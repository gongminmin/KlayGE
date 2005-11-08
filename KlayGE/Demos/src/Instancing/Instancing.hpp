#ifndef _INSTANCING_HPP
#define _INSTANCING_HPP

#include <KlayGE/App3D.hpp>
#include <KlayGE/Font.hpp>
#include <KlayGE/CameraController.hpp>

class Instancing : public KlayGE::App3DFramework
{
public:
	Instancing();

private:
	void InitObjects();
	void DoUpdate(KlayGE::uint32_t pass);

	void InputHandler(KlayGE::InputEngine const & sender, KlayGE::InputAction const & action);

	KlayGE::FontPtr font_;
	boost::shared_ptr<KlayGE::Renderable> renderInstance_;
	boost::shared_ptr<KlayGE::Renderable> renderMesh_;

	std::vector<KlayGE::SceneObjectPtr> scene_objs_;

	bool use_instance_;

	KlayGE::VertexBufferPtr vb_;

	KlayGE::FirstPersonCameraController fpcController_;
};

#endif		// _INSTANCING_HPP
