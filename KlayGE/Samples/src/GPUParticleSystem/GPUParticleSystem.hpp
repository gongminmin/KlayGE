#ifndef _GPUPARTICLESYSTEM_HPP
#define _GPUPARTICLESYSTEM_HPP

#include <KlayGE/App3D.hpp>
#include <KlayGE/Font.hpp>
#include <KlayGE/CameraController.hpp>

class GPUParticleSystemApp : public KlayGE::App3DFramework
{
public:
	GPUParticleSystemApp(std::string const & name, KlayGE::RenderSettings const & settings);

private:
	void InitObjects();

	KlayGE::uint32_t DoUpdate(KlayGE::uint32_t pass);

	void InputHandler(KlayGE::InputEngine const & sender, KlayGE::InputAction const & action);

	KlayGE::FontPtr font_;

	KlayGE::SceneObjectPtr particles_;
	KlayGE::SceneObjectPtr terrain_;

	KlayGE::FirstPersonCameraController fpcController_;

	KlayGE::Timer timer_;
};

#endif		// _GPUPARTICLESYSTEM_HPP
