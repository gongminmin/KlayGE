// Types.hpp
// KlayGE 类型定义头文件
// Ver 2.6.0
// 版权所有(C) 龚敏敏, 2003-2004
// Homepage: http://www.klayge.org
//
// 2.2.0
// 用boost的类型定义代替直接定义 (2004.10.30)
//
// 1.3.8.3
// 初次建立
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#ifndef _TYPES_HPP
#define _TYPES_HPP

#pragma once

#include <boost/cstdint.hpp>

namespace KlayGE
{
#if KLAYGE_COMPILER == MSVC
	#ifndef _WCHAR_T_DEFINED
		typedef unsigned short		wchar_t;
		#define _WCHAR_T_DEFINED
	#endif		// _WCHAR_T_DEFINED
#endif

	using boost::uint64_t;
	using boost::uint32_t;
	using boost::uint16_t;
	using boost::uint8_t;

	using boost::int64_t;
	using boost::int32_t;
	using boost::int16_t;
	using boost::int8_t;

	typedef uint32_t FourCC;
}

#endif		// _TYPES_HPP
