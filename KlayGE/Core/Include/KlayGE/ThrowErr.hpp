// ThrowErr.hpp
// KlayGE 抛出错误 头文件
// Ver 1.3.8.1
// 版权所有(C) 龚敏敏, 2001--2002
// Homepage: http://www.enginedev.com
//
// 1.3.8.1
// 初次建立
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#ifndef _THROWERR_HPP
#define _THROWERR_HPP

#include <string>
#include <exception>

#define NOMINMAX
#include <windows.h>

namespace KlayGE
{
	class Exception : public std::exception
	{
	public:
		Exception(std::string const & errFile, uint32_t errLine, uint32_t errCode, std::string const & msg) throw()
			: errFile_(errFile), errLine_(errLine),
				errCode_(errCode), msg_(msg)
			{ }

		std::string const & ErrorFile() const throw()
			{ return errFile_; }
		uint32_t ErrorLine() const throw()
			{ return errLine_; }

		uint32_t ErrorCode() const throw()
			{ return errCode_; }
		const char* what() const throw()
			{ return msg_.c_str(); }

	public:
		std::string errFile_;
		uint32_t errLine_;

		uint32_t errCode_;
		std::string msg_;
	};
}

#ifdef NDEBUG

// 抛出HRESULT
inline void
THR(HRESULT hr)
	{ throw hr; }

// 抛出HRESULT
inline void
TIF(HRESULT hr)
{
	if (FAILED(hr))
	{
		THR(hr);
	}
}

#else

#define THR(x)			{ throw KlayGE::Exception(__FILE__, __LINE__, x, #x); }

// 如果错误，就抛出错误代码
#define TIF(x)			{ HRESULT _hr = x; if (FAILED(_hr)) { THR(x); } }

#endif

// 断言
inline void
Verify(bool x)
{
	if (!x)
	{
		THR(E_FAIL);
	}
}

#endif		// _THROWERR_HPP
