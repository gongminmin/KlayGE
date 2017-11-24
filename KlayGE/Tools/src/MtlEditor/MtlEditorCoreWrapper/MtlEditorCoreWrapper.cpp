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

	bool MtlEditorCoreWrapper::OpenModel(String^ name)
	{
		return core_->OpenModel(StringToStd(name));
	}

	void MtlEditorCoreWrapper::SaveModel(String^ name)
	{
		core_->SaveAsModel(StringToStd(name));
	}

	System::String^ MtlEditorCoreWrapper::SkyboxName()
	{
		return gcnew String(core_->SkyboxName());
	}

	void MtlEditorCoreWrapper::SkyboxName(String^ name)
	{
		core_->SkyboxName(StringToStd(name));
	}

	void MtlEditorCoreWrapper::DisplaySSVO(bool ssvo)
	{
		core_->DisplaySSVO(ssvo);
	}

	void MtlEditorCoreWrapper::DisplayHDR(bool hdr)
	{
		core_->DisplayHDR(hdr);
	}

	void MtlEditorCoreWrapper::DisplayAA(bool aa)
	{
		core_->DisplayAA(aa);
	}

	void MtlEditorCoreWrapper::DisplayGamma(bool gamma)
	{
		core_->DisplayGamma(gamma);
	}

	void MtlEditorCoreWrapper::DisplayColorGrading(bool cg)
	{
		core_->DisplayColorGrading(cg);
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

	void MtlEditorCoreWrapper::SkeletonOn(int on)
	{
		core_->SkeletonOn(on ? true : false);
	}

	void MtlEditorCoreWrapper::LightOn(int on)
	{
		core_->LightOn(on ? true : false);
	}

	void MtlEditorCoreWrapper::FPSCameraOn(int on)
	{
		core_->FPSCameraOn(on ? true : false);
	}

	void MtlEditorCoreWrapper::LineModeOn(int on)
	{
		core_->LineModeOn(on ? true : false);
	}

	void MtlEditorCoreWrapper::ImposterModeOn(int on)
	{
		core_->ImposterModeOn(on ? true : false);
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

	uint32_t MtlEditorCoreWrapper::NumLods()
	{
		return core_->NumLods();
	}

	void MtlEditorCoreWrapper::ActiveLod(int lod)
	{
		core_->ActiveLod(lod);
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

	uint32_t MtlEditorCoreWrapper::NumMaterials()
	{
		return core_->NumMaterials();
	}

	System::String^ MtlEditorCoreWrapper::MaterialName(uint32_t mtl_id)
	{
		return gcnew String(core_->MaterialName(mtl_id));
	}

	array<float>^ MtlEditorCoreWrapper::AlbedoMaterial(uint32_t mtl_id)
	{
		auto const & clr = core_->AlbedoMaterial(mtl_id);
		array<float>^ ret = { clr.x(), clr.y(), clr.z() };
		return ret;
	}

	float MtlEditorCoreWrapper::MetalnessMaterial(uint32_t mtl_id)
	{
		return core_->MetalnessMaterial(mtl_id);
	}

	float MtlEditorCoreWrapper::GlossinessMaterial(uint32_t mtl_id)
	{
		return core_->GlossinessMaterial(mtl_id);
	}

	array<float>^ MtlEditorCoreWrapper::EmissiveMaterial(uint32_t mtl_id)
	{
		auto const & clr = core_->EmissiveMaterial(mtl_id);
		array<float>^ ret = { clr.x(), clr.y(), clr.z() };
		return ret;
	}

	float MtlEditorCoreWrapper::OpacityMaterial(uint32_t mtl_id)
	{
		return core_->OpacityMaterial(mtl_id);
	}

	String^ MtlEditorCoreWrapper::Texture(uint32_t mtl_id, TextureSlot slot)
	{
		return gcnew String(core_->Texture(mtl_id, static_cast<uint32_t>(slot)));
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

	bool MtlEditorCoreWrapper::TransparentMaterial(uint32_t mtl_id)
	{
		return core_->TransparentMaterial(mtl_id);
	}

	float MtlEditorCoreWrapper::AlphaTestMaterial(uint32_t mtl_id)
	{
		return core_->AlphaTestMaterial(mtl_id);
	}

	bool MtlEditorCoreWrapper::SSSMaterial(uint32_t mtl_id)
	{
		return core_->SSSMaterial(mtl_id);
	}

	bool MtlEditorCoreWrapper::TwoSidedMaterial(uint32_t mtl_id)
	{
		return core_->TwoSidedMaterial(mtl_id);
	}

	void MtlEditorCoreWrapper::MaterialID(uint32_t mesh_id, uint32_t mtl_id)
	{
		core_->MaterialID(mesh_id, mtl_id);
	}

	void MtlEditorCoreWrapper::MaterialName(uint32_t mtl_id, System::String^ name)
	{
		core_->MaterialName(mtl_id, StringToStd(name));
	}

	void MtlEditorCoreWrapper::AlbedoMaterial(uint32_t mtl_id, array<float>^ value)
	{
		float r = value[0];
		float g = value[1];
		float b = value[2];
		core_->AlbedoMaterial(mtl_id, float3(r, g, b));
	}

	void MtlEditorCoreWrapper::MetalnessMaterial(uint32_t mtl_id, float value)
	{
		core_->MetalnessMaterial(mtl_id, value);
	}

	void MtlEditorCoreWrapper::GlossinessMaterial(uint32_t mtl_id, float value)
	{
		core_->GlossinessMaterial(mtl_id, value);
	}

	void MtlEditorCoreWrapper::EmissiveMaterial(uint32_t mtl_id, array<float>^ value)
	{
		float r = value[0];
		float g = value[1];
		float b = value[2];
		core_->EmissiveMaterial(mtl_id, float3(r, g, b));
	}

	void MtlEditorCoreWrapper::OpacityMaterial(uint32_t mtl_id, float value)
	{
		core_->OpacityMaterial(mtl_id, value);
	}

	void MtlEditorCoreWrapper::Texture(uint32_t mtl_id, TextureSlot slot, String^ name)
	{
		core_->Texture(mtl_id, static_cast<uint32_t>(slot), StringToStd(name));
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

	void MtlEditorCoreWrapper::TransparentMaterial(uint32_t mtl_id, bool value)
	{
		core_->TransparentMaterial(mtl_id, value);
	}

	void MtlEditorCoreWrapper::AlphaTestMaterial(uint32_t mtl_id, float value)
	{
		core_->AlphaTestMaterial(mtl_id, value);
	}

	void MtlEditorCoreWrapper::SSSMaterial(uint32_t mtl_id, bool value)
	{
		core_->SSSMaterial(mtl_id, value);
	}

	void MtlEditorCoreWrapper::TwoSidedMaterial(uint32_t mtl_id, bool value)
	{
		core_->TwoSidedMaterial(mtl_id, value);
	}

	uint32_t MtlEditorCoreWrapper::CopyMaterial(uint32_t mtl_id)
	{
		return core_->CopyMaterial(mtl_id);
	}

	uint32_t MtlEditorCoreWrapper::ImportMaterial(System::String^ name)
	{
		return core_->ImportMaterial(StringToStd(name));
	}

	void MtlEditorCoreWrapper::ExportMaterial(uint32_t mtl_id, String^ name)
	{
		core_->ExportMaterial(mtl_id, StringToStd(name));
	}

	uint32_t MtlEditorCoreWrapper::SelectedMesh()
	{
		return core_->SelectedMesh();
	}

	void MtlEditorCoreWrapper::SelectMesh(uint32_t mesh_id)
	{
		core_->SelectMesh(mesh_id);
	}

	void MtlEditorCoreWrapper::UpdateSelectEntityCallback(UpdateSelectEntityDelegate^ callback)
	{
		update_select_entity_delegate_ = callback;

		IntPtr ip = Marshal::GetFunctionPointerForDelegate(callback);
		core_->UpdateSelectEntityCallback(static_cast<MtlEditorCore::UpdateSelectEntityEvent>(ip.ToPointer()));
	}
}
