#ifndef _PARTICLEEDITOR_HPP
#define _PARTICLEEDITOR_HPP

#include <KlayGE/PreDeclare.hpp>
#include <KlayGE/App3D.hpp>
#include <KlayGE/Font.hpp>
#include <KlayGE/CameraController.hpp>
#include <KlayGE/ParticleSystem.hpp>
#include <KlayGE/PostProcess.hpp>
#include <KlayGE/UI.hpp>

class ParticleEditorApp : public KlayGE::App3DFramework
{
public:
	ParticleEditorApp();

private:
	void OnCreate();
	void OnResize(KlayGE::uint32_t width, KlayGE::uint32_t height);
	void DoUpdateOverlay();
	KlayGE::uint32_t DoUpdate(KlayGE::uint32_t pass);

	void InputHandler(KlayGE::InputEngine const & sender, KlayGE::InputAction const & action);
	void OpenHandler(KlayGE::UIButton const & sender);
	void SaveAsHandler(KlayGE::UIButton const & sender);
	void FreqChangedHandler(KlayGE::UISlider const & sender);
	void AngleChangedHandler(KlayGE::UISlider const & sender);
	void DetailXChangedHandler(KlayGE::UISlider const & sender);
	void DetailYChangedHandler(KlayGE::UISlider const & sender);
	void DetailZChangedHandler(KlayGE::UISlider const & sender);
	void MinVelocityChangedHandler(KlayGE::UISlider const & sender);
	void MaxVelocityChangedHandler(KlayGE::UISlider const & sender);
	void MinLifeChangedHandler(KlayGE::UISlider const & sender);
	void MaxLifeChangedHandler(KlayGE::UISlider const & sender);
	void GravityChangedHandler(KlayGE::UISlider const & sender);
	void DensityChangedHandler(KlayGE::UISlider const & sender);
	void FPSCameraHandler(KlayGE::UICheckBox const & sender);
	void ChangeParticleAlphaFromHandler(KlayGE::UITexButton const & sender);
	void ChangeParticleAlphaToHandler(KlayGE::UITexButton const & sender);
	void ChangeParticleColorFromHandler(KlayGE::UITexButton const & sender);
	void ChangeParticleColorToHandler(KlayGE::UITexButton const & sender);
	void CurveTypeChangedHandler(KlayGE::UIComboBox const & sender);

	void LoadParticleAlpha(int id, std::string const & name);
	void LoadParticleColor(int id, KlayGE::Color const & clr);

	void LoadParticleSystem(std::string const & name);
	void SaveParticleSystem(std::string const & name);

	KlayGE::FontPtr font_;

	KlayGE::SceneNodePtr terrain_;

	KlayGE::FirstPersonCameraController fpsController_;

	KlayGE::ParticleEmitterPtr particle_emitter_;
	KlayGE::ParticleUpdaterPtr particle_updater_;
	KlayGE::ParticleSystemPtr ps_;

	bool depth_texture_support_;
	KlayGE::TexturePtr scene_tex_;
	KlayGE::TexturePtr scene_depth_tex_;
	KlayGE::TexturePtr scene_ds_tex_;
	KlayGE::FrameBufferPtr scene_buffer_;
	KlayGE::FrameBufferPtr scene_depth_buffer_;
	KlayGE::PostProcessPtr depth_to_linear_pp_;

	KlayGE::PostProcessPtr copy_pp_;

	KlayGE::UIDialogPtr dialog_;
	int id_open_;
	int id_save_as_;
	int id_gravity_static_;
	int id_gravity_slider_;
	int id_density_static_;
	int id_density_slider_;
	int id_angle_static_;
	int id_angle_slider_;
	int id_freq_static_;
	int id_freq_slider_;
	int id_detail_x_static_;
	int id_detail_x_slider_;
	int id_detail_y_static_;
	int id_detail_y_slider_;
	int id_detail_z_static_;
	int id_detail_z_slider_;
	int id_min_velocity_static_;
	int id_min_velocity_slider_;
	int id_max_velocity_static_;
	int id_max_velocity_slider_;
	int id_min_life_static_;
	int id_min_life_slider_;
	int id_max_life_static_;
	int id_max_life_slider_;
	int id_fps_camera_;
	int id_particle_alpha_from_button_;
	int id_particle_alpha_to_button_;
	int id_particle_color_from_button_;
	int id_particle_color_to_button_;
	int id_curve_type_;
	int id_size_over_life_;
	int id_mass_over_life_;
	int id_opacity_over_life_;

	std::string last_file_path_;
};

#endif		// _PARTICLEEDITOR_HPP
