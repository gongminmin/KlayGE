#pragma once

#include <KlayGE/KlayGE.hpp>
#include <KlayGE/App3D.hpp>
#include <KlayGE/ResLoader.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/CameraController.hpp>
#include <KlayGE/Font.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/RenderEffect.hpp>
#include <KlayGE/SceneObjectHelper.hpp>
#include <KlayGE/Mesh.hpp>
#include <KlayGE/Camera.hpp>

#include <KlayGE/FrameBuffer.hpp>

#include <vector>
#include <sstream>

class MyAppFramework : public KlayGE::App3DFramework
{
public:
	MyAppFramework();

private:
	virtual void InitObjects() KLAYGE_OVERRIDE;
	virtual void OnResize(uint32_t width, uint32_t height) KLAYGE_OVERRIDE;
	virtual void DoUpdateOverlay();
	virtual KlayGE::uint32_t DoUpdate(KlayGE::uint32_t pass);

	KlayGE::FontPtr font_;
	KlayGE::TrackballCameraController obj_controller_;
	KlayGE::TrackballCameraController light_controller_;
	KlayGE::SceneObjectHelperPtr subsurfaceObject_;

	KlayGE::FrameBufferPtr depth_ls_fb_;
	KlayGE::FrameBufferPtr color_fbo_;
	KlayGE::TexturePtr depth_in_ls_tex_, depth_in_ls_ds_tex_;
	KlayGE::TexturePtr shading_tex_, normal_tex_, albedo_tex_;
	KlayGE::TexturePtr depth_tex_, ds_tex_;

	KlayGE::TexturePtr sss_blurred_tex_;

	KlayGE::LightSourcePtr light_;
	KlayGE::SceneObjectLightSourceProxyPtr light_proxy_;
	KlayGE::PostProcessPtr sss_blur_pp_;
	KlayGE::PostProcessPtr translucency_pp_;

	KlayGE::CameraPtr scene_camera_;
	KlayGE::CameraPtr light_camera_;

	KlayGE::PostProcessPtr depth_to_linear_pp_;
};
