#ifndef _DEPTHPEELING_HPP
#define _DEPTHPEELING_HPP

#include <KlayGE/App3D.hpp>
#include <KlayGE/Font.hpp>
#include <KlayGE/CameraController.hpp>
#include <boost/array.hpp>

class DepthPeelingApp : public KlayGE::App3DFramework
{
public:
	DepthPeelingApp(std::string const & name, KlayGE::RenderSettings const & settings);

private:
	void InitObjects();
	void OnResize(KlayGE::uint32_t width, KlayGE::uint32_t height);

	KlayGE::uint32_t DoUpdate(KlayGE::uint32_t pass);

	void InputHandler(KlayGE::InputEngine const & sender, KlayGE::InputAction const & action);

	KlayGE::FontPtr font_;
	KlayGE::SceneObjectPtr polygon_;

	KlayGE::FirstPersonCameraController fpcController_;

	std::vector<KlayGE::FrameBufferPtr> peeling_fbs_;
	std::vector<KlayGE::TexturePtr> peeled_texs_;
	std::vector<KlayGE::RenderViewPtr> peeled_views_;

	KlayGE::TexturePtr depth_texs_[2];
	KlayGE::RenderViewPtr depth_view_;

	KlayGE::RenderViewPtr default_depth_view_;

	boost::array<KlayGE::QueryPtr, 2> oc_queries_;

	KlayGE::PostProcessPtr blend_pp_;

	KlayGE::uint32_t num_layers_;
};

#endif		// _DEPTHPEELING_HPP
