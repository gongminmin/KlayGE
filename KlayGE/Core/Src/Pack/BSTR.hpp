// BSTR.hpp
// KlayGE 打包系统BSTR 头文件
// Ver 3.6.0
// 版权所有(C) 龚敏敏, 2007
// Homepage: http://klayge.sourceforge.net
//
// 3.6.0
// 初次建立 (2007.5.24)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#ifndef _BSTR_HPP
#define _BSTR_HPP

#pragma KLAYGE_ONCE

#include <string>

typedef wchar_t* BSTR;

namespace KlayGE
{
	BSTR AllocBSTR(std::wstring const & sz);
	void FreeBSTR(BSTR bstr);
}

#endif      // _BSTR_HPP
