/**
 * @file kfont.hpp
 * @author Minmin Gong
 *
 * @section DESCRIPTION
 *
 * This source file is part of kfont, a subproject of KlayGE
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

#ifndef _KFONT_KFONT_HPP
#define _KFONT_KFONT_HPP

#pragma once

#include <vector>
#include <istream>

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

#ifdef KLAYGE_COMPILER_MSVC
	#ifndef KFONT_SOURCE
		#ifdef KLAYGE_DEBUG
		#define LIB_FILE_NAME "kfont_" KFL_STRINGIZE(KLAYGE_COMPILER_NAME) "_" KFL_STRINGIZE(KLAYGE_COMPILER_TARGET) "_d.lib"
		#else
		#define LIB_FILE_NAME "kfont_" KFL_STRINGIZE(KLAYGE_COMPILER_NAME) "_" KFL_STRINGIZE(KLAYGE_COMPILER_TARGET) ".lib"
		#endif

		#pragma comment(lib, LIB_FILE_NAME)
		//#pragma message("Linking to lib file: " LIB_FILE_NAME)
		#undef LIB_FILE_NAME
	#endif	// KFONT_SOURCE
#endif	// KLAYGE_COMPILER_MSVC

#ifdef KLAYGE_HAS_DECLSPEC
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
#ifdef KLAYGE_HAS_STRUCT_PACK
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
#ifdef KLAYGE_HAS_STRUCT_PACK
	#pragma pack(pop)
#endif

	class KFONT_API KFont
	{
	public:
#ifdef KLAYGE_HAS_STRUCT_PACK
	#pragma pack(push, 1)
#endif
		struct font_info
		{
			int16_t top;
			int16_t left;
			uint16_t width;
			uint16_t height;
		};
#ifdef KLAYGE_HAS_STRUCT_PACK
	#pragma pack(pop)
#endif

	public:
		KFont();
		
		bool Load(std::string const & file_name);
		bool Load(std::istream& kfont_input);
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

#endif		// _KFONT_KFONT_HPP
