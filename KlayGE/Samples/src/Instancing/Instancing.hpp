#ifndef _INSTANCING_HPP
#define _INSTANCING_HPP

#include <KlayGE/App3D.hpp>
#include <KlayGE/Font.hpp>
#include <KlayGE/CameraController.hpp>
#include <KlayGE/UI.hpp>

class Instancing : public KlayGE::App3DFramework
{
public:
	Instancing(std::string const & name, KlayGE::RenderSettings const & settings);

private:
	void InitObjects();
	void DoUpdate(KlayGE::uint32_t pass);

	void InputHandler(KlayGE::InputEngine const & sender, KlayGE::InputAction const & action);
	void CheckBoxHandler(KlayGE::UICheckBox const & sender);

	KlayGE::FontPtr font_;
	boost::shared_ptr<KlayGE::Renderable> renderInstance_;
	boost::shared_ptr<KlayGE::Renderable> renderMesh_;

	std::vector<KlayGE::SceneObjectPtr> scene_objs_;

	bool use_instance_;

	KlayGE::FirstPersonCameraController fpcController_;

	KlayGE::UIDialogPtr dialog_;
};

#endif		// _INSTANCING_HPP
