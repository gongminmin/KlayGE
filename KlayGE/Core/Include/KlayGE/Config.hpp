#ifndef _CONFIG_HPP
#define _CONFIG_HPP

#if !defined(__cplusplus)
	#error C++ compiler required.
#endif

#if defined(DEBUG) | defined(_DEBUG)
    #define KLAYGE_DEBUG
#endif

// Defines supported compilers
#if defined(__GNUC__)
	// GNU C++

	#define KLAYGE_COMPILER_GCC

	#if __GNUC__ >= 4
		#if __GNUC_MINOR__ >= 3
			#define KLAYGE_COMPILER_VERSION 43
		#elif __GNUC_MINOR__ >= 2
			#define KLAYGE_COMPILER_VERSION 42
		#elif __GNUC_MINOR__ >= 1
			#define KLAYGE_COMPILER_VERSION 41
		#elif __GNUC_MINOR__ >= 0
			#define KLAYGE_COMPILER_VERSION 40
		#endif
	#else
		#error Unknown compiler.
	#endif
#elif defined(_MSC_VER)
	#define KLAYGE_COMPILER_MSVC
	#define KLAYGE_COMPILER_NAME vc

	#define KLAYGE_HAS_DECLSPEC

	#if _MSC_VER >= 1500
		#define KLAYGE_COMPILER_VERSION 90
		#pragma warning(disable: 4251 4275 4819)

		#ifndef _CRT_SECURE_NO_DEPRECATE
			#define _CRT_SECURE_NO_DEPRECATE
		#endif
		#ifndef _SCL_SECURE_NO_DEPRECATE
			#define _SCL_SECURE_NO_DEPRECATE
		#endif

		#ifndef KLAYGE_DEBUG
			#define _SECURE_SCL 0
		#endif
	#elif _MSC_VER >= 1400
		#define KLAYGE_COMPILER_VERSION 80
		#pragma warning(disable: 4251 4275 4819)

		#ifndef _CRT_SECURE_NO_DEPRECATE
			#define _CRT_SECURE_NO_DEPRECATE
		#endif
		#ifndef _SCL_SECURE_NO_DEPRECATE
			#define _SCL_SECURE_NO_DEPRECATE
		#endif

		#ifndef KLAYGE_DEBUG
			#define _SECURE_SCL 0
		#endif
	#else
		#error Unknown compiler.
	#endif
#endif

// Defines supported platforms
#if defined(_XBOX_VER)
	#if _XBOX_VER >= 200
		#define KLAYGE_PLATFORM_XBOX360
	#else
		#error Unknown platform.
	#endif
#elif defined(_WIN32) || defined(__WIN32__) || defined(WIN32)
	#define KLAYGE_PLATFORM_WINDOWS

	#if !defined(__GNUC__) && !defined(KLAYGE_HAS_DECLSPEC)
		#define KLAYGE_HAS_DECLSPEC
	#endif

	#if defined(_WIN64)
		#define KLAYGE_PLATFORM_WIN64
	#else
		#define KLAYGE_PLATFORM_WIN32
	#endif

	// Forces all boost's libraries to be linked as dll
	#ifndef BOOST_ALL_DYN_LINK
		#define BOOST_ALL_DYN_LINK
	#endif

	// Shut min/max in windows.h
	#ifndef NOMINMAX
		#define NOMINMAX
	#endif

	#if defined(__MINGW32__)
		#define KLAYGE_COMPILER_NAME mgw
		#include <_mingw.h>
	#endif
#elif defined(__CYGWIN__)
	#define KLAYGE_PLATFORM_CYGWIN
	#define KLAYGE_COMPILER_NAME cyg
#elif defined(linux) || defined(__linux) || defined(__linux__)
	#define KLAYGE_PLATFORM_LINUX
	#define KLAYGE_COMPILER_NAME gcc
#else
	#error Unknown platform.
#endif

// Defines supported CPUs
#if defined(KLAYGE_COMPILER_MSVC)
	#if defined(KLAYGE_PLATFORM_XBOX360)
		#define KLAYGE_CPU_PPC
		#define KLAYGE_COMPILER_TARGET ppc
	#elif defined(_M_X64)
		#define KLAYGE_CPU_X64
		#define KLAYGE_COMPILER_TARGET x64
	#elif defined(_M_IX86)
		#define KLAYGE_CPU_X86
		#define KLAYGE_COMPILER_TARGET x86
	#else
		#error Unknown CPU type.
	#endif
#elif defined(KLAYGE_COMPILER_GCC)
	#if defined(__x86_64__)
		#define KLAYGE_CPU_X64
		#define KLAYGE_COMPILER_TARGET x64
	#elif defined(__i386__)
		#define KLAYGE_CPU_X86
		#define KLAYGE_COMPILER_TARGET x86
	#else
		#error Unknown CPU type.
	#endif
#endif

// Defines the native endian
#if defined(KLAYGE_CPU_PPC)
	#define KLAYGE_BIG_ENDIAN
#elif defined(KLAYGE_CPU_X86) || defined(KLAYGE_CPU_X64) || defined(KLAYGE_PLATFORM_WINDOWS)
	#define KLAYGE_LITTLE_ENDIAN
#else
	#define KLAYGE_BIG_ENDIAN
#endif

#define _IDENTITY_SUPPORT
#define _SELECT1ST2ND_SUPPORT
#define _PROJECT1ST2ND_SUPPORT
#define _COPYIF_SUPPORT

// Defines some MACROs from compile options
#ifdef KLAYGE_CPU_X64
	#define _SSE_SUPPORT
	#define _SSE2_SUPPORT
	#define _X64_SUPPORT
#elif defined KLAYGE_CPU_X86
	#if defined(KLAYGE_COMPILER_MSVC)
		#if _M_IX86 == 600
			#define _MMX_SUPPORT
		#endif

		#if _M_IX86_FP == 1
			#define _SSE_SUPPORT
		#elif _M_IX86_FP == 2
			#define _SSE_SUPPORT
			#define _SSE2_SUPPORT
		#endif
	#elif defined(KLAYGE_COMPILER_GCC)
		#ifdef __MMX__
			#define _MMX_SUPPORT
		#endif
		#ifdef __SSE__
			#define _SSE_SUPPORT
		#endif
		#ifdef __SSE2__
			#define _SSE2_SUPPORT
		#endif
	#endif
#endif

#define KLAYGE_STRINGIZE(X) KLAYGE_DO_STRINGIZE(X)
#define KLAYGE_DO_STRINGIZE(X) #X

#define KLAYGE_JOIN(X, Y) KLAYGE_DO_JOIN(X, Y)
#define KLAYGE_DO_JOIN(X, Y) KLAYGE_DO_JOIN2(X, Y)
#define KLAYGE_DO_JOIN2(X, Y) X##Y

#define KLAYGE_COMPILER_TOOLSET KLAYGE_STRINGIZE(KLAYGE_JOIN(KLAYGE_COMPILER_NAME, KLAYGE_COMPILER_VERSION))

#ifdef KLAYGE_HAS_DECLSPEC
	#ifdef KLAYGE_CORE_SOURCE		// Build dll
		#define KLAYGE_CORE_API __declspec(dllexport)
	#else							// Use dll
		#define KLAYGE_CORE_API __declspec(dllimport)
	#endif
#else
	#define KLAYGE_CORE_API
#endif // KLAYGE_HAS_DECLSPEC

#endif		// _CONFIG_HPP
