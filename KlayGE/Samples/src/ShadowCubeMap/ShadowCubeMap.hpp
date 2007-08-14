#ifndef _SHADOWCUBEMAP_HPP
#define _SHADOWCUBEMAP_HPP

#include <KlayGE/App3D.hpp>
#include <KlayGE/Font.hpp>
#include <KlayGE/CameraController.hpp>

class ShadowCubeMap : public KlayGE::App3DFramework
{
public:
	ShadowCubeMap(std::string const & name, KlayGE::RenderSettings const & settings);

private:
	void InitObjects();

	KlayGE::uint32_t DoUpdate(KlayGE::uint32_t pass);

	void InputHandler(KlayGE::InputEngine const & sender, KlayGE::InputAction const & action);

	KlayGE::FontPtr font_;

	KlayGE::SceneObjectPtr ground_;
	KlayGE::SceneObjectPtr mesh_;

	KlayGE::FirstPersonCameraController fpcController_;

	KlayGE::FrameBufferPtr shadow_buffers_[6];
	KlayGE::TexturePtr shadow_tex_;

	KlayGE::TexturePtr lamp_tex_;

	KlayGE::float4x4 light_model_;
};

#endif		// _SHADOWCUBEMAP_HPP
