// icstring.hpp
// KlayGE 忽略大小写字符串模板 头文件
// Ver 1.2.8.10
// 版权所有(C) 龚敏敏, 2002
// Homepage: http://www.enginedev.com
//
// 1.2.8.10
// 初次建立 (2002.10.29)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#ifndef _ICSTRING_HPP
#define _ICSTRING_HPP

#include <iostream>
#include <string>
#include <locale>

namespace KlayGE
{
	// 忽略大小写的字符特性
	template <typename charT>
	struct char_ignorecase_traits : public std::char_traits<charT>
	{
		// 忽略大小写的等于比较
		static bool eq(const char_type& ch1, const char_type& ch2)
			{ return std::toupper(ch1, std::locale::classic()) == std::toupper(ch2, std::locale::classic()); }

		// 忽略大小写的小于比较
		static bool lt(const char_type& ch1, const char_type& ch2)
			{ return std::toupper(ch1, std::locale::classic()) < std::toupper(ch2, std::locale::classic()); }

		static int compare(const char_type* str1, const char_type* str2, std::size_t n)
		{
			for (std::size_t i = 0; i < n; ++ i)
			{
				if (!eq(str1[i], str2[i]))
				{
					return lt(str1[i], str2[i]) ? -1 : 1;
				}
			}

			return 0;
		}

		static const char_type* find(const char_type* str, std::size_t n, const char_type& c)
		{
			for (std::size_t i = 0; i < n; ++ i)
			{
				if (eq(str[i], c))
				{
					return &(str[i]);
				}
			}

			return NULL;
		}
	};

	typedef std::basic_string<char, char_ignorecase_traits<char> > icstring;
	typedef std::basic_string<wchar_t, char_ignorecase_traits<wchar_t> > wicstring;

	template <typename charT>
	inline std::basic_ostream<charT>&
	operator<<(std::basic_ostream<charT>& ostrm,
					const std::basic_string<charT, char_ignorecase_traits<charT> >& str)
		{ return ostrm << std::basic_string<charT>(str.data(), str.length()); }
}

#endif			// _ICSTRING_HPP