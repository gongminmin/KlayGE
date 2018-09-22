#ifndef _TUTORIAL_TESSELLATION_HPP
#define _TUTORIAL_TESSELLATION_HPP

#include <KlayGE/App3D.hpp>
#include <KlayGE/Font.hpp>
#include <KlayGE/CameraController.hpp>

class TessellationApp : public KlayGE::App3DFramework
{
public:
	TessellationApp();

private:
	void OnCreate();
	void OnResize(KlayGE::uint32_t width, KlayGE::uint32_t height);
	void DoUpdateOverlay();
	KlayGE::uint32_t DoUpdate(KlayGE::uint32_t pass);

	void InputHandler(KlayGE::InputEngine const & sender, KlayGE::InputAction const & action);
	void TessellationOnHandler(KlayGE::UICheckBox const & sender);
	void Edge0ChangedHandler(KlayGE::UISlider const & sender);
	void Edge1ChangedHandler(KlayGE::UISlider const & sender);
	void Edge2ChangedHandler(KlayGE::UISlider const & sender);
	void InsideChangedHandler(KlayGE::UISlider const & sender);

	KlayGE::FontPtr font_;
	KlayGE::SceneNodePtr polygon_;

	KlayGE::UIDialogPtr dialog_;
	KlayGE::float4 tess_factor_;

	int id_tess_enabled_;
	int id_edge0_static_;
	int id_edge0_slider_;
	int id_edge1_static_;
	int id_edge1_slider_;
	int id_edge2_static_;
	int id_edge2_slider_;
	int id_inside_static_;
	int id_inside_slider_;
};

#endif		// _TUTORIAL_TESSELLATION_HPP
