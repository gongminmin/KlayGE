// ArchiveOpenCallback.cpp
// KlayGE 打包系统打开压缩包回调函数 实现文件 来自7zip
// Ver 3.6.0
// 版权所有(C) 龚敏敏, 2007
// Homepage: http://klayge.sourceforge.net
//
// 3.6.0
// 初次建立 (2007.5.24)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KlayGE/Util.hpp>

#include "BaseDefines.hpp"

#include "BSTR.hpp"
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
			*password = AllocBSTR(password_.c_str());
			return S_OK;
		}
	}

	void CArchiveOpenCallback::Init(std::string const & pw)
	{
		password_is_defined_ = !pw.empty();
		Convert(password_, pw);
	}
}
