/**
 * @file DllLoader.cpp
 * @author Minmin Gong
 *
 * @section DESCRIPTION
 *
 * This source file is part of KFL, a subproject of KlayGE
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

#include <KFL/KFL.hpp>
#include <KFL/ResIdentifier.hpp>

#ifdef KLAYGE_PLATFORM_WINDOWS
#include <windows.h>
#else
#include <dlfcn.h>
#endif

#include <KFL/DllLoader.hpp>

namespace KlayGE
{
	DllLoader::DllLoader()
		: dll_handle_(nullptr)
	{
	}

	DllLoader::~DllLoader()
	{
		this->Free();
	}

	bool DllLoader::Load(std::string const & dll_name)
	{
#ifdef KLAYGE_PLATFORM_WINDOWS
#ifdef KLAYGE_PLATFORM_WINDOWS_DESKTOP
		dll_handle_ = static_cast<void*>(::LoadLibraryExA(dll_name.c_str(), nullptr, 0));
#else
		std::wstring wname;
		Convert(wname, dll_name);
		dll_handle_ = static_cast<void*>(::LoadPackagedLibrary(wname.c_str(), 0));
#endif
#else
		dll_handle_ = ::dlopen(dll_name.c_str(), RTLD_LAZY);
#endif

		return (dll_handle_ != nullptr);
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
