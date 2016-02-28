#include <KlayGE/KlayGE.hpp>

#ifdef KLAYGE_COMPILER_MSVC
#pragma warning(push)
#pragma warning(disable: 4793) // boost::signals2::detail::do_postconstruct/do_predestruct can't have /clr
#endif
#include "../MtlEditorCore/MtlEditorCore.hpp"
#include "../MtlEditorCore/Commands.hpp"
#ifdef KLAYGE_COMPILER_MSVC
#pragma warning(pop)
#endif

#include "MtlEditorCoreWrapper.hpp"

using namespace System;

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

	String^ StdToString(std::string const & str)
	{
		return gcnew String(str.c_str());
	}

	String^ WStdToString(std::wstring const & str)
	{
		return gcnew String(str.c_str());
	}

	float LinearToSRGB(float linear)
	{
		if (linear < 0.0031308f)
		{
			return 12.92f * linear;
		}
		else
		{
			const float ALPHA = 0.055f;
			return (1 + ALPHA) * pow(linear, 1 / 2.4f) - ALPHA;
		}
	}

	float SRGBToLinear(float srgb)
	{
		if (srgb < 0.04045f)
		{
			return srgb / 12.92f;
		}
		else
		{
			const float ALPHA = 0.055f;
			return pow((srgb + ALPHA) / (1 + ALPHA), 2.4f);
		}
	}

	Windows::Media::Color FloatPtrToColor(float const * clr)
	{
		float temp[3];
		for (int i = 0; i < 3; ++ i)
		{
			temp[i] = LinearToSRGB(clr[i]);
		}
		return Windows::Media::Color::FromArgb(255,
			static_cast<uint8_t>(Math::Max(Math::Min(static_cast<int>(temp[0] * 255 + 0.5f), 255), 0)),
			static_cast<uint8_t>(Math::Max(Math::Min(static_cast<int>(temp[1] * 255 + 0.5f), 255), 0)),
			static_cast<uint8_t>(Math::Max(Math::Min(static_cast<int>(temp[2] * 255 + 0.5f), 255), 0)));
	}

	void ColorToFloatPtr(Windows::Media::Color clr, float output[3])
	{
		output[0] = clr.R / 255.0f;
		output[1] = clr.G / 255.0f;
		output[2] = clr.B / 255.0f;
		for (int i = 0; i < 3; ++i)
		{
			output[i] = SRGBToLinear(output[i]);
		}
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
		return WStdToString(core_->MeshName(index));
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

	Windows::Media::Color MtlEditorCoreWrapper::AmbientMaterial(uint32_t mtl_id)
	{
		return FloatPtrToColor(&core_->AmbientMaterial(mtl_id).x());
	}

	Windows::Media::Color MtlEditorCoreWrapper::DiffuseMaterial(uint32_t mtl_id)
	{
		return FloatPtrToColor(&core_->DiffuseMaterial(mtl_id).x());
	}

	Windows::Media::Color MtlEditorCoreWrapper::SpecularMaterial(uint32_t mtl_id)
	{
		return FloatPtrToColor(&core_->SpecularMaterial(mtl_id).x());
	}

	float MtlEditorCoreWrapper::ShininessMaterial(uint32_t mtl_id)
	{
		return core_->ShininessMaterial(mtl_id);
	}

	Windows::Media::Color MtlEditorCoreWrapper::EmitMaterial(uint32_t mtl_id)
	{
		return FloatPtrToColor(&core_->EmitMaterial(mtl_id).x());
	}

	float MtlEditorCoreWrapper::OpacityMaterial(uint32_t mtl_id)
	{
		return core_->OpacityMaterial(mtl_id);
	}

	String^ MtlEditorCoreWrapper::DiffuseTexture(uint32_t mtl_id)
	{
		return StdToString(core_->DiffuseTexture(mtl_id));
	}

	String^MtlEditorCoreWrapper::SpecularTexture(uint32_t mtl_id)
	{
		return StdToString(core_->SpecularTexture(mtl_id));
	}

	String^ MtlEditorCoreWrapper::ShininessTexture(uint32_t mtl_id)
	{
		return StdToString(core_->ShininessTexture(mtl_id));
	}

	String^ MtlEditorCoreWrapper::NormalTexture(uint32_t mtl_id)
	{
		return StdToString(core_->NormalTexture(mtl_id));
	}

	String^ MtlEditorCoreWrapper::HeightTexture(uint32_t mtl_id)
	{
		return StdToString(core_->HeightTexture(mtl_id));
	}

	String^ MtlEditorCoreWrapper::EmitTexture(uint32_t mtl_id)
	{
		return StdToString(core_->EmitTexture(mtl_id));
	}

	String^ MtlEditorCoreWrapper::OpacityTexture(uint32_t mtl_id)
	{
		return StdToString(core_->OpacityTexture(mtl_id));
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

	uint32_t MtlEditorCoreWrapper::NumHistroyCmds()
	{
		return core_->NumHistroyCmds();
	}

	System::String^ MtlEditorCoreWrapper::HistroyCmdName(uint32_t index)
	{
		return StdToString(core_->HistroyCmdName(index));
	}

	uint32_t MtlEditorCoreWrapper::EndCmdIndex()
	{
		return core_->NumHistroyCmds();
	}

	void MtlEditorCoreWrapper::AmbientMaterial(uint32_t mtl_id, System::Windows::Media::Color value)
	{
		float clr[3];
		ColorToFloatPtr(value, clr);
		core_->ExecuteCommand(MtlEditorCommandPtr(new MtlEditorCommandSetAmbientMaterial(core_, mtl_id, clr)));
	}

	void MtlEditorCoreWrapper::DiffuseMaterial(uint32_t mtl_id, System::Windows::Media::Color value)
	{
		float clr[3];
		ColorToFloatPtr(value, clr);
		core_->ExecuteCommand(MtlEditorCommandPtr(new MtlEditorCommandSetDiffuseMaterial(core_, mtl_id, clr)));
	}

	void MtlEditorCoreWrapper::SpecularMaterial(uint32_t mtl_id, System::Windows::Media::Color value)
	{
		float clr[3];
		ColorToFloatPtr(value, clr);
		core_->ExecuteCommand(MtlEditorCommandPtr(new MtlEditorCommandSetSpecularMaterial(core_, mtl_id, clr)));
	}

	void MtlEditorCoreWrapper::ShininessMaterial(uint32_t mtl_id, float value)
	{
		core_->ExecuteCommand(MtlEditorCommandPtr(new MtlEditorCommandSetShininessMaterial(core_, mtl_id, value)));
	}

	void MtlEditorCoreWrapper::EmitMaterial(uint32_t mtl_id, System::Windows::Media::Color value)
	{
		float clr[3];
		ColorToFloatPtr(value, clr);
		core_->ExecuteCommand(MtlEditorCommandPtr(new MtlEditorCommandSetEmitMaterial(core_, mtl_id, clr)));
	}

	void MtlEditorCoreWrapper::OpacityMaterial(uint32_t mtl_id, float value)
	{
		core_->ExecuteCommand(MtlEditorCommandPtr(new MtlEditorCommandSetOpacityMaterial(core_, mtl_id, value)));
	}

	void MtlEditorCoreWrapper::DiffuseTexture(uint32_t mtl_id, String^ name)
	{
		core_->ExecuteCommand(MtlEditorCommandPtr(new MtlEditorCommandSetDiffuseTexture(core_, mtl_id, StringToStd(name).c_str())));
	}

	void MtlEditorCoreWrapper::SpecularTexture(uint32_t mtl_id, String^ name)
	{
		core_->ExecuteCommand(MtlEditorCommandPtr(new MtlEditorCommandSetSpecularTexture(core_, mtl_id, StringToStd(name).c_str())));
	}

	void MtlEditorCoreWrapper::ShininessTexture(uint32_t mtl_id, String^ name)
	{
		core_->ExecuteCommand(MtlEditorCommandPtr(new MtlEditorCommandSetShininessTexture(core_, mtl_id, StringToStd(name).c_str())));
	}

	void MtlEditorCoreWrapper::NormalTexture(uint32_t mtl_id, String^ name)
	{
		core_->ExecuteCommand(MtlEditorCommandPtr(new MtlEditorCommandSetNormalTexture(core_, mtl_id, StringToStd(name).c_str())));
	}

	void MtlEditorCoreWrapper::HeightTexture(uint32_t mtl_id, String^ name)
	{
		core_->ExecuteCommand(MtlEditorCommandPtr(new MtlEditorCommandSetHeightTexture(core_, mtl_id, StringToStd(name).c_str())));
	}

	void MtlEditorCoreWrapper::EmitTexture(uint32_t mtl_id, String^ name)
	{
		core_->ExecuteCommand(MtlEditorCommandPtr(new MtlEditorCommandSetEmitTexture(core_, mtl_id, StringToStd(name).c_str())));
	}

	void MtlEditorCoreWrapper::OpacityTexture(uint32_t mtl_id, String^ name)
	{
		core_->ExecuteCommand(MtlEditorCommandPtr(new MtlEditorCommandSetOpacityTexture(core_, mtl_id, StringToStd(name).c_str())));
	}

	void MtlEditorCoreWrapper::DetailMode(uint32_t mtl_id, uint32_t value)
	{
		core_->ExecuteCommand(MtlEditorCommandPtr(new MtlEditorCommandSetDetailMode(core_, mtl_id, value)));
	}

	void MtlEditorCoreWrapper::HeightOffset(uint32_t mtl_id, float value)
	{
		core_->ExecuteCommand(MtlEditorCommandPtr(new MtlEditorCommandSetHeightOffset(core_, mtl_id, value)));
	}

	void MtlEditorCoreWrapper::HeightScale(uint32_t mtl_id, float value)
	{
		core_->ExecuteCommand(MtlEditorCommandPtr(new MtlEditorCommandSetHeightScale(core_, mtl_id, value)));
	}

	void MtlEditorCoreWrapper::EdgeTessHint(uint32_t mtl_id, float value)
	{
		core_->ExecuteCommand(MtlEditorCommandPtr(new MtlEditorCommandSetEdgeTessHint(core_, mtl_id, value)));
	}

	void MtlEditorCoreWrapper::InsideTessHint(uint32_t mtl_id, float value)
	{
		core_->ExecuteCommand(MtlEditorCommandPtr(new MtlEditorCommandSetInsideTessHint(core_, mtl_id, value)));
	}

	void MtlEditorCoreWrapper::MinTess(uint32_t mtl_id, float value)
	{
		core_->ExecuteCommand(MtlEditorCommandPtr(new MtlEditorCommandSetMinTess(core_, mtl_id, value)));
	}

	void MtlEditorCoreWrapper::MaxTess(uint32_t mtl_id, float value)
	{
		core_->ExecuteCommand(MtlEditorCommandPtr(new MtlEditorCommandSetMaxTess(core_, mtl_id, value)));
	}

	uint32_t MtlEditorCoreWrapper::SelectedMesh()
	{
		return core_->SelectedMesh();
	}

	void MtlEditorCoreWrapper::SelectMesh(uint32_t mesh_id)
	{
		return core_->SelectMesh(mesh_id);
	}

	void MtlEditorCoreWrapper::Undo()
	{
		core_->Undo();
	}

	void MtlEditorCoreWrapper::Redo()
	{
		core_->Redo();
	}

	void MtlEditorCoreWrapper::ClearHistroy()
	{
		core_->ClearHistroy();
	}
}
