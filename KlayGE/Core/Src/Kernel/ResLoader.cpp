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
#include <boost/filesystem.hpp>

#if defined KLAYGE_PLATFORM_WINDOWS
#include <windows.h>
#elif defined KLAYGE_PLATFORM_LINUX
#elif defined KLAYGE_PLATFORM_ANDROID
#include <KlayGE/Context.hpp>
#include <android/asset_manager.h>
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
#elif defined KLAYGE_PLATFORM_LINUX || defined KLAYGE_PLATFORM_ANDROID
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

#ifdef KLAYGE_PLATFORM_ANDROID
			exe_path_ = exe_path_.substr(0, exe_path_.find_last_of("/"));
			exe_path_ = exe_path_.substr(exe_path_.find_last_of("/") + 1);
			exe_path_ = exe_path_.substr(0, exe_path_.find_last_of("-"));
			exe_path_ = "/data/data/" + exe_path_;
#endif
		}
#endif

		pathes_.push_back("");

		this->AddPath("");
		this->AddPath("../");
		this->AddPath("../../media/RenderFX/");
		this->AddPath("../../media/Models/");
		this->AddPath("../../media/Textures/2D/");
		this->AddPath("../../media/Textures/3D/");
		this->AddPath("../../media/Textures/Cube/");
		this->AddPath("../../media/Textures/Juda/");
		this->AddPath("../../media/Fonts/");
		this->AddPath("../../media/PostProcessors/");
	}

	void ResLoader::AddPath(std::string const & path)
	{
		boost::filesystem::path new_path(path);
		if (!new_path.is_complete())
		{
			boost::filesystem::path full_path = exe_path_ / new_path;
			if (!boost::filesystem::exists(full_path))
			{
#ifndef KLAYGE_PLATFORM_ANDROID
				full_path = boost::filesystem::current_path() / new_path;
				if (!boost::filesystem::exists(full_path))
				{
					return;
				}
#else
				return;
#endif
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
		if (('/' == name[0]) || ('\\' == name[0]))
		{
			if (boost::filesystem::exists(name))
			{
				return name;
			}
		}
		else
		{
			typedef BOOST_TYPEOF(pathes_) PathesType;
			BOOST_FOREACH(PathesType::const_reference path, pathes_)
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
						if (boost::filesystem::exists(pkt_name)
								&& (boost::filesystem::is_regular_file(pkt_name)
										|| boost::filesystem::is_symlink(pkt_name)))
						{
							std::string::size_type const password_offset = pkt_name.find("|");
							std::string password;
							if (password_offset != std::string::npos)
							{
								password = pkt_name.substr(password_offset + 1);
								pkt_name = pkt_name.substr(0, password_offset - 1);
							}
							std::string const file_name = res_name.substr(pkt_offset + 2);

							uint64_t timestamp = boost::filesystem::last_write_time(pkt_name);
							ResIdentifierPtr pkt_file = MakeSharedPtr<ResIdentifier>(name, timestamp,
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
			}
		}

#ifdef KLAYGE_PLATFORM_ANDROID
		android_app* state = Context::Instance().AppState();
		AAssetManager* am = state->activity->assetManager;
		AAsset* asset = AAssetManager_open(am, name.c_str(), AASSET_MODE_UNKNOWN);
		if (asset != NULL)
		{
			AAsset_close(asset);
			return name;
		}
#endif

		return "";
	}

	ResIdentifierPtr ResLoader::Open(std::string const & name)
	{
		if (('/' == name[0]) || ('\\' == name[0]))
		{
			if (boost::filesystem::exists(name))
			{
				uint64_t timestamp = boost::filesystem::last_write_time(name);
				return MakeSharedPtr<ResIdentifier>(name, timestamp,
					MakeSharedPtr<std::ifstream>(name.c_str(), std::ios_base::binary));
			}
		}
		else
		{
			typedef BOOST_TYPEOF(pathes_) PathesType;
			BOOST_FOREACH(PathesType::const_reference path, pathes_)
			{
				std::string const res_name(path + name);

				if (boost::filesystem::exists(res_name))
				{
					uint64_t timestamp = boost::filesystem::last_write_time(res_name);
					return MakeSharedPtr<ResIdentifier>(name, timestamp,
						MakeSharedPtr<std::ifstream>(res_name.c_str(), std::ios_base::binary));
				}
				else
				{
					std::string::size_type const pkt_offset(res_name.find("//"));
					if (pkt_offset != std::string::npos)
					{
						std::string pkt_name = res_name.substr(0, pkt_offset);
						if (boost::filesystem::exists(pkt_name)
								&& (boost::filesystem::is_regular_file(pkt_name)
										|| boost::filesystem::is_symlink(pkt_name)))
						{
							std::string::size_type const password_offset = pkt_name.find("|");
							std::string password;
							if (password_offset != std::string::npos)
							{
								password = pkt_name.substr(password_offset + 1);
								pkt_name = pkt_name.substr(0, password_offset - 1);
							}
							std::string const file_name = res_name.substr(pkt_offset + 2);

							uint64_t timestamp = boost::filesystem::last_write_time(pkt_name);
							ResIdentifierPtr pkt_file = MakeSharedPtr<ResIdentifier>(name, timestamp,
								MakeSharedPtr<std::ifstream>(pkt_name.c_str(), std::ios_base::binary));
							if (*pkt_file)
							{
								boost::shared_ptr<std::iostream> packet_file = MakeSharedPtr<std::stringstream>();
								Extract7z(pkt_file, password, file_name, packet_file);
								return MakeSharedPtr<ResIdentifier>(name, timestamp, packet_file);
							}
						}
					}
				}
			}
		}

#ifdef KLAYGE_PLATFORM_ANDROID
		android_app* state = Context::Instance().AppState();
		AAssetManager* am = state->activity->assetManager;
		AAsset* asset = AAssetManager_open(am, name.c_str(), AASSET_MODE_UNKNOWN);
		if (asset != NULL)
		{
			boost::shared_ptr<std::stringstream> asset_file = MakeSharedPtr<std::stringstream>(std::ios_base::in | std::ios_base::out | std::ios_base::binary);

			int total = 0;
			int bytes = 0;
			char buf[1024];
			while ((bytes = AAsset_read(asset, buf, sizeof(buf))) > 0)
			{
				total += bytes;
				asset_file->write(buf, bytes);
			}

			AAsset_close(asset);

			return MakeSharedPtr<ResIdentifier>(name, 0, asset_file);
		}
#endif

		return ResIdentifierPtr();
	}

	void* ResLoader::SyncQuery(ResLoadingDescPtr const & res_desc)
	{
		if (res_desc->HasSubThreadStage())
		{
			res_desc->SubThreadStage();
		}
		return res_desc->MainThreadStage();
	}

	boost::function<void*()> ResLoader::ASyncQuery(ResLoadingDescPtr const & res_desc)
	{
		boost::shared_ptr<joiner<void> > async_thread;
		if (res_desc->HasSubThreadStage())
		{
			async_thread = MakeSharedPtr<joiner<void> >(GlobalThreadPool()(boost::bind(&ResLoader::ASyncSubThreadFunc, this, res_desc)));
		}
		return boost::bind(&ResLoader::ASyncFunc, this, res_desc, async_thread);
	}

	void ResLoader::ASyncSubThreadFunc(ResLoadingDescPtr const & res_desc)
	{
		res_desc->SubThreadStage();
	}

	void* ResLoader::ASyncFunc(ResLoadingDescPtr const & res_desc, boost::shared_ptr<joiner<void> > const & loading_thread)
	{
		if (res_desc->HasSubThreadStage())
		{
			(*loading_thread)();
		}
		return res_desc->MainThreadStage();
	}
}
