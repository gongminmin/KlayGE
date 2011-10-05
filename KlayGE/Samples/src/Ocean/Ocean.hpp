#ifndef _OCEAN_HPP
#define _OCEAN_HPP

#include <KlayGE/App3D.hpp>
#include <KlayGE/Font.hpp>
#include <KlayGE/CameraController.hpp>
#include <KlayGE/UI.hpp>

class OceanApp : public KlayGE::App3DFramework
{
public:
	OceanApp();

	bool ConfirmDevice() const;

private:
	void InitObjects();
	void OnResize(KlayGE::uint32_t width, KlayGE::uint32_t height);
	void DoUpdateOverlay();
	KlayGE::uint32_t DoUpdate(KlayGE::uint32_t pass);

	void InputHandler(KlayGE::InputEngine const & sender, KlayGE::InputAction const & action);

	void DMapDimChangedHandler(KlayGE::UISlider const & sender);
	void PatchLengthChangedHandler(KlayGE::UISlider const & sender);
	void TimeScaleChangedHandler(KlayGE::UISlider const & sender);
	void WaveAmplitudeChangedHandler(KlayGE::UISlider const & sender);
	void WindSpeedXChangedHandler(KlayGE::UISlider const & sender);
	void WindSpeedYChangedHandler(KlayGE::UISlider const & sender);
	void WindDependencyChangedHandler(KlayGE::UISlider const & sender);
	void ChoppyScaleChangedHandler(KlayGE::UISlider const & sender);
	void FPSCameraHandler(KlayGE::UICheckBox const & sender);

	KlayGE::FontPtr font_;
	KlayGE::SceneObjectPtr terrain_;
	KlayGE::SceneObjectPtr ocean_;
	KlayGE::SceneObjectPtr sky_box_;
	KlayGE::SceneObjectPtr sun_flare_;
	KlayGE::PostProcessPtr copy_pp_;
	KlayGE::PostProcessPtr depth_to_linear_pp_;

	KlayGE::FirstPersonCameraController fpcController_;

	KlayGE::TexturePtr refraction_tex_;
	KlayGE::TexturePtr refraction_depth_tex_;
	KlayGE::FrameBufferPtr refraction_fb_;
	KlayGE::TexturePtr reflection_tex_;
	KlayGE::TexturePtr reflection_blur_tex_;
	KlayGE::FrameBufferPtr reflection_fb_;
	KlayGE::PostProcessPtr blur_y_;
	KlayGE::TexturePtr composed_tex_;
	KlayGE::FrameBufferPtr composed_fb_;

	KlayGE::UIDialogPtr dialog_params_;
	int id_dmap_dim_static_;
	int id_dmap_dim_slider_;
	int id_patch_length_static_;
	int id_patch_length_slider_;
	int id_time_scale_static_;
	int id_time_scale_slider_;
	int id_wave_amplitude_static_;
	int id_wave_amplitude_slider_;
	int id_wind_speed_x_static_;
	int id_wind_speed_x_slider_;
	int id_wind_speed_y_static_;
	int id_wind_speed_y_slider_;
	int id_wind_dependency_static_;
	int id_wind_dependency_slider_;
	int id_choppy_scale_static_;
	int id_choppy_scale_slider_;
	int id_fps_camera_;
};

#endif		// _OCEAN_HPP
