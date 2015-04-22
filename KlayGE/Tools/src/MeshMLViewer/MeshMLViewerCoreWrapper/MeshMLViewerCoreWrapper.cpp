#include <KlayGE/KlayGE.hpp>

#ifdef KLAYGE_COMPILER_MSVC
#pragma warning(push)
#pragma warning(disable: 4793)
#endif
#include "../MeshMLViewerCore/MeshMLViewerCore.hpp"
#ifdef KLAYGE_COMPILER_MSVC
#pragma warning(pop)
#endif

#include "MeshMLViewerCoreWrapper.hpp"

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


	MeshMLViewerCoreWrapper::MeshMLViewerCoreWrapper(IntPtr native_wnd)
	{
		Context::Instance().LoadCfg("KlayGE.cfg");

		ContextCfg cfg = Context::Instance().Config();
		cfg.deferred_rendering = true;
		Context::Instance().Config(cfg);

		core_ = new MeshMLViewerCore(native_wnd.ToPointer());
		core_->Create();
	}

	MeshMLViewerCoreWrapper::~MeshMLViewerCoreWrapper()
	{
		core_->Destroy();
		delete core_;
	}

	void MeshMLViewerCoreWrapper::Refresh()
	{
		core_->Refresh();
	}

	void MeshMLViewerCoreWrapper::Resize(uint32_t width, uint32_t height)
	{
		core_->Resize(width, height);
	}

	void MeshMLViewerCoreWrapper::OpenModel(String^ name)
	{
		core_->OpenModel(StringToStd(name));
	}

	void MeshMLViewerCoreWrapper::SaveModel(String^ name)
	{
		core_->SaveAsModel(StringToStd(name));
	}

	unsigned int MeshMLViewerCoreWrapper::NumFrames()
	{
		return core_->NumFrames();
	}

	void MeshMLViewerCoreWrapper::CurrFrame(float frame)
	{
		core_->CurrFrame(frame);
	}

	float MeshMLViewerCoreWrapper::ModelFrameRate()
	{
		return core_->ModelFrameRate();
	}

	void MeshMLViewerCoreWrapper::SkinningOn(int on)
	{
		core_->SkinningOn(on ? true : false);
	}

	void MeshMLViewerCoreWrapper::SmoothMeshOn(int on)
	{
		core_->SmoothMeshOn(on ? true : false);
	}

	void MeshMLViewerCoreWrapper::FPSCameraOn(int on)
	{
		core_->FPSCameraOn(on ? true : false);
	}

	void MeshMLViewerCoreWrapper::LineModeOn(int on)
	{
		core_->LineModeOn(on ? true : false);
	}

	void MeshMLViewerCoreWrapper::Visualize(int index)
	{
		core_->Visualize(index);
	}

	void MeshMLViewerCoreWrapper::MouseMove(int x, int y, uint32_t button)
	{
		core_->MouseMove(x, y, button);
	}

	void MeshMLViewerCoreWrapper::MouseUp(int x, int y, uint32_t button)
	{
		core_->MouseUp(x, y, button);
	}

	void MeshMLViewerCoreWrapper::MouseDown(int x, int y, uint32_t button)
	{
		core_->MouseDown(x, y, button);
	}

	void MeshMLViewerCoreWrapper::KeyPress(int key)
	{
		core_->KeyPress(key);
	}

	uint32_t MeshMLViewerCoreWrapper::NumMeshes()
	{
		return core_->NumMeshes();
	}

	String^ MeshMLViewerCoreWrapper::MeshName(uint32_t index)
	{
		return WStdToString(core_->MeshName(index));
	}

	uint32_t MeshMLViewerCoreWrapper::NumVertexStreams(uint32_t mesh_id)
	{
		return core_->NumVertexStreams(mesh_id);
	}

	uint32_t MeshMLViewerCoreWrapper::NumVertexStreamUsages(uint32_t mesh_id, uint32_t stream_index)
	{
		return core_->NumVertexStreamUsages(mesh_id, stream_index);
	}

	uint32_t MeshMLViewerCoreWrapper::VertexStreamUsage(uint32_t mesh_id, uint32_t stream_index, uint32_t usage_index)
	{
		return core_->VertexStreamUsage(mesh_id, stream_index, usage_index);
	}

	uint32_t MeshMLViewerCoreWrapper::MaterialID(uint32_t mesh_id)
	{
		return core_->MaterialID(mesh_id);
	}

	Windows::Media::Color MeshMLViewerCoreWrapper::AmbientMaterial(uint32_t mtl_id)
	{
		return FloatPtrToColor(&core_->AmbientMaterial(mtl_id).x());
	}

	Windows::Media::Color MeshMLViewerCoreWrapper::DiffuseMaterial(uint32_t mtl_id)
	{
		return FloatPtrToColor(&core_->DiffuseMaterial(mtl_id).x());
	}

	Windows::Media::Color MeshMLViewerCoreWrapper::SpecularMaterial(uint32_t mtl_id)
	{
		return FloatPtrToColor(&core_->SpecularMaterial(mtl_id).x());
	}

	float MeshMLViewerCoreWrapper::ShininessMaterial(uint32_t mtl_id)
	{
		return core_->ShininessMaterial(mtl_id);
	}

	Windows::Media::Color MeshMLViewerCoreWrapper::EmitMaterial(uint32_t mtl_id)
	{
		return FloatPtrToColor(&core_->EmitMaterial(mtl_id).x());
	}

	float MeshMLViewerCoreWrapper::OpacityMaterial(uint32_t mtl_id)
	{
		return core_->OpacityMaterial(mtl_id);
	}

	String^ MeshMLViewerCoreWrapper::DiffuseTexture(uint32_t mtl_id)
	{
		return StdToString(core_->DiffuseTexture(mtl_id));
	}

	String^MeshMLViewerCoreWrapper::SpecularTexture(uint32_t mtl_id)
	{
		return StdToString(core_->SpecularTexture(mtl_id));
	}

	String^ MeshMLViewerCoreWrapper::ShininessTexture(uint32_t mtl_id)
	{
		return StdToString(core_->ShininessTexture(mtl_id));
	}

	String^ MeshMLViewerCoreWrapper::NormalTexture(uint32_t mtl_id)
	{
		return StdToString(core_->NormalTexture(mtl_id));
	}

	String^ MeshMLViewerCoreWrapper::HeightTexture(uint32_t mtl_id)
	{
		return StdToString(core_->HeightTexture(mtl_id));
	}

	String^ MeshMLViewerCoreWrapper::EmitTexture(uint32_t mtl_id)
	{
		return StdToString(core_->EmitTexture(mtl_id));
	}

	String^ MeshMLViewerCoreWrapper::OpacityTexture(uint32_t mtl_id)
	{
		return StdToString(core_->OpacityTexture(mtl_id));
	}

	uint32_t MeshMLViewerCoreWrapper::SelectedMesh()
	{
		return core_->SelectedMesh();
	}

	void MeshMLViewerCoreWrapper::SelectMesh(uint32_t mesh_id)
	{
		return core_->SelectMesh(mesh_id);
	}
}
