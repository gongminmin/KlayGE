#ifndef _PARTICLESYSTEMAPP_HPP
#define _PARTICLESYSTEMAPP_HPP

#include <KlayGE/PreDeclare.hpp>
#include <KlayGE/App3D.hpp>
#include <KlayGE/Font.hpp>
#include <KlayGE/CameraController.hpp>
#include <KlayGE/ParticleSystem.hpp>
#include <KlayGE/Timer.hpp>
#include <KlayGE/PostProcess.hpp>
#include <KlayGE/UI.hpp>

#include <boost/function.hpp>

struct Particle
{
	KlayGE::float3 pos;
	KlayGE::float3 vel;
	float life;
	float birth_time;
};

class ParticleEditorApp : public KlayGE::App3DFramework
{
public:
	ParticleEditorApp(std::string const & name, KlayGE::RenderSettings const & settings);

private:
	void InitObjects();
	void OnResize(KlayGE::uint32_t width, KlayGE::uint32_t height);
	KlayGE::uint32_t DoUpdate(KlayGE::uint32_t pass);

	void MouseDownHandler(KlayGE::uint32_t buttons, KlayGE::Vector_T<KlayGE::int32_t, 2> const & pt);
	void MouseUpHandler(KlayGE::uint32_t buttons, KlayGE::Vector_T<KlayGE::int32_t, 2> const & pt);
	void MouseOverHandler(KlayGE::uint32_t buttons, KlayGE::Vector_T<KlayGE::int32_t, 2> const & pt);

	void InputHandler(KlayGE::InputEngine const & sender, KlayGE::InputAction const & action);
	void OpenHandler(KlayGE::UIButton const & sender);
	void SaveAsHandler(KlayGE::UIButton const & sender);
	void AngleChangedHandler(KlayGE::UISlider const & sender);
	void LifeChangedHandler(KlayGE::UISlider const & sender);
	void DensityChangedHandler(KlayGE::UISlider const & sender);
	void VelocityChangedHandler(KlayGE::UISlider const & sender);
	void FPSCameraHandler(KlayGE::UICheckBox const & sender);
	void ChangeParticleTexHandler(KlayGE::UITexButton const & sender);

	void LoadParticleTex(std::string const & name);

	void LoadParticleSystem(std::string const & name);
	void SaveParticleSystem(std::string const & name);

	KlayGE::FontPtr font_;

	KlayGE::SceneObjectPtr particles_;
	KlayGE::SceneObjectPtr terrain_;

	KlayGE::FirstPersonCameraController fpsController_;

	boost::shared_ptr<KlayGE::ParticleSystem<Particle> > ps_;
	float init_life_;

	std::string particle_tex_;

	KlayGE::Timer timer_;

	KlayGE::TexturePtr scene_tex_;
	KlayGE::FrameBufferPtr scene_buffer_;

	KlayGE::PostProcessPtr copy_pp_;

	KlayGE::UIDialogPtr dialog_;
	int id_open_;
	int id_save_as_;
	int id_angle_static_;
	int id_angle_slider_;
	int id_life_static_;
	int id_life_slider_;
	int id_density_static_;
	int id_density_slider_;
	int id_velocity_static_;
	int id_velocity_slider_;
	int id_fps_camera_;
	int id_particle_tex_button_;
};

#endif		// _PARTICLESYSTEMAPP_HPP
