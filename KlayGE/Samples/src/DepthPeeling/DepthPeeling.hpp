#ifndef _DEPTHPEELING_HPP
#define _DEPTHPEELING_HPP

#include <KlayGE/App3D.hpp>
#include <KlayGE/Font.hpp>
#include <KlayGE/CameraController.hpp>
#include <KlayGE/UI.hpp>
#include <boost/array.hpp>

class DepthPeelingApp : public KlayGE::App3DFramework
{
public:
	DepthPeelingApp(std::string const & name, KlayGE::RenderSettings const & settings);

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
	KlayGE::RenderViewPtr peeled_depth_view_;

	KlayGE::TexturePtr depth_texs_[2];
	KlayGE::RenderViewPtr depth_view_[2];

	boost::array<KlayGE::QueryPtr, 2> oc_queries_;

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
