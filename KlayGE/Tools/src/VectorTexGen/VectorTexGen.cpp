/**
 * @file VectorTexGen.cpp
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
#include <KFL/Util.hpp>
#include <KFL/Math.hpp>
#include <KFL/ErrorHandling.hpp>
#include <KlayGE/Texture.hpp>
#include <KlayGE/DistanceField.hpp>
#include <KlayGE/ResLoader.hpp>

#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <cstring>
#include <atomic>

#ifndef KLAYGE_DEBUG
#define CXXOPTS_NO_RTTI
#endif
#include <cxxopts.hpp>

#if defined(KLAYGE_COMPILER_MSVC)
#pragma warning(push)
#pragma warning(disable: 4244) // Conversion from doubel to int64
#pragma warning(disable: 4456) // Declaration of 'name' hides previous local declaration
#pragma warning(disable: 4702) // Unreachable code
#endif
#define NANOSVG_IMPLEMENTATION
#include <nanosvg.h>
#if defined(KLAYGE_COMPILER_MSVC)
#pragma warning(pop)
#endif
#define NANOSVGRAST_IMPLEMENTATION
#include <nanosvgrast.h>

using namespace std;
using namespace KlayGE;

void Quantizer(std::vector<float> const & dist_data, uint8_t* quan_dist, uint32_t stride)
{
	for (size_t i = 0; i < dist_data.size(); ++ i)
	{
		quan_dist[i * stride] = static_cast<uint8_t>((MathLib::clamp(dist_data[i], -1.0f, 1.0f) * 0.5f + 0.5f) * 255);
	}
}

int main(int argc, char* argv[])
{
	std::string in_name;
	std::string out_name;
	uint32_t num_channels;
	bool svg_input;

	cxxopts::Options options("ImageConv", "KlayGE Vector Texture Converter");
	options.add_options()
		("H,help", "Produce help message.")
		("I,input-name", "Input name (svg or dds).", cxxopts::value<std::string>())
		("O,output-name", "Output name. Default is input-name.dds or svg, input-name.df.dds for dds.", cxxopts::value<std::string>())
		("C,channels", "Number of channels.", cxxopts::value<uint32_t>(num_channels)->default_value("4"))
		("v,version", "Version.");

	int const argc_backup = argc;
	auto vm = options.parse(argc, argv);

	if ((argc_backup <= 1) || (vm.count("help") > 0))
	{
		cout << options.help() << endl;
		return 1;
	}
	if (vm.count("version") > 0)
	{
		cout << "KlayGE Vector Texture Converter, Version 1.0.0" << endl;
		return 1;
	}
	if (vm.count("input-name") > 0)
	{
		in_name = vm["input-name"].as<std::string>();
	}
	else
	{
		cout << "Input name was not set." << endl;
		cout << options.help() << endl;
		return 1;
	}

	auto dot_pos = in_name.find_last_of('.');
	auto ext_name = in_name.substr(dot_pos + 1);
	if (ext_name == "dds")
	{
		svg_input = false;
	}
	else if (ext_name == "svg")
	{
		svg_input = true;
	}
	else
	{
		std::cerr << "Unsupported file format. Must be svg or dds." << std::endl;
		return 1;
	}

	if ((num_channels != 1) && (num_channels != 2) && (num_channels != 4))
	{
		std::cerr << "Unsupported channels. Must be 1, 2, or 4." << std::endl;
		return 1;
	}

	if (vm.count("output-name") > 0)
	{
		out_name = vm["output-name"].as<std::string>();
	}
	else
	{
		out_name = in_name.substr(0, dot_pos);
		if (!svg_input)
		{
			out_name += ".df";
		}
		out_name += ".dds";
	}

	if (ResLoader::Instance().Locate(in_name).empty())
	{
		std::cerr << "Could NOT find " << in_name << std::endl;
		Context::Destroy();
		return 1;
	}

	uint32_t width, height, ras_width, ras_height;
	ElementFormat format;
	std::vector<uint8_t> rgba;
	if (svg_input)
	{
		if (num_channels != 4)
		{
			std::cerr << "Unsupported channels. Must be 4 for svg." << std::endl;
			Context::Destroy();
			return 1;
		}

		NSVGimage* image = nsvgParseFromFile(in_name.c_str(), "px", 96);
		if (!image)
		{
			std::cerr << "Could NOT open " << in_name << " as an SVG." << std::endl;
			Context::Destroy();
			return 1;
		}

		width = (static_cast<uint32_t>(image->width + 0.5f) + 3) / 4;
		height = (static_cast<uint32_t>(image->height + 0.5f) + 3) / 4;
		switch (num_channels)
		{
		case 1:
			format = EF_R8;
			break;

		case 2:
			format = EF_GR8;
			break;

		case 4:
			format = EF_ABGR8;
			break;

		default:
			KFL_UNREACHABLE("Unsupported channels");
		}

		ras_width = width * 4;
		ras_height = height * 4;

		rgba.resize(ras_width * ras_height * 4);

		NSVGrasterizer* rast = nsvgCreateRasterizer();
		nsvgRasterize(rast, image, 0, 0, 1, &rgba[0], ras_width, ras_height, ras_width * 4);

		nsvgDeleteRasterizer(rast);
		nsvgDelete(image);
	}
	else
	{
		TexturePtr in_tex = LoadSoftwareTexture(in_name);
		ras_width = in_tex->Width(0);
		ras_height = in_tex->Height(0);
		auto const depth = in_tex->Depth(0);
		format = in_tex->Format();
		auto const& init_data = checked_cast<SoftwareTexture&>(*in_tex).SubresourceData();

		if (NumComponents(format) != num_channels)
		{
			std::cerr << "Unsupported pixel format. Must be " << num_channels << " channels." << std::endl;
			Context::Destroy();
			return 1;
		}

		rgba.resize(ras_width * ras_height * num_channels);
		ResizeTexture(&rgba[0], ras_width * num_channels, ras_width * ras_height * num_channels, format, ras_width, ras_height, 1,
			init_data[0].data, init_data[0].row_pitch, init_data[0].slice_pitch, format, ras_width, ras_height, depth, true);

		width = std::max(ras_width / 4, 1U);
		height = std::max(ras_height / 4, 1U);
	}

	cout << "\tInput name: " << in_name << endl;
	cout << "\tOutput name: " << out_name << endl;
	cout << "\tDistance field width: " << width << endl;
	cout << "\tDistance field height: " << height << endl;
	cout << "\tNumber of channels: " << num_channels << endl;
	cout << endl;

	std::vector<std::vector<float>> ras_data(num_channels);
	std::vector<std::vector<float>> dist_data(num_channels);

	for (uint32_t i = 0; i < num_channels; ++ i)
	{
		ras_data[i].resize(ras_width * ras_height);
		dist_data[i].resize(width * height);
	}

	for (uint32_t y = 0; y < ras_height; ++ y)
	{
		for (uint32_t x = 0; x < ras_width; ++ x)
		{
			for (uint32_t c = 0; c < num_channels; ++ c)
			{
				ras_data[c][y * ras_width + x] = (rgba[(y * ras_width + x) * num_channels + c] > 127) ? 1.0f : 0.0f;
			};
		}
	}

	cout << "Compute distance field..." << endl;
	for (uint32_t i = 0; i < num_channels; ++ i)
	{
		std::vector<float> aa_2x_data(ras_data.size() / 4);
		Downsample2x(ras_data[i], ras_width, ras_height, aa_2x_data);

		ComputeDistance(aa_2x_data, ras_width / 2, ras_height / 2, dist_data[i]);
	}

	cout << "Quantize..." << endl;
	std::vector<uint8_t> quan_dist(width * height * num_channels);
	for (uint32_t i = 0; i < num_channels; ++ i)
	{
		Quantizer(dist_data[i], &quan_dist[i], num_channels);
	}

	ElementInitData init_data;
	init_data.data = &quan_dist[0];
	init_data.row_pitch = width * num_channels;
	init_data.slice_pitch = width * height * num_channels;
	TexturePtr out_tex = MakeSharedPtr<SoftwareTexture>(Texture::TT_2D, width, height, 1, 1, 1, format, true);
	out_tex->CreateHWResource(MakeSpan<1>(init_data), nullptr);
	SaveTexture(out_tex, out_name);

	Context::Destroy();
}
