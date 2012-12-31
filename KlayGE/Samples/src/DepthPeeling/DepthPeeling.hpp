#ifndef _DEPTHPEELING_HPP
#define _DEPTHPEELING_HPP

#include <KlayGE/App3D.hpp>
#include <KlayGE/Font.hpp>
#include <KlayGE/CameraController.hpp>
#include <KlayGE/UI.hpp>

class DepthPeelingApp : public KlayGE::App3DFramework
{
public:
	DepthPeelingApp();

	bool ConfirmDevice() const;

private:
	void InitObjects();
	void OnResize(KlayGE::uint32_t width, KlayGE::uint32_t height);

	void DoUpdateOverlay();
	KlayGE::uint32_t DoUpdate(KlayGE::uint32_t pass);

	void InputHandler(KlayGE::InputEngine const & sender, KlayGE::InputAction const & action);
	void UsePeelingHandler(KlayGE::UICheckBox const & sender);
	void CtrlCameraHandler(KlayGE::UICheckBox const & sender);
	void LayerChangedHandler(KlayGE::UIComboBox const & sender);

	KlayGE::FontPtr font_;
	KlayGE::SceneObjectPtr polygon_;

	KlayGE::FirstPersonCameraController fpcController_;

	std::vector<KlayGE::FrameBufferPtr> peeling_fbs_;
	std::vector<KlayGE::TexturePtr> peeled_texs_;
	std::vector<KlayGE::RenderViewPtr> peeled_views_;
	
	KlayGE::array<KlayGE::TexturePtr, 2> depth_texs_;
	KlayGE::array<KlayGE::RenderViewPtr, 2> depth_view_;

	KlayGE::array<KlayGE::ConditionalRenderPtr, 2> oc_queries_;

	KlayGE::PostProcessPtr blend_pp_;

	KlayGE::uint32_t num_layers_;

	bool use_depth_peeling_;
	KlayGE::UIDialogPtr dialog_peeling_;
	KlayGE::UIDialogPtr dialog_layer_;
	int id_use_depth_peeling_;
	int id_ctrl_camera_;
	int id_layer_combo_;
	int id_layer_tex_;
};

#endif		// _DEPTHPEELING_HPP
