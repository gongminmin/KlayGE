// util.hpp
// KlayGE 实用函数 头文件
// Ver 2.5.0
// 版权所有(C) 龚敏敏, 2005
// Homepage: http://www.klayge.org
//
// 2.5.0
// 初次建立 (2005.5.1)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#ifndef _UTIL_HPP
#define _UTIL_HPP

#include <string>

namespace KlayGE
{
	std::string tstr_to_str(std::basic_string<TCHAR> const & tstr);

	bool is_mesh(INode* node);
	bool is_bone(INode* node);
}

#endif		// _UTIL_HPP
