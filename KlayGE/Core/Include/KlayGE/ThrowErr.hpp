// ThrowErr.hpp
// KlayGE 抛出错误 头文件
// Ver 3.7.0
// 版权所有(C) 龚敏敏, 2001-2008
// Homepage: http://klayge.sourceforge.net
//
// 3.7.0
// 改用Boost.System的异常 (2008.5.21)
//
// 1.3.8.1
// 初次建立
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#ifndef _THROWERR_HPP
#define _THROWERR_HPP

#include <string>
#include <stdexcept>

#include <boost/system/error_code.hpp>
#include <boost/system/system_error.hpp>

#ifndef _HRESULT_DEFINED
#define _HRESULT_DEFINED
typedef long HRESULT;
#endif

namespace KlayGE
{
	std::string CombineFileLine(std::string const & file, int line);
}

#define THR(x)			{ throw boost::system::system_error(boost::system::posix_error::make_error_code(x), KlayGE::CombineFileLine(__FILE__, __LINE__)); }

// 如果错误，就抛出错误代码
#define TIF(x)			{ HRESULT _hr = x; if (static_cast<HRESULT>(_hr) < 0) { throw std::runtime_error(KlayGE::CombineFileLine(__FILE__, __LINE__)); } }

// 断言
inline void
Verify(bool x)
{
	if (!x)
	{
		THR(boost::system::posix_error::not_supported);
	}
}

#endif		// _THROWERR_HPP
