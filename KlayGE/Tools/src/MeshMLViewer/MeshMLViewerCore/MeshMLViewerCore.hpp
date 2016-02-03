#ifndef _MESHMLVIEWERCORE_HPP
#define _MESHMLVIEWERCORE_HPP

#pragma once

#include <KlayGE/PreDeclare.hpp>
#include <KlayGE/App3D.hpp>
#include <KlayGE/Font.hpp>
#include <KlayGE/CameraController.hpp>
#include "Model.hpp"

#ifdef KLAYGE_MESHML_VIEWER_CORE_SOURCE		// Build dll
#define KLAYGE_MESHML_VIEWER_CORE_API KLAYGE_SYMBOL_EXPORT
#else										// Use dll
#define KLAYGE_MESHML_VIEWER_CORE_API KLAYGE_SYMBOL_IMPORT
#endif

namespace KlayGE
{
	class KLAYGE_MESHML_VIEWER_CORE_API MeshMLViewerCore : public App3DFramework
	{
	public:
		explicit MeshMLViewerCore(void* native_wnd);

		virtual bool ConfirmDevice() const override;

		void Resize(uint32_t width, uint32_t height);

		void OpenModel(std::string const & name);
		void SaveAsModel(std::string const & name);

		uint32_t NumFrames() const;
		void CurrFrame(float frame);
		float ModelFrameRate() const;
		uint32_t NumMeshes() const;
		std::wstring const & MeshName(uint32_t index) const;
		uint32_t NumVertexStreams(uint32_t mesh_id) const;
		uint32_t NumVertexStreamUsages(uint32_t mesh_id, uint32_t stream_index) const;
		uint32_t VertexStreamUsage(uint32_t mesh_id, uint32_t stream_index, uint32_t usage_index) const;
		uint32_t MaterialID(uint32_t mesh_id) const;
		float3 const & AmbientMaterial(uint32_t mtl_id) const;
		float3 const & DiffuseMaterial(uint32_t mtl_id) const;
		float3 const & SpecularMaterial(uint32_t mtl_id) const;
		float ShininessMaterial(uint32_t mtl_id) const;
		float3 const & EmitMaterial(uint32_t mtl_id) const;
		float OpacityMaterial(uint32_t mtl_id) const;
		std::string const & DiffuseTexture(uint32_t mtl_id) const;
		std::string const & SpecularTexture(uint32_t mtl_id) const;
		std::string const & ShininessTexture(uint32_t mtl_id) const;
		std::string const & NormalTexture(uint32_t mtl_id) const;
		std::string const & HeightTexture(uint32_t mtl_id) const;
		std::string const & EmitTexture(uint32_t mtl_id) const;
		std::string const & OpacityTexture(uint32_t mtl_id) const;
		uint32_t SelectedMesh() const;
		void SelectMesh(uint32_t mesh_id);

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
		virtual void OnCreate() override;
		virtual void OnDestroy() override;
		virtual void OnResize(uint32_t width, uint32_t height) override;
		virtual void DoUpdateOverlay() override;
		virtual uint32_t DoUpdate(uint32_t pass) override;

		void UpdateSelectedMesh();

	private:
		FontPtr font_;

		LightSourcePtr light_;

		SceneObjectPtr model_;
		SceneObjectPtr axis_;
		SceneObjectPtr grid_;
		SceneObjectPtr sky_box_;

		FirstPersonCameraController fps_controller_;
		TrackballCameraController tb_controller_;
		bool is_fps_camera_;

		DeferredRenderingLayer* deferred_rendering_;

		bool skinning_;
		float curr_frame_;

		std::string last_file_path_;

		bool mouse_down_in_wnd_;
		bool mouse_tracking_mode_;
		int2 last_mouse_pt_;

		FrameBufferPtr selective_fb_;
		TexturePtr selective_tex_;
		TexturePtr selective_cpu_tex_;
		bool update_selective_buffer_;
		uint32_t selected_obj_;
		SceneObjectPtr selected_bb_;
	};
}

#endif		// _MESHMLVIEWERCORE_HPP
