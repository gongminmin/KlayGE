/**
 * @file ResLoader.cpp
 * @author Minmin Gong
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

#include <KlayGE/KlayGE.hpp>
#include <KFL/Util.hpp>
#include <KlayGE/Extract7z.hpp>
#include <KFL/Thread.hpp>

#include <fstream>
#include <sstream>
#ifdef KLAYGE_TR2_LIBRARY_FILESYSTEM_V2_SUPPORT
	#include <filesystem>
	namespace KlayGE
	{
		namespace filesystem = std::tr2::sys;
	}
#else
	#include <boost/filesystem.hpp>
	namespace KlayGE
	{
		namespace filesystem = boost::filesystem;
	}
#endif

#if defined KLAYGE_PLATFORM_WINDOWS
#include <windows.h>
#elif defined KLAYGE_PLATFORM_LINUX
#elif defined KLAYGE_PLATFORM_ANDROID
#include <KlayGE/Context.hpp>
#include <android/asset_manager.h>
#endif

#include <KlayGE/ResLoader.hpp>

namespace
{
	KlayGE::mutex singleton_mutex;
}

namespace KlayGE
{
	shared_ptr<ResLoader> ResLoader::res_loader_instance_;

	ResLoader::ResLoader()
		: quit_(false)
	{
#if defined KLAYGE_PLATFORM_WINDOWS
#if defined KLAYGE_PLATFORM_WINDOWS_DESKTOP
		char buf[MAX_PATH];
		GetModuleFileNameA(nullptr, buf, sizeof(buf));
		exe_path_ = buf;
		exe_path_ = exe_path_.substr(0, exe_path_.rfind("\\"));
#endif
#elif defined KLAYGE_PLATFORM_LINUX || defined KLAYGE_PLATFORM_ANDROID
		{
			char line[1024];
			void const * symbol = "";
			FILE* fp = fopen("/proc/self/maps", "r");
			if (fp != nullptr)
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

		paths_.push_back("");

#if defined KLAYGE_PLATFORM_WINDOWS_METRO
		this->AddPath("Assets/");
#else
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
#endif

		loading_thread_ = MakeSharedPtr<joiner<void> >(Context::Instance().ThreadPool()(
				bind(&ResLoader::LoadingThreadFunc, this)));
	}

	ResLoader::~ResLoader()
	{
		quit_ = true;
		(*loading_thread_)();
	}

	ResLoader& ResLoader::Instance()
	{
		if (!res_loader_instance_)
		{
			unique_lock<mutex> lock(singleton_mutex);
			if (!res_loader_instance_)
			{
				res_loader_instance_ = MakeSharedPtr<ResLoader>();
			}
		}
		return *res_loader_instance_;
	}

	void ResLoader::Destroy()
	{
		res_loader_instance_.reset();
	}

	std::string ResLoader::RealPath(std::string const & path)
	{
		filesystem::path new_path(path);
#ifdef KLAYGE_TR2_LIBRARY_FILESYSTEM_V2_SUPPORT
		if (!new_path.is_complete())
#else
		if (!new_path.is_absolute())
#endif
		{
			filesystem::path full_path = filesystem::path(exe_path_) / new_path;
			if (!filesystem::exists(full_path))
			{
#ifndef KLAYGE_PLATFORM_ANDROID
#ifdef KLAYGE_TR2_LIBRARY_FILESYSTEM_V2_SUPPORT
				full_path = filesystem::current_path<filesystem::path>() / new_path;
#else
				full_path = filesystem::current_path() / new_path;
#endif
				if (!filesystem::exists(full_path))
				{
					return "";
				}
#else
				return "";
#endif
			}
			new_path = full_path;
		}
		std::string path_str = new_path.string();

		if (path_str[path_str.length() - 1] != '/')
		{
			path_str.push_back('/');
		}

		return path_str;
	}

	void ResLoader::AddPath(std::string const & path)
	{
		std::string real_path = this->RealPath(path);
		if (!real_path.empty())
		{
			paths_.push_back(real_path);
		}
	}

	void ResLoader::DelPath(std::string const & path)
	{
		std::string real_path = this->RealPath(path);
		if (!real_path.empty())
		{
			KLAYGE_AUTO(iter, std::find(paths_.begin(), paths_.end(), real_path));
			if (iter != paths_.end())
			{
				paths_.erase(iter);
			}
		}
	}

	std::string ResLoader::Locate(std::string const & name)
	{
		if (('/' == name[0]) || ('\\' == name[0]))
		{
			if (filesystem::exists(filesystem::path(name)))
			{
				return name;
			}
		}
		else
		{
			typedef KLAYGE_DECLTYPE(paths_) PathsType;
			KLAYGE_FOREACH(PathsType::const_reference path, paths_)
			{
				std::string const res_name(path + name);

				if (filesystem::exists(filesystem::path(res_name)))
				{
					return res_name;
				}
				else
				{
					std::string::size_type const pkt_offset(res_name.find("//"));
					if (pkt_offset != std::string::npos)
					{
						std::string pkt_name = res_name.substr(0, pkt_offset);
						filesystem::path pkt_path(pkt_name);
						if (filesystem::exists(pkt_path)
								&& (filesystem::is_regular_file(pkt_path)
										|| filesystem::is_symlink(pkt_path)))
						{
							std::string::size_type const password_offset = pkt_name.find("|");
							std::string password;
							if (password_offset != std::string::npos)
							{
								password = pkt_name.substr(password_offset + 1);
								pkt_name = pkt_name.substr(0, password_offset - 1);
							}
							std::string const file_name = res_name.substr(pkt_offset + 2);

							uint64_t timestamp = filesystem::last_write_time(pkt_path);
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
		if (asset != nullptr)
		{
			AAsset_close(asset);
			return name;
		}
#endif

#ifndef KLAYGE_PLATFORM_WINDOWS_METRO
		return "";
#else
		std::string::size_type pos = name.rfind('/');
		if (std::string::npos == pos)
		{
			pos = name.rfind('\\');
		}
		if (pos != std::string::npos)
		{
			std::string file_name = name.substr(pos + 1);
			return this->Locate(file_name);
		}
		else
		{
			return "";
		}
#endif
	}

	ResIdentifierPtr ResLoader::Open(std::string const & name)
	{
		if (('/' == name[0]) || ('\\' == name[0]))
		{
			filesystem::path res_path(name);
			if (filesystem::exists(res_path))
			{
				uint64_t timestamp = filesystem::last_write_time(res_path);
				return MakeSharedPtr<ResIdentifier>(name, timestamp,
					MakeSharedPtr<std::ifstream>(name.c_str(), std::ios_base::binary));
			}
		}
		else
		{
			typedef KLAYGE_DECLTYPE(paths_) PathsType;
			KLAYGE_FOREACH(PathsType::const_reference path, paths_)
			{
				std::string const res_name(path + name);

				filesystem::path res_path(res_name);
				if (filesystem::exists(res_path))
				{
					uint64_t timestamp = filesystem::last_write_time(res_path);
					return MakeSharedPtr<ResIdentifier>(name, timestamp,
						MakeSharedPtr<std::ifstream>(res_name.c_str(), std::ios_base::binary));
				}
				else
				{
					std::string::size_type const pkt_offset(res_name.find("//"));
					if (pkt_offset != std::string::npos)
					{
						std::string pkt_name = res_name.substr(0, pkt_offset);
						filesystem::path pkt_path(pkt_name);
						if (filesystem::exists(pkt_path)
								&& (filesystem::is_regular_file(pkt_path)
										|| filesystem::is_symlink(pkt_path)))
						{
							std::string::size_type const password_offset = pkt_name.find("|");
							std::string password;
							if (password_offset != std::string::npos)
							{
								password = pkt_name.substr(password_offset + 1);
								pkt_name = pkt_name.substr(0, password_offset - 1);
							}
							std::string const file_name = res_name.substr(pkt_offset + 2);

							uint64_t timestamp = filesystem::last_write_time(pkt_path);
							ResIdentifierPtr pkt_file = MakeSharedPtr<ResIdentifier>(name, timestamp,
								MakeSharedPtr<std::ifstream>(pkt_name.c_str(), std::ios_base::binary));
							if (*pkt_file)
							{
								shared_ptr<std::iostream> packet_file = MakeSharedPtr<std::stringstream>();
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
		if (asset != nullptr)
		{
			shared_ptr<std::stringstream> asset_file = MakeSharedPtr<std::stringstream>(std::ios_base::in | std::ios_base::out | std::ios_base::binary);

			int bytes = 0;
			char buf[1024];
			while ((bytes = AAsset_read(asset, buf, sizeof(buf))) > 0)
			{
				asset_file->write(buf, bytes);
			}

			AAsset_close(asset);

			return MakeSharedPtr<ResIdentifier>(name, 0, asset_file);
		}
#endif


#ifndef KLAYGE_PLATFORM_WINDOWS_METRO
		return ResIdentifierPtr();
#else
		std::string::size_type pos = name.rfind('/');
		if (std::string::npos == pos)
		{
			pos = name.rfind('\\');
		}
		if (pos != std::string::npos)
		{
			std::string file_name = name.substr(pos + 1);
			return this->Open(file_name);
		}
		else
		{
			return ResIdentifierPtr();
		}
#endif
	}

	shared_ptr<void> ResLoader::SyncQuery(ResLoadingDescPtr const & res_desc)
	{
		this->RemoveUnrefResources();

		shared_ptr<void> loaded_res = this->FindMatchLoadedResource(res_desc);
		shared_ptr<void> res;
		if (!loaded_res)
		{
			if (res_desc->HasSubThreadStage())
			{
				res_desc->SubThreadStage();
			}

			res = res_desc->MainThreadStage();
			this->AddLoadedResource(res_desc, res);
		}
		else
		{
			if (res_desc->StateLess())
			{
				res = loaded_res;
			}
			else
			{
				res = res_desc->CloneResourceFrom(loaded_res);
				if (res != loaded_res)
				{
					this->AddLoadedResource(res_desc, res);
				}
			}
		}

		return res;
	}

	function<shared_ptr<void>()> ResLoader::ASyncQuery(ResLoadingDescPtr const & res_desc)
	{
		this->RemoveUnrefResources();

		shared_ptr<void> loaded_res = this->FindMatchLoadedResource(res_desc);
		if (!loaded_res)
		{
			shared_ptr<volatile bool> async_is_done;
			bool found = false;
			typedef KLAYGE_DECLTYPE(loading_res_) LoadingResQueueType;
			KLAYGE_FOREACH(LoadingResQueueType::const_reference lrq, loading_res_)
			{
				if (lrq.first->Match(*res_desc))
				{
					res_desc->CopyDataFrom(*lrq.first);
					async_is_done = lrq.second;
					found = true;
					break;
				}
			}

			if (found)
			{
				return bind(ResLoader::ASyncFunctor(loaded_res), res_desc, async_is_done);
			}
			else
			{
				if (res_desc->HasSubThreadStage())
				{
					async_is_done = MakeSharedPtr<bool>(false);
					loading_res_.push_back(std::make_pair(res_desc, async_is_done));
					loading_res_queue_.push(std::make_pair(res_desc, async_is_done));
					return bind(ResLoader::ASyncFunctor(loaded_res), res_desc, async_is_done);
				}
				else
				{
					shared_ptr<void> res = res_desc->MainThreadStage();
					this->AddLoadedResource(res_desc, res);
					return ResLoader::ASyncFunctor(res);
				}
			}
		}
		else
		{
			shared_ptr<void> res;
			if (res_desc->StateLess())
			{
				res = loaded_res;
			}
			else
			{
				res = res_desc->CloneResourceFrom(loaded_res);
				if (res != loaded_res)
				{
					this->AddLoadedResource(res_desc, res);
				}
			}
			return ResLoader::ASyncFunctor(res);
		}
	}

	void ResLoader::Unload(shared_ptr<void> const & res)
	{
		for (KLAYGE_AUTO(iter, loaded_res_.begin()); iter != loaded_res_.end(); ++ iter)
		{
			if (res == iter->second.lock())
			{
				loaded_res_.erase(iter);
				break;
			}
		}
	}

	void ResLoader::AddLoadedResource(ResLoadingDescPtr const & res_desc, shared_ptr<void> const & res)
	{
		bool found = false;
		typedef KLAYGE_DECLTYPE(loaded_res_) CachedDescType;
		KLAYGE_FOREACH(CachedDescType::reference c_desc, loaded_res_)
		{
			if (c_desc.first == res_desc)
			{
				c_desc.second = weak_ptr<void>(res);
				found = true;
				break;
			}
		}
		if (!found)
		{
			loaded_res_.push_back(std::make_pair(res_desc, weak_ptr<void>(res)));
		}
	}

	shared_ptr<void> ResLoader::FindMatchLoadedResource(ResLoadingDescPtr const & res_desc)
	{
		shared_ptr<void> loaded_res;
		typedef KLAYGE_DECLTYPE(loaded_res_) LoadedResType;
		KLAYGE_FOREACH(LoadedResType::const_reference lr, loaded_res_)
		{
			if (lr.first->Match(*res_desc))
			{
				loaded_res = lr.second.lock();
				break;
			}
		}
		return loaded_res;
	}

	void ResLoader::RemoveUnrefResources()
	{
		for (KLAYGE_AUTO(iter, loaded_res_.begin()); iter != loaded_res_.end();)
		{
			if (!iter->second.lock())
			{
				iter = loaded_res_.erase(iter);
			}
			else
			{
				++ iter;
			}
		}
	}

	void ResLoader::Update()
	{
		for (KLAYGE_AUTO(iter, loading_res_.begin()); iter != loading_res_.end();)
		{
			if (*(iter->second))
			{
				iter = loading_res_.erase(iter);
			}
			else
			{
				++ iter;
			}
		}
	}

	void ResLoader::LoadingThreadFunc()
	{
		while (!quit_)
		{
			std::pair<ResLoadingDescPtr, shared_ptr<volatile bool> > res_pair;
			while (loading_res_queue_.pop(res_pair))
			{
				res_pair.first->SubThreadStage();
				*res_pair.second = true;
			}

			Sleep(10);
		}
	}


	ResLoader::ASyncFunctor::ASyncFunctor(shared_ptr<void> const & res)
		: res_(res)
	{
	}

	shared_ptr<void> ResLoader::ASyncFunctor::operator()(ResLoadingDescPtr const & res_desc,
		shared_ptr<volatile bool> const & is_done)
	{
		if (!res_)
		{
			if (*is_done)
			{
				ResLoader& rl = ResLoader::Instance();
				shared_ptr<void> loaded_res = rl.FindMatchLoadedResource(res_desc);
				if (!loaded_res)
				{
					res_ = res_desc->MainThreadStage();
					rl.AddLoadedResource(res_desc, res_);
				}
				else
				{
					if (res_desc->StateLess())
					{
						res_ = loaded_res;
					}
					else
					{
						res_ = res_desc->CloneResourceFrom(loaded_res);
						if (res_ != loaded_res)
						{
							rl.AddLoadedResource(res_desc, res_);
						}
					}
				}
			}
		}
		return res_;
	}

	shared_ptr<void> ResLoader::ASyncFunctor::operator()()
	{
		return res_;
	}
}
