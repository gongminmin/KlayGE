#ifndef _DEFERREDRENDERING_HPP
#define _DEFERREDRENDERING_HPP

#include <KlayGE/App3D.hpp>
#include <KlayGE/Font.hpp>
#include <KlayGE/CameraController.hpp>
#include <KlayGE/UI.hpp>

#include "DeferredRenderingLayer.hpp"

class DeferredRenderingApp : public KlayGE::App3DFramework
{
public:
	DeferredRenderingApp();

	bool ConfirmDevice() const;

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
	KlayGE::RenderModelPtr scene_model_;
	std::vector<KlayGE::SceneObjectPtr> scene_objs_;
	KlayGE::SceneObjectPtr point_light_src_;
	KlayGE::SceneObjectPtr spot_light_src_[2];

	KlayGE::SceneObjectPtr sky_box_;

	KlayGE::FirstPersonCameraController fpcController_;

	KlayGE::DeferredRenderingLayerPtr deferred_rendering_;

	KlayGE::PostProcessPtr edge_anti_alias_;

	KlayGE::TexturePtr ssao_tex_;
	KlayGE::PostProcessPtr ssao_pp_;

	KlayGE::TexturePtr blur_ssao_tex_;
	KlayGE::PostProcessPtr blur_pp_;

	KlayGE::TexturePtr hdr_tex_;
	KlayGE::HDRPostProcessPtr hdr_pp_;

	KlayGE::PostProcessPtr debug_pp_;

	KlayGE::UIDialogPtr dialog_;
	int buffer_type_;
	bool anti_alias_enabled_;
	bool ssao_enabled_;

	int id_buffer_combo_;
	int id_anti_alias_;
	int id_ssao_;
	int id_ctrl_camera_;

	KlayGE::AmbientLightSourcePtr ambient_light_;
	KlayGE::PointLightSourcePtr point_light_;
	KlayGE::SpotLightSourcePtr spot_light_[2];

	size_t num_objs_rendered_;
	size_t num_renderable_rendered_;
	size_t num_primitives_rendered_;
	size_t num_vertices_rendered_;
};

#endif		// _DEFERREDRENDERING_HPP
