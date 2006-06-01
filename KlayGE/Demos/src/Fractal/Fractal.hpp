#ifndef _FRACTAL_HPP
#define _FRACTAL_HPP

#include <KlayGE/App3D.hpp>
#include <KlayGE/Font.hpp>
#include <KlayGE/CameraController.hpp>

class Fractal : public KlayGE::App3DFramework
{
public:
	Fractal();

private:
	void InitObjects();
	void OnResize(KlayGE::uint32_t width, KlayGE::uint32_t height);

	KlayGE::uint32_t NumPasses() const;
	void DoUpdate(KlayGE::uint32_t pass);

	KlayGE::FontPtr font_;
	boost::shared_ptr<KlayGE::Renderable> renderFractal_;
	boost::shared_ptr<KlayGE::Renderable> renderPlane_;

	KlayGE::FrameBufferPtr render_buffer_[2];
	KlayGE::TexturePtr rendered_tex_[2];
};

#endif		// _FRACTAL_HPP
