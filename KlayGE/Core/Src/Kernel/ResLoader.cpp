// ResLoader.cpp
// KlayGE 资源载入器 实现文件
// Ver 3.6.0
// 版权所有(C) 龚敏敏, 2006-2007
// Homepage: http://www.klayge.org
//
// 3.6.0
// 使用新的kpk格式 (2007.5.24)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KlayGE/Util.hpp>
#include <KlayGE/Extract7z.hpp>

#include <fstream>
#include <sstream>
#include <boost/typeof/typeof.hpp>
#include <boost/foreach.hpp>
#define BOOST_FILESYSTEM_VERSION 3
#include <boost/filesystem.hpp>

#if defined KLAYGE_PLATFORM_WINDOWS
#include <windows.h>
#elif defined KLAYGE_PLATFORM_LINUX
#endif

#include <KlayGE/ResLoader.hpp>

namespace KlayGE
{
	ResLoader& ResLoader::Instance()
	{
		static ResLoader resLoader;
		return resLoader;
	}

	ResLoader::ResLoader()
	{
#if defined KLAYGE_PLATFORM_WINDOWS
		char buf[MAX_PATH];
		GetModuleFileNameA(NULL, buf, sizeof(buf));
		exe_path_ = buf;
		exe_path_ = exe_path_.substr(0, exe_path_.rfind("\\"));
#elif defined KLAYGE_PLATFORM_LINUX
		{
			char line[1024];
			void const * symbol = "";
			FILE* fp = fopen("/proc/self/maps", "r");
			if (fp != NULL)
			{
				while (!feof(fp))
				{
					unsigned long start, end;
					if (!fgets(line, sizeof(line), fp))
					{
						continue;
					}
					if (!strstr(line, " r-xp ") || !strchr(line, '/'))
					{
						continue;
					}

					sscanf(line, "%lx-%lx ", &start, &end);
					if ((symbol >= reinterpret_cast<void const *>(start)) && (symbol < reinterpret_cast<void const *>(end)))
					{
						exe_path_ = strchr(line, '/');
						exe_path_ = exe_path_.substr(0, exe_path_.rfind("/"));
					}
				}
				fclose(fp);
			}
		}
#endif

		pathes_.push_back("");

		this->AddPath("");
		this->AddPath("../media/RenderFX/");
		this->AddPath("../media/Models/");
		this->AddPath("../media/Textures/2D/");
		this->AddPath("../media/Textures/3D/");
		this->AddPath("../media/Textures/Cube/");
		this->AddPath("../media/Fonts/");
		this->AddPath("../media/PostProcessors/");
	}

	void ResLoader::AddPath(std::string const & path)
	{
		boost::filesystem::path new_path(path);
		if (!new_path.is_complete())
		{
			boost::filesystem::path full_path = exe_path_ / new_path;
			if (!boost::filesystem::exists(full_path))
			{
				full_path = boost::filesystem::current_path() / new_path;
				if (!boost::filesystem::exists(full_path))
				{
					return;
				}
			}
			new_path = full_path;
		}
		std::string path_str = new_path.string();
		if (path_str[path_str.length() - 1] != '/')
		{
			path_str.push_back('/');
		}
		pathes_.push_back(path_str);
	}

	std::string ResLoader::Locate(std::string const & name)
	{
		BOOST_FOREACH(BOOST_TYPEOF(pathes_)::const_reference path, pathes_)
		{
			std::string const res_name(path + name);

			if (boost::filesystem::exists(res_name))
			{
				return res_name;
			}
			else
			{
				std::string::size_type const pkt_offset(res_name.find("//"));
				if (pkt_offset != std::string::npos)
				{
					std::string pkt_name = res_name.substr(0, pkt_offset);
					std::string::size_type const password_offset = pkt_name.find("|");
					std::string password;
					if (password_offset != std::string::npos)
					{
						password = pkt_name.substr(password_offset + 1);
						pkt_name = pkt_name.substr(0, password_offset - 1);
					}
					std::string const file_name = res_name.substr(pkt_offset + 2);

					ResIdentifierPtr pkt_file = MakeSharedPtr<ResIdentifier>(name,
						MakeSharedPtr<std::ifstream>(pkt_name.c_str(), std::ios_base::binary));
					if (*pkt_file)
					{
						if (Find7z(pkt_file, password, file_name) != 0xFFFFFFFF)
						{
							return res_name;
						}
					}
				}
			}
		}

		return "";
	}

	ResIdentifierPtr ResLoader::Load(std::string const & name)
	{
		BOOST_FOREACH(BOOST_TYPEOF(pathes_)::const_reference path, pathes_)
		{
			std::string const res_name(path + name);

			if (boost::filesystem::exists(res_name))
			{
				return MakeSharedPtr<ResIdentifier>(name,
					MakeSharedPtr<std::ifstream>(res_name.c_str(), std::ios_base::binary));
			}
			else
			{
				std::string::size_type const pkt_offset(res_name.find("//"));
				if (pkt_offset != std::string::npos)
				{
					std::string pkt_name = res_name.substr(0, pkt_offset);
					std::string::size_type const password_offset = pkt_name.find("|");
					std::string password;
					if (password_offset != std::string::npos)
					{
						password = pkt_name.substr(password_offset + 1);
						pkt_name = pkt_name.substr(0, password_offset - 1);
					}
					std::string const file_name = res_name.substr(pkt_offset + 2);

					ResIdentifierPtr pkt_file = MakeSharedPtr<ResIdentifier>(name,
						MakeSharedPtr<std::ifstream>(pkt_name.c_str(), std::ios_base::binary));
					if (*pkt_file)
					{
						boost::shared_ptr<std::iostream> packet_file = MakeSharedPtr<std::stringstream>();
						Extract7z(pkt_file, password, file_name, packet_file);
						return MakeSharedPtr<ResIdentifier>(name, packet_file);
					}
				}
			}
		}

		return ResIdentifierPtr();
	}
}
