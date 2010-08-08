#ifndef _SUBSURFACE_HPP
#define _SUBSURFACE_HPP

#include <KlayGE/App3D.hpp>
#include <KlayGE/Font.hpp>
#include <KlayGE/CameraController.hpp>
#include <KlayGE/UI.hpp>
#include "Model.hpp"

class SubSurfaceApp : public KlayGE::App3DFramework
{
public:
	SubSurfaceApp();

private:
	void InitObjects();
	void OnResize(KlayGE::uint32_t width, KlayGE::uint32_t height);
	void DoUpdateOverlay();
	KlayGE::uint32_t DoUpdate(KlayGE::uint32_t pass);

	void InputHandler(KlayGE::InputEngine const & sender, KlayGE::InputAction const & action);
	void SigmaChangedHandler(KlayGE::UISlider const & sender);
	void MtlThicknessChangedHandler(KlayGE::UISlider const & sender);

	KlayGE::FontPtr font_;

	KlayGE::SceneObjectPtr model_;

	KlayGE::TrackballCameraController tbController_;

	KlayGE::TexturePtr back_face_depth_tex_;
	KlayGE::FrameBufferPtr back_face_depth_fb_;

	KlayGE::UIDialogPtr dialog_params_;
	int id_sigma_static_;
	int id_sigma_slider_;
	int id_mtl_thickness_static_;
	int id_mtl_thickness_slider_;
};

#endif		// _SUBSURFACE_HPP
