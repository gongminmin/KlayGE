#include <KlayGE/KlayGE.hpp>
#include <KFL/Util.hpp>
#include <KFL/Timer.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/ResLoader.hpp>
#include <KlayGE/Texture.hpp>
#include <KlayGE/Mesh.hpp>
#include <KFL/Math.hpp>
#include <KlayGE/App3D.hpp>
#include <KlayGE/RenderFactory.hpp>

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>

#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/algorithm/string/case_conv.hpp>

#include <boost/filesystem.hpp>

#ifdef KLAYGE_CXX11_LIBRARY_SUPPORT
	#include <regex>

	namespace KlayGE
	{
		using std::regex;
		using std::regex_match;
		using std::smatch;
	}
#else
	#include <boost/regex.hpp>

	namespace KlayGE
	{
		using boost::regex;
		using boost::regex_match;
		using boost::smatch;
	}
#endif

#ifdef KLAYGE_COMPILER_MSVC
#pragma warning(push)
#pragma warning(disable: 4100 4251 4275 4273 4512 4701 4702)
#endif
#include <boost/program_options.hpp>
#ifdef KLAYGE_COMPILER_MSVC
#pragma warning(pop)
#endif
#ifdef KLAYGE_COMPILER_MSVC
#pragma warning(push)
#pragma warning(disable: 4127 6328)
#endif
#include <boost/tokenizer.hpp>
#ifdef KLAYGE_COMPILER_MSVC
#pragma warning(pop)
#endif

#include <KlayGE/JudaTexture.hpp>

using namespace std;
using namespace KlayGE;

std::string DosWildcardToRegex(std::string const & wildcard)
{
	std::string ret;
	for (size_t i = 0; i < wildcard.size(); ++ i)
	{
		switch (wildcard[i])
		{
		case '*':
			ret.append(".*");
			break;

		case '?':
			ret.append(".");
			break;

		case '+':
		case '(':
		case ')':
		case '^':
		case '$':
		case '.':
		case '{':
		case '}':
		case '[':
		case ']':
		case '|':
		case '\\':
			ret.push_back('\\');
			ret.push_back(wildcard[i]);
			break;

		default:
			ret.push_back(wildcard[i]);
			break;
		}
	}

	return ret;
}

void Deploy(std::vector<std::string> const & res_names, std::string const & res_type, std::string const & platform)
{
	std::ofstream ofs("convert.bat");

	ofs << "@echo off" << std::endl << std::endl;
		
	if (("pc_dx11" == platform) || ("pc_dx10" == platform) || ("pc_gl4" == platform) || ("pc_gl3" == platform))
	{
		if (("diffuse" == res_type)
			|| ("specular" == res_type)
			|| ("emit" == res_type))
		{
			for (size_t i = 0; i < res_names.size(); ++ i)
			{
				ofs << "ForceTexSRGB \"" << res_names[i] << "\" temp.dds" << std::endl;
				ofs << "Mipmapper temp.dds" << std::endl; 
				ofs << "TexCompressor BC1 temp.dds \"" << res_names[i] << "\"" << std::endl;
				ofs << "del temp.dds" << std::endl;
			}
		}
		else if ("normal" == res_type)
		{
			for (size_t i = 0; i < res_names.size(); ++ i)
			{
				ofs << "Mipmapper \"" << res_names[i] << "\" temp.dds" << std::endl; 
				ofs << "NormalMapCompressor temp.dds \"" << res_names[i] << "\" BC5" << std::endl;
				ofs << "del temp.dds" << std::endl;
			}
		}
		else if ("bump" == res_type)
		{
			for (size_t i = 0; i < res_names.size(); ++ i)
			{
				ofs << "Bump2Normal \"" << res_names[i] << "\" temp.dds 0.4" << std::endl;
				ofs << "Mipmapper temp.dds" << std::endl; 
				ofs << "NormalMapCompressor temp.dds \"" << res_names[i] << "\" BC5" << std::endl;
				ofs << "del temp.dds" << std::endl;
			}
		}
		else if ("cubemap" == res_type)
		{
			for (size_t i = 0; i < res_names.size(); ++ i)
			{
				ofs << "HDRCompressor \"" << res_names[i] << "\" R16 BC5" << std::endl;
			}
		}
		else if ("model" == res_type)
		{
			for (size_t i = 0; i < res_names.size(); ++ i)
			{
				SyncLoadModel(res_names[i], EAH_GPU_Read | EAH_Immutable);
			}
		}
	}
	else if (("pc_dx9" == platform) || ("pc_gl2" == platform))
	{
		if (("diffuse" == res_type)
			|| ("specular" == res_type)
			|| ("emit" == res_type))
		{
			for (size_t i = 0; i < res_names.size(); ++ i)
			{
				ofs << "ForceTexSRGB \"" << res_names[i] << "\" temp.dds" << std::endl;
				ofs << "Mipmapper temp.dds" << std::endl; 
				ofs << "TexCompressor BC1 temp.dds \"" << res_names[i] << "\"" << std::endl;
				ofs << "del temp.dds" << std::endl;
			}
		}
		else if ("normal" == res_type)
		{
			for (size_t i = 0; i < res_names.size(); ++ i)
			{
				ofs << "Mipmapper \"" << res_names[i] << "\" temp.dds" << std::endl; 
				ofs << "NormalMapCompressor temp.dds \"" << res_names[i] << "\" BC3" << std::endl;
				ofs << "del temp.dds" << std::endl;
			}
		}
		else if ("bump" == res_type)
		{
			for (size_t i = 0; i < res_names.size(); ++ i)
			{
				ofs << "Bump2Normal \"" << res_names[i] << "\" temp.dds 0.4" << std::endl;
				ofs << "Mipmapper temp.dds" << std::endl; 
				ofs << "NormalMapCompressor temp.dds \"" << res_names[i] << "\" BC3" << std::endl;
				ofs << "del temp.dds" << std::endl;
			}
		}
		else if ("cubemap" == res_type)
		{
			for (size_t i = 0; i < res_names.size(); ++ i)
			{
				ofs << "HDRCompressor \"" << res_names[i] << "\" R16 BC3" << std::endl;
			}
		}
		else if ("model" == res_type)
		{
			for (size_t i = 0; i < res_names.size(); ++ i)
			{
				SyncLoadModel(res_names[i], EAH_GPU_Read | EAH_Immutable);
			}
		}
	}
	else if ("android_tegra3" == platform)
	{
		if (("diffuse" == res_type)
			|| ("specular" == res_type)
			|| ("emit" == res_type))
		{
			for (size_t i = 0; i < res_names.size(); ++ i)
			{
				ofs << "Mipmapper \"" << res_names[i] << "\" temp.dds" << std::endl; 
				ofs << "TexCompressor BC1 temp.dds \"" << res_names[i] << "\"" << std::endl;
				ofs << "del temp.dds" << std::endl;
			}
		}
		else if ("normal" == res_type)
		{
			for (size_t i = 0; i < res_names.size(); ++ i)
			{
				ofs << "Mipmapper \"" << res_names[i] << "\" temp.dds" << std::endl; 
				ofs << "NormalMapCompressor temp.dds \"" << res_names[i] << "\" BC3" << std::endl;
				ofs << "del temp.dds" << std::endl;
			}
		}
		else if ("bump" == res_type)
		{
			for (size_t i = 0; i < res_names.size(); ++ i)
			{
				ofs << "Bump2Normal \"" << res_names[i] << "\" temp.dds 0.4" << std::endl;
				ofs << "Mipmapper temp.dds" << std::endl; 
				ofs << "NormalMapCompressor temp.dds \"" << res_names[i] << "\" BC3" << std::endl;
				ofs << "del temp.dds" << std::endl;
			}
		}
		else if ("cubemap" == res_type)
		{
			for (size_t i = 0; i < res_names.size(); ++ i)
			{
				ofs << "HDRCompressor \"" << res_names[i] << "\" R16F BC3" << std::endl;
			}
		}
		else if ("model" == res_type)
		{
			for (size_t i = 0; i < res_names.size(); ++ i)
			{
				SyncLoadModel(res_names[i], EAH_GPU_Read | EAH_Immutable);
			}
		}
	}

	ofs.close();

	if (res_type != "model")
	{
		system("convert.bat");
		system("del convert.bat");
	}
}

int main(int argc, char* argv[])
{
	std::vector<std::string> res_names;
	std::string res_type;
	std::string platform;

	boost::program_options::options_description desc("Allowed options");
	desc.add_options()
		("help,H", "Produce help message")
		("input-name,I", boost::program_options::value<std::string>(), "Input resource name.")
		("type,T", boost::program_options::value<std::string>(), "Resource type.")
		("platform,P", boost::program_options::value<std::string>(), "Platform name.")
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
		cout << "KlayGE PlatformDeployer, Version 1.0.0" << endl;
		return 1;
	}
	if (vm.count("input-name") > 0)
	{
		std::string input_name_str = vm["input-name"].as<std::string>();

		boost::char_separator<char> sep("", ",;");
		boost::tokenizer<boost::char_separator<char> > tok(input_name_str, sep);
		for (KLAYGE_AUTO(beg, tok.begin()); beg != tok.end(); ++ beg)
		{
			std::string arg = *beg;
			if ((std::string::npos == arg.find("*")) && (std::string::npos == arg.find("?")))
			{
				res_names.push_back(arg);
			}
			else
			{
				regex const filter(DosWildcardToRegex(arg));

				boost::filesystem::directory_iterator end_itr;
				for (boost::filesystem::directory_iterator i("."); i != end_itr; ++ i)
				{
					if (boost::filesystem::is_regular_file(i->status()))
					{
						smatch what;
						std::string const name = i->path().filename().string();
						if (regex_match(name, what, filter))
						{
							res_names.push_back(name);
						}
					}
				}
			}
		}
	}
	else
	{
		cout << "Need input resources names." << endl;
		return 1;
	}
	if (vm.count("type") > 0)
	{
		res_type = vm["type"].as<std::string>();
	}
	else
	{
		std::string ext_name = boost::filesystem::extension(res_names[0]);
		if (".dds" == ext_name)
		{
			res_type = "diffuse";
		}
		else if (".meshml" == ext_name)
		{
			res_type = "model";
		}
		else
		{
			cout << "Need resource type name." << endl;
			return 1;
		}
	}
	if (vm.count("platform") > 0)
	{
		platform = vm["platform"].as<std::string>();
	}
	else
	{
		platform = "pc_dx11";
	}

	boost::algorithm::to_lower(res_type);
	boost::algorithm::to_lower(platform);

	Context::Instance().LoadCfg("KlayGE.cfg");
	ContextCfg context_cfg = Context::Instance().Config();
	if (("pc_dx11" == platform) || ("pc_dx10" == platform) || ("pc_dx9" == platform))
	{
		context_cfg.render_factory_name = "D3D11";
	}
	if (("pc_gl4" == platform) || ("pc_gl3" == platform) || ("pc_gl2" == platform))
	{
		context_cfg.render_factory_name = "OpenGL";
	}
	if ("android_tegra3" == platform)
	{
		context_cfg.render_factory_name = "OpenGLES";
	}
	Context::Instance().Config(context_cfg);

	Deploy(res_names, res_type, platform);

	return 0;
}
