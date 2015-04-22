#ifndef _MTLEDITORCOREWRAPPER_HPP
#define _MTLEDITORCOREWRAPPER_HPP

#pragma once

namespace KlayGE
{
	public ref class MtlEditorCoreWrapper
	{
	public:
		explicit MtlEditorCoreWrapper(System::IntPtr native_wnd);
		~MtlEditorCoreWrapper();

		void Refresh();
		void Resize(uint32_t width, uint32_t height);

		void OpenModel(System::String^ name);
		void SaveModel(System::String^ name);

		unsigned int NumFrames();
		void CurrFrame(float frame);
		float ModelFrameRate();

		void SkinningOn(int on);
		void FPSCameraOn(int on);
		void Visualize(int index);

		void MouseMove(int x, int y, uint32_t button);
		void MouseUp(int x, int y, uint32_t button);
		void MouseDown(int x, int y, uint32_t button);
		void KeyPress(int key);

		uint32_t NumMeshes();
		System::String^ MeshName(uint32_t index);

		uint32_t MaterialID(uint32_t mesh_id);
		System::Windows::Media::Color AmbientMaterial(uint32_t mtl_id);
		System::Windows::Media::Color DiffuseMaterial(uint32_t mtl_id);
		System::Windows::Media::Color SpecularMaterial(uint32_t mtl_id);
		float ShininessMaterial(uint32_t mtl_id);
		System::Windows::Media::Color EmitMaterial(uint32_t mtl_id);
		float OpacityMaterial(uint32_t mtl_id);
		System::String^ DiffuseTexture(uint32_t mtl_id);
		System::String^ SpecularTexture(uint32_t mtl_id);
		System::String^ ShininessTexture(uint32_t mtl_id);
		System::String^ NormalTexture(uint32_t mtl_id);
		System::String^ HeightTexture(uint32_t mtl_id);
		System::String^ EmitTexture(uint32_t mtl_id);
		System::String^ OpacityTexture(uint32_t mtl_id);

		uint32_t NumHistroyCmds();
		System::String^ HistroyCmdName(uint32_t index);
		uint32_t EndCmdIndex();

		void AmbientMaterial(uint32_t mtl_id, System::Windows::Media::Color value);
		void DiffuseMaterial(uint32_t mtl_id, System::Windows::Media::Color value);
		void SpecularMaterial(uint32_t mtl_id, System::Windows::Media::Color value);
		void ShininessMaterial(uint32_t mtl_id, float value);
		void EmitMaterial(uint32_t mtl_id, System::Windows::Media::Color value);
		void OpacityMaterial(uint32_t mtl_id, float value);
		void DiffuseTexture(uint32_t mtl_id, System::String^ name);
		void SpecularTexture(uint32_t mtl_id, System::String^ name);
		void ShininessTexture(uint32_t mtl_id, System::String^ name);
		void NormalTexture(uint32_t mtl_id, System::String^ name);
		void HeightTexture(uint32_t mtl_id, System::String^ name);
		void EmitTexture(uint32_t mtl_id, System::String^ name);
		void OpacityTexture(uint32_t mtl_id, System::String^ name);

		uint32_t SelectedMesh();
		void SelectMesh(uint32_t mesh_id);

		void Undo();
		void Redo();
		void ClearHistroy();

	private:
		MtlEditorCore* core_;
	};
}

#endif		// _MTLEDITORCOREWRAPPER_HPP
