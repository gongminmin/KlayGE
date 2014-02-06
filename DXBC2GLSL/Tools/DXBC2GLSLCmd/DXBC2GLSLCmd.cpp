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
		dxbc2glsl.FeedDXBC(&data[0], GSV_400);
		std::string glsl = dxbc2glsl.GLSLString();
		if (!screen_only)
		{
			out << glsl;
		}
		std::cout << glsl;
	}
	catch (std::exception& ex)
	{
		std::cout << "Error(s) in conversion:" << std::endl;
		std::cout << ex.what() << std::endl;
		std::cout << "Please send this information and your bytecode file to webmaster at klayge.org. We'll fix this ASAP." << std::endl;
	}
}
