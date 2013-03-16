#ifndef _DEFERREDRENDERING_HPP
#define _DEFERREDRENDERING_HPP

#include <KlayGE/App3D.hpp>
#include <KlayGE/Font.hpp>
#include <KlayGE/CameraController.hpp>
#include <KlayGE/UI.hpp>

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
	void IllumChangedHandler(KlayGE::UIComboBox const & sender);
	void ILScaleChangedHandler(KlayGE::UISlider const & sender);
	void SSVOHandler(KlayGE::UICheckBox const & sender);
	void HDRHandler(KlayGE::UICheckBox const & sender);
	void AntiAliasHandler(KlayGE::UICheckBox const & sender);
	void CtrlCameraHandler(KlayGE::UICheckBox const & sender);

	KlayGE::FontPtr font_;
	KlayGE::RenderModelPtr scene_model_;
	std::vector<KlayGE::SceneObjectPtr> scene_objs_;
	KlayGE::SceneObjectPtr point_light_src_;
	KlayGE::SceneObjectPtr spot_light_src_[3];

	KlayGE::SceneObjectPtr sky_box_;

	KlayGE::function<KlayGE::RenderModelPtr()> model_ml_;
	KlayGE::function<KlayGE::TexturePtr()> y_cube_tl_;
	KlayGE::function<KlayGE::TexturePtr()> c_cube_tl_;
	uint32_t loading_percentage_;

	KlayGE::FirstPersonCameraController fpcController_;

	KlayGE::DeferredRenderingLayerPtr deferred_rendering_;

	KlayGE::PostProcessPtr debug_pp_;

	KlayGE::UIDialogPtr dialog_;
	int buffer_type_;
	bool ssvo_enabled_;
	bool hdr_enabled_;
	int anti_alias_enabled_;

	float il_scale_;

	int id_buffer_combo_;
	int id_illum_combo_;
	int id_il_scale_static_;
	int id_il_scale_slider_;
	int id_ssvo_;
	int id_hdr_;
	int id_aa_;
	int id_ctrl_camera_;

	KlayGE::PointLightSourcePtr point_light_;
	KlayGE::SpotLightSourcePtr spot_light_[3];

	size_t num_objs_rendered_;
	size_t num_renderable_rendered_;
	size_t num_primitives_rendered_;
	size_t num_vertices_rendered_;
};

#endif		// _DEFERREDRENDERING_HPP
