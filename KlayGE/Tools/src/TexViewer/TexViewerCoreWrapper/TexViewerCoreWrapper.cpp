#include <KlayGE/KlayGE.hpp>

#include "../TexViewerCore/TexViewerCore.hpp"

#include "TexViewerCoreWrapper.hpp"

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


	TexViewerCoreWrapper::TexViewerCoreWrapper(IntPtr native_wnd)
	{
		Context::Instance().LoadCfg("KlayGE.cfg");

		ContextCfg cfg = Context::Instance().Config();
		cfg.graphics_cfg.hdr = false;
		cfg.graphics_cfg.ppaa = false;
		cfg.graphics_cfg.gamma = true;
		cfg.graphics_cfg.color_grading = false;
		Context::Instance().Config(cfg);

		core_ = new TexViewerCore(native_wnd.ToPointer());
		core_->Create();

		elem_fmt_mapping_.insert(cliext::make_pair(static_cast<uint64_t>(EF_Unknown), ElementFormat::EF_Unknown));

		elem_fmt_mapping_.insert(cliext::make_pair(static_cast<uint64_t>(EF_A8), ElementFormat::EF_A8));

		elem_fmt_mapping_.insert(cliext::make_pair(static_cast<uint64_t>(EF_R5G6B5), ElementFormat::EF_R5G6B5));
		elem_fmt_mapping_.insert(cliext::make_pair(static_cast<uint64_t>(EF_A1RGB5), ElementFormat::EF_A1RGB5));
		elem_fmt_mapping_.insert(cliext::make_pair(static_cast<uint64_t>(EF_ARGB4), ElementFormat::EF_ARGB4));

		elem_fmt_mapping_.insert(cliext::make_pair(static_cast<uint64_t>(EF_R8), ElementFormat::EF_R8));
		elem_fmt_mapping_.insert(cliext::make_pair(static_cast<uint64_t>(EF_SIGNED_R8), ElementFormat::EF_SIGNED_R8));
		elem_fmt_mapping_.insert(cliext::make_pair(static_cast<uint64_t>(EF_GR8), ElementFormat::EF_GR8));
		elem_fmt_mapping_.insert(cliext::make_pair(static_cast<uint64_t>(EF_SIGNED_GR8), ElementFormat::EF_SIGNED_GR8));
		elem_fmt_mapping_.insert(cliext::make_pair(static_cast<uint64_t>(EF_BGR8), ElementFormat::EF_BGR8));
		elem_fmt_mapping_.insert(cliext::make_pair(static_cast<uint64_t>(EF_SIGNED_BGR8), ElementFormat::EF_SIGNED_BGR8));
		elem_fmt_mapping_.insert(cliext::make_pair(static_cast<uint64_t>(EF_ARGB8), ElementFormat::EF_ARGB8));
		elem_fmt_mapping_.insert(cliext::make_pair(static_cast<uint64_t>(EF_ABGR8), ElementFormat::EF_ABGR8));
		elem_fmt_mapping_.insert(cliext::make_pair(static_cast<uint64_t>(EF_SIGNED_ABGR8), ElementFormat::EF_SIGNED_ABGR8));
		elem_fmt_mapping_.insert(cliext::make_pair(static_cast<uint64_t>(EF_A2BGR10), ElementFormat::EF_A2BGR10));
		elem_fmt_mapping_.insert(cliext::make_pair(static_cast<uint64_t>(EF_SIGNED_A2BGR10), ElementFormat::EF_SIGNED_A2BGR10));

		elem_fmt_mapping_.insert(cliext::make_pair(static_cast<uint64_t>(EF_R8UI), ElementFormat::EF_R8UI));
		elem_fmt_mapping_.insert(cliext::make_pair(static_cast<uint64_t>(EF_R8I), ElementFormat::EF_R8I));
		elem_fmt_mapping_.insert(cliext::make_pair(static_cast<uint64_t>(EF_GR8UI), ElementFormat::EF_GR8UI));
		elem_fmt_mapping_.insert(cliext::make_pair(static_cast<uint64_t>(EF_GR8I), ElementFormat::EF_GR8I));
		elem_fmt_mapping_.insert(cliext::make_pair(static_cast<uint64_t>(EF_BGR8UI), ElementFormat::EF_BGR8UI));
		elem_fmt_mapping_.insert(cliext::make_pair(static_cast<uint64_t>(EF_BGR8I), ElementFormat::EF_BGR8I));
		elem_fmt_mapping_.insert(cliext::make_pair(static_cast<uint64_t>(EF_ABGR8UI), ElementFormat::EF_ABGR8UI));
		elem_fmt_mapping_.insert(cliext::make_pair(static_cast<uint64_t>(EF_ABGR8I), ElementFormat::EF_ABGR8I));
		elem_fmt_mapping_.insert(cliext::make_pair(static_cast<uint64_t>(EF_A2BGR10UI), ElementFormat::EF_A2BGR10UI));
		elem_fmt_mapping_.insert(cliext::make_pair(static_cast<uint64_t>(EF_A2BGR10I), ElementFormat::EF_A2BGR10I));

		elem_fmt_mapping_.insert(cliext::make_pair(static_cast<uint64_t>(EF_R16), ElementFormat::EF_R16));
		elem_fmt_mapping_.insert(cliext::make_pair(static_cast<uint64_t>(EF_SIGNED_R16), ElementFormat::EF_SIGNED_R16));
		elem_fmt_mapping_.insert(cliext::make_pair(static_cast<uint64_t>(EF_GR16), ElementFormat::EF_GR16));
		elem_fmt_mapping_.insert(cliext::make_pair(static_cast<uint64_t>(EF_SIGNED_GR16), ElementFormat::EF_SIGNED_GR16));
		elem_fmt_mapping_.insert(cliext::make_pair(static_cast<uint64_t>(EF_BGR16), ElementFormat::EF_BGR16));
		elem_fmt_mapping_.insert(cliext::make_pair(static_cast<uint64_t>(EF_SIGNED_BGR16), ElementFormat::EF_SIGNED_BGR16));
		elem_fmt_mapping_.insert(cliext::make_pair(static_cast<uint64_t>(EF_ABGR16), ElementFormat::EF_ABGR16));
		elem_fmt_mapping_.insert(cliext::make_pair(static_cast<uint64_t>(EF_SIGNED_ABGR16), ElementFormat::EF_SIGNED_ABGR16));
		elem_fmt_mapping_.insert(cliext::make_pair(static_cast<uint64_t>(EF_R32), ElementFormat::EF_R32));
		elem_fmt_mapping_.insert(cliext::make_pair(static_cast<uint64_t>(EF_SIGNED_R32), ElementFormat::EF_SIGNED_R32));
		elem_fmt_mapping_.insert(cliext::make_pair(static_cast<uint64_t>(EF_GR32), ElementFormat::EF_GR32));
		elem_fmt_mapping_.insert(cliext::make_pair(static_cast<uint64_t>(EF_SIGNED_GR32), ElementFormat::EF_SIGNED_GR32));
		elem_fmt_mapping_.insert(cliext::make_pair(static_cast<uint64_t>(EF_BGR32), ElementFormat::EF_BGR32));
		elem_fmt_mapping_.insert(cliext::make_pair(static_cast<uint64_t>(EF_SIGNED_BGR32), ElementFormat::EF_SIGNED_BGR32));
		elem_fmt_mapping_.insert(cliext::make_pair(static_cast<uint64_t>(EF_ABGR32), ElementFormat::EF_ABGR32));
		elem_fmt_mapping_.insert(cliext::make_pair(static_cast<uint64_t>(EF_SIGNED_ABGR32), ElementFormat::EF_SIGNED_ABGR32));

		elem_fmt_mapping_.insert(cliext::make_pair(static_cast<uint64_t>(EF_R16UI), ElementFormat::EF_R16UI));
		elem_fmt_mapping_.insert(cliext::make_pair(static_cast<uint64_t>(EF_R16I), ElementFormat::EF_R16I));
		elem_fmt_mapping_.insert(cliext::make_pair(static_cast<uint64_t>(EF_GR16UI), ElementFormat::EF_GR16UI));
		elem_fmt_mapping_.insert(cliext::make_pair(static_cast<uint64_t>(EF_GR16I), ElementFormat::EF_GR16I));
		elem_fmt_mapping_.insert(cliext::make_pair(static_cast<uint64_t>(EF_BGR16UI), ElementFormat::EF_BGR16UI));
		elem_fmt_mapping_.insert(cliext::make_pair(static_cast<uint64_t>(EF_BGR16I), ElementFormat::EF_BGR16I));
		elem_fmt_mapping_.insert(cliext::make_pair(static_cast<uint64_t>(EF_ABGR16UI), ElementFormat::EF_ABGR16UI));
		elem_fmt_mapping_.insert(cliext::make_pair(static_cast<uint64_t>(EF_ABGR16I), ElementFormat::EF_ABGR16I));
		elem_fmt_mapping_.insert(cliext::make_pair(static_cast<uint64_t>(EF_R32UI), ElementFormat::EF_R32UI));
		elem_fmt_mapping_.insert(cliext::make_pair(static_cast<uint64_t>(EF_R32I), ElementFormat::EF_R32I));
		elem_fmt_mapping_.insert(cliext::make_pair(static_cast<uint64_t>(EF_GR32UI), ElementFormat::EF_GR32UI));
		elem_fmt_mapping_.insert(cliext::make_pair(static_cast<uint64_t>(EF_GR32I), ElementFormat::EF_GR32I));
		elem_fmt_mapping_.insert(cliext::make_pair(static_cast<uint64_t>(EF_BGR32UI), ElementFormat::EF_BGR32UI));
		elem_fmt_mapping_.insert(cliext::make_pair(static_cast<uint64_t>(EF_BGR32I), ElementFormat::EF_BGR32I));
		elem_fmt_mapping_.insert(cliext::make_pair(static_cast<uint64_t>(EF_ABGR32UI), ElementFormat::EF_ABGR32UI));
		elem_fmt_mapping_.insert(cliext::make_pair(static_cast<uint64_t>(EF_ABGR32I), ElementFormat::EF_ABGR32I));

		elem_fmt_mapping_.insert(cliext::make_pair(static_cast<uint64_t>(EF_R16F), ElementFormat::EF_R16F));
		elem_fmt_mapping_.insert(cliext::make_pair(static_cast<uint64_t>(EF_GR16F), ElementFormat::EF_GR16F));
		elem_fmt_mapping_.insert(cliext::make_pair(static_cast<uint64_t>(EF_B10G11R11F), ElementFormat::EF_B10G11R11F));
		elem_fmt_mapping_.insert(cliext::make_pair(static_cast<uint64_t>(EF_BGR16F), ElementFormat::EF_BGR16F));
		elem_fmt_mapping_.insert(cliext::make_pair(static_cast<uint64_t>(EF_ABGR16F), ElementFormat::EF_ABGR16F));
		elem_fmt_mapping_.insert(cliext::make_pair(static_cast<uint64_t>(EF_R32F), ElementFormat::EF_R32F));
		elem_fmt_mapping_.insert(cliext::make_pair(static_cast<uint64_t>(EF_GR32F), ElementFormat::EF_GR32F));
		elem_fmt_mapping_.insert(cliext::make_pair(static_cast<uint64_t>(EF_BGR32F), ElementFormat::EF_BGR32F));
		elem_fmt_mapping_.insert(cliext::make_pair(static_cast<uint64_t>(EF_ABGR32F), ElementFormat::EF_ABGR32F));

		elem_fmt_mapping_.insert(cliext::make_pair(static_cast<uint64_t>(EF_BC1), ElementFormat::EF_BC1));
		elem_fmt_mapping_.insert(cliext::make_pair(static_cast<uint64_t>(EF_SIGNED_BC1), ElementFormat::EF_SIGNED_BC1));
		elem_fmt_mapping_.insert(cliext::make_pair(static_cast<uint64_t>(EF_BC2), ElementFormat::EF_BC2));
		elem_fmt_mapping_.insert(cliext::make_pair(static_cast<uint64_t>(EF_SIGNED_BC2), ElementFormat::EF_SIGNED_BC2));
		elem_fmt_mapping_.insert(cliext::make_pair(static_cast<uint64_t>(EF_BC3), ElementFormat::EF_BC3));
		elem_fmt_mapping_.insert(cliext::make_pair(static_cast<uint64_t>(EF_SIGNED_BC3), ElementFormat::EF_SIGNED_BC3));
		elem_fmt_mapping_.insert(cliext::make_pair(static_cast<uint64_t>(EF_BC4), ElementFormat::EF_BC4));
		elem_fmt_mapping_.insert(cliext::make_pair(static_cast<uint64_t>(EF_SIGNED_BC4), ElementFormat::EF_SIGNED_BC4));
		elem_fmt_mapping_.insert(cliext::make_pair(static_cast<uint64_t>(EF_BC5), ElementFormat::EF_BC5));
		elem_fmt_mapping_.insert(cliext::make_pair(static_cast<uint64_t>(EF_SIGNED_BC5), ElementFormat::EF_SIGNED_BC5));
		elem_fmt_mapping_.insert(cliext::make_pair(static_cast<uint64_t>(EF_BC6), ElementFormat::EF_BC6));
		elem_fmt_mapping_.insert(cliext::make_pair(static_cast<uint64_t>(EF_SIGNED_BC6), ElementFormat::EF_SIGNED_BC6));
		elem_fmt_mapping_.insert(cliext::make_pair(static_cast<uint64_t>(EF_BC7), ElementFormat::EF_BC7));

		elem_fmt_mapping_.insert(cliext::make_pair(static_cast<uint64_t>(EF_ETC1), ElementFormat::EF_ETC1));
		elem_fmt_mapping_.insert(cliext::make_pair(static_cast<uint64_t>(EF_ETC2_R11), ElementFormat::EF_ETC2_R11));
		elem_fmt_mapping_.insert(cliext::make_pair(static_cast<uint64_t>(EF_SIGNED_ETC2_R11), ElementFormat::EF_SIGNED_ETC2_R11));
		elem_fmt_mapping_.insert(cliext::make_pair(static_cast<uint64_t>(EF_ETC2_GR11), ElementFormat::EF_ETC2_GR11));
		elem_fmt_mapping_.insert(cliext::make_pair(static_cast<uint64_t>(EF_SIGNED_ETC2_GR11), ElementFormat::EF_SIGNED_ETC2_GR11));
		elem_fmt_mapping_.insert(cliext::make_pair(static_cast<uint64_t>(EF_ETC2_BGR8), ElementFormat::EF_ETC2_BGR8));
		elem_fmt_mapping_.insert(cliext::make_pair(static_cast<uint64_t>(EF_ETC2_BGR8_SRGB), ElementFormat::EF_ETC2_BGR8_SRGB));
		elem_fmt_mapping_.insert(cliext::make_pair(static_cast<uint64_t>(EF_ETC2_A1BGR8), ElementFormat::EF_ETC2_A1BGR8));
		elem_fmt_mapping_.insert(cliext::make_pair(static_cast<uint64_t>(EF_ETC2_A1BGR8_SRGB), ElementFormat::EF_ETC2_A1BGR8_SRGB));
		elem_fmt_mapping_.insert(cliext::make_pair(static_cast<uint64_t>(EF_ETC2_ABGR8), ElementFormat::EF_ETC2_ABGR8));
		elem_fmt_mapping_.insert(cliext::make_pair(static_cast<uint64_t>(EF_ETC2_ABGR8_SRGB), ElementFormat::EF_ETC2_ABGR8_SRGB));

		elem_fmt_mapping_.insert(cliext::make_pair(static_cast<uint64_t>(EF_D16), ElementFormat::EF_D16));
		elem_fmt_mapping_.insert(cliext::make_pair(static_cast<uint64_t>(EF_D24S8), ElementFormat::EF_D24S8));
		elem_fmt_mapping_.insert(cliext::make_pair(static_cast<uint64_t>(EF_D32F), ElementFormat::EF_D32F));

		elem_fmt_mapping_.insert(cliext::make_pair(static_cast<uint64_t>(EF_ARGB8_SRGB), ElementFormat::EF_ARGB8_SRGB));
		elem_fmt_mapping_.insert(cliext::make_pair(static_cast<uint64_t>(EF_ABGR8_SRGB), ElementFormat::EF_ABGR8_SRGB));
		elem_fmt_mapping_.insert(cliext::make_pair(static_cast<uint64_t>(EF_BC1_SRGB), ElementFormat::EF_BC1_SRGB));
		elem_fmt_mapping_.insert(cliext::make_pair(static_cast<uint64_t>(EF_BC2_SRGB), ElementFormat::EF_BC2_SRGB));
		elem_fmt_mapping_.insert(cliext::make_pair(static_cast<uint64_t>(EF_BC3_SRGB), ElementFormat::EF_BC3_SRGB));
		elem_fmt_mapping_.insert(cliext::make_pair(static_cast<uint64_t>(EF_BC4_SRGB), ElementFormat::EF_BC4_SRGB));
		elem_fmt_mapping_.insert(cliext::make_pair(static_cast<uint64_t>(EF_BC5_SRGB), ElementFormat::EF_BC5_SRGB));
		elem_fmt_mapping_.insert(cliext::make_pair(static_cast<uint64_t>(EF_BC7_SRGB), ElementFormat::EF_BC7_SRGB));
	}

	TexViewerCoreWrapper::~TexViewerCoreWrapper()
	{
		delete core_;
	}

	void TexViewerCoreWrapper::Refresh()
	{
		core_->Refresh();
	}

	void TexViewerCoreWrapper::Resize(uint32_t width, uint32_t height)
	{
		core_->Resize(width, height);
	}

	void TexViewerCoreWrapper::OpenTexture(String^ name)
	{
		core_->OpenTexture(StringToStd(name));
	}

	void TexViewerCoreWrapper::SaveAsTexture(String^ name)
	{
		core_->SaveAsTexture(StringToStd(name));
	}

	TexViewerCoreWrapper::TextureType TexViewerCoreWrapper::Type()
	{
		return static_cast<TextureType>(core_->TextureType());
	}

	uint32_t TexViewerCoreWrapper::ArraySize()
	{
		return core_->ArraySize();
	}

	uint32_t TexViewerCoreWrapper::NumMipmaps()
	{
		return core_->NumMipmaps();
	}

	uint32_t TexViewerCoreWrapper::Width(uint32_t mipmap)
	{
		return core_->Width(mipmap);
	}

	uint32_t TexViewerCoreWrapper::Height(uint32_t mipmap)
	{
		return core_->Height(mipmap);
	}

	uint32_t TexViewerCoreWrapper::Depth(uint32_t mipmap)
	{
		return core_->Depth(mipmap);
	}

	TexViewerCoreWrapper::ElementFormat TexViewerCoreWrapper::Format()
	{
		return elem_fmt_mapping_[core_->Format()];
	}

	void TexViewerCoreWrapper::ArrayIndex(uint32_t array_index)
	{
		core_->ArrayIndex(array_index);
	}

	void TexViewerCoreWrapper::Face(uint32_t face)
	{
		core_->Face(face);
	}

	void TexViewerCoreWrapper::DepthIndex(uint32_t depth_index)
	{
		core_->DepthIndex(depth_index);
	}

	void TexViewerCoreWrapper::MipmapLevel(uint32_t mipmap)
	{
		core_->MipmapLevel(mipmap);
	}

	void TexViewerCoreWrapper::Stops(float value)
	{
		core_->Stops(value);
	}

	void TexViewerCoreWrapper::OffsetAndZoom(float x, float y, float zoom)
	{
		core_->OffsetAndZoom(x, y, zoom);
	}

	void TexViewerCoreWrapper::ColorMask(bool r, bool g, bool b, bool a)
	{
		core_->ColorMask(r, g, b, a);
	}

	array<float>^ TexViewerCoreWrapper::RetrieveColor(uint32_t x, uint32_t y)
	{
		auto const & clr = core_->RetrieveColor(x, y);
		array<float>^ ret = { clr.x(), clr.y(), clr.z(), clr.w() };
		return ret;
	}
}
