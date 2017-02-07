/**
 * @file DXBC2GLSLCmd.cpp
 * @author Shenghua Lin, Minmin Gong
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

#include <DXBC2GLSL/DXBC2GLSL.hpp>
#include <iostream>
#include <fstream>
#include <string>

void usage()
{
	std::cerr << "DirectX Bytecode to GLSL Converter.\n";
	std::cerr << "This program is free software, released under a GPL license\n";
	std::cerr << "Not affiliated with or endorsed by Microsoft in any way\n";
	std::cerr << "Latest version available from http://www.klayge.org/\n";
	std::cerr << "\n";
	std::cerr << "Usage: DXBC2GLSLCmd FILE [OUTPUT]\n";
	std::cerr << std::endl;
}

int main(int argc, char** argv)
{
	if (argc < 2)
	{
		usage();
		return 1;
	}

	std::vector<char> data;
	std::ifstream in(argv[1], std::ios_base::in | std::ios_base::binary);
	std::ofstream out;
	bool screen_only = false;
	if (argc < 3)
	{
		screen_only = true;
	}
	else
	{
		out.open(argv[2]);
	}

	char c;
	in >> std::noskipws;
	while (in >> c)
	{
		data.push_back(c);
	}
	in.close();

	try
	{
		DXBC2GLSL::DXBC2GLSL dxbc2glsl;
		dxbc2glsl.FeedDXBC(&data[0], true, true, STP_Fractional_Odd, STOP_Triangle_CW, GSV_430);
		std::string glsl = dxbc2glsl.GLSLString();
		if (!screen_only)
		{
			out << glsl;
		}
		std::cout << glsl << std::endl;

		if (dxbc2glsl.NumInputParams() > 0)
		{
			std::cout << "Input:" << std::endl;
			for (uint32_t i = 0; i < dxbc2glsl.NumInputParams(); ++ i)
			{
				std::cout << "\t" << dxbc2glsl.InputParam(i).semantic_name
					<< dxbc2glsl.InputParam(i).semantic_index << std::endl;
			}
			std::cout << std::endl;
		}
		if (dxbc2glsl.NumOutputParams() > 0)
		{
			std::cout << "Output:" << std::endl;
			for (uint32_t i = 0; i < dxbc2glsl.NumOutputParams(); ++ i)
			{
				std::cout << "\t" << dxbc2glsl.OutputParam(i).semantic_name
					<< dxbc2glsl.OutputParam(i).semantic_index << std::endl;
			}
			std::cout << std::endl;
		}

		for (uint32_t i = 0; i < dxbc2glsl.NumCBuffers(); ++ i)
		{
			std::cout << "CBuffer " << i << ":" << std::endl;
			for (uint32_t j = 0; j < dxbc2glsl.NumVariables(i); ++ j)
			{
				std::cout << "\t" << dxbc2glsl.VariableName(i, j)
					<< ' ' << (dxbc2glsl.VariableUsed(i, j) ? "USED" : "UNUSED");
				std::cout << std::endl;
			}
			std::cout << std::endl;
		}

		if (dxbc2glsl.NumResources() > 0)
		{
			std::cout << "Resource:" << std::endl;
			for (uint32_t i = 0; i < dxbc2glsl.NumResources(); ++ i)
			{
				std::cout << "\t" << dxbc2glsl.ResourceName(i) << " : "
					<< dxbc2glsl.ResourceBindPoint(i)
					<< ' ' << (dxbc2glsl.ResourceUsed(i) ? "USED" : "UNUSED");
				std::cout << std::endl;
			}
			std::cout << std::endl;
		}

		if (dxbc2glsl.GSInputPrimitive() != SP_Undefined)
		{
			std::cout << "GS input primitive: " << ShaderPrimitiveName(dxbc2glsl.GSInputPrimitive()) << std::endl;

			std::cout << "GS output:" << std::endl;
			for (uint32_t i = 0; i < dxbc2glsl.NumGSOutputTopology(); ++ i)
			{
				std::cout << "\t" << ShaderPrimitiveTopologyName(dxbc2glsl.GSOutputTopology(i)) << std::endl;
			}

			std::cout << "Max GS output vertex " << dxbc2glsl.MaxGSOutputVertex() << std::endl << std::endl;
		}
	}
	catch (std::exception& ex)
	{
		std::cout << "Error(s) in conversion:" << std::endl;
		std::cout << ex.what() << std::endl;
		std::cout << "Please send this information and your bytecode file to webmaster at klayge.org. We'll fix this ASAP." << std::endl;
	}
}
