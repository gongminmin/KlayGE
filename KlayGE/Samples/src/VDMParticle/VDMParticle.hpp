#ifndef _VDMPARTICLE_HPP
#define _VDMPARTICLE_HPP

#include <KlayGE/App3D.hpp>
#include <KlayGE/Font.hpp>
#include <KlayGE/CameraController.hpp>
#include <KlayGE/UI.hpp>

enum ParticleRenderingType
{
	PRT_FullRes,
	PRT_NaiveHalfRes,
	PRT_NaiveQuarterRes,
	PRT_VDMQuarterRes
};
	
class VDMParticleApp : public KlayGE::App3DFramework
{
public:
	VDMParticleApp();

	bool ConfirmDevice() const override;

private:
	void OnCreate();
	void OnResize(KlayGE::uint32_t width, KlayGE::uint32_t height);

	void DoUpdateOverlay();
	KlayGE::uint32_t DoUpdate(KlayGE::uint32_t pass);

	void InputHandler(KlayGE::InputEngine const & sender, KlayGE::InputAction const & action);
	void ParticleRenderingTypeChangedHandler(KlayGE::UIComboBox const & sender);
	void CtrlCameraHandler(KlayGE::UICheckBox const & sender);

	KlayGE::FontPtr font_;
	
	std::vector<KlayGE::SceneNodePtr> scene_objs_;
	KlayGE::LightSourcePtr light_;

	KlayGE::ParticleEmitterPtr particle_emitter_;
	KlayGE::ParticleUpdaterPtr particle_updater_;
	KlayGE::ParticleSystemPtr ps_;

	KlayGE::TexturePtr scene_tex_;
	KlayGE::TexturePtr scene_depth_tex_;
	KlayGE::TexturePtr scene_ds_tex_;
	KlayGE::FrameBufferPtr scene_fb_;
	KlayGE::PostProcessPtr depth_to_linear_pp_;
	KlayGE::PostProcessPtr copy_pp_;
	KlayGE::PostProcessPtr add_copy_pp_;
	KlayGE::PostProcessPtr vdm_composition_pp_;

	KlayGE::FirstPersonCameraController fpc_controller_;

	std::vector<KlayGE::TexturePtr> low_res_color_texs_;
	std::vector<KlayGE::TexturePtr> low_res_max_ds_texs_;
	std::vector<KlayGE::RenderViewPtr> low_res_max_ds_views_;

	KlayGE::FrameBufferPtr half_res_fb_;

	KlayGE::FrameBufferPtr quarter_res_fb_;

	KlayGE::FrameBufferPtr vdm_quarter_res_fb_;
	KlayGE::TexturePtr vdm_transition_tex_;
	KlayGE::TexturePtr vdm_count_tex_;

	KlayGE::PostProcessPtr depth_to_max_pp_;
	KlayGE::PostProcessPtr copy_to_depth_pp_;

	ParticleRenderingType particle_rendering_type_;

	KlayGE::UIDialogPtr dialog_;
	int id_particle_rendering_type_static_;
	int id_particle_rendering_type_combo_;
	int id_ctrl_camera_;
};

#endif		// _VDMPARTICLE_HPP
