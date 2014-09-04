#include <KlayGE/KlayGE.hpp>
#include "MtlEditorCore.hpp"

#ifdef KLAYGE_COMPILER_MSVC
#define APIENTRY __stdcall
#else
#define APIENTRY
#endif

using namespace KlayGE;

extern "C"
{
	__declspec(dllexport) void* APIENTRY Create(void* native_wnd)
	{
		Context::Instance().LoadCfg("KlayGE.cfg");

		ContextCfg cfg = Context::Instance().Config();
		cfg.deferred_rendering = true;
		Context::Instance().Config(cfg);

		MtlEditorCore* core = new MtlEditorCore(native_wnd);
		core->Create();
		return core;
	}

	__declspec(dllexport) void APIENTRY Destroy(MtlEditorCore* core)
	{
		core->Destroy();
		delete core;
	}

	__declspec(dllexport) void APIENTRY Refresh(MtlEditorCore* core)
	{
		core->Refresh();
	}

	__declspec(dllexport) void APIENTRY Resize(MtlEditorCore* core, uint32_t width, uint32_t height)
	{
		core->Resize(width, height);
	}

	__declspec(dllexport) void APIENTRY OpenModel(MtlEditorCore* core, char const * name)
	{
		core->OpenModel(name);
	}
	__declspec(dllexport) void APIENTRY SaveModel(MtlEditorCore* core, char const * name)
	{
		core->SaveAsModel(name);
	}

	__declspec(dllexport) unsigned int APIENTRY NumFrames(MtlEditorCore* core)
	{
		return core->NumFrames();
	}
	__declspec(dllexport) void APIENTRY CurrFrame(MtlEditorCore* core, float frame)
	{
		core->CurrFrame(frame);
	}
	__declspec(dllexport) float APIENTRY ModelFrameRate(MtlEditorCore* core)
	{
		return core->ModelFrameRate();
	}

	__declspec(dllexport) void APIENTRY SkinningOn(MtlEditorCore* core, int on)
	{
		core->SkinningOn(on ? true : false);
	}
	__declspec(dllexport) void APIENTRY FPSCameraOn(MtlEditorCore* core, int on)
	{
		core->FPSCameraOn(on ? true : false);
	}

	__declspec(dllexport) void APIENTRY Visualize(MtlEditorCore* core, int index)
	{
		core->Visualize(index);
	}

	__declspec(dllexport) void APIENTRY MouseMove(MtlEditorCore* core, int x, int y, uint32_t button)
	{
		core->MouseMove(x, y, button);
	}
	__declspec(dllexport) void APIENTRY MouseUp(MtlEditorCore* core, int x, int y, uint32_t button)
	{
		core->MouseUp(x, y, button);
	}
	__declspec(dllexport) void APIENTRY MouseDown(MtlEditorCore* core, int x, int y, uint32_t button)
	{
		core->MouseDown(x, y, button);
	}
	__declspec(dllexport) void APIENTRY KeyPress(MtlEditorCore* core, int key)
	{
		core->KeyPress(key);
	}

	__declspec(dllexport) uint32_t APIENTRY NumMeshes(MtlEditorCore* core)
	{
		return core->NumMeshes();
	}

	__declspec(dllexport) wchar_t const * APIENTRY MeshName(MtlEditorCore* core, uint32_t index)
	{
		return core->MeshName(index);
	}

	__declspec(dllexport) uint32_t APIENTRY MaterialID(MtlEditorCore* core, uint32_t mesh_index)
	{
		return core->MaterialID(mesh_index);
	}

	__declspec(dllexport) float* APIENTRY GetAmbientMaterial(MtlEditorCore* core, uint32_t material_index)
	{
		return core->AmbientMaterial(material_index);
	}
	__declspec(dllexport) float* APIENTRY GetDiffuseMaterial(MtlEditorCore* core, uint32_t material_index)
	{
		return core->DiffuseMaterial(material_index);
	}
	__declspec(dllexport) float* APIENTRY GetSpecularMaterial(MtlEditorCore* core, uint32_t material_index)
	{
		return core->SpecularMaterial(material_index);
	}
	__declspec(dllexport) float APIENTRY GetShininessMaterial(MtlEditorCore* core, uint32_t material_index)
	{
		return core->ShininessMaterial(material_index);
	}
	__declspec(dllexport) float* APIENTRY GetEmitMaterial(MtlEditorCore* core, uint32_t material_index)
	{
		return core->EmitMaterial(material_index);
	}
	__declspec(dllexport) float APIENTRY GetOpacityMaterial(MtlEditorCore* core, uint32_t material_index)
	{
		return core->OpacityMaterial(material_index);
	}
	__declspec(dllexport) char const * APIENTRY GetDiffuseTexture(MtlEditorCore* core, uint32_t material_index)
	{
		return core->DiffuseTexture(material_index);
	}
	__declspec(dllexport) char const * APIENTRY GetSpecularTexture(MtlEditorCore* core, uint32_t material_index)
	{
		return core->SpecularTexture(material_index);
	}
	__declspec(dllexport) char const * APIENTRY GetShininessTexture(MtlEditorCore* core, uint32_t material_index)
	{
		return core->ShininessTexture(material_index);
	}
	__declspec(dllexport) char const * APIENTRY GetBumpTexture(MtlEditorCore* core, uint32_t material_index)
	{
		return core->BumpTexture(material_index);
	}
	__declspec(dllexport) char const * APIENTRY GetHeightTexture(MtlEditorCore* core, uint32_t material_index)
	{
		return core->HeightTexture(material_index);
	}
	__declspec(dllexport) char const * APIENTRY GetEmitTexture(MtlEditorCore* core, uint32_t material_index)
	{
		return core->EmitTexture(material_index);
	}
	__declspec(dllexport) char const * APIENTRY GetOpacityTexture(MtlEditorCore* core, uint32_t material_index)
	{
		return core->OpacityTexture(material_index);
	}

	__declspec(dllexport) void APIENTRY SetAmbientMaterial(MtlEditorCore* core,
			uint32_t material_index, float* value)
	{
		core->AmbientMaterial(material_index, value);
	}
	__declspec(dllexport) void APIENTRY SetDiffuseMaterial(MtlEditorCore* core,
			uint32_t material_index, float* value)
	{
		core->DiffuseMaterial(material_index, value);
	}
	__declspec(dllexport) void APIENTRY SetSpecularMaterial(MtlEditorCore* core,
			uint32_t material_index, float* value)
	{
		core->SpecularMaterial(material_index, value);
	}
	__declspec(dllexport) void APIENTRY SetShininessMaterial(MtlEditorCore* core,
			uint32_t material_index, float value)
	{
		core->ShininessMaterial(material_index, value);
	}
	__declspec(dllexport) void APIENTRY SetEmitMaterial(MtlEditorCore* core,
			uint32_t material_index, float* value)
	{
		core->EmitMaterial(material_index, value);
	}
	__declspec(dllexport) void APIENTRY SetOpacityMaterial(MtlEditorCore* core,
			uint32_t material_index, float value)
	{
		core->OpacityMaterial(material_index, value);
	}
	__declspec(dllexport) void APIENTRY SetDiffuseTexture(MtlEditorCore* core,
			uint32_t material_index, char const * name)
	{
		core->DiffuseTexture(material_index, name);
	}
	__declspec(dllexport) void APIENTRY SetSpecularTexture(MtlEditorCore* core,
			uint32_t material_index, char const * name)
	{
		core->SpecularTexture(material_index, name);
	}
	__declspec(dllexport) void APIENTRY SetShininessTexture(MtlEditorCore* core,
			uint32_t material_index, char const * name)
	{
		core->ShininessTexture(material_index, name);
	}
	__declspec(dllexport) void APIENTRY SetBumpTexture(MtlEditorCore* core,
			uint32_t material_index, char const * name)
	{
		core->BumpTexture(material_index, name);
	}
	__declspec(dllexport) void APIENTRY SetHeightTexture(MtlEditorCore* core,
			uint32_t material_index, char const * name)
	{
		core->HeightTexture(material_index, name);
	}
	__declspec(dllexport) void APIENTRY SetEmitTexture(MtlEditorCore* core,
			uint32_t material_index, char const * name)
	{
		core->EmitTexture(material_index, name);
	}
	__declspec(dllexport) void APIENTRY SetOpacityTexture(MtlEditorCore* core,
			uint32_t material_index, char const * name)
	{
		core->OpacityTexture(material_index, name);
	}

	__declspec(dllexport) uint32_t APIENTRY SelectedMesh(MtlEditorCore* core)
	{
		return core->SelectedMesh();
	}
	__declspec(dllexport) void APIENTRY SelectMesh(MtlEditorCore* core, uint32_t mesh_index)
	{
		return core->SelectMesh(mesh_index);
	}
}
