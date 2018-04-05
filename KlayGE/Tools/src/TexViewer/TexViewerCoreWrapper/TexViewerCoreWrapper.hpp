#ifndef _TEX_VIEWER_CORE_WRAPPER_HPP
#define _TEX_VIEWER_CORE_WRAPPER_HPP

#pragma once

namespace KlayGE
{
	public ref class TexViewerCoreWrapper
	{
	public:
		enum class TextureType
		{
			TT_1D = Texture::TT_1D,
			TT_2D = Texture::TT_2D,
			TT_3D = Texture::TT_3D,
			TT_Cube = Texture::TT_Cube
		};

		enum class ElementFormat
		{
			EF_Unknown = 0,

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

	public:
		explicit TexViewerCoreWrapper(System::IntPtr native_wnd);
		~TexViewerCoreWrapper();

		void Refresh();
		void Resize(uint32_t width, uint32_t height);

		void OpenTexture(System::String^ name);
		void SaveAsTexture(System::String^ name);

		TextureType Type();
		uint32_t ArraySize();
		uint32_t NumMipmaps();
		uint32_t Width(uint32_t mipmap);
		uint32_t Height(uint32_t mipmap);
		uint32_t Depth(uint32_t mipmap);
		ElementFormat Format();

		void ArrayIndex(uint32_t array_index);
		void Face(uint32_t face);
		void DepthIndex(uint32_t depth_index);
		void MipmapLevel(uint32_t mipmap);
		void Stops(float value);
		void OffsetAndZoom(float x, float y, float zoom);
		void ColorMask(bool r, bool g, bool b, bool a);

		array<float>^ RetrieveColor(uint32_t x, uint32_t y);

	private:
		TexViewerCore* core_;
	};
}

#endif		// _TEX_VIEWER_CORE_WRAPPER_HPP
