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

	void Update();

	KlayGE::FontPtr font_;

	KlayGE::FirstPersonCameraController fpcController_;
	boost::shared_ptr<KlayGE::StaticMesh> mesh_;

	KlayGE::TexturePtr ascii_tex_;
	KlayGE::TexturePtr ascii_lums_tex_;

	KlayGE::RenderTexturePtr render_buffer_;
	KlayGE::TexturePtr rendered_tex_;

	KlayGE::TexturePtr downsample_tex_;

	KlayGE::RenderEngine::RenderTargetListIterator screen_iter_;
	KlayGE::RenderEngine::RenderTargetListIterator render_buffer_iter_;

	boost::shared_ptr<KlayGE::Renderable> renderQuad_;

	KlayGE::uint32_t action_map_id_;

	bool show_ascii_;
};

#endif		// _ASCIIARTS_HPP