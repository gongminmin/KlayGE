#ifndef _CAUSTICSMAP_HPP
#define _CAUSTICSMAP_HPP

#include <KlayGE/App3D.hpp>
#include <KlayGE/Font.hpp>
#include <KlayGE/CameraController.hpp>

class CausticsMapApp : public KlayGE::App3DFramework
{
public:
	CausticsMapApp();

	//Caustics Map Parameter
	bool IsEnableDualFaceCaustics()
	{
		return enable_dual_face_caustics_;
	}
	float RefractIndex()
	{
		return refract_idx_;
	}
	float LightDensity()
	{
		return light_density_;
	}
	float PointSize()
	{
		return point_size_;
	}
	KlayGE::LightSourcePtr const& GetLightSource()
	{
		return light_;
	}
	KlayGE::TexturePtr const & GetBackgroundDepthTex() const
	{
		return background_depth_tex_;
	}
	KlayGE::TexturePtr const & GetRefractObjNormalFrontTex() const
	{
		return refract_obj_N_texture_f_;
	}
	KlayGE::TexturePtr const & GetRefractObjDepthFrontTex() const
	{
		return refract_obj_depth_tex_f_;
	}
	KlayGE::TexturePtr const & GetRefractObjNormalBackTex() const
	{
		return refract_obj_N_texture_b_;
	}
	KlayGE::TexturePtr const & GetRefractObjDepthBackTex() const
	{
		return refract_obj_depth_tex_b_;
	}
	KlayGE::TexturePtr GetCausticsMap()
	{
		return caustics_texture_filtered_;
	}

private:
	void OnCreate();
	void InitUI();
	void InitBuffer();
	void InitCubeSM();
	void InitEnvCube();
	void OnResize(KlayGE::uint32_t width, KlayGE::uint32_t height);
	void DoUpdateOverlay();
	KlayGE::uint32_t DoUpdate(KlayGE::uint32_t pass);
	void InputHandler(KlayGE::InputEngine const & sender, KlayGE::InputAction const & action);

	//UI Handler
	void RefractIndexHandler(KlayGE::UISlider const & sender);
	void LightDensityHandler(KlayGE::UISlider const & sender);
	void PointSizeHandler(KlayGE::UISlider const & sender);
	void DualFaceCausticsCheckBoxHandler(KlayGE::UICheckBox const & sender);
	void ModelSelectionComboBox(KlayGE::UIComboBox const & sender);

private:
	bool depth_texture_support_;

	KlayGE::FontPtr font_;
	KlayGE::TrackballCameraController trackball_controller_;
	KlayGE::SceneObjectLightSourceProxyPtr light_proxy_;
	KlayGE::LightSourcePtr light_;
	KlayGE::LightSourcePtr dummy_light_;

	KlayGE::SceneNodePtr skybox_;

	KlayGE::TexturePtr refract_obj_N_texture_f_;
	KlayGE::TexturePtr refract_obj_N_texture_b_;
	KlayGE::TexturePtr refract_obj_ds_tex_f_;
	KlayGE::ShaderResourceViewPtr refract_obj_ds_srv_f_;
	KlayGE::TexturePtr refract_obj_ds_tex_b_;
	KlayGE::ShaderResourceViewPtr refract_obj_ds_srv_b_;
	KlayGE::TexturePtr background_ds_tex_;
	KlayGE::ShaderResourceViewPtr background_ds_srv_;
	KlayGE::TexturePtr refract_obj_depth_tex_f_;
	KlayGE::RenderTargetViewPtr refract_obj_depth_rtv_f_;
	KlayGE::TexturePtr refract_obj_depth_tex_b_;
	KlayGE::RenderTargetViewPtr refract_obj_depth_rtv_b_;
	KlayGE::TexturePtr background_depth_tex_;
	KlayGE::RenderTargetViewPtr background_depth_rtv_;
	KlayGE::FrameBufferPtr background_fb_;	
	KlayGE::FrameBufferPtr refract_obj_fb_d_f_;
	KlayGE::FrameBufferPtr refract_obj_fb_d_b_;
	KlayGE::FrameBufferPtr refract_obj_fb_f_;
	KlayGE::FrameBufferPtr refract_obj_fb_b_;

	KlayGE::TexturePtr caustics_texture_;
	KlayGE::ShaderResourceViewPtr caustics_srv_;
	KlayGE::TexturePtr caustics_texture_filtered_;
	KlayGE::FrameBufferPtr caustics_fb_;
	KlayGE::PostProcessPtr caustics_map_pps_;

	KlayGE::FrameBufferPtr shadow_cube_buffer_;
	KlayGE::TexturePtr shadow_cube_tex_;
	KlayGE::PostProcessPtr sm_filter_pps_[6];

	KlayGE::FrameBufferPtr env_cube_buffer_;
	KlayGE::TexturePtr env_cube_tex_;
	KlayGE::PostProcessPtr env_filter_pps_[6];
	KlayGE::LightSourcePtr dummy_light_env_;

	KlayGE::TexturePtr scene_texture_;
	KlayGE::FrameBufferPtr scene_fb_;

	KlayGE::PostProcessPtr copy_pp_;
	KlayGE::PostProcessPtr depth_to_linear_pp_;

	KlayGE::SceneNodePtr plane_obj_;
	KlayGE::RenderablePtr plane_renderable_;
	KlayGE::SceneNodePtr refract_obj_;
	KlayGE::RenderModelPtr refract_model_;
	KlayGE::SceneNodePtr bunny_obj_;
	KlayGE::RenderModelPtr bunny_model_;
	KlayGE::SceneNodePtr sphere_obj_;
	KlayGE::RenderModelPtr sphere_model_;
	KlayGE::RenderablePtr caustics_grid_;

	KlayGE::UIDialogPtr dialog_;
	bool enable_dual_face_caustics_;
	float refract_idx_;
	float light_density_;
	float point_size_;
};

#endif		// _CAUSTICSMAP_HPP
