#ifndef _SKINNEDMESH_HPP
#define _SKINNEDMESH_HPP

#include <KlayGE/App3D.hpp>
#include <KlayGE/Font.hpp>
#include <KlayGE/CameraController.hpp>
#include <KlayGE/UI.hpp>
#include "Model.hpp"

class SkinnedMeshApp : public KlayGE::App3DFramework
{
public:
	SkinnedMeshApp(std::string const & name, KlayGE::RenderSettings const & settings);

private:
	void InitObjects();
	void OnResize(KlayGE::uint32_t width, KlayGE::uint32_t height);
	KlayGE::uint32_t DoUpdate(KlayGE::uint32_t pass);

	void OpenModel(std::string const & name);

	void InputHandler(KlayGE::InputEngine const & sender, KlayGE::InputAction const & action);
	void OpenHandler(KlayGE::UIButton const & sender);
	void SkinnedHandler(KlayGE::UICheckBox const & sender);
	void FrameChangedHandler(KlayGE::UISlider const & sender);
	void PlayHandler(KlayGE::UICheckBox const & sender);
	void FPSCameraHandler(KlayGE::UICheckBox const & sender);
	void MeshChangedHandler(KlayGE::UIComboBox const & sender);

	KlayGE::FontPtr font_;

	boost::shared_ptr<DetailedSkinnedModel> model_;
	KlayGE::SceneObjectPtr axis_;
	KlayGE::SceneObjectPtr grid_;

	KlayGE::FirstPersonCameraController fpsController_;
	KlayGE::TrackballCameraController tbController_;

	float last_time_;
	int frame_;

	bool skinned_;
	bool play_;
	KlayGE::UIDialogPtr dialog_;
	int id_open_;
	int id_skinned_;
	int id_frame_static_;
	int id_frame_slider_;
	int id_play_;
	int id_fps_camera_;
	int id_mesh_;
	int id_vertex_streams_;
	int id_textures_;
};

#endif		// _SKINNEDMESH_HPP
