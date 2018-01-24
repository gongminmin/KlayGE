#ifndef KLAYGE_SAMPLES_OIT_HPP
#define KLAYGE_SAMPLES_OIT_HPP

#include <KlayGE/App3D.hpp>
#include <KlayGE/Font.hpp>
#include <KlayGE/CameraController.hpp>
#include <KlayGE/UI.hpp>

enum OITMode
{
	OM_No = 0,
	OM_DepthPeeling,
	OM_PerPixelLinkedLists,
	OM_AdaptiveTransparency
};

class OITApp : public KlayGE::App3DFramework
{
public:
	OITApp();

private:
	void OnCreate();
	void OnResize(KlayGE::uint32_t width, KlayGE::uint32_t height);

	void DoUpdateOverlay();
	KlayGE::uint32_t DoUpdate(KlayGE::uint32_t pass);

	void InputHandler(KlayGE::InputEngine const & sender, KlayGE::InputAction const & action);
	void OITModeHandler(KlayGE::UIComboBox const & sender);
	void AlphaHandler(KlayGE::UISlider const & sender);
	void LayerChangedHandler(KlayGE::UIComboBox const & sender);

	KlayGE::FontPtr font_;
	KlayGE::SceneObjectPtr polygon_;
	KlayGE::SceneObjectPtr sky_box_;

	KlayGE::TrackballCameraController tb_controller_;

	bool depth_texture_support_;

	// Depth Peeling
	std::vector<KlayGE::FrameBufferPtr> peeling_fbs_;
	std::vector<KlayGE::TexturePtr> peeled_texs_;
	std::array<KlayGE::FrameBufferPtr, 2> depth_fbs_;
	std::array<KlayGE::TexturePtr, 2> depth_texs_;
	std::array<KlayGE::RenderViewPtr, 2> depth_views_;
	std::array<KlayGE::ConditionalRenderPtr, 2> oc_queries_;
	KlayGE::PostProcessPtr blend_pp_;
	KlayGE::uint32_t num_layers_;

	// Per Pixel Linked Lists
	KlayGE::FrameBufferPtr opaque_bg_fb_;
	KlayGE::TexturePtr opaque_bg_tex_;
	// Per Pixel Linked Lists & Adaptive Transparency
	KlayGE::FrameBufferPtr linked_list_fb_;
	KlayGE::GraphicsBufferPtr frag_link_buf_;
	KlayGE::GraphicsBufferPtr start_offset_buf_;
	KlayGE::UnorderedAccessViewPtr frag_link_uav_;
	KlayGE::UnorderedAccessViewPtr start_offset_uav_;

	OITMode oit_mode_;
	KlayGE::UIDialogPtr dialog_oit_;
	KlayGE::UIDialogPtr dialog_layer_;
	int id_oit_mode_;
	int id_alpha_static_;
	int id_alpha_slider_;
	int id_layer_combo_;
	int id_layer_tex_;
};

#endif		// KLAYGE_SAMPLES_OIT_HPP
