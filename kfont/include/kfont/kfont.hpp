#ifndef _KFONT_HPP
#define _KFONT_HPP

#pragma once

#if defined(_WIN32) || defined(__WIN32__) || defined(WIN32)
#define KFONT_PLATFORM_WINDOWS
#define NOMINMAX
// Forces all boost's libraries to be linked as dll
#ifndef BOOST_ALL_DYN_LINK
	#define BOOST_ALL_DYN_LINK
#endif
#elif defined(__ANDROID__)
#define KFONT_PLATFORM_ANDROID
#endif

#include <vector>

#include <boost/cstdint.hpp>
#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable: 4100 6011 6334)
#endif
#include <boost/unordered_map.hpp>
#include <boost/functional/hash.hpp>
#if defined(_MSC_VER)
#pragma warning(pop)
#endif

#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable: 4127)
#endif
#include <boost/pool/pool_alloc.hpp>
#if defined(_MSC_VER)
#pragma warning(pop)
#endif

#if defined(DEBUG) | defined(_DEBUG)
    #define KFONT_DEBUG
#endif

#if defined(_MSC_VER)
	#define KFONT_HAS_DECLSPEC

	#if _MSC_VER >= 1400
		#pragma warning(disable: 4251 4275 4819)

		#ifndef _CRT_SECURE_NO_DEPRECATE
			#define _CRT_SECURE_NO_DEPRECATE
		#endif
		#ifndef _SCL_SECURE_NO_DEPRECATE
			#define _SCL_SECURE_NO_DEPRECATE
		#endif
	#endif

	#ifndef KFONT_SOURCE
		#if defined(_M_X64)
			#if defined(KFONT_DEBUG)
				#pragma comment(lib, "kfont_vc_x64_d.lib")
			#else
				#pragma comment(lib, "kfont_vc_x64.lib")
			#endif
		#elif(_M_IX86)
			#if defined(KFONT_DEBUG)
				#pragma comment(lib, "kfont_vc_x86_d.lib")
			#else
				#pragma comment(lib, "kfont_vc_x86.lib")
			#endif
		#endif
	#endif
#endif

// Defines the native endian
#if defined(_M_ARM) || defined(__arm__)
	#ifdef __ARMEB__
		#define KFONT_BIG_ENDIAN
	#else
		#define KFONT_LITTLE_ENDIAN
	#endif
#elif defined(_M_IX86) || defined(_M_X64) || defined(KFONT_PLATFORM_WINDOWS) || defined(__x86_64__) || defined(__i386__)
	#define KFONT_LITTLE_ENDIAN
#else
	#define KFONT_BIG_ENDIAN
#endif

#if defined(KFONT_PLATFORM_WINDOWS)
	#if !defined(__GNUC__) && !defined(KFONT_HAS_DECLSPEC)
		#define KFONT_HAS_DECLSPEC
	#endif

	#if defined(__MINGW32__)
		#define KFONT_HAS_DECLSPEC
	#endif
#endif

#ifdef KFONT_HAS_DECLSPEC
	#ifdef KFONT_SOURCE		// Build dll
		#define KFONT_API __declspec(dllexport)
	#else							// Use dll
		#define KFONT_API __declspec(dllimport)
	#endif
#else
	#define KFONT_API
#endif // KFONT_HAS_DECLSPEC

namespace KlayGE
{
#ifdef _MSC_VER
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

#ifdef KFONT_PLATFORM_WINDOWS
	#pragma pack(push, 1)
#endif
	struct kfont_header
	{
		uint32_t fourcc;
		uint32_t version;
		uint32_t start_ptr;
		uint32_t validate_chars;
		uint32_t non_empty_chars;
		uint32_t char_size;

		int16_t base;
		int16_t scale;
	};
#ifdef KFONT_PLATFORM_WINDOWS
	#pragma pack(pop)
#endif

	class KFONT_API KFont
	{
	public:
#ifdef KFONT_PLATFORM_WINDOWS
	#pragma pack(push, 1)
#endif
		struct font_info
		{
			int16_t top;
			int16_t left;
			uint16_t width;
			uint16_t height;
		};
#ifdef KFONT_PLATFORM_WINDOWS
	#pragma pack(pop)
#endif

	public:
		KFont();
		
		bool Load(std::string const & file_name);
		bool Save(std::string const & file_name);

		uint32_t CharSize() const;
		int16_t DistBase() const;
		int16_t DistScale() const;

		std::pair<int32_t, uint32_t> const & CharIndexAdvance(wchar_t ch) const;
		int32_t CharIndex(wchar_t ch) const;
		uint32_t CharAdvance(wchar_t ch) const;

		font_info const & CharInfo(int32_t index) const;
		void GetDistanceData(uint8_t* p, uint32_t pitch, int32_t index) const;
		void GetLZMADistanceData(uint8_t const *& p, uint32_t& size, int32_t index) const;

		void CharSize(uint32_t size);
		void DistBase(int16_t base);
		void DistScale(int16_t scale);
		void SetDistanceData(wchar_t ch, uint8_t const * p, uint32_t adv, font_info const & fi);
		void SetLZMADistanceData(wchar_t ch, uint8_t const * p, uint32_t size, uint32_t adv, font_info const & fi);
		void Compact();

	private:
		uint32_t char_size_;
		int16_t dist_base_;
		int16_t dist_scale_;
		boost::unordered_map<int32_t, std::pair<int32_t, uint32_t>, boost::hash<int32_t>, std::equal_to<int32_t>,
			boost::fast_pool_allocator<std::pair<int32_t, std::pair<int32_t, uint32_t> > > > char_index_advance_;
		std::vector<font_info> char_info_;
		std::vector<size_t> distances_addr_;
		std::vector<uint8_t> distances_lzma_;
	};
}

#endif		// _KFONT_HPP
