#ifndef _OCEAN_HPP
#define _OCEAN_HPP

#include <KlayGE/App3D.hpp>
#include <KlayGE/Font.hpp>
#include <KlayGE/CameraController.hpp>
#include <KlayGE/UI.hpp>

class OceanApp : public KlayGE::App3DFramework
{
public:
	OceanApp(std::string const & name, KlayGE::RenderSettings const & settings);

private:
	void InitObjects();
	void OnResize(KlayGE::uint32_t width, KlayGE::uint32_t height);
	void DoUpdateOverlay();
	KlayGE::uint32_t DoUpdate(KlayGE::uint32_t pass);

	void InputHandler(KlayGE::InputEngine const & sender, KlayGE::InputAction const & action);

	KlayGE::FontPtr font_;
	KlayGE::SceneObjectPtr ocean_;
	KlayGE::SceneObjectPtr sky_box_;
	KlayGE::SceneObjectPtr sun_flare_;

	KlayGE::FirstPersonCameraController fpcController_;

	KlayGE::TexturePtr reflection_tex_;
	KlayGE::FrameBufferPtr reflection_fb_;
};

#endif		// _OCEAN_HPP
