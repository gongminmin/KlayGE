/**
 * @file ImageConv.cpp
 * @author Minmin Gong
 *
 * @section DESCRIPTION
 *
 * This source file is part of KlayGE
 * For the latest info, see http://www.klayge.org
 *
 * @section LICENSE
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published
 * by the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 * You may alternatively use this source under the terms of
 * the KlayGE Proprietary License (KPL). You can obtained such a license
 * from http://www.klayge.org/licensing/.
 */

#include <KlayGE/KlayGE.hpp>
#include <KFL/CXX17/filesystem.hpp>
#include <KFL/ErrorHandling.hpp>
#include <KFL/Math.hpp>
#include <KFL/Util.hpp>
#include <KlayGE/ResLoader.hpp>
#include <KlayGE/Texture.hpp>

#include <iostream>
#include <vector>
#include <string>

#if defined(KLAYGE_COMPILER_CLANGC2)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-variable" // Ignore unused variable (mpl_assertion_in_line_xxx) in boost
#endif
#include <boost/program_options.hpp>
#if defined(KLAYGE_COMPILER_CLANGC2)
#pragma clang diagnostic pop
#endif

#include <FreeImage.h>

using namespace std;
using namespace KlayGE;

namespace
{
	bool ConvertImage(std::string const & input_name, std::string const & output_name)
	{
		FREE_IMAGE_FORMAT fif = FreeImage_GetFileType(input_name.c_str(), 0);
		if (fif == FIF_UNKNOWN) 
		{
			fif = FreeImage_GetFIFFromFilename(input_name.c_str());
		}
		if (fif == FIF_UNKNOWN)
		{
			return false;
		}

		FIBITMAP* dib = nullptr;
		if (FreeImage_FIFSupportsReading(fif))
		{
			dib = FreeImage_Load(fif, input_name.c_str());
		}
		if (!dib)
		{
			return false;
		}

		uint32_t const width = FreeImage_GetWidth(dib);
		uint32_t const height = FreeImage_GetHeight(dib);
		if ((width == 0) || (height == 0))
		{
			return false;
		}

		FreeImage_FlipVertical(dib);

		ElementFormat format = EF_ABGR8;
		FREE_IMAGE_TYPE const image_type = FreeImage_GetImageType(dib);
		switch (image_type)
		{
		case FIT_BITMAP:
			{
				uint32_t const bpp = FreeImage_GetBPP(dib);
				uint32_t const r_mask = FreeImage_GetRedMask(dib);
				uint32_t const g_mask = FreeImage_GetGreenMask(dib);
				uint32_t const b_mask = FreeImage_GetBlueMask(dib);
				switch (bpp)
				{
				case 1:
				case 4:
				case 8:
				case 24:
					{
						if (bpp == 24)
						{
							if ((r_mask == 0xFF0000) && (g_mask == 0xFF00) && (b_mask == 0xFF))
							{
								format = EF_ARGB8;
							}
							else if ((r_mask == 0xFF) && (g_mask == 0xFF00) && (b_mask == 0xFF0000))
							{
								format = EF_ABGR8;
							}
						}
						else
						{
							format = EF_ARGB8;
						}
						auto dib_32bpp = FreeImage_ConvertTo32Bits(dib);
						FreeImage_Unload(dib);
						dib = dib_32bpp;
					}
					break;

				case 16:
					if ((r_mask == (0x1F << 10)) && (g_mask == (0x1F << 5)) && (b_mask == 0x1F))
					{
						format = EF_A1RGB5;
					}
					else if ((r_mask == (0x1F << 11)) && (g_mask == (0x3F << 5)) && (b_mask == 0x1F))
					{
						format = EF_R5G6B5;
					}
					break;

				case 32:
					if ((r_mask == 0xFF0000) && (g_mask == 0xFF00) && (b_mask == 0xFF))
					{
						format = EF_ARGB8;
					}
					else if ((r_mask == 0xFF) && (g_mask == 0xFF00) && (b_mask == 0xFF0000))
					{
						format = EF_ABGR8;
					}
					break;

				default:
					KFL_UNREACHABLE("Unsupported bpp.");
				}
			}
			break;

		case FIT_UINT16:
			format = EF_R16UI;
			break;

		case FIT_INT16:
			format = EF_R16I;
			break;

		case FIT_UINT32:
			format = EF_R32UI;
			break;

		case FIT_INT32:
			format = EF_R32I;
			break;

		case FIT_FLOAT:
			format = EF_R32F;
			break;

		case FIT_COMPLEX:
			format = EF_GR32F;
			break;

		case FIT_RGB16:
			format = EF_ABGR16;
			{
				auto dib_abgr16 = FreeImage_ConvertToRGBA16(dib);
				FreeImage_Unload(dib);
				dib = dib_abgr16;
			}
			break;

		case FIT_RGBA16:
			format = EF_ABGR16;
			break;

		case FIT_RGBF:
			format = EF_ABGR32F;
			{
				auto dib_abgr32f = FreeImage_ConvertToRGBAF(dib);
				FreeImage_Unload(dib);
				dib = dib_abgr32f;
			}
			break;

		case FIT_RGBAF:
			format = EF_ABGR32F;
			break;

		default:
			KFL_UNREACHABLE("Unsupported image type.");
		}

		uint8_t* const bits = FreeImage_GetBits(dib);
		if (bits == nullptr)
		{
			return false;
		}

		ElementInitData init_data;
		init_data.data = bits;
		init_data.row_pitch = FreeImage_GetPitch(dib);
		init_data.slice_pitch = height * init_data.row_pitch;
		SaveTexture(output_name, Texture::TT_2D, width, height, 1, 1, 1, format, init_data);
	
		FreeImage_Unload(dib);

		return true;
	}
}

int main(int argc, char* argv[])
{
	std::string input_name;
	std::string output_name;
	bool quiet = false;

	boost::program_options::options_description desc("Allowed options");
	desc.add_options()
		("help,H", "Produce help message")
		("input-path,I", boost::program_options::value<std::string>(), "Input image path.")
		("output-path,O", boost::program_options::value<std::string>(), "(Optional) Output image path.")
		("quiet,q", boost::program_options::value<bool>()->implicit_value(true), "Quiet mode.")
		("version,v", "Version.");

	boost::program_options::variables_map vm;
	boost::program_options::store(boost::program_options::parse_command_line(argc, argv, desc), vm);
	boost::program_options::notify(vm);

	if ((argc <= 1) || (vm.count("help") > 0))
	{
		cout << desc << endl;
		return 1;
	}
	if (vm.count("version") > 0)
	{
		cout << "KlayGE Image Converter, Version 1.0.0" << endl;
		return 1;
	}
	if (vm.count("input-path") > 0)
	{
		input_name = vm["input-path"].as<std::string>();
	}
	else
	{
		cout << "Need input image path." << endl;
		return 1;
	}
	if (vm.count("output-path") > 0)
	{
		output_name = vm["output-path"].as<std::string>();
	}
	if (vm.count("quiet") > 0)
	{
		quiet = vm["quiet"].as<bool>();
	}

	std::string file_name = ResLoader::Instance().Locate(input_name);
	if (file_name.empty())
	{
		cout << "Could NOT find " << input_name << endl;
		Context::Destroy();
		return 1;
	}

	if (output_name.empty())
	{
		filesystem::path input_path(file_name);
		output_name = (input_path.parent_path() / input_path.stem()).string();
		if (input_path.extension() == "dds")
		{
			output_name += "_converted";
		}
		output_name += ".dds";
	}

	bool succ = ConvertImage(file_name, output_name);

	if (succ && !quiet)
	{
		cout << "Image has been saved to " << output_name << "." << endl;
	}

	Context::Destroy();

	return 0;
}
