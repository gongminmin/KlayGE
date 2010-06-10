// DllLoader.cpp
// KlayGE DLL载入器 实现文件
// Ver 3.8.0
// 版权所有(C) 龚敏敏, 2007-2008
// Homepage: http://www.klayge.org
//
// 3.8.0
// 移到Kernel (2008.10.14)
//
// 3.6.0
// 初次建立 (2007.5.24)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KlayGE/ResLoader.hpp>

#ifdef KLAYGE_PLATFORM_WINDOWS
#include <windows.h>
#else
#include <dlfcn.h>
#endif

#include <KlayGE/DllLoader.hpp>

namespace KlayGE
{
	DllLoader::DllLoader()
		: dll_handle_(NULL)
	{
	}

	DllLoader::~DllLoader()
	{
		this->Free();
	}

	void DllLoader::Load(std::string const & dll_name)
	{
		std::string name = ResLoader::Instance().Locate(dll_name);

#ifdef KLAYGE_PLATFORM_WINDOWS
		dll_handle_ = static_cast<void*>(::LoadLibraryA(name.c_str()));
#else
		dll_handle_ = ::dlopen(name.c_str(), RTLD_LAZY);
#endif
	}

	void DllLoader::Free()
	{
		if (dll_handle_)
		{
#ifdef KLAYGE_PLATFORM_WINDOWS
			::FreeLibrary(static_cast<HMODULE>(dll_handle_));
#else
			::dlclose(dll_handle_);
#endif
		}
	}

	void* DllLoader::GetProcAddress(std::string const & proc_name)
	{
#ifdef KLAYGE_PLATFORM_WINDOWS
		return reinterpret_cast<void*>(::GetProcAddress(static_cast<HMODULE>(dll_handle_), proc_name.c_str()));
#else
		return ::dlsym(dll_handle_, proc_name.c_str());
#endif
	}
}
