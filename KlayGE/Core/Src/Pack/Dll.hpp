// Dll.hpp
// KlayGE 打包系统DLL载入器 头文件
// Ver 3.6.0
// 版权所有(C) 龚敏敏, 2007
// Homepage: http://klayge.sourceforge.net
//
// 3.6.0
// 初次建立 (2007.5.24)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#ifndef _DLL_HPP
#define _DLL_HPP

#include <string>

namespace KlayGE
{
	class DllLoader
	{
	public:
		DllLoader();
		~DllLoader();

		void Load(std::string const & dll_name);
		void Free();

		void* GetProcAddress(std::string const & proc_name);

	private:
		void* dll_handle_;
	};
}

#endif		// _DLL_HPP
