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
#include <KlayGE/Extract7z.hpp>

#include <fstream>
#include <sstream>
#include <boost/typeof/typeof.hpp>
#include <boost/foreach.hpp>

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
	}

	void ResLoader::AddPath(std::string const & path)
	{
		if (path[path.length() - 1] != '/')
		{
			pathes_.push_back(path + '/');
		}
		else
		{
			pathes_.push_back(path);
		}
	}

	std::string ResLoader::Locate(std::string const & name)
	{
		BOOST_FOREACH(BOOST_TYPEOF(pathes_)::const_reference path, pathes_)
		{
			std::string const resName(path + name);

			std::ifstream ifs(resName.c_str(), std::ios_base::binary);

			if (!ifs.fail())
			{
				return resName;
			}
			else
			{
				std::string::size_type const offset(resName.rfind(".7z/"));
				if (offset != std::string::npos)
				{
					std::string const pktName(resName.substr(0, offset + 4));
					std::string const fileName(resName.substr(offset + 5));

					boost::shared_ptr<std::istream> pktFile(new std::ifstream(pktName.c_str(), std::ios_base::binary));
					if (!pktFile->fail())
					{
						return resName;
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
			std::string const resName(path + name);

			ResIdentifierPtr ret(new std::ifstream(resName.c_str(), std::ios_base::binary));

			if (!ret->fail())
			{
				return ret;
			}
			else
			{
				std::string::size_type const offset(resName.rfind(".7z/"));
				if (offset != std::string::npos)
				{
					std::string const pktName(resName.substr(0, offset + 4));
					std::string const fileName(resName.substr(offset + 5));

					boost::shared_ptr<std::istream> pktFile(new std::ifstream(pktName.c_str(), std::ios_base::binary));
					if (!pktFile->fail())
					{
						boost::shared_ptr<std::iostream> packetFile(new std::stringstream);
						Extract7z(pktFile, "", fileName, packetFile);
						return packetFile;
					}
				}
			}
		}

		BOOST_ASSERT(false);
		return ResIdentifierPtr();
	}
}
