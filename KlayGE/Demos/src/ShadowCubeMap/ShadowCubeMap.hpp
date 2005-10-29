#ifndef _SHADOWCUBEMAP_HPP
#define _SHADOWCUBEMAP_HPP

#include <KlayGE/App3D.hpp>
#include <KlayGE/Font.hpp>
#include <KlayGE/CameraController.hpp>

class ShadowCubeMap : public KlayGE::App3DFramework
{
public:
	ShadowCubeMap();

private:
	void InitObjects();

	KlayGE::uint32_t NumPasses() const;
	void DoUpdate(KlayGE::uint32_t pass);

	void InputHandler(KlayGE::InputEngine const & sender, KlayGE::InputAction const & action);

	KlayGE::FontPtr font_;

	boost::shared_ptr<KlayGE::Renderable> renderGround_;
	KlayGE::RenderModelPtr renderMesh_;

	KlayGE::FirstPersonCameraController fpcController_;

	KlayGE::RenderTargetPtr screen_buffer_;

	KlayGE::RenderTexturePtr shadow_buffer_;
	KlayGE::TexturePtr shadow_tex_;

	KlayGE::TexturePtr lamp_tex_;

	KlayGE::Matrix4 light_model_;
};

#endif		// _SHADOWCUBEMAP_HPP
