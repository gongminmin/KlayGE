#ifndef _CONFIG_HPP
#define _CONFIG_HPP

#if !defined(__cplusplus)
	#error C++ compiler required.
#endif

#if defined(DEBUG) | defined(_DEBUG)
    #define KLAYGE_DEBUG
#endif

// 定义编译器
#if defined(__GNUC__)
	// GNU C++

	#define KLAYGE_COMPILER_GCC

	#if __GNUC__ >= 4
		#define KLAYGE_COMPILER_VERSION 4.0
	#elif __GNUC__ >= 3
		#define KLAYGE_COMPILER_VERSION 3.0
	#else
		#error Unknown compiler.
	#endif
#elif defined(_MSC_VER)
	// Microsoft Visual C++
	//
	// Must remain the last #elif since some other vendors (Metrowerks, for example)
	// also #define _MSC_VER

	#define KLAYGE_COMPILER_MSVC

	#if _MSC_VER >= 1400
		#define KLAYGE_COMPILER_VERSION 8.0
		#pragma warning(disable: 4819)
	#elif _MSC_VER >= 1310
		#define KLAYGE_COMPILER_VERSION 7.1
	#else
		#error Unknown compiler.
	#endif
#endif

// 定义操作平台
#if defined(_XBOX_VER)
	#if _XBOX_VER >= 200
		#define KLAYGE_PLATFORM_XBOX360
	#else
		#error Unknown platform.
	#endif
#elif defined(_WIN32) || defined(__WIN32__) || defined(WIN32)
	#define KLAYGE_PLATFORM_WINDOWS

	// 关闭windows.h的min/max
	#define NOMINMAX

	#if defined(__MINGW32__)
		#include <_mingw.h>
	#endif
#elif defined(__CYGWIN__)
	#define KLAYGE_PLATFORM_CYGWIN
#elif defined(linux) || defined(__linux) || defined(__linux__)
	#define KLAYGE_PLATFORM_LINUX
#else
	#error Unknown platform.
#endif

// 定义机器架构
#if defined(KLAYGE_COMPILER_MSVC)
	#if defined(_M_X64)
		#define KLAYGE_MACHINE_X64
	#elif defined(_M_IX86)
		#define KLAYGE_MACHINE_X86
	#else
		#error Unknown machine.
	#endif
#elif defined(KLAYGE_COMPILER_GCC)
	#if defined(__x86_64__)
		#define KLAYGE_MACHINE_X64
	#elif defined(__i386__)
		#define KLAYGE_MACHINE_X86
	#else
		#error Unknown machine.
	#endif
#endif

// 定义本地的endian方式
#if defined(KLAYGE_MACHINE_X86) || defined(KLAYGE_MACHINE_X64) || defined(KLAYGE_PLATFORM_WINDOWS)
	#define KLAYGE_LITTLE_ENDIAN
#else
	#define KLAYGE_BIG_ENDIAN
#endif

// 定义各种编译期选项
#define _SELECT1ST2ND_SUPPORT
#define _COPYIF_SUPPORT

#endif		// _CONFIG_HPP
