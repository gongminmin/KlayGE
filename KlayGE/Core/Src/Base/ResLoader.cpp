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
#include <KFL/Hash.hpp>
#include <KFL/Util.hpp>
#include <KlayGE/Package.hpp>
#include <KFL/CXX17/filesystem.hpp>

#if defined KLAYGE_PLATFORM_LINUX
#include <cstring>
#endif
#include <fstream>
#include <sstream>

#if defined KLAYGE_PLATFORM_WINDOWS_DESKTOP
#include <windows.h>
#elif defined KLAYGE_PLATFORM_WINDOWS_STORE
#include <winrt/Windows.ApplicationModel.Core.h>
#include <winrt/Windows.Storage.h>

namespace uwp
{
	using winrt::hstring;

	using namespace winrt::Windows::Foundation;
	using namespace winrt::Windows::ApplicationModel;
	using namespace winrt::Windows::Storage;
}

#include <KFL/ErrorHandling.hpp>
#elif defined KLAYGE_PLATFORM_LINUX
#elif defined KLAYGE_PLATFORM_ANDROID
#include <android_native_app_glue.h>
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
	class AAssetStreamBuf : public KlayGE::MemInputStreamBuf
	{
	public:
		explicit AAssetStreamBuf(AAsset* asset)
			: MemInputStreamBuf(AAsset_getBuffer(asset), AAsset_getLength(asset)),
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
	std::unique_ptr<ResLoader> ResLoader::res_loader_instance_;

	ResLoadingDesc::~ResLoadingDesc() noexcept = default;

	ResLoader::ResLoader()
	{
#if defined KLAYGE_PLATFORM_WINDOWS
#if defined KLAYGE_PLATFORM_WINDOWS_DESKTOP
		char buf[MAX_PATH];
		::GetModuleFileNameA(nullptr, buf, sizeof(buf));
		exe_path_ = buf;
		exe_path_ = exe_path_.substr(0, exe_path_.rfind("\\"));
		local_path_ = exe_path_ + "/";
#else
		auto package = uwp::Package::Current();
		auto installed_loc_storage_item = package.InstalledLocation().as<uwp::IStorageItem>();
		auto const installed_loc_folder_name = installed_loc_storage_item.Path();
		Convert(exe_path_, installed_loc_folder_name);

		auto app_data = uwp::ApplicationData::Current();
		auto local_folder_storage_item = app_data.LocalFolder().as<uwp::IStorageItem>();
		auto const local_folder_name = local_folder_storage_item.Path();
		Convert(local_path_, local_folder_name);
		local_path_ += "\\";
#endif
#elif defined KLAYGE_PLATFORM_LINUX
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

			local_path_ = exe_path_;
		}
#elif defined KLAYGE_PLATFORM_DARWIN
		uint32_t size = 0;
		_NSGetExecutablePath(nullptr, &size);
		auto buffer = MakeUniquePtr<char[]>(size + 1);
		_NSGetExecutablePath(buffer.get(), &size);
		buffer[size] = '\0';
		exe_path_ = buffer.get();
		exe_path_ = exe_path_.substr(0, exe_path_.find_last_of("/") + 1);
		local_path_ = exe_path_;
#endif

		paths_.push_back(std::make_tuple(CT_HASH(""), 0, "", PackagePtr()));

#if defined KLAYGE_PLATFORM_WINDOWS_STORE
		this->AddPath("Assets/");
		this->AddPath(local_path_);
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

#if defined(KLAYGE_PLATFORM_WINDOWS_DESKTOP) || defined(KLAYGE_PLATFORM_LINUX) || defined(KLAYGE_PLATFORM_DARWIN)
		this->AddPath("..");
		this->AddPath("../../media/RenderFX");
		this->AddPath("../../media/Models");
#if KLAYGE_IS_DEV_PLATFORM
		this->AddPath("../../media/PlatConf");
#endif
		this->AddPath("../../media/Textures/2D");
		this->AddPath("../../media/Textures/3D");
		this->AddPath("../../media/Textures/Cube");
		this->AddPath("../../media/Textures/Juda");
		this->AddPath("../../media/Fonts");
		this->AddPath("../../media/PostProcessors");
#endif
#endif

		loading_thread_ = MakeUniquePtr<joiner<void>>(Context::Instance().ThreadPool()(
			[this] { this->LoadingThreadFunc(); }));
	}

	ResLoader::~ResLoader()
	{
		quit_ = true;

		{
			std::unique_lock<std::mutex> lock(loading_res_queue_mutex_, std::try_to_lock);
			non_empty_loading_res_queue_ = true;
			loading_res_queue_cv_.notify_one();
		}

		(*loading_thread_)();
	}

	ResLoader& ResLoader::Instance()
	{
		if (!res_loader_instance_)
		{
			std::lock_guard<std::mutex> lock(singleton_mutex);
			if (!res_loader_instance_)
			{
				res_loader_instance_ = MakeUniquePtr<ResLoader>();
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

	std::string ResLoader::AbsPath(std::string_view path)
	{
		std::string path_str(path);
		std::filesystem::path new_path(path_str);
		if (!new_path.is_absolute())
		{
			std::filesystem::path full_path = std::filesystem::path(exe_path_) / new_path;
			std::error_code ec;
			if (!std::filesystem::exists(full_path, ec))
			{
#ifndef KLAYGE_PLATFORM_ANDROID
				try
				{
					full_path = std::filesystem::current_path() / new_path;
				}
				catch (...)
				{
					full_path = new_path;
				}
				if (!std::filesystem::exists(full_path, ec))
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

	std::string ResLoader::RealPath(std::string_view path)
	{
		std::string package_path;
		std::string password;
		std::string path_in_package;
		return this->RealPath(path, package_path, password, path_in_package);
	}

	std::string ResLoader::RealPath(std::string_view path,
		std::string& package_path, std::string& password, std::string& path_in_package)
	{
		package_path = "";
		password = "";
		path_in_package = "";

		std::string abs_path = this->AbsPath(path);
		if (abs_path.empty())
		{
			this->DecomposePackageName(path, package_path, password, path_in_package);
			if (!package_path.empty())
			{
				std::string real_package_path = this->RealPath(package_path);
				real_package_path.pop_back();

				package_path = real_package_path;

				abs_path = real_package_path;
				if (!password.empty())
				{
					abs_path += "|" + password;
				}
				if (!path_in_package.empty())
				{
					abs_path += "/" + path_in_package;
				}
				if (abs_path.back() != '/')
				{
					abs_path.push_back('/');
				}
			}
		}
		else
		{
			this->DecomposePackageName(abs_path, package_path, password, path_in_package);

			if (abs_path.back() != '/')
			{
				abs_path.push_back('/');
			}
		}

		return abs_path;
	}

	void ResLoader::DecomposePackageName(std::string_view path,
		std::string& package_path, std::string& password, std::string& path_in_package)
	{
		package_path = "";
		password = "";
		path_in_package = "";

		size_t start_offset = 0;
		for (;;)
		{
			auto const pkt_offset = path.find(".7z", start_offset);
			if (pkt_offset != std::string_view::npos)
			{
				package_path = std::string(path.substr(0, pkt_offset + 3));
				std::filesystem::path pkt_path(package_path);
				std::error_code ec;
				if (std::filesystem::exists(pkt_path, ec)
					&& (std::filesystem::is_regular_file(pkt_path) || std::filesystem::is_symlink(pkt_path)))
				{
					auto const next_slash_offset = path.find('/', pkt_offset + 3);
					if ((path.size() > pkt_offset + 3) && (path[pkt_offset + 3] == '|'))
					{
						auto const password_start_offset = pkt_offset + 4;
						if (next_slash_offset != std::string_view::npos)
						{
							password = std::string(path.substr(password_start_offset, next_slash_offset - password_start_offset));
						}
						else
						{
							password = std::string(path.substr(password_start_offset));
						}
					}
					if (next_slash_offset != std::string_view::npos)
					{
						path_in_package = std::string(path.substr(next_slash_offset + 1));
					}
					break;
				}
				else
				{
					start_offset = pkt_offset + 3;
				}
			}
			else
			{
				break;
			}
		}
	}

	void ResLoader::AddPath(std::string_view phy_path)
	{
		this->Mount("", phy_path);
	}

	void ResLoader::DelPath(std::string_view phy_path)
	{
		this->Unmount("", phy_path);
	}

	bool ResLoader::IsInPath(std::string_view phy_path)
	{
		std::string_view virtual_path = "";

		std::lock_guard<std::mutex> lock(paths_mutex_);

		std::string real_path = this->RealPath(phy_path);
		if (!real_path.empty())
		{
			std::string virtual_path_str(virtual_path);
			if (!virtual_path.empty() && (virtual_path.back() != '/'))
			{
				virtual_path_str.push_back('/');
			}
			uint64_t const virtual_path_hash = HashRange(virtual_path_str.begin(), virtual_path_str.end());

			bool found = false;
			for (auto const & path : paths_)
			{
				if ((std::get<0>(path) == virtual_path_hash) && (std::get<2>(path) == real_path))
				{
					found = true;
					break;
				}
			}

			return found;
		}
		else
		{
			return false;
		}
	}

	void ResLoader::Mount(std::string_view virtual_path, std::string_view phy_path)
	{
		std::lock_guard<std::mutex> lock(paths_mutex_);

		std::string package_path;
		std::string password;
		std::string path_in_package;
		std::string real_path = this->RealPath(phy_path,
			package_path, password, path_in_package);
		if (!real_path.empty())
		{
			std::string virtual_path_str(virtual_path);
			if (!virtual_path.empty() && (virtual_path.back() != '/'))
			{
				virtual_path_str.push_back('/');
			}
			uint64_t const virtual_path_hash = HashRange(virtual_path_str.begin(), virtual_path_str.end());

			bool found = false;
			for (auto const & path : paths_)
			{
				if ((std::get<0>(path) == virtual_path_hash) && (std::get<2>(path) == real_path))
				{
					found = true;
					break;
				}
			}

			if (!found)
			{
				PackagePtr package;
				if (!package_path.empty())
				{
					for (auto const & path : paths_)
					{
						auto const & p = std::get<3>(path);
						if (p && package_path == p->ArchiveStream()->ResName())
						{
							package = p;
							break;
						}
					}
					if (!package)
					{
						uint64_t const timestamp = std::filesystem::last_write_time(package_path).time_since_epoch().count();
						auto package_res = MakeSharedPtr<ResIdentifier>(
							package_path, timestamp, MakeSharedPtr<std::ifstream>(package_path.c_str(), std::ios_base::binary));

						package = MakeSharedPtr<Package>(package_res, password);
					}
				}

				paths_.push_back(std::make_tuple(virtual_path_hash, static_cast<uint32_t>(virtual_path_str.size()), real_path, package));
			}
		}
	}

	void ResLoader::Unmount(std::string_view virtual_path, std::string_view phy_path)
	{
		std::lock_guard<std::mutex> lock(paths_mutex_);

		std::string real_path = this->RealPath(phy_path);
		if (!real_path.empty())
		{
			std::string virtual_path_str(virtual_path);
			if (!virtual_path.empty() && (virtual_path.back() != '/'))
			{
				virtual_path_str.push_back('/');
			}
			uint64_t const virtual_path_hash = HashRange(virtual_path_str.begin(), virtual_path_str.end());

			for (auto iter = paths_.begin(); iter != paths_.end(); ++ iter)
			{
				if ((std::get<0>(*iter) == virtual_path_hash) && (std::get<2>(*iter) == real_path))
				{
					paths_.erase(iter);
					break;
				}
			}
		}
	}

	std::string ResLoader::Locate(std::string_view name)
	{
		if (name.empty())
		{
			return "";
		}

#if defined(KLAYGE_PLATFORM_ANDROID)
		AAsset* asset = this->LocateFileAndroid(name);
		if (asset != nullptr)
		{
			AAsset_close(asset);
			return std::string(name);
		}
#elif defined(KLAYGE_PLATFORM_IOS)
		return this->LocateFileIOS(name);
#else
		{
			std::lock_guard<std::mutex> lock(paths_mutex_);
			for (auto const & path : paths_)
			{
				if ((std::get<1>(path) != 0) || (HashRange(name.begin(), name.begin() + std::get<1>(path)) == std::get<0>(path)))
				{
					std::string res_name(std::get<2>(path) + std::string(name.substr(std::get<1>(path))));
#if defined KLAYGE_PLATFORM_WINDOWS
					std::replace(res_name.begin(), res_name.end(), '\\', '/');
#endif

					std::error_code ec;
					if (std::filesystem::exists(std::filesystem::path(res_name), ec))
					{
						return res_name;
					}
					else
					{
						std::string package_path;
						std::string password;
						std::string path_in_package;
						this->DecomposePackageName(res_name, package_path, password, path_in_package);
						auto const & package = std::get<3>(path);
						if (!package_path.empty() && package && (package_path == package->ArchiveStream()->ResName()))
						{
							if (package->Locate(path_in_package))
							{
								return res_name;
							}
						}
					}
				}

				if ((std::get<1>(path) == 0) && std::filesystem::path(name.begin(), name.end()).is_absolute())
				{
					break;
				}
			}
		}
#if defined KLAYGE_PLATFORM_WINDOWS_STORE
		std::string const & res_name = this->LocateFileWinRT(name);
		if (!res_name.empty())
		{
			return this->Locate(res_name);
		}
#endif
#endif

		return "";
	}

	ResIdentifierPtr ResLoader::Open(std::string_view name)
	{
		if (name.empty())
		{
			return ResIdentifierPtr();
		}

#if defined(KLAYGE_PLATFORM_ANDROID)
		AAsset* asset = this->LocateFileAndroid(name);
		if (asset != nullptr)
		{
			std::shared_ptr<AAssetStreamBuf> asb = MakeSharedPtr<AAssetStreamBuf>(asset);
			std::shared_ptr<std::istream> asset_file = MakeSharedPtr<std::istream>(asb.get());
			return MakeSharedPtr<ResIdentifier>(name, 0, asset_file, asb);
		}
#elif defined(KLAYGE_PLATFORM_IOS)
		std::string const & res_name = this->LocateFileIOS(name);
		if (!res_name.empty())
		{
			std::filesystem::path res_path(res_name);
			uint64_t const timestamp = std::filesystem::last_write_time(res_path).time_since_epoch().count();
			return MakeSharedPtr<ResIdentifier>(name, timestamp,
				MakeSharedPtr<std::ifstream>(res_name.c_str(), std::ios_base::binary));
		}
#else
		{
			std::lock_guard<std::mutex> lock(paths_mutex_);
			for (auto const & path : paths_)
			{
				if ((std::get<1>(path) != 0) || (HashRange(name.begin(), name.begin() + std::get<1>(path)) == std::get<0>(path)))
				{
					std::string res_name(std::get<2>(path) + std::string(name.substr(std::get<1>(path))));
#if defined KLAYGE_PLATFORM_WINDOWS
					std::replace(res_name.begin(), res_name.end(), '\\', '/');
#endif

					std::filesystem::path res_path(res_name);
					std::error_code ec;
					if (std::filesystem::exists(res_path, ec))
					{
						uint64_t const timestamp = std::filesystem::last_write_time(res_path).time_since_epoch().count();
						return MakeSharedPtr<ResIdentifier>(
							name, timestamp, MakeSharedPtr<std::ifstream>(res_name.c_str(), std::ios_base::binary));
					}
					else
					{
						std::string package_path;
						std::string password;
						std::string path_in_package;
						this->DecomposePackageName(res_name, package_path, password, path_in_package);
						auto const & package = std::get<3>(path);
						if (!package_path.empty() && package && (package_path == package->ArchiveStream()->ResName()))
						{
							auto res = package->Extract(path_in_package, name);
							if (res)
							{
								return res;
							}
						}
					}
				}

				if ((std::get<1>(path) == 0) && std::filesystem::path(name.begin(), name.end()).is_absolute())
				{
					break;
				}
			}
		}
#if defined(KLAYGE_PLATFORM_WINDOWS_STORE)
		std::string const & res_name = this->LocateFileWinRT(name);
		if (!res_name.empty())
		{
			return this->Open(res_name);
		}
#endif
#endif

		return ResIdentifierPtr();
	}

	uint64_t ResLoader::Timestamp(std::string_view name)
	{
		uint64_t timestamp = 0;
		auto res_path = this->Locate(name);
		if (!res_path.empty())
		{
#if !defined(KLAYGE_PLATFORM_ANDROID)
			timestamp = std::filesystem::last_write_time(res_path).time_since_epoch().count();
#endif
		}

		return timestamp;
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
			std::shared_ptr<volatile LoadingStatus> async_is_done;
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
				*async_is_done = LS_Complete;
			}
			else
			{
				res = res_desc->CreateResource();
			}

			if (res_desc->HasSubThreadStage())
			{
				res_desc->SubThreadStage();
			}

			res_desc->MainThreadStage();
			res = res_desc->Resource();
			this->AddLoadedResource(res_desc, res);
		}

		return res;
	}

	std::shared_ptr<void> ResLoader::ASyncQuery(ResLoadingDescPtr const & res_desc)
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
			std::shared_ptr<volatile LoadingStatus> async_is_done;
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
				res = res_desc->Resource();

				if (!res_desc->StateLess())
				{
					std::lock_guard<std::mutex> lock(loading_mutex_);
					loading_res_.emplace_back(res_desc, async_is_done);
				}
			}
			else
			{
				if (res_desc->HasSubThreadStage())
				{
					res = res_desc->CreateResource();

					async_is_done = MakeSharedPtr<LoadingStatus>(LS_Loading);

					{
						std::lock_guard<std::mutex> lock(loading_mutex_);
						loading_res_.emplace_back(res_desc, async_is_done);
					}
					{
						std::unique_lock<std::mutex> lock(loading_res_queue_mutex_, std::try_to_lock);
						loading_res_queue_.emplace_back(res_desc, async_is_done);
						non_empty_loading_res_queue_ = true;
						loading_res_queue_cv_.notify_one();
					}
				}
				else
				{
					res_desc->MainThreadStage();
					res = res_desc->Resource();
					this->AddLoadedResource(res_desc, res);
				}
			}
		}
		return res;
	}

	void ResLoader::Unload(std::shared_ptr<void> const & res)
	{
		std::lock_guard<std::mutex> lock(loaded_mutex_);

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
		std::lock_guard<std::mutex> lock(loaded_mutex_);

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
			loaded_res_.emplace_back(res_desc, std::weak_ptr<void>(res));
		}
	}

	std::shared_ptr<void> ResLoader::FindMatchLoadedResource(ResLoadingDescPtr const & res_desc)
	{
		std::lock_guard<std::mutex> lock(loaded_mutex_);

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
		std::lock_guard<std::mutex> lock(loaded_mutex_);

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
		std::vector<std::pair<ResLoadingDescPtr, std::shared_ptr<volatile LoadingStatus>>> tmp_loading_res;
		{
			std::lock_guard<std::mutex> lock(loading_mutex_);
			tmp_loading_res = loading_res_;
		}

		for (auto& lrq : tmp_loading_res)
		{
			if (LS_Complete == *lrq.second)
			{
				ResLoadingDescPtr const & res_desc = lrq.first;

				std::shared_ptr<void> res;
				std::shared_ptr<void> loaded_res = this->FindMatchLoadedResource(res_desc);
				if (loaded_res)
				{
					if (!res_desc->StateLess())
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
					res_desc->MainThreadStage();
					res = res_desc->Resource();
					this->AddLoadedResource(res_desc, res);
				}
			}
		}
		for (auto& lrq : tmp_loading_res)
		{
			if (LS_Complete == *lrq.second)
			{
				*lrq.second = LS_CanBeRemoved;
			}
		}

		{
			std::lock_guard<std::mutex> lock(loading_mutex_);
			for (auto iter = loading_res_.begin(); iter != loading_res_.end();)
			{
				if (LS_CanBeRemoved == *(iter->second))
				{
					iter = loading_res_.erase(iter);
				}
				else
				{
					++ iter;
				}
			}
		}
	}

	void ResLoader::LoadingThreadFunc()
	{
		while (!quit_)
		{
			std::vector<std::pair<ResLoadingDescPtr, std::shared_ptr<volatile LoadingStatus>>> loading_res_queue_copy;

			{
				std::unique_lock<std::mutex> lock(loading_res_queue_mutex_);
				loading_res_queue_cv_.wait(lock, [this] { return non_empty_loading_res_queue_; });

				loading_res_queue_copy.swap(loading_res_queue_);
				non_empty_loading_res_queue_ = false;
			}

			for (auto& res_pair : loading_res_queue_copy)
			{
				if (LS_Loading == *res_pair.second)
				{
					res_pair.first->SubThreadStage();
					*res_pair.second = LS_Complete;
				}
			}

			Sleep(10);
		}
	}

#if defined(KLAYGE_PLATFORM_ANDROID)
	AAsset* ResLoader::LocateFileAndroid(std::string_view name)
	{
		android_app* state = Context::Instance().AppState();
		AAssetManager* am = state->activity->assetManager;
		return AAssetManager_open(am, std::string(name).c_str(), AASSET_MODE_UNKNOWN);
	}
#elif defined(KLAYGE_PLATFORM_IOS)
	std::string ResLoader::LocateFileIOS(std::string_view name)
	{
		std::string res_name;
		std::string::size_type found = name.find_last_of(".");
		if (found != std::string::npos)
		{
			std::string::size_type found2 = name.find_last_of("/");
			CFBundleRef main_bundle = CFBundleGetMainBundle();
			CFStringRef file_name = CFStringCreateWithCString(kCFAllocatorDefault,
				std::string(name.substr(found2 + 1, found - found2 - 1)).c_str(), kCFStringEncodingASCII);
			CFStringRef file_ext = CFStringCreateWithCString(kCFAllocatorDefault,
				std::string(name.substr(found + 1)).c_str(), kCFStringEncodingASCII);
			CFURLRef file_url = CFBundleCopyResourceURL(main_bundle, file_name, file_ext, NULL);
			CFRelease(file_name);
			CFRelease(file_ext);
			if (file_url != nullptr)
			{
				CFStringRef file_path = CFURLCopyFileSystemPath(file_url, kCFURLPOSIXPathStyle);

				res_name = CFStringGetCStringPtr(file_path, CFStringGetSystemEncoding());

				CFRelease(file_url);
				CFRelease(file_path);
			}
		}
		return res_name;
	}
#elif defined(KLAYGE_PLATFORM_WINDOWS_STORE)
	std::string ResLoader::LocateFileWinRT(std::string_view name)
	{
		std::string res_name;
		std::string::size_type pos = name.rfind('/');
		if (std::string::npos == pos)
		{
			pos = name.rfind('\\');
		}
		if (pos != std::string::npos)
		{
			res_name = name.substr(pos + 1);
		}
		return res_name;
	}
#endif
}
