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
		core->ExecuteCommand(MakeSharedPtr<MtlEditorCommandSetCurrFrame>(core, frame));
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
		return core->MeshName(index).c_str();
	}

	__declspec(dllexport) uint32_t APIENTRY MaterialID(MtlEditorCore* core, uint32_t mesh_id)
	{
		return core->MaterialID(mesh_id);
	}

	__declspec(dllexport) float const * APIENTRY GetAmbientMaterial(MtlEditorCore* core, uint32_t mtl_id)
	{
		return &core->AmbientMaterial(mtl_id).x();
	}
	__declspec(dllexport) float const * APIENTRY GetDiffuseMaterial(MtlEditorCore* core, uint32_t mtl_id)
	{
		return &core->DiffuseMaterial(mtl_id).x();
	}
	__declspec(dllexport) float const * APIENTRY GetSpecularMaterial(MtlEditorCore* core, uint32_t mtl_id)
	{
		return &core->SpecularMaterial(mtl_id).x();
	}
	__declspec(dllexport) float APIENTRY GetShininessMaterial(MtlEditorCore* core, uint32_t mtl_id)
	{
		return core->ShininessMaterial(mtl_id);
	}
	__declspec(dllexport) float const * APIENTRY GetEmitMaterial(MtlEditorCore* core, uint32_t mtl_id)
	{
		return &core->EmitMaterial(mtl_id).x();
	}
	__declspec(dllexport) float APIENTRY GetOpacityMaterial(MtlEditorCore* core, uint32_t mtl_id)
	{
		return core->OpacityMaterial(mtl_id);
	}
	__declspec(dllexport) char const * APIENTRY GetDiffuseTexture(MtlEditorCore* core, uint32_t mtl_id)
	{
		return core->DiffuseTexture(mtl_id).c_str();
	}
	__declspec(dllexport) char const * APIENTRY GetSpecularTexture(MtlEditorCore* core, uint32_t mtl_id)
	{
		return core->SpecularTexture(mtl_id).c_str();
	}
	__declspec(dllexport) char const * APIENTRY GetShininessTexture(MtlEditorCore* core, uint32_t mtl_id)
	{
		return core->ShininessTexture(mtl_id).c_str();
	}
	__declspec(dllexport) char const * APIENTRY GetNormalTexture(MtlEditorCore* core, uint32_t mtl_id)
	{
		return core->NormalTexture(mtl_id).c_str();
	}
	__declspec(dllexport) char const * APIENTRY GetHeightTexture(MtlEditorCore* core, uint32_t mtl_id)
	{
		return core->HeightTexture(mtl_id).c_str();
	}
	__declspec(dllexport) char const * APIENTRY GetEmitTexture(MtlEditorCore* core, uint32_t mtl_id)
	{
		return core->EmitTexture(mtl_id).c_str();
	}
	__declspec(dllexport) char const * APIENTRY GetOpacityTexture(MtlEditorCore* core, uint32_t mtl_id)
	{
		return core->OpacityTexture(mtl_id).c_str();
	}

	__declspec(dllexport) uint32_t APIENTRY NumHistroyCmds(MtlEditorCore* core)
	{
		return core->NumHistroyCmds();
	}
	__declspec(dllexport) char const * APIENTRY HistroyCmdName(MtlEditorCore* core, uint32_t index)
	{
		return core->HistroyCmdName(index);
	}
	__declspec(dllexport) uint32_t APIENTRY EndCmdIndex(MtlEditorCore* core)
	{
		return core->EndCmdIndex();
	}

	__declspec(dllexport) void APIENTRY SetAmbientMaterial(MtlEditorCore* core, uint32_t mtl_id, float* value)
	{
		core->ExecuteCommand(MakeSharedPtr<MtlEditorCommandSetAmbientMaterial>(core, mtl_id, value));
	}
	__declspec(dllexport) void APIENTRY SetDiffuseMaterial(MtlEditorCore* core, uint32_t mtl_id, float* value)
	{
		core->ExecuteCommand(MakeSharedPtr<MtlEditorCommandSetDiffuseMaterial>(core, mtl_id, value));
	}
	__declspec(dllexport) void APIENTRY SetSpecularMaterial(MtlEditorCore* core, uint32_t mtl_id, float* value)
	{
		core->ExecuteCommand(MakeSharedPtr<MtlEditorCommandSetSpecularMaterial>(core, mtl_id, value));
	}
	__declspec(dllexport) void APIENTRY SetShininessMaterial(MtlEditorCore* core, uint32_t mtl_id, float value)
	{
		core->ExecuteCommand(MakeSharedPtr<MtlEditorCommandSetShininessMaterial>(core, mtl_id, value));
	}
	__declspec(dllexport) void APIENTRY SetEmitMaterial(MtlEditorCore* core, uint32_t mtl_id, float* value)
	{
		core->ExecuteCommand(MakeSharedPtr<MtlEditorCommandSetEmitMaterial>(core, mtl_id, value));
	}
	__declspec(dllexport) void APIENTRY SetOpacityMaterial(MtlEditorCore* core, uint32_t mtl_id, float value)
	{
		core->ExecuteCommand(MakeSharedPtr<MtlEditorCommandSetOpacityMaterial>(core, mtl_id, value));
	}
	__declspec(dllexport) void APIENTRY SetDiffuseTexture(MtlEditorCore* core, uint32_t mtl_id, char const * name)
	{
		core->ExecuteCommand(MakeSharedPtr<MtlEditorCommandSetDiffuseTexture>(core, mtl_id, name));
	}
	__declspec(dllexport) void APIENTRY SetSpecularTexture(MtlEditorCore* core, uint32_t mtl_id, char const * name)
	{
		core->ExecuteCommand(MakeSharedPtr<MtlEditorCommandSetSpecularTexture>(core, mtl_id, name));
	}
	__declspec(dllexport) void APIENTRY SetShininessTexture(MtlEditorCore* core, uint32_t mtl_id, char const * name)
	{
		core->ExecuteCommand(MakeSharedPtr<MtlEditorCommandSetShininessTexture>(core, mtl_id, name));
	}
	__declspec(dllexport) void APIENTRY SetNormalTexture(MtlEditorCore* core, uint32_t mtl_id, char const * name)
	{
		core->ExecuteCommand(MakeSharedPtr<MtlEditorCommandSetNormalTexture>(core, mtl_id, name));
	}
	__declspec(dllexport) void APIENTRY SetHeightTexture(MtlEditorCore* core, uint32_t mtl_id, char const * name)
	{
		core->ExecuteCommand(MakeSharedPtr<MtlEditorCommandSetHeightTexture>(core, mtl_id, name));
	}
	__declspec(dllexport) void APIENTRY SetEmitTexture(MtlEditorCore* core, uint32_t mtl_id, char const * name)
	{
		core->ExecuteCommand(MakeSharedPtr<MtlEditorCommandSetEmitTexture>(core, mtl_id, name));
	}
	__declspec(dllexport) void APIENTRY SetOpacityTexture(MtlEditorCore* core, uint32_t mtl_id, char const * name)
	{
		core->ExecuteCommand(MakeSharedPtr<MtlEditorCommandSetOpacityTexture>(core, mtl_id, name));
	}

	__declspec(dllexport) uint32_t APIENTRY SelectedMesh(MtlEditorCore* core)
	{
		return core->SelectedMesh();
	}
	__declspec(dllexport) void APIENTRY SelectMesh(MtlEditorCore* core, uint32_t mesh_id)
	{
		core->ExecuteCommand(MakeSharedPtr<MtlEditorCommandSelectMesh>(core, mesh_id));
	}

	__declspec(dllexport) void APIENTRY Undo(MtlEditorCore* core)
	{
		core->Undo();
	}
	__declspec(dllexport) void APIENTRY Redo(MtlEditorCore* core)
	{
		core->Redo();
	}
	__declspec(dllexport) void APIENTRY ClearHistroy(MtlEditorCore* core)
	{
		core->ClearHistroy();
	}
}
