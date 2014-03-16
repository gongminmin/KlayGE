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
	std::cerr << "Usage: fxdis FILE [OUTPUT]\n";
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

	KlayGE::shared_ptr<DXBCContainer> dxbc = DXBCParse(&data[0]);
	if (dxbc)
	{
		if (dxbc->shader_chunk)
		{
			KlayGE::shared_ptr<ShaderProgram> shader = ShaderParse(*dxbc);
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
