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
		float CurrFrame();
		void CurrFrame(float frame);
		float ModelFrameRate();

		void SkinningOn(int on);
		void FPSCameraOn(int on);
		void LineModeOn(int on);
		void Visualize(int index);

		void MouseMove(int x, int y, uint32_t button);
		void MouseUp(int x, int y, uint32_t button);
		void MouseDown(int x, int y, uint32_t button);
		void KeyPress(int key);

		uint32_t NumMeshes();
		System::String^ MeshName(uint32_t index);

		uint32_t NumVertexStreams(uint32_t mesh_id);
		uint32_t NumVertexStreamUsages(uint32_t mesh_id, uint32_t stream_index);
		uint32_t VertexStreamUsage(uint32_t mesh_id, uint32_t stream_index, uint32_t usage_index);

		uint32_t MaterialID(uint32_t mesh_id);
		array<float>^ AmbientMaterial(uint32_t mtl_id);
		array<float>^ DiffuseMaterial(uint32_t mtl_id);
		array<float>^ SpecularMaterial(uint32_t mtl_id);
		float ShininessMaterial(uint32_t mtl_id);
		array<float>^ EmitMaterial(uint32_t mtl_id);
		float OpacityMaterial(uint32_t mtl_id);
		System::String^ DiffuseTexture(uint32_t mtl_id);
		System::String^ SpecularTexture(uint32_t mtl_id);
		System::String^ ShininessTexture(uint32_t mtl_id);
		System::String^ NormalTexture(uint32_t mtl_id);
		System::String^ HeightTexture(uint32_t mtl_id);
		System::String^ EmitTexture(uint32_t mtl_id);
		System::String^ OpacityTexture(uint32_t mtl_id);
		uint32_t DetailMode(uint32_t mtl_id);
		float HeightOffset(uint32_t mtl_id);
		float HeightScale(uint32_t mtl_id);
		float EdgeTessHint(uint32_t mtl_id);
		float InsideTessHint(uint32_t mtl_id);
		float MinTess(uint32_t mtl_id);
		float MaxTess(uint32_t mtl_id);

		void AmbientMaterial(uint32_t mtl_id, array<float>^ value);
		void DiffuseMaterial(uint32_t mtl_id, array<float>^ value);
		void SpecularMaterial(uint32_t mtl_id, array<float>^ value);
		void ShininessMaterial(uint32_t mtl_id, float value);
		void EmitMaterial(uint32_t mtl_id, array<float>^ value);
		void OpacityMaterial(uint32_t mtl_id, float value);
		void DiffuseTexture(uint32_t mtl_id, System::String^ name);
		void SpecularTexture(uint32_t mtl_id, System::String^ name);
		void ShininessTexture(uint32_t mtl_id, System::String^ name);
		void NormalTexture(uint32_t mtl_id, System::String^ name);
		void HeightTexture(uint32_t mtl_id, System::String^ name);
		void EmitTexture(uint32_t mtl_id, System::String^ name);
		void OpacityTexture(uint32_t mtl_id, System::String^ name);
		void DetailMode(uint32_t mtl_id, uint32_t value);
		void HeightOffset(uint32_t mtl_id, float value);
		void HeightScale(uint32_t mtl_id, float value);
		void EdgeTessHint(uint32_t mtl_id, float value);
		void InsideTessHint(uint32_t mtl_id, float value);
		void MinTess(uint32_t mtl_id, float value);
		void MaxTess(uint32_t mtl_id, float value);

		uint32_t SelectedMesh();
		void SelectMesh(uint32_t mesh_id);

	public:
		delegate void UpdateSelectEntityDelegate(uint32_t obj_id);

		void UpdateSelectEntityCallback(UpdateSelectEntityDelegate^ callback);

	private:
		MtlEditorCore* core_;

		UpdateSelectEntityDelegate^ update_select_entity_delegate_;
	};
}

#endif		// _MTLEDITORCOREWRAPPER_HPP
