#ifndef _REFRACT_HPP
#define _REFRACT_HPP

#include <KlayGE/App3D.hpp>
#include <KlayGE/Font.hpp>
#include <KlayGE/CameraController.hpp>

class Refract : public KlayGE::App3DFramework
{
public:
	Refract();

private:
	void InitObjects();
	KlayGE::uint32_t Refract::NumPasses() const;
	void DoUpdate(KlayGE::uint32_t pass);
	void OnResize(KlayGE::uint32_t width, KlayGE::uint32_t height);

	void InputHandler(KlayGE::InputEngine const & sender, KlayGE::InputAction const & action);

	KlayGE::FontPtr font_;
	KlayGE::SceneObjectPtr refractor_;
	KlayGE::SceneObjectPtr sky_box_;

	KlayGE::FirstPersonCameraController fpcController_;

	KlayGE::TexturePtr y_cube_map_;
	KlayGE::TexturePtr c_cube_map_;

	KlayGE::FrameBufferPtr render_buffer_;
	KlayGE::TexturePtr rendered_tex_;

	KlayGE::TexturePtr downsample_tex_;
	KlayGE::TexturePtr blurx_tex_;
	KlayGE::TexturePtr blury_tex_;
	std::vector<KlayGE::TexturePtr> lum_texs_;
	KlayGE::TexturePtr lum_exp_tex_;

	KlayGE::RenderablePtr renderToneMapping_;
	KlayGE::RenderablePtr renderDownsampler_;
	KlayGE::RenderablePtr renderBlurX_;
	KlayGE::RenderablePtr renderBlurY_;
	KlayGE::RenderablePtr renderBloom_;
	std::vector<KlayGE::RenderablePtr> renderSumLums_;
	KlayGE::RenderablePtr renderAdaptedLum_;
};

#endif		// _REFRACT_HPP
