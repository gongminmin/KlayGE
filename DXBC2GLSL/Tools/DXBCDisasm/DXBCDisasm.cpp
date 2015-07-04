/**
 * @file DXBCDisasm.cpp
 * @author Shenghua Lin, Minmin Gong, Luca Barbieri
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
#include "ASMGen.hpp"
#include <iostream>
#include <fstream>

void usage()
{
	std::cerr << "DirectX Bytecode Disassembler.\n";
	std::cerr << "This program is free software, released under a GPL license\n";
	std::cerr << "Not affiliated with or endorsed by Microsoft in any way\n";
	std::cerr << "Latest version available from http://www.klayge.org/\n";
	std::cerr << "\n";
	std::cerr << "Usage: DXBCDisasm FILE [OUTPUT]\n";
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

	std::shared_ptr<DXBCContainer> dxbc = DXBCParse(&data[0]);
	if (dxbc)
	{
		if (dxbc->shader_chunk)
		{
			std::shared_ptr<ShaderProgram> shader = ShaderParse(*dxbc);
			if (shader)
			{
				ASMGen converter(shader);
				if (!screen_only)
				{
					converter.ToASM(out);
				}
				converter.ToASM(std::cout);
			}
		}
	}
}
