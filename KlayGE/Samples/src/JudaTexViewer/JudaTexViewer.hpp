#ifndef _JUDATEXVIEWER_HPP
#define _JUDATEXVIEWER_HPP

#include <KlayGE/App3D.hpp>
#include <KlayGE/Font.hpp>
#include <KlayGE/CameraController.hpp>
#include <KlayGE/UI.hpp>

#include <KlayGE/JudaTexture.hpp>

class JudaTexViewer : public KlayGE::App3DFramework
{
public:
	JudaTexViewer();

private:
	void OnCreate();
	void OnResize(KlayGE::uint32_t width, KlayGE::uint32_t height);

	void OpenHandler(KlayGE::UIButton const & sender);

	void DoUpdateOverlay();
	KlayGE::uint32_t DoUpdate(KlayGE::uint32_t pass);

	void InputHandler(KlayGE::InputEngine const & sender, KlayGE::InputAction const & action);

	void OpenJudaTex(std::string const & name);

	KlayGE::JudaTexturePtr juda_tex_;

	KlayGE::FontPtr font_;
	KlayGE::RenderablePtr tile_renderable_;
	KlayGE::RenderablePtr grid_border_renderable_;
	KlayGE::SceneNodePtr node_;

	KlayGE::GraphicsBufferPtr tile_pos_vb_;

	KlayGE::uint32_t num_tiles_;
	KlayGE::uint32_t tile_size_;
	KlayGE::uint32_t sx_, sy_, ex_, ey_;

	KlayGE::int2 last_mouse_pt_;
	KlayGE::float2 position_;
	float scale_;

	KlayGE::UIDialogPtr dialog_;
	int id_open_;

	std::string last_file_path_;

	KlayGE::float4x4 mat_tile_scaling_;
	KlayGE::float4x4 mat_translation_;
	KlayGE::float4x4 mat_scaling_;
};

#endif		// _JUDATEXVIEWER_HPP
