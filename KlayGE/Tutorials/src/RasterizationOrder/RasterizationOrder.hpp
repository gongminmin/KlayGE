#ifndef _RASTERIZATIONORDER_HPP
#define _RASTERIZATIONORDER_HPP

#include <KlayGE/App3D.hpp>
#include <KlayGE/Font.hpp>
#include <KlayGE/CameraController.hpp>

class RasterizationOrderApp : public KlayGE::App3DFramework
{
public:
	RasterizationOrderApp();

private:
	void OnCreate();
	void OnResize(KlayGE::uint32_t width, KlayGE::uint32_t height);

	void InputHandler(KlayGE::InputEngine const & sender, KlayGE::InputAction const & action);
	void ColorMapHandler(KlayGE::UICheckBox const & sender);
	void CaptureHandler(KlayGE::UIButton const & sender);

	void DoUpdateOverlay();
	KlayGE::uint32_t DoUpdate(KlayGE::uint32_t pass);

	void CaptureFrame();

	KlayGE::FontPtr font_;
	KlayGE::RenderablePtr render_quad_;

	KlayGE::GraphicsBufferPtr ras_order_buff_;
	KlayGE::UnorderedAccessViewPtr ras_order_uav_;
	KlayGE::TexturePtr ras_order_tex_;
	KlayGE::FrameBufferPtr ras_order_fb_;

	KlayGE::PostProcessPtr copy_pp_;

	KlayGE::UIDialogPtr dialog_params_;
	int id_color_map_;
	int id_capture_;
};

#endif		// _RASTERIZATIONORDER_HPP
