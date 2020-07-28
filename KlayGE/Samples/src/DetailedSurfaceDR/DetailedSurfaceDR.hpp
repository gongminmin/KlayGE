#ifndef KLAYGE_SAMPLES_DETAILED_SURFACE_DR_HPP
#define KLAYGE_SAMPLES_DETAILED_SURFACE_DR_HPP

#include <KlayGE/App3D.hpp>
#include <KlayGE/CameraController.hpp>
#include <KlayGE/Font.hpp>
#include <KlayGE/UI.hpp>

class DetailedSurfaceApp : public KlayGE::App3DFramework
{
public:
	DetailedSurfaceApp();

private:
	enum class DetailTypes : uint32_t
	{
		None = 0,
		Bump,
		Parallax,
		ParallaxOcclusion,
		FlatTessellation,
		SmoothTessellation,

		Count,
	};

private:
	void UpdateMaterial();

	void OnCreate();
	void OnResize(KlayGE::uint32_t width, KlayGE::uint32_t height);

	void DoUpdateOverlay();
	KlayGE::uint32_t DoUpdate(KlayGE::uint32_t pass);

	void InputHandler(KlayGE::InputEngine const& sender, KlayGE::InputAction const& action);
	void ScaleChangedHandler(KlayGE::UISlider const& sender);
	void DetailTypeChangedHandler(KlayGE::UIComboBox const& sender);
	void OcclusionHandler(KlayGE::UICheckBox const& sender);
	void NaLengthHandler(KlayGE::UICheckBox const& sender);
	void WireframeHandler(KlayGE::UICheckBox const& sender);

	KlayGE::FontPtr font_;
	KlayGE::RenderModelPtr polygon_model_;

	KlayGE::TrackballCameraController tb_controller_;

	KlayGE::DeferredRenderingLayer* deferred_rendering_;

	KlayGE::LightSourcePtr light_;

	KlayGE::UIDialogPtr dialog_;
	float height_scale_;
	bool use_occlusion_map_ = false;
	uint32_t material_index_ = static_cast<uint32_t>(DetailTypes::ParallaxOcclusion);

	int id_scale_static_;
	int id_scale_slider_;
	int id_detail_type_static_;
	int id_detail_type_combo_;
	int id_occlusion_;
	int id_na_length_;
	int id_wireframe_;

	KlayGE::RenderMaterialPtr mtls_[2][static_cast<uint32_t>(DetailTypes::Count)];
};

#endif // KLAYGE_SAMPLES_DETAILED_SURFACE_DR_HPP
