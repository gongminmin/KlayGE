#include <KlayGE/KlayGE.hpp>
#include <KlayGE/LZSS/LZSS.hpp>

#include <fstream>
#include <sstream>

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

	ResIdentifierPtr ResLoader::Load(std::string const & name)
	{
		for (size_t i = 0; i < pathes_.size(); ++ i)
		{
			std::string const resName(pathes_[i] + name);

			ResIdentifierPtr ret(new std::ifstream(resName.c_str(), std::ios_base::binary));

			if (!ret->fail())
			{
				return ret;
			}
			else
			{
				std::string::size_type const offset(resName.rfind(".pkt/"));
				std::string const pktName(resName.substr(0, offset + 4));
				std::string const fileName(resName.substr(offset + 5));

				boost::shared_ptr<std::istream> pktFile(new std::ifstream(pktName.c_str(), std::ios_base::binary));
				if (!pktFile->fail())
				{
					UnPkt unPkt;
					unPkt.Open(pktFile);

					unPkt.LocateFile(fileName);

					std::vector<char> data(unPkt.CurFileSize());
					unPkt.ReadCurFile(&data[0]);

					boost::shared_ptr<std::iostream> packetFile(new std::stringstream);
					packetFile->write(&data[0], data.size());

					return packetFile;
				}
			}
		}

		throw;
	}
}
