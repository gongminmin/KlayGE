#ifndef _GPUPARTICLESYSTEM_HPP
#define _GPUPARTICLESYSTEM_HPP

#include <KlayGE/App3D.hpp>
#include <KlayGE/Font.hpp>
#include <KlayGE/CameraController.hpp>
#include <KlayGE/PostProcess.hpp>

class GPUParticleSystemApp : public KlayGE::App3DFramework
{
public:
	GPUParticleSystemApp();

private:
	void OnCreate();
	void OnResize(KlayGE::uint32_t width, KlayGE::uint32_t height);

	void DoUpdateOverlay();
	KlayGE::uint32_t DoUpdate(KlayGE::uint32_t pass);

	void InputHandler(KlayGE::InputEngine const & sender, KlayGE::InputAction const & action);

	KlayGE::FontPtr font_;

	KlayGE::SceneNodePtr particles_;
	KlayGE::RenderablePtr particles_renderable_;
	KlayGE::SceneNodePtr terrain_;

	KlayGE::TrackballCameraController tb_controller_;

	KlayGE::TexturePtr scene_tex_;
	KlayGE::FrameBufferPtr scene_buffer_;

	KlayGE::TexturePtr fog_tex_;
	KlayGE::ShaderResourceViewPtr fog_srv_;
	KlayGE::FrameBufferPtr fog_buffer_;

	KlayGE::PostProcessPtr blend_pp_;
};

#endif		// _GPUPARTICLESYSTEM_HPP
