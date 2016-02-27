#ifndef _MESHMLVIEWERCOREWRAPPER_HPP
#define _MESHMLVIEWERCOREWRAPPER_HPP

#pragma once

namespace KlayGE
{
	public ref class MeshMLViewerCoreWrapper
	{
	public:
		explicit MeshMLViewerCoreWrapper(System::IntPtr native_wnd);
		~MeshMLViewerCoreWrapper();

		void Refresh();
		void Resize(uint32_t width, uint32_t height);

		void OpenModel(System::String^ name);
		void SaveModel(System::String^ name);

		unsigned int NumFrames();
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

		uint32_t SelectedMesh();
		void SelectMesh(uint32_t mesh_id);

	private:
		MeshMLViewerCore* core_;
	};
}

#endif		// _MESHMLVIEWERCOREWRAPPER_HPP
