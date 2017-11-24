#ifndef _MTLEDITORCOREWRAPPER_HPP
#define _MTLEDITORCOREWRAPPER_HPP

#pragma once

#include <KlayGE/RenderMaterial.hpp>

namespace KlayGE
{
	public ref class MtlEditorCoreWrapper
	{
	public:
		enum class TextureSlot
		{
			TS_Albedo = RenderMaterial::TS_Albedo,
			TS_Metalness = RenderMaterial::TS_Metalness,
			TS_Glossiness = RenderMaterial::TS_Glossiness,
			TS_Emissive = RenderMaterial::TS_Emissive,
			TS_Normal = RenderMaterial::TS_Normal,
			TS_Height = RenderMaterial::TS_Height
		};

	public:
		explicit MtlEditorCoreWrapper(System::IntPtr native_wnd);
		~MtlEditorCoreWrapper();

		void Refresh();
		void Resize(uint32_t width, uint32_t height);

		bool OpenModel(System::String^ name);
		void SaveModel(System::String^ name);

		System::String^ SkyboxName();
		void SkyboxName(System::String^ name);

		void DisplaySSVO(bool ssvo);
		void DisplayHDR(bool hdr);
		void DisplayAA(bool aa);
		void DisplayGamma(bool gamma);
		void DisplayColorGrading(bool cg);

		unsigned int NumFrames();
		float CurrFrame();
		void CurrFrame(float frame);
		float ModelFrameRate();

		void SkinningOn(int on);
		void SkeletonOn(int on);
		void LightOn(int on);
		void FPSCameraOn(int on);
		void LineModeOn(int on);
		void ImposterModeOn(int on);
		void Visualize(int index);

		void MouseMove(int x, int y, uint32_t button);
		void MouseUp(int x, int y, uint32_t button);
		void MouseDown(int x, int y, uint32_t button);
		void KeyPress(int key);

		uint32_t NumLods();
		void ActiveLod(int lod);

		uint32_t NumMeshes();
		System::String^ MeshName(uint32_t index);

		uint32_t NumVertexStreams(uint32_t mesh_id);
		uint32_t NumVertexStreamUsages(uint32_t mesh_id, uint32_t stream_index);
		uint32_t VertexStreamUsage(uint32_t mesh_id, uint32_t stream_index, uint32_t usage_index);

		uint32_t MaterialID(uint32_t mesh_id);
		uint32_t NumMaterials();
		System::String^ MaterialName(uint32_t mtl_id);
		array<float>^ AlbedoMaterial(uint32_t mtl_id);
		float MetalnessMaterial(uint32_t mtl_id);
		float GlossinessMaterial(uint32_t mtl_id);
		array<float>^ EmissiveMaterial(uint32_t mtl_id);
		float OpacityMaterial(uint32_t mtl_id);
		System::String^ Texture(uint32_t mtl_id, TextureSlot slot);
		uint32_t DetailMode(uint32_t mtl_id);
		float HeightOffset(uint32_t mtl_id);
		float HeightScale(uint32_t mtl_id);
		float EdgeTessHint(uint32_t mtl_id);
		float InsideTessHint(uint32_t mtl_id);
		float MinTess(uint32_t mtl_id);
		float MaxTess(uint32_t mtl_id);
		bool TransparentMaterial(uint32_t mtl_id);
		float AlphaTestMaterial(uint32_t mtl_id);
		bool SSSMaterial(uint32_t mtl_id);
		bool TwoSidedMaterial(uint32_t mtl_id);

		void MaterialID(uint32_t mesh_id, uint32_t mtl_id);
		void MaterialName(uint32_t mtl_id, System::String^ name);
		void AlbedoMaterial(uint32_t mtl_id, array<float>^ value);
		void MetalnessMaterial(uint32_t mtl_id, float value);
		void GlossinessMaterial(uint32_t mtl_id, float value);
		void EmissiveMaterial(uint32_t mtl_id, array<float>^ value);
		void OpacityMaterial(uint32_t mtl_id, float value);
		void Texture(uint32_t mtl_id, TextureSlot slot, System::String^ name);
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
		uint32_t ImportMaterial(System::String^ name);
		void ExportMaterial(uint32_t mtl_id, System::String^ name);

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
