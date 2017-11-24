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

		void Resize(uint32_t width, uint32_t height);

		bool OpenModel(std::string const & name);
		void SaveAsModel(std::string const & name);

		char const * SkyboxName() const;
		void SkyboxName(std::string const & name);

		void DisplaySSVO(bool ssvo);
		void DisplayHDR(bool hdr);
		void DisplayAA(bool aa);
		void DisplayGamma(bool gamma);
		void DisplayColorGrading(bool cg);

		uint32_t NumFrames() const;
		float CurrFrame() const;
		float ModelFrameRate() const;

		uint32_t NumLods() const;
		void ActiveLod(int32_t lod);

		uint32_t NumMeshes() const;
		wchar_t const * MeshName(uint32_t index) const;

		uint32_t NumVertexStreams(uint32_t mesh_id) const;
		uint32_t NumVertexStreamUsages(uint32_t mesh_id, uint32_t stream_index) const;
		uint32_t VertexStreamUsage(uint32_t mesh_id, uint32_t stream_index, uint32_t usage_index) const;
		uint32_t SelectedMesh() const;
		uint32_t MaterialID(uint32_t mesh_id) const;
		uint32_t NumMaterials() const;
		char const * MaterialName(uint32_t mtl_id) const;
		float3 const & AlbedoMaterial(uint32_t mtl_id) const;
		float MetalnessMaterial(uint32_t mtl_id) const;
		float GlossinessMaterial(uint32_t mtl_id) const;
		float3 const & EmissiveMaterial(uint32_t mtl_id) const;
		float OpacityMaterial(uint32_t mtl_id) const;
		char const * Texture(uint32_t mtl_id, uint32_t slot) const;
		uint32_t DetailMode(uint32_t mtl_id) const;
		float HeightOffset(uint32_t mtl_id) const;
		float HeightScale(uint32_t mtl_id) const;
		float EdgeTessHint(uint32_t mtl_id) const;
		float InsideTessHint(uint32_t mtl_id) const;
		float MinTess(uint32_t mtl_id) const;
		float MaxTess(uint32_t mtl_id) const;
		bool TransparentMaterial(uint32_t mtl_id) const;
		float AlphaTestMaterial(uint32_t mtl_id) const;
		bool SSSMaterial(uint32_t mtl_id) const;
		bool TwoSidedMaterial(uint32_t mtl_id) const;

		void CurrFrame(float frame);
		void SelectMesh(uint32_t mesh_id);
		void MaterialID(uint32_t mesh_id, uint32_t mtl_id);
		void MaterialName(uint32_t mtl_id, std::string const & name);
		void AlbedoMaterial(uint32_t mtl_id, float3 const & value);
		void MetalnessMaterial(uint32_t mtl_id, float value);
		void GlossinessMaterial(uint32_t mtl_id, float value);
		void EmissiveMaterial(uint32_t mtl_id, float3 const & value);
		void OpacityMaterial(uint32_t mtl_id, float value);
		void Texture(uint32_t mtl_id, uint32_t slot, std::string const & name);
		void DetailMode(uint32_t mtl_id, uint32_t value);
		void HeightOffset(uint32_t mtl_id, float value);
		void HeightScale(uint32_t mtl_id, float value);
		void EdgeTessHint(uint32_t mtl_id, float value);
		void InsideTessHint(uint32_t mtl_id, float value);
		void MinTess(uint32_t mtl_id, float value);
		void MaxTess(uint32_t mtl_id, float value);
		void TransparentMaterial(uint32_t mtl_id, bool value);
		void AlphaTestMaterial(uint32_t mtl_id, float value);
		void SSSMaterial(uint32_t mtl_id, bool value);
		void TwoSidedMaterial(uint32_t mtl_id, bool value);

		uint32_t CopyMaterial(uint32_t mtl_id);
		uint32_t ImportMaterial(std::string const & name);
		void ExportMaterial(uint32_t mtl_id, std::string const & name);

		void SkinningOn(bool on);
		void SkeletonOn(bool on);
		void LightOn(bool on);
		void FPSCameraOn(bool on);
		void LineModeOn(bool on);
		void ImposterModeOn(bool on);
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

		LightSourcePtr ambient_light_;
		LightSourcePtr main_light_;

		SceneObjectPtr model_;
		SceneObjectPtr skeleton_model_;
		SceneObjectPtr imposter_;
		SceneObjectPtr axis_;
		SceneObjectPtr grid_;
		SceneObjectPtr sky_box_;

		FirstPersonCameraController fps_controller_;
		TrackballCameraController tb_controller_;
		bool is_fps_camera_;

		DeferredRenderingLayer* deferred_rendering_;

		std::string skybox_name_;
		TexturePtr default_cube_map_;

		bool skinning_;
		float curr_frame_;

		bool imposter_mode_;

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
