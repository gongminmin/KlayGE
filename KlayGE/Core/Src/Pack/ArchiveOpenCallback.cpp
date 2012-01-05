// ArchiveOpenCallback.cpp
// KlayGE 打包系统打开压缩包回调函数 实现文件 来自7zip
// Ver 3.6.0
// 版权所有(C) 龚敏敏, 2007
// Homepage: http://www.klayge.org
//
// 3.6.0
// 初次建立 (2007.5.24)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KlayGE/Util.hpp>

#include <CPP/Common/MyWindows.h>

#include "ArchiveOpenCallback.hpp"

namespace KlayGE
{
	STDMETHODIMP CArchiveOpenCallback::SetTotal(const uint64_t* /*files*/, const uint64_t* /*bytes*/)
	{
		return S_OK;
	}

	STDMETHODIMP CArchiveOpenCallback::SetCompleted(const uint64_t* /*files*/, const uint64_t* /*bytes*/)
	{
		return S_OK;
	}

	STDMETHODIMP CArchiveOpenCallback::CryptoGetTextPassword(BSTR* password)
	{
		if (!password_is_defined_)
		{
			return E_ABORT;
		}
		else
		{
#ifdef KLAYGE_PLATFORM_WINDOWS
			*password = SysAllocString(password_.c_str());
#else
			*password = NULL;
#endif
			return S_OK;
		}
	}

	void CArchiveOpenCallback::Init(std::string const & pw)
	{
		password_is_defined_ = !pw.empty();
		Convert(password_, pw);
	}
}
