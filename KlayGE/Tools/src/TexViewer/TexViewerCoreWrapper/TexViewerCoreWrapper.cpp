#include <KlayGE/KlayGE.hpp>

#include <iterator>

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
		cfg.render_factory_name = "D3D11";
		cfg.graphics_cfg.hdr = false;
		cfg.graphics_cfg.ppaa = false;
		cfg.graphics_cfg.gamma = true;
		cfg.graphics_cfg.color_grading = false;
		Context::Instance().Config(cfg);

		core_ = new TexViewerCore(native_wnd.ToPointer());
		core_->Create();
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
		static KlayGE::ElementFormat constexpr elem_fmt_mapping[] =
		{
			EF_Unknown,

			EF_A8,

			EF_R5G6B5,
			EF_A1RGB5,
			EF_ARGB4,

			EF_R8,
			EF_SIGNED_R8,
			EF_GR8,
			EF_SIGNED_GR8,
			EF_BGR8,
			EF_SIGNED_BGR8,
			EF_ARGB8,
			EF_ABGR8,
			EF_SIGNED_ABGR8,
			EF_A2BGR10,
			EF_SIGNED_A2BGR10,

			EF_R8UI,
			EF_R8I,
			EF_GR8UI,
			EF_GR8I,
			EF_BGR8UI,
			EF_BGR8I,
			EF_ABGR8UI,
			EF_ABGR8I,
			EF_A2BGR10UI,
			EF_A2BGR10I,

			EF_R16,
			EF_SIGNED_R16,
			EF_GR16,
			EF_SIGNED_GR16,
			EF_BGR16,
			EF_SIGNED_BGR16,
			EF_ABGR16,
			EF_SIGNED_ABGR16,
			EF_R32,
			EF_SIGNED_R32,
			EF_GR32,
			EF_SIGNED_GR32,
			EF_BGR32,
			EF_SIGNED_BGR32,
			EF_ABGR32,
			EF_SIGNED_ABGR32,

			EF_R16UI,
			EF_R16I,
			EF_GR16UI,
			EF_GR16I,
			EF_BGR16UI,
			EF_BGR16I,
			EF_ABGR16UI,
			EF_ABGR16I,
			EF_R32UI,
			EF_R32I,
			EF_GR32UI,
			EF_GR32I,
			EF_BGR32UI,
			EF_BGR32I,
			EF_ABGR32UI,
			EF_ABGR32I,

			EF_R16F,
			EF_GR16F,
			EF_B10G11R11F,
			EF_BGR16F,
			EF_ABGR16F,
			EF_R32F,
			EF_GR32F,
			EF_BGR32F,
			EF_ABGR32F,

			EF_BC1,
			EF_SIGNED_BC1,
			EF_BC2,
			EF_SIGNED_BC2,
			EF_BC3,
			EF_SIGNED_BC3,
			EF_BC4,
			EF_SIGNED_BC4,
			EF_BC5,
			EF_SIGNED_BC5,
			EF_BC6,
			EF_SIGNED_BC6,
			EF_BC7,

			EF_ETC1,
			EF_ETC2_R11,
			EF_SIGNED_ETC2_R11,
			EF_ETC2_GR11,
			EF_SIGNED_ETC2_GR11,
			EF_ETC2_BGR8,
			EF_ETC2_BGR8_SRGB,
			EF_ETC2_A1BGR8,
			EF_ETC2_A1BGR8_SRGB,
			EF_ETC2_ABGR8,
			EF_ETC2_ABGR8_SRGB,

			EF_D16,
			EF_D24S8,
			EF_D32F,

			EF_ARGB8_SRGB,
			EF_ABGR8_SRGB,
			EF_BC1_SRGB,
			EF_BC2_SRGB,
			EF_BC3_SRGB,
			EF_BC4_SRGB,
			EF_BC5_SRGB,
			EF_BC7_SRGB
		};

		auto const format = core_->Format();
		for (size_t i = 0; i < std::size(elem_fmt_mapping); ++ i)
		{
			if (elem_fmt_mapping[i] == format)
			{
				return static_cast<TexViewerCoreWrapper::ElementFormat>(i);
			}
		}

		return ElementFormat::EF_Unknown;
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
