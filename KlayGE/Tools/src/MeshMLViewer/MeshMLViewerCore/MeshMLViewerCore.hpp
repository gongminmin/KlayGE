#ifndef _MESHMLVIEWERCORE_HPP
#define _MESHMLVIEWERCORE_HPP

#pragma once

#include <KlayGE/PreDeclare.hpp>
#include <KlayGE/App3D.hpp>
#include <KlayGE/Font.hpp>
#include <KlayGE/CameraController.hpp>
#include "Model.hpp"

namespace KlayGE
{
	class MeshMLViewerCore : public App3DFramework
	{
	public:
		explicit MeshMLViewerCore(void* native_wnd);

		virtual bool ConfirmDevice() const KLAYGE_OVERRIDE;

		void Resize(uint32_t width, uint32_t height);

		void OpenModel(std::string const & name);
		void SaveAsModel(std::string const & name);

		uint32_t NumFrames() const;
		void CurrFrame(float frame);
		float ModelFrameRate() const;
		uint32_t NumMeshes() const;
		wchar_t const * MeshName(uint32_t index) const;
		uint32_t NumVertexStreams(uint32_t mesh_index) const;
		uint32_t NumVertexStreamUsages(uint32_t mesh_index, uint32_t stream_index) const;
		uint32_t VertexStreamUsage(uint32_t mesh_index, uint32_t stream_index, uint32_t usage_index) const;
		uint32_t MaterialID(uint32_t mesh_index) const;
		float* AmbientMaterial(uint32_t material_index) const;
		float* DiffuseMaterial(uint32_t material_index) const;
		float* SpecularMaterial(uint32_t material_index) const;
		float ShininessMaterial(uint32_t material_index) const;
		float* EmitMaterial(uint32_t material_index) const;
		float OpacityMaterial(uint32_t material_index) const;
		char const * DiffuseTexture(uint32_t material_index) const;
		char const * SpecularTexture(uint32_t material_index) const;
		char const * ShininessTexture(uint32_t material_index) const;
		char const * BumpTexture(uint32_t material_index) const;
		char const * HeightTexture(uint32_t material_index) const;
		char const * EmitTexture(uint32_t material_index) const;
		char const * OpacityTexture(uint32_t material_index) const;
		uint32_t SelectedObject() const;

		void SkinningOn(bool on);
		void SmoothMeshOn(bool on);
		void FPSCameraOn(bool on);
		void LineModeOn(bool on);
		void Visualize(int index);
		void MouseMove(int x, int y, uint32_t button);
		void MouseUp(int x, int y, uint32_t button);
		void MouseDown(int x, int y, uint32_t button);
		void KeyPress(int key);

	private:
		virtual void InitObjects() KLAYGE_OVERRIDE;
		virtual void DelObjects() KLAYGE_OVERRIDE;
		virtual void OnResize(uint32_t width, uint32_t height) KLAYGE_OVERRIDE;
		virtual void DoUpdateOverlay() KLAYGE_OVERRIDE;
		virtual uint32_t DoUpdate(uint32_t pass) KLAYGE_OVERRIDE;

	private:
		FontPtr font_;

		PointLightSourcePtr point_light_;

		SceneObjectPtr model_;
		SceneObjectPtr axis_;
		SceneObjectPtr grid_;
		SceneObjectPtr sky_box_;

		FirstPersonCameraController fps_controller_;
		TrackballCameraController tb_controller_;
		bool is_fps_camera_;

		DeferredRenderingLayerPtr deferred_rendering_;

		bool skinning_;

		std::string last_file_path_;

		bool mouse_down_in_wnd_;
		bool mouse_tracking_mode_;
		int2 last_mouse_pt_;

		FrameBufferPtr selective_fb_;
		TexturePtr selective_tex_;
		TexturePtr selective_cpu_tex_;
		bool update_selective_buffer_;
		uint32_t selected_obj_;
	};
}

#endif		// _MESHMLVIEWERCORE_HPP
