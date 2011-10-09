#ifndef _CAUSTICSMAP_HPP
#define _CAUSTICSMAP_HPP

#include <KlayGE\App3D.hpp>
#include <KlayGE\Font.hpp>
#include <KlayGE\CameraController.hpp>

struct CausticsInputTexture
{
	KlayGE::TexturePtr refract_obj_N_texture_f;
	KlayGE::TexturePtr refract_obj_N_texture_b;
	KlayGE::TexturePtr refract_obj_depth_tex_f;
	KlayGE::TexturePtr refract_obj_depth_tex_b;
	KlayGE::TexturePtr background_depth_tex;
};

class CausticsMapApp : public KlayGE::App3DFramework
{
public:
	CausticsMapApp();

	bool ConfirmDevice() const;

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
	uint64_t OcclusionQueryResult()
	{
		return occlusion_pixs_;
	}
	KlayGE::LightSourcePtr GetLightSource()
	{
		return light_;
	}
	CausticsInputTexture const & GetCausticsInputTexture()
	{
		static CausticsInputTexture input_texture;

		input_texture.background_depth_tex = background_depth_tex_;
		input_texture.refract_obj_N_texture_f = refract_obj_N_texture_f_;
		input_texture.refract_obj_depth_tex_f = refract_obj_depth_tex_f_;
		input_texture.refract_obj_N_texture_b = refract_obj_N_texture_b_;
		input_texture.refract_obj_depth_tex_b = refract_obj_depth_tex_b_;

		return input_texture;
	}
	KlayGE::TexturePtr GetCausticsMap()
	{
		return caustics_texture_filtered_;
	}

private:
	void InitObjects();
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
	KlayGE::FontPtr font_;
	KlayGE::TrackballCameraController trackball_controller_;
	KlayGE::SceneObjectPtr light_proxy_;
	KlayGE::LightSourcePtr light_;
	KlayGE::LightSourcePtr dummy_light_;

	KlayGE::TexturePtr y_cube_map_;
	KlayGE::TexturePtr c_cube_map_;
	KlayGE::SceneObjectPtr sky_box_;

	KlayGE::TexturePtr refract_obj_N_texture_f_;
	KlayGE::TexturePtr refract_obj_N_texture_b_;
	KlayGE::TexturePtr refract_obj_depth_tex_f_;
	KlayGE::TexturePtr refract_obj_depth_tex_b_;
	KlayGE::TexturePtr background_depth_tex_;
	KlayGE::FrameBufferPtr background_fb_;
	KlayGE::FrameBufferPtr refract_obj_fb_f_;
	KlayGE::FrameBufferPtr refract_obj_fb_b_;

	KlayGE::TexturePtr caustics_texture_;
	KlayGE::TexturePtr caustics_texture_filtered_;
	KlayGE::FrameBufferPtr caustics_fb_;
	KlayGE::PostProcessPtr caustics_map_pps_;
	KlayGE::OcclusionQueryPtr occlusion_query_;
	KlayGE::uint64_t occlusion_pixs_;

	KlayGE::FrameBufferPtr shadow_cube_buffer_;
	KlayGE::TexturePtr shadow_tex_;
	KlayGE::TexturePtr shadow_cube_tex_;
	KlayGE::PostProcessPtr sm_filter_pps_[6];

	KlayGE::FrameBufferPtr env_cube_buffers_[6];
	KlayGE::RenderViewPtr env_cube_rvs_[6];
	KlayGE::RenderViewPtr env_cube_depth_rvs_[6];
	KlayGE::TexturePtr env_cube_tex_;
	KlayGE::LightSourcePtr dummy_light_env_;

	KlayGE::TexturePtr scene_texture_;
	KlayGE::FrameBufferPtr scene_fb_;

	KlayGE::PostProcessPtr copy_pp_;

	KlayGE::SceneObjectPtr plane_object_;
	KlayGE::SceneObjectPtr refract_obj_;
	KlayGE::SceneObjectPtr bunny_;
	KlayGE::SceneObjectPtr sphere_;
	KlayGE::RenderablePtr caustics_grid_;

	KlayGE::UIDialogPtr dialog_;
	bool enable_dual_face_caustics_;
	float refract_idx_;
	float light_density_;
	float point_size_;
};

#endif		// _CAUSTICSMAP_HPP