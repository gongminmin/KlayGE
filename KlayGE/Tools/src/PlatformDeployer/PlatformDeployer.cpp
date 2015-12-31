#include <KlayGE/KlayGE.hpp>
#include <KFL/Util.hpp>
#include <KlayGE/JudaTexture.hpp>
#include <KlayGE/ResLoader.hpp>
#include <KFL/XMLDom.hpp>

#include <iostream>
#include <fstream>
#include <vector>

#include <boost/algorithm/string/case_conv.hpp>

#if defined(KLAYGE_TS_LIBRARY_FILESYSTEM_V3_SUPPORT)
	#include <experimental/filesystem>
#elif defined(KLAYGE_TS_LIBRARY_FILESYSTEM_V2_SUPPORT)
	#include <filesystem>
	namespace std
	{
		namespace experimental
		{
			namespace filesystem = std::tr2::sys;
		}
	}
#else
	#if defined(KLAYGE_COMPILER_GCC)
		#pragma GCC diagnostic push
		#pragma GCC diagnostic ignored "-Wdeprecated-declarations" // Ignore auto_ptr declaration
	#endif
	#include <boost/filesystem.hpp>
	#if defined(KLAYGE_COMPILER_GCC)
		#pragma GCC diagnostic pop
	#endif
	namespace std
	{
		namespace experimental
		{
			namespace filesystem = boost::filesystem;
		}
	}
#endif
#ifdef KLAYGE_CXX11_LIBRARY_REGEX_SUPPORT
	#include <regex>
#else
	#include <boost/regex.hpp>
	namespace std
	{
		using boost::regex;
		using boost::regex_match;
		using boost::smatch;
	}
#endif

#if defined(KLAYGE_COMPILER_GCC)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations" // Ignore auto_ptr declaration
#endif
#include <boost/program_options.hpp>
#if defined(KLAYGE_COMPILER_GCC)
#pragma GCC diagnostic pop
#endif
#if defined(KLAYGE_COMPILER_GCC)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations" // Ignore auto_ptr declaration
#endif
#include <boost/algorithm/string/split.hpp>
#if defined(KLAYGE_COMPILER_GCC)
#pragma GCC diagnostic pop
#endif
#include <boost/algorithm/string/trim.hpp>

using namespace std;
using namespace KlayGE;
using namespace std::experimental;

struct OfflineRenderDeviceCaps
{
	std::string platform;
	uint8_t major_version;
	uint8_t minor_version;

	bool bc1_support : 1;
	bool bc3_support : 1;
	bool bc5_support : 1;
	bool bc7_support : 1;
	bool etc1_support : 1;
	bool r16_support : 1;
	bool r16f_support : 1;
	bool srgb_support : 1;
};

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

int RetrieveAttrValue(XMLNodePtr node, std::string const & attr_name, int default_value)
{
	XMLAttributePtr attr = node->Attrib(attr_name);
	if (attr)
	{
		return attr->ValueInt();
	}

	return default_value;
}

std::string RetrieveAttrValue(XMLNodePtr node, std::string const & attr_name, std::string const & default_value)
{
	XMLAttributePtr attr = node->Attrib(attr_name);
	if (attr)
	{
		return attr->ValueString();
	}

	return default_value;
}

int RetrieveNodeValue(XMLNodePtr root, std::string const & node_name, int default_value)
{
	XMLNodePtr node = root->FirstNode(node_name);
	if (node)
	{
		return RetrieveAttrValue(node, "value", default_value);
	}

	return default_value;
}

std::string RetrieveNodeValue(XMLNodePtr root, std::string const & node_name, std::string const & default_value)
{
	XMLNodePtr node = root->FirstNode(node_name);
	if (node)
	{
		return RetrieveAttrValue(node, "value", default_value);
	}

	return default_value;
}

OfflineRenderDeviceCaps LoadPlatformConfig(std::string const & platform)
{
	ResIdentifierPtr plat = ResLoader::Instance().Open("PlatConf/" + platform + ".plat");

	KlayGE::XMLDocument doc;
	XMLNodePtr root = doc.Parse(plat);

	OfflineRenderDeviceCaps caps;

	caps.platform = RetrieveAttrValue(root, "name", "");
	caps.major_version = static_cast<uint8_t>(RetrieveAttrValue(root, "major_version", 0));
	caps.minor_version = static_cast<uint8_t>(RetrieveAttrValue(root, "minor_version", 0));

	caps.bc1_support = RetrieveNodeValue(root, "bc1_support", 0) ? true : false;
	caps.bc3_support = RetrieveNodeValue(root, "bc3_support", 0) ? true : false;
	caps.bc5_support = RetrieveNodeValue(root, "bc5_support", 0) ? true : false;
	caps.bc7_support = RetrieveNodeValue(root, "bc7_support", 0) ? true : false;
	caps.etc1_support = RetrieveNodeValue(root, "etc1_support", 0) ? true : false;
	caps.r16_support = RetrieveNodeValue(root, "r16_support", 0) ? true : false;
	caps.r16f_support = RetrieveNodeValue(root, "r16f_support", 0) ? true : false;
	caps.srgb_support = RetrieveNodeValue(root, "srgb_support", 0) ? true : false;

	return caps;
}

void Deploy(std::vector<std::string> const & res_names, std::string const & res_type, OfflineRenderDeviceCaps const & caps)
{
	std::ofstream ofs("convert.bat");

	ofs << "@echo off" << std::endl << std::endl;
	
	if (("diffuse" == res_type)
		|| ("specular" == res_type)
		|| ("emit" == res_type))
	{
		for (size_t i = 0; i < res_names.size(); ++i)
		{
			if (caps.srgb_support)
			{
				ofs << "ForceTexSRGB \"" << res_names[i] << "\" temp.dds" << std::endl;
			}
			else
			{
				ofs << "copy \"" << res_names[i] << "\" temp.dds" << std::endl;
			}
			ofs << "Mipmapper temp.dds" << std::endl;
			if (caps.bc7_support)
			{
				ofs << "TexCompressor BC7 temp.dds \"" << res_names[i] << "\"" << std::endl;
			}
			else if (caps.bc1_support)
			{
				ofs << "TexCompressor BC1 temp.dds \"" << res_names[i] << "\"" << std::endl;
			}
			else if (caps.etc1_support)
			{
				ofs << "TexCompressor ETC1 temp.dds \"" << res_names[i] << "\"" << std::endl;
			}
			else
			{
				ofs << "copy temp.dds \"" << res_names[i] << "\"" << std::endl;
			}
			ofs << "del temp.dds" << std::endl;
		}
	}
	else if ("normal" == res_type)
	{
		for (size_t i = 0; i < res_names.size(); ++ i)
		{
			ofs << "Mipmapper \"" << res_names[i] << "\" temp.dds" << std::endl;
			if (caps.bc5_support)
			{
				ofs << "NormalMapCompressor temp.dds \"" << res_names[i] << "\" BC5" << std::endl;
			}
			else if (caps.bc3_support)
			{
				ofs << "NormalMapCompressor temp.dds \"" << res_names[i] << "\" BC3" << std::endl;
			}
			else
			{
				ofs << "copy temp.dds \"" << res_names[i] << "\"" << std::endl;
			}
			ofs << "del temp.dds" << std::endl;
		}
	}
	else if ("bump" == res_type)
	{
		for (size_t i = 0; i < res_names.size(); ++ i)
		{
			ofs << "Bump2Normal \"" << res_names[i] << "\" temp.dds 0.4" << std::endl;
			ofs << "Mipmapper temp.dds" << std::endl; 
			if (caps.bc5_support)
			{
				ofs << "NormalMapCompressor temp.dds \"" << res_names[i] << "\" BC5" << std::endl;
			}
			else if (caps.bc3_support)
			{
				ofs << "NormalMapCompressor temp.dds \"" << res_names[i] << "\" BC3" << std::endl;
			}
			else
			{
				ofs << "copy temp.dds \"" << res_names[i] << "\"" << std::endl;
			}
			ofs << "del temp.dds" << std::endl;
		}
	}
	else if ("cubemap" == res_type)
	{
		std::string y_fmt;
		std::string c_fmt;
		if (caps.r16_support)
		{
			y_fmt = "R16";
		}
		else if (caps.r16f_support)
		{
			y_fmt = "R16F";
		}
		if (caps.bc5_support)
		{
			c_fmt = "BC5";
		}
		else if (caps.bc3_support)
		{
			c_fmt = "BC3";
		}

		for (size_t i = 0; i < res_names.size(); ++ i)
		{
			
			ofs << "HDRCompressor \"" << res_names[i] << "\" " << y_fmt << ' ' << c_fmt << std::endl;
		}
	}
	else if ("model" == res_type)
	{
		for (size_t i = 0; i < res_names.size(); ++ i)
		{
			ofs << "MeshMLJIT -I \"" << res_names[i] << "\" -P " << caps.platform << std::endl;
		}
	}
	else if ("effect" == res_type)
	{
		for (size_t i = 0; i < res_names.size(); ++ i)
		{
			ofs << "FXMLJIT " << caps.platform << " \"" << res_names[i] << "\"" << std::endl;
		}
	}

	ofs.close();

	if ((res_type != "cubemap") && (res_type != "model") && (res_type != "effect"))
	{
		system("convert.bat");
		system("del convert.bat");
	}
}

int main(int argc, char* argv[])
{
	ResLoader::Instance().AddPath("../../Tools/media/PlatformDeployer");

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

		std::vector<std::string> tokens;
		boost::algorithm::split(tokens, input_name_str, boost::is_any_of(",;"));
		for (auto& arg : tokens)
		{
			boost::algorithm::trim(arg);
			if ((std::string::npos == arg.find('*')) && (std::string::npos == arg.find('?')))
			{
				res_names.push_back(arg);
			}
			else
			{
				std::regex const filter(DosWildcardToRegex(arg));

				filesystem::directory_iterator end_itr;
				for (filesystem::directory_iterator i("."); i != end_itr; ++ i)
				{
					if (filesystem::is_regular_file(i->status()))
					{
						std::smatch what;
#ifdef KLAYGE_TS_LIBRARY_FILESYSTEM_V2_SUPPORT
						std::string const name = i->path().filename();
#else
						std::string const name = i->path().filename().string();
#endif
						if (std::regex_match(name, what, filter))
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
#ifdef KLAYGE_TS_LIBRARY_FILESYSTEM_V2_SUPPORT
		std::string ext_name = filesystem::path(res_names[0]).extension();
#else
		std::string ext_name = filesystem::path(res_names[0]).extension().string();
#endif
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
		platform = "d3d_11_0";
	}

	boost::algorithm::to_lower(res_type);
	boost::algorithm::to_lower(platform);

	if (("pc_dx11" == platform) || ("pc_dx10" == platform) || ("pc_dx9" == platform) || ("win_tegra3" == platform)
		|| ("pc_gl4" == platform) || ("pc_gl3" == platform) || ("pc_gl2" == platform)
		|| ("android_tegra3" == platform) || ("ios" == platform))
	{
		if ("pc_dx11" == platform)
		{
			platform = "d3d_11_0";
		}
		else if ("pc_dx10" == platform)
		{
			platform = "d3d_10_0";
		}
		else if ("pc_dx9" == platform)
		{
			platform = "d3d_9_3";
		}
		else if ("win_tegra3" == platform)
		{
			platform = "d3d_9_1";
		}
		else if ("pc_gl4" == platform)
		{
			platform = "gl_4_0";
		}
		else if ("pc_gl3" == platform)
		{
			platform = "gl_3_0";
		}
		else if ("pc_gl2" == platform)
		{
			platform = "gl_2_0";
		}
		else if ("android_tegra3" == platform)
		{
			platform = "gles_2_0";
		}
		else if ("ios" == platform)
		{
			platform = "gles_2_0";
		}
	}

	OfflineRenderDeviceCaps caps = LoadPlatformConfig(platform);
	Deploy(res_names, res_type, caps);

	Context::Destroy();

	return 0;
}
