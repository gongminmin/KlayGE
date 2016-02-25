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

#include "Commands.hpp"

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
		std::wstring const & MeshName(uint32_t index) const;
		uint32_t SelectedMesh() const;
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
		uint32_t NumHistroyCmds() const;
		char const * HistroyCmdName(uint32_t index) const;
		uint32_t EndCmdIndex() const;

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

		void SkinningOn(bool on);
		void FPSCameraOn(bool on);
		void Visualize(int index);
		void MouseMove(int x, int y, uint32_t button);
		void MouseUp(int x, int y, uint32_t button);
		void MouseDown(int x, int y, uint32_t button);
		void KeyPress(int key);

		void ExecuteCommand(MtlEditorCommandPtr const & cmd);
		void Undo();
		void Redo();
		void ClearHistroy();

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

		std::vector<MtlEditorCommandPtr> command_history_;
		uint32_t end_command_index_;
	};
}

#endif		// _MTL_EDITOR_CORE_HPP
