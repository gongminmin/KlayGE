// DllLoader.hpp
// KlayGE DLL载入器 头文件
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

#ifndef _DLLLOADER_HPP
#define _DLLLOADER_HPP

#pragma once

#include <string>

namespace KlayGE
{
	class KLAYGE_CORE_API DllLoader
	{
	public:
		DllLoader();
		~DllLoader();

		bool Load(std::string const & dll_name);
		void Free();

		void* GetProcAddress(std::string const & proc_name);

	private:
		void* dll_handle_;
	};
}

#endif		// _DLLLOADER_HPP
