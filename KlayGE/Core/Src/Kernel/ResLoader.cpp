// ResLoader.cpp
// KlayGE 资源载入器 实现文件
// Ver 3.6.0
// 版权所有(C) 龚敏敏, 2006-2007
// Homepage: http://klayge.sourceforge.net
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
#include <boost/filesystem.hpp>

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
		pathes_.push_back("");

		this->AddPath("");
		this->AddPath("../media/RenderFX/");
		this->AddPath("../media/Models/");
		this->AddPath("../media/Textures/2D/");
		this->AddPath("../media/Textures/3D/");
		this->AddPath("../media/Textures/Cube/");
		this->AddPath("../media/Fonts/");
	}

	void ResLoader::AddPath(std::string const & path)
	{
		boost::filesystem::path new_path(path);
		if (!new_path.is_complete())
		{
			new_path = boost::filesystem::current_path() / new_path;
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
					std::string pkt_name = res_name.substr(0, pkt_offset - 1);
					std::string::size_type const password_offset = pkt_name.find("|");
					std::string password = pkt_name.substr(password_offset + 1);
					pkt_name = pkt_name.substr(0, password_offset - 1);
					std::string const file_name = res_name.substr(pkt_offset + 2);

					ResIdentifierPtr pkt_file = MakeSharedPtr<std::ifstream>(pkt_name.c_str(), std::ios_base::binary);
					if (*pkt_file)
					{
						if (Find7z(pkt_file, password, file_name))
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
				return MakeSharedPtr<std::ifstream>(res_name.c_str(), std::ios_base::binary);
			}
			else
			{
				std::string::size_type const pkt_offset(res_name.find("//"));
				if (pkt_offset != std::string::npos)
				{
					std::string pkt_name = res_name.substr(0, pkt_offset - 1);
					std::string::size_type const password_offset = pkt_name.find("|");
					std::string password = pkt_name.substr(password_offset + 1);
					pkt_name = pkt_name.substr(0, password_offset - 1);
					std::string const file_name = res_name.substr(pkt_offset + 2);

					ResIdentifierPtr pkt_file = MakeSharedPtr<std::ifstream>(pkt_name.c_str(), std::ios_base::binary);
					if (*pkt_file)
					{
						boost::shared_ptr<std::iostream> packet_file = MakeSharedPtr<std::stringstream>();
						Extract7z(pkt_file, password, file_name, packet_file);
						return packet_file;
					}
				}
			}
		}

		BOOST_ASSERT(false);
		return ResIdentifierPtr();
	}
}
