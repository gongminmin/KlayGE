#include <KlayGE/KlayGE.hpp>

#ifdef KLAYGE_COMPILER_MSVC
#pragma warning(push)
#pragma warning(disable: 4793) // boost::signals2::detail::do_postconstruct/do_predestruct can't have /clr
#endif
#include "../MtlEditorCore/MtlEditorCore.hpp"
#ifdef KLAYGE_COMPILER_MSVC
#pragma warning(pop)
#endif

#include "MtlEditorCoreWrapper.hpp"

using namespace System;
using namespace System::Runtime::InteropServices;

namespace KlayGE
{
	std::string StringToStd(String^ str)
	{
		using namespace System::Runtime::InteropServices;
		char const * char_str = static_cast<char const *>((Marshal::StringToHGlobalAnsi(str)).ToPointer());
		std::string std_str = char_str;
		Marshal::FreeHGlobal(IntPtr(static_cast<void*>(const_cast<char*>(char_str))));
		return std_str;
	}


	MtlEditorCoreWrapper::MtlEditorCoreWrapper(IntPtr native_wnd)
	{
		Context::Instance().LoadCfg("KlayGE.cfg");

		ContextCfg cfg = Context::Instance().Config();
		cfg.deferred_rendering = true;
		Context::Instance().Config(cfg);

		core_ = new MtlEditorCore(native_wnd.ToPointer());
		core_->Create();
	}

	MtlEditorCoreWrapper::~MtlEditorCoreWrapper()
	{
		core_->Destroy();
		delete core_;
	}

	void MtlEditorCoreWrapper::Refresh()
	{
		core_->Refresh();
	}

	void MtlEditorCoreWrapper::Resize(uint32_t width, uint32_t height)
	{
		core_->Resize(width, height);
	}

	void MtlEditorCoreWrapper::OpenModel(String^ name)
	{
		core_->OpenModel(StringToStd(name));
	}

	void MtlEditorCoreWrapper::SaveModel(String^ name)
	{
		core_->SaveAsModel(StringToStd(name));
	}

	unsigned int MtlEditorCoreWrapper::NumFrames()
	{
		return core_->NumFrames();
	}

	float MtlEditorCoreWrapper::CurrFrame()
	{
		return core_->CurrFrame();
	}

	void MtlEditorCoreWrapper::CurrFrame(float frame)
	{
		core_->CurrFrame(frame);
	}

	float MtlEditorCoreWrapper::ModelFrameRate()
	{
		return core_->ModelFrameRate();
	}

	void MtlEditorCoreWrapper::SkinningOn(int on)
	{
		core_->SkinningOn(on ? true : false);
	}

	void MtlEditorCoreWrapper::FPSCameraOn(int on)
	{
		core_->FPSCameraOn(on ? true : false);
	}

	void MtlEditorCoreWrapper::LineModeOn(int on)
	{
		core_->LineModeOn(on ? true : false);
	}

	void MtlEditorCoreWrapper::Visualize(int index)
	{
		core_->Visualize(index);
	}

	void MtlEditorCoreWrapper::MouseMove(int x, int y, uint32_t button)
	{
		core_->MouseMove(x, y, button);
	}

	void MtlEditorCoreWrapper::MouseUp(int x, int y, uint32_t button)
	{
		core_->MouseUp(x, y, button);
	}

	void MtlEditorCoreWrapper::MouseDown(int x, int y, uint32_t button)
	{
		core_->MouseDown(x, y, button);
	}

	void MtlEditorCoreWrapper::KeyPress(int key)
	{
		core_->KeyPress(key);
	}

	uint32_t MtlEditorCoreWrapper::NumMeshes()
	{
		return core_->NumMeshes();
	}

	String^ MtlEditorCoreWrapper::MeshName(uint32_t index)
	{
		return gcnew String(core_->MeshName(index));
	}

	uint32_t MtlEditorCoreWrapper::NumVertexStreams(uint32_t mesh_id)
	{
		return core_->NumVertexStreams(mesh_id);
	}

	uint32_t MtlEditorCoreWrapper::NumVertexStreamUsages(uint32_t mesh_id, uint32_t stream_index)
	{
		return core_->NumVertexStreamUsages(mesh_id, stream_index);
	}

	uint32_t MtlEditorCoreWrapper::VertexStreamUsage(uint32_t mesh_id, uint32_t stream_index, uint32_t usage_index)
	{
		return core_->VertexStreamUsage(mesh_id, stream_index, usage_index);
	}

	uint32_t MtlEditorCoreWrapper::MaterialID(uint32_t mesh_id)
	{
		return core_->MaterialID(mesh_id);
	}

	array<float>^ MtlEditorCoreWrapper::AmbientMaterial(uint32_t mtl_id)
	{
		auto const & clr = core_->AmbientMaterial(mtl_id);
		array<float>^ ret = { clr.x(), clr.y(), clr.z() };
		return ret;
	}

	array<float>^ MtlEditorCoreWrapper::DiffuseMaterial(uint32_t mtl_id)
	{
		auto const & clr = core_->DiffuseMaterial(mtl_id);
		array<float>^ ret = { clr.x(), clr.y(), clr.z() };
		return ret;
	}

	array<float>^ MtlEditorCoreWrapper::SpecularMaterial(uint32_t mtl_id)
	{
		auto const & clr = core_->SpecularMaterial(mtl_id);
		array<float>^ ret = { clr.x(), clr.y(), clr.z() };
		return ret;
	}

	float MtlEditorCoreWrapper::ShininessMaterial(uint32_t mtl_id)
	{
		return core_->ShininessMaterial(mtl_id);
	}

	array<float>^ MtlEditorCoreWrapper::EmitMaterial(uint32_t mtl_id)
	{
		auto const & clr = core_->EmitMaterial(mtl_id);
		array<float>^ ret = { clr.x(), clr.y(), clr.z() };
		return ret;
	}

	float MtlEditorCoreWrapper::OpacityMaterial(uint32_t mtl_id)
	{
		return core_->OpacityMaterial(mtl_id);
	}

	String^ MtlEditorCoreWrapper::DiffuseTexture(uint32_t mtl_id)
	{
		return gcnew String(core_->DiffuseTexture(mtl_id));
	}

	String^MtlEditorCoreWrapper::SpecularTexture(uint32_t mtl_id)
	{
		return gcnew String(core_->SpecularTexture(mtl_id));
	}

	String^ MtlEditorCoreWrapper::ShininessTexture(uint32_t mtl_id)
	{
		return gcnew String(core_->ShininessTexture(mtl_id));
	}

	String^ MtlEditorCoreWrapper::NormalTexture(uint32_t mtl_id)
	{
		return gcnew String(core_->NormalTexture(mtl_id));
	}

	String^ MtlEditorCoreWrapper::HeightTexture(uint32_t mtl_id)
	{
		return gcnew String(core_->HeightTexture(mtl_id));
	}

	String^ MtlEditorCoreWrapper::EmitTexture(uint32_t mtl_id)
	{
		return gcnew String(core_->EmitTexture(mtl_id));
	}

	String^ MtlEditorCoreWrapper::OpacityTexture(uint32_t mtl_id)
	{
		return gcnew String(core_->OpacityTexture(mtl_id));
	}

	uint32_t MtlEditorCoreWrapper::DetailMode(uint32_t mtl_id)
	{
		return core_->DetailMode(mtl_id);
	}

	float MtlEditorCoreWrapper::HeightOffset(uint32_t mtl_id)
	{
		return core_->HeightOffset(mtl_id);
	}

	float MtlEditorCoreWrapper::HeightScale(uint32_t mtl_id)
	{
		return core_->HeightScale(mtl_id);
	}

	float MtlEditorCoreWrapper::EdgeTessHint(uint32_t mtl_id)
	{
		return core_->EdgeTessHint(mtl_id);
	}

	float MtlEditorCoreWrapper::InsideTessHint(uint32_t mtl_id)
	{
		return core_->InsideTessHint(mtl_id);
	}

	float MtlEditorCoreWrapper::MinTess(uint32_t mtl_id)
	{
		return core_->MinTess(mtl_id);
	}

	float MtlEditorCoreWrapper::MaxTess(uint32_t mtl_id)
	{
		return core_->MaxTess(mtl_id);
	}

	void MtlEditorCoreWrapper::AmbientMaterial(uint32_t mtl_id, array<float>^ value)
	{
		float r = value[0];
		float g = value[1];
		float b = value[2];
		core_->AmbientMaterial(mtl_id, float3(r, g, b));
	}

	void MtlEditorCoreWrapper::DiffuseMaterial(uint32_t mtl_id, array<float>^ value)
	{
		float r = value[0];
		float g = value[1];
		float b = value[2];
		core_->DiffuseMaterial(mtl_id, float3(r, g, b));
	}

	void MtlEditorCoreWrapper::SpecularMaterial(uint32_t mtl_id, array<float>^ value)
	{
		float r = value[0];
		float g = value[1];
		float b = value[2];
		core_->SpecularMaterial(mtl_id, float3(r, g, b));
	}

	void MtlEditorCoreWrapper::ShininessMaterial(uint32_t mtl_id, float value)
	{
		core_->ShininessMaterial(mtl_id, value);
	}

	void MtlEditorCoreWrapper::EmitMaterial(uint32_t mtl_id, array<float>^ value)
	{
		float r = value[0];
		float g = value[1];
		float b = value[2];
		core_->EmitMaterial(mtl_id, float3(r, g, b));
	}

	void MtlEditorCoreWrapper::OpacityMaterial(uint32_t mtl_id, float value)
	{
		core_->OpacityMaterial(mtl_id, value);
	}

	void MtlEditorCoreWrapper::DiffuseTexture(uint32_t mtl_id, String^ name)
	{
		core_->DiffuseTexture(mtl_id, StringToStd(name));
	}

	void MtlEditorCoreWrapper::SpecularTexture(uint32_t mtl_id, String^ name)
	{
		core_->SpecularTexture(mtl_id, StringToStd(name));
	}

	void MtlEditorCoreWrapper::ShininessTexture(uint32_t mtl_id, String^ name)
	{
		core_->ShininessTexture(mtl_id, StringToStd(name));
	}

	void MtlEditorCoreWrapper::NormalTexture(uint32_t mtl_id, String^ name)
	{
		core_->NormalTexture(mtl_id, StringToStd(name));
	}

	void MtlEditorCoreWrapper::HeightTexture(uint32_t mtl_id, String^ name)
	{
		core_->HeightTexture(mtl_id, StringToStd(name));
	}

	void MtlEditorCoreWrapper::EmitTexture(uint32_t mtl_id, String^ name)
	{
		core_->EmitTexture(mtl_id, StringToStd(name));
	}

	void MtlEditorCoreWrapper::OpacityTexture(uint32_t mtl_id, String^ name)
	{
		core_->OpacityTexture(mtl_id, StringToStd(name));
	}

	void MtlEditorCoreWrapper::DetailMode(uint32_t mtl_id, uint32_t value)
	{
		core_->DetailMode(mtl_id, value);
	}

	void MtlEditorCoreWrapper::HeightOffset(uint32_t mtl_id, float value)
	{
		core_->HeightOffset(mtl_id, value);
	}

	void MtlEditorCoreWrapper::HeightScale(uint32_t mtl_id, float value)
	{
		core_->HeightScale(mtl_id, value);
	}

	void MtlEditorCoreWrapper::EdgeTessHint(uint32_t mtl_id, float value)
	{
		core_->EdgeTessHint(mtl_id, value);
	}

	void MtlEditorCoreWrapper::InsideTessHint(uint32_t mtl_id, float value)
	{
		core_->InsideTessHint(mtl_id, value);
	}

	void MtlEditorCoreWrapper::MinTess(uint32_t mtl_id, float value)
	{
		core_->MinTess(mtl_id, value);
	}

	void MtlEditorCoreWrapper::MaxTess(uint32_t mtl_id, float value)
	{
		core_->MaxTess(mtl_id, value);
	}

	uint32_t MtlEditorCoreWrapper::SelectedMesh()
	{
		return core_->SelectedMesh();
	}

	void MtlEditorCoreWrapper::SelectMesh(uint32_t mesh_id)
	{
		return core_->SelectMesh(mesh_id);
	}

	void MtlEditorCoreWrapper::UpdateSelectEntityCallback(UpdateSelectEntityDelegate^ callback)
	{
		update_select_entity_delegate_ = callback;

		IntPtr ip = Marshal::GetFunctionPointerForDelegate(callback);
		core_->UpdateSelectEntityCallback(static_cast<MtlEditorCore::UpdateSelectEntityEvent>(ip.ToPointer()));
	}
}
