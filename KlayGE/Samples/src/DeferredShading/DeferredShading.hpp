#ifndef _CARTOON_HPP
#define _CARTOON_HPP

#include <KlayGE/App3D.hpp>
#include <KlayGE/Font.hpp>
#include <KlayGE/CameraController.hpp>
#include <KlayGE/UI.hpp>

class DeferredShadingApp : public KlayGE::App3DFramework
{
public:
	DeferredShadingApp(std::string const & name, KlayGE::RenderSettings const & settings);

private:
	void InitObjects();

	KlayGE::uint32_t DoUpdate(KlayGE::uint32_t pass);

	void OnResize(KlayGE::uint32_t width, KlayGE::uint32_t height);

	void InputHandler(KlayGE::InputEngine const & sender, KlayGE::InputAction const & action);
	void BufferChangedHandler(KlayGE::UIComboBox const & sender);
	void AntiAliasHandler(KlayGE::UICheckBox const & sender);
	void CtrlCameraHandler(KlayGE::UICheckBox const & sender);

	KlayGE::FontPtr font_;
	KlayGE::SceneObjectPtr torus_;

	KlayGE::FirstPersonCameraController fpcController_;

	KlayGE::FrameBufferPtr g_buffer_;
	KlayGE::TexturePtr normal_depth_tex_;
	KlayGE::TexturePtr diffuse_specular_tex_;
	KlayGE::PostProcessPtr deferred_shading_;

	KlayGE::FrameBufferPtr shaded_buffer_;
	KlayGE::TexturePtr shaded_tex_;
	KlayGE::PostProcessPtr edge_anti_alias_;

	KlayGE::UIDialogPtr dialog_;
	int buffer_type_;
	bool anti_alias_enabled_;

	int id_buffer_combo_;
	int id_anti_alias_;
	int id_ctrl_camera_;
};

#endif		// _CARTOON_HPP
