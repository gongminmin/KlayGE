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

	void DoUpdateOverlay();
	KlayGE::uint32_t DoUpdate(KlayGE::uint32_t pass);

	void OnResize(KlayGE::uint32_t width, KlayGE::uint32_t height);

	void InputHandler(KlayGE::InputEngine const & sender, KlayGE::InputAction const & action);
	void BufferChangedHandler(KlayGE::UIComboBox const & sender);
	void AntiAliasHandler(KlayGE::UICheckBox const & sender);
	void SSAOHandler(KlayGE::UICheckBox const & sender);
	void CtrlCameraHandler(KlayGE::UICheckBox const & sender);

	KlayGE::FontPtr font_;
	KlayGE::SceneObjectPtr torus_;
	KlayGE::SceneObjectPtr point_light_src_;
	KlayGE::SceneObjectPtr spot_light_src_[2];

	KlayGE::SceneObjectPtr sky_box_;

	KlayGE::FirstPersonCameraController fpcController_;

	KlayGE::FrameBufferPtr g_buffer_;
	KlayGE::TexturePtr normal_depth_tex_;
	KlayGE::TexturePtr diffuse_specular_tex_;
	KlayGE::PostProcessPtr deferred_shading_;

	KlayGE::FrameBufferPtr shaded_buffer_;
	KlayGE::TexturePtr shaded_tex_;
	KlayGE::PostProcessPtr edge_anti_alias_;

	KlayGE::FrameBufferPtr ssao_buffer_;
	KlayGE::TexturePtr ssao_tex_;
	KlayGE::PostProcessPtr ssao_pp_;

	KlayGE::FrameBufferPtr blur_ssao_buffer_;
	KlayGE::TexturePtr blur_ssao_tex_;
	KlayGE::PostProcessPtr blur_pp_;

	KlayGE::FrameBufferPtr hdr_buffer_;
	KlayGE::TexturePtr hdr_tex_;
	KlayGE::HDRPostProcessPtr hdr_pp_;

	KlayGE::UIDialogPtr dialog_;
	int buffer_type_;
	bool anti_alias_enabled_;
	bool ssao_enabled_;

	int id_buffer_combo_;
	int id_anti_alias_;
	int id_ssao_;
	int id_ctrl_camera_;

	int ambient_light_id_;
	int point_light_id_;
	int spot_light_id_[2];;
};

#endif		// _CARTOON_HPP
