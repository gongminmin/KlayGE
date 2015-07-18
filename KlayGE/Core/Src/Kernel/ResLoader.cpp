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

#include <fstream>
#include <sstream>
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

#if defined KLAYGE_PLATFORM_WINDOWS
#include <windows.h>
#elif defined KLAYGE_PLATFORM_LINUX
#elif defined KLAYGE_PLATFORM_ANDROID
#include <android/asset_manager.h>
#include <KFL/CustomizedStreamBuf.hpp>
#elif defined KLAYGE_PLATFORM_DARWIN
#include <mach-o/dyld.h>
#elif defined KLAYGE_PLATFORM_IOS
#include <CoreFoundation/CoreFoundation.h>
#endif

#include <KlayGE/ResLoader.hpp>

namespace
{
	std::mutex singleton_mutex;

#ifdef KLAYGE_PLATFORM_ANDROID
	class AAssetStreamBuf : public KlayGE::MemStreamBuf
	{
	public:
		explicit AAssetStreamBuf(AAsset* asset)
			: MemStreamBuf(AAsset_getBuffer(asset), 
					static_cast<uint8_t const *>(AAsset_getBuffer(asset)) + AAsset_getLength(asset)),
				asset_(asset)
		{
			BOOST_ASSERT(asset_ != nullptr);
		}

		~AAssetStreamBuf()
		{
			AAsset_close(asset_);
		}

	private:
		AAsset* asset_;
	};
#endif
}

namespace KlayGE
{
	std::shared_ptr<ResLoader> ResLoader::res_loader_instance_;

	ResLoader::ResLoader()
		: quit_(false)
	{
#if defined KLAYGE_PLATFORM_WINDOWS
#if defined KLAYGE_PLATFORM_WINDOWS_DESKTOP
		char buf[MAX_PATH];
		::GetModuleFileNameA(nullptr, buf, sizeof(buf));
		exe_path_ = buf;
		exe_path_ = exe_path_.substr(0, exe_path_.rfind("\\"));
#else
		Windows::ApplicationModel::Package^ package = Windows::ApplicationModel::Package::Current;
		Windows::Storage::StorageFolder^ installed_loc = package->InstalledLocation;
		Convert(exe_path_, installed_loc->Path->Data());
#endif
#elif defined KLAYGE_PLATFORM_LINUX || defined KLAYGE_PLATFORM_ANDROID
		{
			FILE* fp = fopen("/proc/self/maps", "r");
			if (fp != nullptr)
			{
				while (!feof(fp))
				{
					char line[1024];
					unsigned long start, end;
					if (!fgets(line, sizeof(line), fp))
					{
						continue;
					}
					if (!strstr(line, " r-xp ") || !strchr(line, '/'))
					{
						continue;
					}

					void const * symbol = "";
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
#elif defined KLAYGE_PLATFORM_DARWIN
		uint32_t size = 0;
		_NSGetExecutablePath(nullptr, &size);
		std::vector<char> buffer(size + 1, '\0');
		_NSGetExecutablePath(buffer.data(), &size);
		exe_path_ = buffer.data();
		exe_path_ = exe_path_.substr(0, exe_path_.find_last_of("/") + 1);
#endif

		paths_.push_back("");

#if defined KLAYGE_PLATFORM_WINDOWS_RUNTIME
		this->AddPath("Assets/");
#else
		this->AddPath("");
#if defined KLAYGE_PLATFORM_WINDOWS_DESKTOP
		::GetCurrentDirectoryA(sizeof(buf), buf);
		char* colon = std::find(buf, buf + sizeof(buf), ':');
		BOOST_ASSERT(colon != buf + sizeof(buf));
		colon[1] = '/';
		colon[2] = '\0';
		this->AddPath(buf);
#endif
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

		loading_thread_ = MakeSharedPtr<joiner<void>>(Context::Instance().ThreadPool()(
				std::bind(&ResLoader::LoadingThreadFunc, this)));
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
			std::lock_guard<std::mutex> lock(singleton_mutex);
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

	void ResLoader::Suspend()
	{
		// TODO
	}

	void ResLoader::Resume()
	{
		// TODO
	}

	std::string ResLoader::AbsPath(std::string const & path)
	{
		using namespace std::experimental;

		filesystem::path new_path(path);
#ifdef KLAYGE_TS_LIBRARY_FILESYSTEM_V2_SUPPORT
		if (!new_path.is_complete())
#else
		if (!new_path.is_absolute())
#endif
		{
			filesystem::path full_path = filesystem::path(exe_path_) / new_path;
			if (!filesystem::exists(full_path))
			{
#ifndef KLAYGE_PLATFORM_ANDROID
#ifdef KLAYGE_TS_LIBRARY_FILESYSTEM_V2_SUPPORT
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
		std::string ret = new_path.string();
#if defined KLAYGE_PLATFORM_WINDOWS
		std::replace(ret.begin(), ret.end(), '\\', '/');
#endif
		return ret;
	}

	std::string ResLoader::RealPath(std::string const & path)
	{
		std::string abs_path = this->AbsPath(path);
		if (!abs_path.empty() && (abs_path[abs_path.length() - 1] != '/'))
		{
			abs_path.push_back('/');
		}

		return abs_path;
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
			auto iter = std::find(paths_.begin(), paths_.end(), real_path);
			if (iter != paths_.end())
			{
				paths_.erase(iter);
			}
		}
	}

	std::string ResLoader::Locate(std::string const & name)
	{
#if defined(KLAYGE_PLATFORM_ANDROID)
		android_app* state = Context::Instance().AppState();
		AAssetManager* am = state->activity->assetManager;
		AAsset* asset = AAssetManager_open(am, name.c_str(), AASSET_MODE_UNKNOWN);
		if (asset != nullptr)
		{
			AAsset_close(asset);
			return name;
		}
#elif defined(KLAYGE_PLATFORM_IOS)
		std::string::size_type found = name.find_last_of(".");
		if (found != std::string::npos)
		{
			std::string::size_type found2 = name.find_last_of("/");
			CFBundleRef main_bundle = CFBundleGetMainBundle();
			CFStringRef file_name = CFStringCreateWithCString(kCFAllocatorDefault,
				name.substr(found2 + 1, found - found2 - 1).c_str(), kCFStringEncodingASCII);
			CFStringRef file_ext = CFStringCreateWithCString(kCFAllocatorDefault,
				name.substr(found + 1).c_str(), kCFStringEncodingASCII);
			CFURLRef file_url = CFBundleCopyResourceURL(main_bundle, file_name, file_ext, NULL);
			CFRelease(file_name);
			CFRelease(file_ext);
			if (file_url != nullptr)
			{
				CFStringRef file_path = CFURLCopyFileSystemPath(file_url, kCFURLPOSIXPathStyle);

				char const * path = CFStringGetCStringPtr(file_path, CFStringGetSystemEncoding());
				std::string const res_name(path);

				CFRelease(file_url);
				CFRelease(file_path);

				return res_name;
			}
		}
#else
		using namespace std::experimental;

		for (auto const & path : paths_)
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

#ifdef KLAYGE_TS_LIBRARY_FILESYSTEM_V3_SUPPORT
						uint64_t timestamp = filesystem::last_write_time(pkt_path).time_since_epoch().count();
#else
						uint64_t timestamp = filesystem::last_write_time(pkt_path);
#endif
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

#if defined KLAYGE_PLATFORM_WINDOWS_RUNTIME
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
#endif
#endif

		return "";
	}

	ResIdentifierPtr ResLoader::Open(std::string const & name)
	{
#if defined(KLAYGE_PLATFORM_ANDROID)
		android_app* state = Context::Instance().AppState();
		AAssetManager* am = state->activity->assetManager;
		AAsset* asset = AAssetManager_open(am, name.c_str(), AASSET_MODE_UNKNOWN);
		if (asset != nullptr)
		{
			std::shared_ptr<AAssetStreamBuf> asb = MakeSharedPtr<AAssetStreamBuf>(asset);
			std::shared_ptr<std::istream> asset_file = MakeSharedPtr<std::istream>(asb.get());
			return MakeSharedPtr<ResIdentifier>(name, 0, asset_file, asb);
		}
#elif defined(KLAYGE_PLATFORM_IOS)
		std::string::size_type found = name.find_last_of(".");
		if (found != std::string::npos)
		{
			std::string::size_type found2 = name.find_last_of("/");
			CFBundleRef main_bundle = CFBundleGetMainBundle();
			CFStringRef file_name = CFStringCreateWithCString(kCFAllocatorDefault,
				name.substr(found2 + 1, found - found2 - 1).c_str(), kCFStringEncodingASCII);
			CFStringRef file_ext = CFStringCreateWithCString(kCFAllocatorDefault,
				name.substr(found + 1).c_str(), kCFStringEncodingASCII);
			CFURLRef file_url = CFBundleCopyResourceURL(main_bundle, file_name, file_ext, NULL);
			CFRelease(file_name);
			CFRelease(file_ext);
			if (file_url != nullptr)
			{
				CFStringRef file_path = CFURLCopyFileSystemPath(file_url, kCFURLPOSIXPathStyle);
				
				char const * path = CFStringGetCStringPtr(file_path, CFStringGetSystemEncoding());
				std::string const res_name(path);
				
				CFRelease(file_url);
				CFRelease(file_path);
				
				filesystem::path res_path(res_name);
#ifdef KLAYGE_TS_LIBRARY_FILESYSTEM_V3_SUPPORT
				uint64_t timestamp = filesystem::last_write_time(res_path).time_since_epoch().count();
#else
				uint64_t timestamp = filesystem::last_write_time(res_path);
#endif
				
				return MakeSharedPtr<ResIdentifier>(name, timestamp,
					MakeSharedPtr<std::ifstream>(res_name.c_str(), std::ios_base::binary));
			}
		}
#else
		using namespace std::experimental;

		for (auto const & path : paths_)
		{
			std::string const res_name(path + name);

			filesystem::path res_path(res_name);
			if (filesystem::exists(res_path))
			{
#ifdef KLAYGE_TS_LIBRARY_FILESYSTEM_V3_SUPPORT
				uint64_t timestamp = filesystem::last_write_time(res_path).time_since_epoch().count();
#else
				uint64_t timestamp = filesystem::last_write_time(res_path);
#endif
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

#ifdef KLAYGE_TS_LIBRARY_FILESYSTEM_V3_SUPPORT
						uint64_t timestamp = filesystem::last_write_time(pkt_path).time_since_epoch().count();
#else
						uint64_t timestamp = filesystem::last_write_time(pkt_path);
#endif
						ResIdentifierPtr pkt_file = MakeSharedPtr<ResIdentifier>(name, timestamp,
							MakeSharedPtr<std::ifstream>(pkt_name.c_str(), std::ios_base::binary));
						if (*pkt_file)
						{
							std::shared_ptr<std::iostream> packet_file = MakeSharedPtr<std::stringstream>();
							Extract7z(pkt_file, password, file_name, packet_file);
							return MakeSharedPtr<ResIdentifier>(name, timestamp, packet_file);
						}
					}
				}
			}
		}

#if defined(KLAYGE_PLATFORM_WINDOWS_RUNTIME)
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
#endif
#endif

		return ResIdentifierPtr();
	}

	std::shared_ptr<void> ResLoader::SyncQuery(ResLoadingDescPtr const & res_desc)
	{
		this->RemoveUnrefResources();

		std::shared_ptr<void> loaded_res = this->FindMatchLoadedResource(res_desc);
		std::shared_ptr<void> res;
		if (loaded_res)
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
		else
		{
			if (res_desc->HasSubThreadStage())
			{
				res_desc->SubThreadStage();
			}

			res = res_desc->MainThreadStage();
			this->AddLoadedResource(res_desc, res);
		}

		return res;
	}

	std::function<std::shared_ptr<void>()> ResLoader::ASyncQuery(ResLoadingDescPtr const & res_desc)
	{
		this->RemoveUnrefResources();

		std::shared_ptr<void> loaded_res = this->FindMatchLoadedResource(res_desc);
		if (loaded_res)
		{
			std::shared_ptr<void> res;
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
			return ResLoader::ASyncReuseFunctor(res);
		}
		else
		{
			std::shared_ptr<volatile bool> async_is_done;
			bool found = false;
			{
				std::lock_guard<std::mutex> lock(loading_mutex_);

				for (auto const & lrq : loading_res_)
				{
					if (lrq.first->Match(*res_desc))
					{
						res_desc->CopyDataFrom(*lrq.first);
						async_is_done = lrq.second;
						found = true;
						break;
					}
				}
			}

			if (found)
			{
				return ResLoader::ASyncRecreateFunctor(loaded_res, res_desc, async_is_done);
			}
			else
			{
				if (res_desc->HasSubThreadStage())
				{
					{
						std::lock_guard<std::mutex> lock(loading_mutex_);

						async_is_done = MakeSharedPtr<bool>(false);
						loading_res_.push_back(std::make_pair(res_desc, async_is_done));
					}
					loading_res_queue_.push(std::make_pair(res_desc, async_is_done));
					return ResLoader::ASyncRecreateFunctor(loaded_res, res_desc, async_is_done);
				}
				else
				{
					std::shared_ptr<void> res = res_desc->MainThreadStage();
					this->AddLoadedResource(res_desc, res);
					return ResLoader::ASyncReuseFunctor(res);
				}
			}
		}
	}

	void ResLoader::Unload(std::shared_ptr<void> const & res)
	{
		std::lock_guard<std::mutex> lock(loading_mutex_);

		for (auto iter = loaded_res_.begin(); iter != loaded_res_.end(); ++ iter)
		{
			if (res == iter->second.lock())
			{
				loaded_res_.erase(iter);
				break;
			}
		}
	}

	void ResLoader::AddLoadedResource(ResLoadingDescPtr const & res_desc, std::shared_ptr<void> const & res)
	{
		std::lock_guard<std::mutex> lock(loading_mutex_);

		bool found = false;
		for (auto& c_desc : loaded_res_)
		{
			if (c_desc.first == res_desc)
			{
				c_desc.second = std::weak_ptr<void>(res);
				found = true;
				break;
			}
		}
		if (!found)
		{
			loaded_res_.push_back(std::make_pair(res_desc, std::weak_ptr<void>(res)));
		}
	}

	std::shared_ptr<void> ResLoader::FindMatchLoadedResource(ResLoadingDescPtr const & res_desc)
	{
		std::lock_guard<std::mutex> lock(loading_mutex_);

		std::shared_ptr<void> loaded_res;
		for (auto const & lr : loaded_res_)
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
		std::lock_guard<std::mutex> lock(loading_mutex_);

		for (auto iter = loaded_res_.begin(); iter != loaded_res_.end();)
		{
			if (iter->second.lock())
			{
				++ iter;
			}
			else
			{
				iter = loaded_res_.erase(iter);
			}
		}
	}

	void ResLoader::Update()
	{
		std::lock_guard<std::mutex> lock(loading_mutex_);

		for (auto iter = loading_res_.begin(); iter != loading_res_.end();)
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
			std::pair<ResLoadingDescPtr, std::shared_ptr<volatile bool>> res_pair;
			while (loading_res_queue_.pop(res_pair))
			{
				res_pair.first->SubThreadStage();
				*res_pair.second = true;
			}

			Sleep(10);
		}
	}


	ResLoader::ASyncRecreateFunctor::ASyncRecreateFunctor(std::shared_ptr<void> const & res,
				ResLoadingDescPtr const & res_desc, std::shared_ptr<volatile bool> const & is_done)
		: res_(res), res_desc_(res_desc), is_done_(is_done)
	{
	}

	std::shared_ptr<void> ResLoader::ASyncRecreateFunctor::operator()()
	{
		if (!res_)
		{
			if (*is_done_)
			{
				ResLoader& rl = ResLoader::Instance();
				std::shared_ptr<void> loaded_res = rl.FindMatchLoadedResource(res_desc_);
				if (loaded_res)
				{
					if (res_desc_->StateLess())
					{
						res_ = loaded_res;
					}
					else
					{
						res_ = res_desc_->CloneResourceFrom(loaded_res);
						if (res_ != loaded_res)
						{
							rl.AddLoadedResource(res_desc_, res_);
						}
					}
				}
				else
				{
					res_ = res_desc_->MainThreadStage();
					rl.AddLoadedResource(res_desc_, res_);
				}
			}
		}
		return res_;
	}

	ResLoader::ASyncReuseFunctor::ASyncReuseFunctor(std::shared_ptr<void> const & res)
		: res_(res)
	{
	}

	std::shared_ptr<void> ResLoader::ASyncReuseFunctor::operator()()
	{
		return res_;
	}
}
