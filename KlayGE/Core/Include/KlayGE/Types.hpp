// Types.hpp
// KlayGE 类型定义头文件
// Ver 1.3.8.3
// 版权所有(C) 龚敏敏, 2003
// Homepage: http://www.enginedev.com
//
// 1.3.8.3
// 初次建立
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#ifndef _TYPES_HPP
#define _TYPES_HPP

#include <string>
#include <KlayGE/alloc.hpp>

namespace KlayGE
{
	#ifndef _WCHAR_T_DEFINED
		typedef unsigned short		wchar_t;
		#define _WCHAR_T_DEFINED
	#endif		// _WCHAR_T_DEFINED

	typedef unsigned int		UINT;
	typedef unsigned __int64	U64;
	typedef unsigned long		U32;
	typedef unsigned short		U16;
	typedef unsigned char		U8;

	typedef __int64				S64;
	typedef int					S32;
	typedef short				S16;
	typedef char				S8;

	typedef std::basic_string<char, std::char_traits<char>, alloc<char> > String;
	typedef std::basic_string<wchar_t, std::char_traits<wchar_t>, alloc<wchar_t> > WString;

	typedef U32 FourCC;

	// 产生FourCC
	template <char ch0, char ch1, char ch2, char ch3>
	struct MakeFourCC
	{
		enum { value = (ch0 << 0) + (ch1 << 8) + (ch2 << 16) + (ch3 << 24) };
	};
}

#endif		// _TYPES_HPP