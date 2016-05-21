#ifndef _MTL_EDITOR_CORE_HPP
#define _MTL_EDITOR_CORE_HPP

#pragma once

#include <KlayGE/PreDeclare.hpp>
#include <KlayGE/App3D.hpp>
#include <KlayGE/Font.hpp>
#include <KlayGE/CameraController.hpp>
#include "Model.hpp"

#ifdef KLAYGE_MTL_EDITOR_CORE_SOURCE		// Build dll
#define KLAYGE_MTL_EDITOR_CORE_API KLAYGE_SYMBOL_EXPORT
#else							// Use dll
#define KLAYGE_MTL_EDITOR_CORE_API KLAYGE_SYMBOL_IMPORT
#endif

namespace KlayGE
{
	class KLAYGE_MTL_EDITOR_CORE_API MtlEditorCore : public App3DFramework
	{
	public:
		explicit MtlEditorCore(void* native_wnd);

		virtual bool ConfirmDevice() const override;

		void Resize(uint32_t width, uint32_t height);

		void OpenModel(std::string const & name);
		void SaveAsModel(std::string const & name);

		uint32_t NumFrames() const;
		float CurrFrame() const;
		float ModelFrameRate() const;
		uint32_t NumMeshes() const;
		wchar_t const * MeshName(uint32_t index) const;
		uint32_t NumVertexStreams(uint32_t mesh_id) const;
		uint32_t NumVertexStreamUsages(uint32_t mesh_id, uint32_t stream_index) const;
		uint32_t VertexStreamUsage(uint32_t mesh_id, uint32_t stream_index, uint32_t usage_index) const;
		uint32_t SelectedMesh() const;
		uint32_t MaterialID(uint32_t mesh_id) const;
		float3 const & AmbientMaterial(uint32_t mtl_id) const;
		float3 const & DiffuseMaterial(uint32_t mtl_id) const;
		float3 const & SpecularMaterial(uint32_t mtl_id) const;
		float ShininessMaterial(uint32_t mtl_id) const;
		float3 const & EmitMaterial(uint32_t mtl_id) const;
		float OpacityMaterial(uint32_t mtl_id) const;
		char const * DiffuseTexture(uint32_t mtl_id) const;
		char const * SpecularTexture(uint32_t mtl_id) const;
		char const * ShininessTexture(uint32_t mtl_id) const;
		char const * NormalTexture(uint32_t mtl_id) const;
		char const * HeightTexture(uint32_t mtl_id) const;
		char const * EmitTexture(uint32_t mtl_id) const;
		char const * OpacityTexture(uint32_t mtl_id) const;
		uint32_t DetailMode(uint32_t mtl_id) const;
		float HeightOffset(uint32_t mtl_id) const;
		float HeightScale(uint32_t mtl_id) const;
		float EdgeTessHint(uint32_t mtl_id) const;
		float InsideTessHint(uint32_t mtl_id) const;
		float MinTess(uint32_t mtl_id) const;
		float MaxTess(uint32_t mtl_id) const;

		void CurrFrame(float frame);
		void SelectMesh(uint32_t mesh_id);
		void AmbientMaterial(uint32_t mtl_id, float3 const & value);
		void DiffuseMaterial(uint32_t mtl_id, float3 const & value);
		void SpecularMaterial(uint32_t mtl_id, float3 const & value);
		void ShininessMaterial(uint32_t mtl_id, float value);
		void EmitMaterial(uint32_t mtl_id, float3 const & value);
		void OpacityMaterial(uint32_t mtl_id, float value);
		void DiffuseTexture(uint32_t mtl_id, std::string const & name);
		void SpecularTexture(uint32_t mtl_id, std::string const & name);
		void ShininessTexture(uint32_t mtl_id, std::string const & name);
		void NormalTexture(uint32_t mtl_id, std::string const & name);
		void HeightTexture(uint32_t mtl_id, std::string const & name);
		void EmitTexture(uint32_t mtl_id, std::string const & name);
		void OpacityTexture(uint32_t mtl_id, std::string const & name);
		void DetailMode(uint32_t mtl_id, uint32_t value);
		void HeightOffset(uint32_t mtl_id, float value);
		void HeightScale(uint32_t mtl_id, float value);
		void EdgeTessHint(uint32_t mtl_id, float value);
		void InsideTessHint(uint32_t mtl_id, float value);
		void MinTess(uint32_t mtl_id, float value);
		void MaxTess(uint32_t mtl_id, float value);

		void SkinningOn(bool on);
		void FPSCameraOn(bool on);
		void LineModeOn(bool on);
		void Visualize(int index);
		void MouseMove(int x, int y, uint32_t button);
		void MouseUp(int x, int y, uint32_t button);
		void MouseDown(int x, int y, uint32_t button);
		void KeyPress(int key);

	// Callbacks
	public:
		typedef void(__stdcall *UpdateSelectEntityEvent)(uint32_t obj_id);

		void UpdateSelectEntityCallback(UpdateSelectEntityEvent callback)
		{
			update_select_entity_event_ = callback;
		}

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

		UpdateSelectEntityEvent update_select_entity_event_;
	};
}

#endif		// _MTL_EDITOR_CORE_HPP
