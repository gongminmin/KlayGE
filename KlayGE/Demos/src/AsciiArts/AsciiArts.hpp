#ifndef _ASCIIARTS_HPP
#define _ASCIIARTS_HPP

#include <KlayGE/App3D.hpp>
#include <KlayGE/Font.hpp>
#include <KlayGE/CameraController.hpp>

class AsciiArts : public KlayGE::App3DFramework
{
public:
	AsciiArts();

private:
	void InitObjects();
	void OnResize(KlayGE::uint32_t width, KlayGE::uint32_t height);

	KlayGE::uint32_t NumPasses() const;
	void DoUpdate(KlayGE::uint32_t pass);

	void BuildAsciiLumsTex();

	void RenderInfos();

	void InputHandler(KlayGE::InputEngine const & sender, KlayGE::InputAction const & action);

private:
	KlayGE::FontPtr font_;

	KlayGE::FirstPersonCameraController fpcController_;
	KlayGE::SceneObjectPtr obj_;

	KlayGE::TexturePtr ascii_lums_tex_;

	KlayGE::RenderTargetPtr screen_buffer_;

	KlayGE::RenderTexturePtr render_buffer_;
	KlayGE::TexturePtr rendered_tex_;

	KlayGE::TexturePtr downsample_tex_;

	boost::shared_ptr<KlayGE::Renderable> renderQuad_;

	bool show_ascii_;
};

#endif		// _ASCIIARTS_HPP