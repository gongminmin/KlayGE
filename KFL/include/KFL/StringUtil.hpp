/**
 * @file StringUtil.hpp
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

#ifndef KFL_STRING_UTIL_HPP
#define KFL_STRING_UTIL_HPP

#pragma once

#include <algorithm>
#include <locale>
#include <string_view>
#include <vector>

#include <KFL/Util.hpp>

namespace KlayGE
{
	namespace StringUtil
	{
		namespace Detail
		{
			template <typename CharType>
			class EqualToT
			{
			public:
				using result_type = bool;

				EqualToT(CharType ch) noexcept : ch_(ch)
				{
				}

				EqualToT(EqualToT const& rhs) noexcept : ch_(rhs.ch_)
				{
				}

				EqualToT(EqualToT&& rhs) noexcept : ch_(std::move(rhs.ch_))
				{
				}

				EqualToT& operator=(EqualToT const& rhs) noexcept
				{
					ch_ = rhs.ch_;
				}

				EqualToT& operator=(EqualToT&& rhs) noexcept
				{
					ch_ = std::move(rhs.ch_);
				}

				bool operator()(CharType ch) const noexcept
				{
					return ch_ == ch;
				}

			private:
				CharType ch_;
			};

			template <typename CharType>
			class IsAnyOfT
			{
			public:
				using result_type = bool;

				template <typename RangeType>
				IsAnyOfT(RangeType const& set)
				{
					set_ = std::vector<CharType>(std::begin(set), std::end(set));
					std::sort(set_.begin(), set_.end());
					set_.erase(std::unique(set_.begin(), set_.end()), set_.end());
				}

				IsAnyOfT(IsAnyOfT const& rhs) : set_(rhs.set_)
				{
				}

				IsAnyOfT(IsAnyOfT&& rhs) noexcept : set_(std::move(rhs.set_))
				{
				}

				IsAnyOfT& operator=(IsAnyOfT const& rhs)
				{
					set_ = rhs.set_;
				}

				IsAnyOfT& operator=(IsAnyOfT&& rhs) noexcept
				{
					set_ = std::move(rhs.set_);
				}

				bool operator()(CharType ch) const
				{
					return std::binary_search(set_.begin(), set_.end(), ch);
				}

			private:
				std::vector<CharType> set_;
			};

			template <typename RangeType>
			struct RangeValue
			{
				using value_type = typename RangeType::value_type;
			};

			template <typename CharType, size_t size>
			struct RangeValue<CharType[size]>
			{
				using value_type = CharType;
			};

			template <typename CharType, size_t size>
			struct RangeValue<CharType const[size]>
			{
				using value_type = CharType const;
			};

			template <typename T>
			inline bool IsRawChar(T const& str) noexcept
			{
				KFL_UNUSED(str);
				return false;
			}

			inline bool IsRawChar(char* str) noexcept
			{
				KFL_UNUSED(str);
				return true;
			}

			inline bool IsRawChar(char const* str) noexcept
			{
				KFL_UNUSED(str);
				return true;
			}

			inline bool IsRawChar(wchar_t* str) noexcept
			{
				KFL_UNUSED(str);
				return true;
			}

			inline bool IsRawChar(wchar_t const* str) noexcept
			{
				KFL_UNUSED(str);
				return true;
			}

			template <typename RangeType>
			inline size_t RangeLength(RangeType const& input)
			{
				using CharType = typename Detail::RangeValue<RangeType>::value_type;
				if (IsRawChar(input))
				{
					return std::basic_string_view<CharType>(input).size();
				}
				else
				{
					return std::cend(input) - std::cbegin(input);
				}
			}
		} // namespace Detail

		template <typename CharType>
		inline Detail::EqualToT<CharType> EqualTo(CharType ch)
		{
			return Detail::EqualToT<CharType>(ch);
		}

		template <typename RangeType>
		inline Detail::IsAnyOfT<typename Detail::RangeValue<RangeType>::value_type> IsAnyOf(RangeType const& set)
		{
			return Detail::IsAnyOfT<typename Detail::RangeValue<RangeType>::value_type>(set);
		}

		template <typename CharType>
		bool IsSpace(CharType ch, std::locale const& loc)
		{
			return std::use_facet<std::ctype<CharType>>(loc).is(std::ctype_base::space, ch);
		}

		template <typename CharType>
		bool IsSpace(CharType ch)
		{
			return IsSpace(ch, std::locale());
		}

		template <typename RangeType, typename UnaryPredicate>
		inline std::vector<std::basic_string_view<typename Detail::RangeValue<RangeType>::value_type>> Split(
			RangeType const& input, UnaryPredicate pred)
		{
			using CharType = typename Detail::RangeValue<RangeType>::value_type;

			std::vector<std::basic_string_view<CharType>> ret;

			auto const begin_iter = std::cbegin(input);
			auto const end_iter = begin_iter + Detail::RangeLength(input);

			auto iter = begin_iter;
			while (iter != end_iter)
			{
				while ((iter != end_iter) && pred(*iter))
				{
					++iter;
				}

				auto token_begin_iter = iter;
				while ((iter != end_iter) && !pred(*iter))
				{
					++iter;
				}

				if (token_begin_iter != iter)
				{
					ret.push_back(std::basic_string_view<CharType>(&(*token_begin_iter), iter - token_begin_iter));
				}

				if (iter != end_iter)
				{
					++iter;
				}
			}

			return ret;
		}

		template <typename RangeType>
		inline std::basic_string_view<typename Detail::RangeValue<RangeType>::value_type> TrimLeft(
			RangeType const& input, std::locale const& loc)
		{
			using CharType = typename Detail::RangeValue<RangeType>::value_type;

			auto const begin_iter = std::cbegin(input);
			auto const end_iter = begin_iter + Detail::RangeLength(input);

			auto iter = begin_iter;
			while ((iter != end_iter) && IsSpace(*iter, loc))
			{
				++iter;
			}

			if (iter == end_iter)
			{
				return std::basic_string_view<CharType>();
			}
			else
			{
				return std::basic_string_view<CharType>(&(*iter), end_iter - iter);
			}
		}

		template <typename RangeType>
		inline std::basic_string_view<typename Detail::RangeValue<RangeType>::value_type> TrimLeft(RangeType const& input)
		{
			return TrimLeft(input, std::locale());
		}

		template <typename RangeType>
		inline std::basic_string_view<typename Detail::RangeValue<RangeType>::value_type> TrimRight(
			RangeType const& input, std::locale const& loc)
		{
			using CharType = typename Detail::RangeValue<RangeType>::value_type;

			auto const begin_iter = std::cbegin(input);
			auto const end_iter = begin_iter + Detail::RangeLength(input);

			auto iter = end_iter;
			while ((iter != begin_iter) && IsSpace(*(iter - 1), loc))
			{
				--iter;
			}

			if (iter == begin_iter)
			{
				return std::basic_string_view<CharType>();
			}
			else
			{
				return std::basic_string_view<CharType>(&input[0], iter - begin_iter);
			}
		}

		template <typename RangeType>
		inline std::basic_string_view<typename Detail::RangeValue<RangeType>::value_type> TrimRight(RangeType const& input)
		{
			return TrimRight(input, std::locale());
		}

		template <typename RangeType>
		inline std::basic_string_view<typename Detail::RangeValue<RangeType>::value_type> Trim(
			RangeType const& input, std::locale const& loc)
		{
			return TrimRight(TrimLeft(input, loc), loc);
		}

		template <typename RangeType>
		inline std::basic_string_view<typename Detail::RangeValue<RangeType>::value_type> Trim(RangeType const& input)
		{
			return Trim(input, std::locale());
		}

		template <typename RangeType>
		inline void ToLower(RangeType& arg, std::locale const& loc)
		{
			using CharType = typename Detail::RangeValue<RangeType>::value_type;
			std::for_each(std::begin(arg), std::end(arg), [&loc](CharType& ch) { ch = static_cast<CharType>(std::tolower(ch, loc)); });
		}

		template <typename RangeType>
		inline void ToLower(RangeType& arg)
		{
			ToLower(arg, std::locale());
		}

		template <typename RangeType>
		inline void ToUpper(RangeType& arg, std::locale const& loc)
		{
			using CharType = typename Detail::RangeValue<RangeType>::value_type;
			std::for_each(std::begin(arg), std::end(arg), [&loc](CharType& ch) { ch = static_cast<CharType>(std::toupper(ch, loc)); });
		}

		template <typename RangeType>
		inline void ToUpper(RangeType& arg)
		{
			ToUpper(arg, std::locale());
		}

		template <typename RangeType1, typename RangeType2>
		bool CaseInsensitiveLexicographicalCompare(RangeType1 const& lhs, RangeType2 const& rhs, std::locale const& loc)
		{
			auto const begin_lhs_iter = std::cbegin(lhs);
			auto const end_lhs_iter = begin_lhs_iter + Detail::RangeLength(lhs);

			auto const begin_rhs_iter = std::cbegin(rhs);
			auto const end_rhs_iter = begin_rhs_iter + Detail::RangeLength(rhs);

			auto lhs_iter = begin_lhs_iter;
			auto rhs_iter = begin_rhs_iter;
			while ((lhs_iter != end_lhs_iter) && (rhs_iter != end_rhs_iter))
			{
				auto const lower_lhs = std::tolower(*lhs_iter, loc);
				auto const lower_rhs = std::tolower(*rhs_iter, loc);
				if (lower_lhs > lower_rhs)
				{
					return false;
				}
				else if (lower_lhs < lower_rhs)
				{
					return true;
				}

				++lhs_iter;
				++rhs_iter;
			}
			return (lhs_iter == end_lhs_iter) && (rhs_iter != end_rhs_iter);
		}

		template <typename RangeType1, typename RangeType2>
		bool CaseInsensitiveLexicographicalCompare(RangeType1 const& lhs, RangeType2 const& rhs)
		{
			return CaseInsensitiveLexicographicalCompare(lhs, rhs, std::locale());
		}
	} // namespace StringUtil
} // namespace KlayGE

#endif // KFL_STRING_UTIL_HPP
