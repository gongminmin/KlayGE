#ifndef KLAYGE_SAMPLE_SKINNED_MESH_HPP
#define KLAYGE_SAMPLE_SKINNED_MESH_HPP

#pragma once

#include <KlayGE/PreDeclare.hpp>

#include <KlayGE/App3D.hpp>
#include <KlayGE/CameraController.hpp>

class SkinnedMeshApp : public KlayGE::App3DFramework
{
public:
	SkinnedMeshApp();

private:
	void OnCreate() override;
	void OnResize(KlayGE::uint32_t width, KlayGE::uint32_t height) override;
	void DoUpdateOverlay() override;
	KlayGE::uint32_t DoUpdate(KlayGE::uint32_t pass) override;

	void InputHandler(KlayGE::InputEngine const& sender, KlayGE::InputAction const& action);
	void SkinningHandler(KlayGE::UICheckBox const& sender);
	void PlayingHandler(KlayGE::UICheckBox const& sender);

	KlayGE::DeferredRenderingLayer* deferred_rendering_{};

	KlayGE::SkinnedModelPtr skinned_model_;
	bool playing_{false};
	float frame_{0.0f};
	bool skinning_{true};

	KlayGE::FontPtr font_;
	KlayGE::TrackballCameraController obj_controller_;

	KlayGE::CameraPtr scene_camera_;

	KlayGE::UIDialogPtr dialog_params_;
	int id_skinning_{};
	int id_playing_{};
};

#endif // KLAYGE_SAMPLE_SKINNED_MESH_HPP
