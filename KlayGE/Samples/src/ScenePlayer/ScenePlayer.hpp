#ifndef _SCENEPLAYER_HPP
#define _SCENEPLAYER_HPP

#include <KlayGE/App3D.hpp>
#include <KlayGE/Font.hpp>
#include <KlayGE/CameraController.hpp>
#include <KlayGE/UI.hpp>
#include <KlayGE/PostProcess.hpp>
#include <KlayGE/Script.hpp>

class ScenePlayerApp : public KlayGE::App3DFramework
{
public:
	ScenePlayerApp();

private:
	void OnCreate();
	void OnResize(KlayGE::uint32_t width, KlayGE::uint32_t height);

	void DoUpdateOverlay();
	KlayGE::uint32_t DoUpdate(KlayGE::uint32_t pass);

	void LoadScene(std::string const & name);

	void InputHandler(KlayGE::InputEngine const & sender, KlayGE::InputAction const & action);
	void OpenHandler(KlayGE::UIButton const & sender);
	void IllumChangedHandler(KlayGE::UIComboBox const & sender);
	void ILScaleChangedHandler(KlayGE::UISlider const & sender);
	void SSGIHandler(KlayGE::UICheckBox const & sender);
	void SSVOHandler(KlayGE::UICheckBox const & sender);
	void HDRHandler(KlayGE::UICheckBox const & sender);
	void AAHandler(KlayGE::UICheckBox const & sender);
	void ColorGradingHandler(KlayGE::UICheckBox const & sender);
	void CtrlCameraHandler(KlayGE::UICheckBox const & sender);

	KlayGE::FontPtr font_;

	std::vector<KlayGE::RenderModelPtr> scene_models_;
	std::vector<KlayGE::SceneNodePtr> scene_objs_;
	KlayGE::SceneNodePtr sky_box_;

	std::vector<KlayGE::LightSourcePtr> lights_;
	std::vector<KlayGE::SceneNodePtr> light_proxies_;

	KlayGE::FirstPersonCameraController fpcController_;

	KlayGE::DeferredRenderingLayer* deferred_rendering_;

	KlayGE::UIDialogPtr dialog_;

	float il_scale_;

	int id_open_;
	int id_illum_combo_;
	int id_il_scale_static_;
	int id_il_scale_slider_;
	int id_ssgi_;
	int id_ssvo_;
	int id_hdr_;
	int id_aa_;
	int id_cg_;
	int id_ctrl_camera_;

	std::string last_file_path_;
};

#endif		// _SCENEPLAYER_HPP
