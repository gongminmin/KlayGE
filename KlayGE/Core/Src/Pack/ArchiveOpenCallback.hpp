// ArchiveOpenCallback.hpp
// KlayGE 打包系统打开压缩包回调函数 头文件 来自7zip
// Ver 3.6.0
// 版权所有(C) 龚敏敏, 2007
// Homepage: http://www.klayge.org
//
// 3.6.0
// 初次建立 (2007.5.24)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#ifndef _ARCHIVE_OPEN_CALLBACK_HPP
#define _ARCHIVE_OPEN_CALLBACK_HPP

#pragma once

#include <KlayGE/atomic.hpp>

#include <string>

#include <CPP/7zip/Archive/IArchive.h>
#include <CPP/7zip/IPassword.h>

namespace KlayGE
{
	class CArchiveOpenCallback : public IArchiveOpenCallback, public ICryptoGetTextPassword
	{
	public:
		STDMETHOD_(ULONG, AddRef)()
		{
			++ ref_count_;
			return ref_count_.value();
		}
		STDMETHOD_(ULONG, Release)()
		{
			-- ref_count_;
			if (0 == ref_count_.value())
			{
				delete this;
				return 0;
			}
			return ref_count_.value();
		}

		STDMETHOD(QueryInterface)(REFGUID iid, void** outObject)
		{
			if (IID_ICryptoGetTextPassword == iid)
			{
				*outObject = static_cast<ICryptoGetTextPassword*>(this);
				this->AddRef();
				return S_OK;
			}
			else
			{
				if (IID_IArchiveOpenCallback == iid)
				{
					*outObject = static_cast<IArchiveOpenCallback*>(this);
					this->AddRef();
					return S_OK;
				}
				else
				{
					return E_NOINTERFACE;
				}
			}
		}

		STDMETHOD(SetTotal)(const uint64_t* files, const uint64_t* bytes);
		STDMETHOD(SetCompleted)(const uint64_t* files, const uint64_t* bytes);

		// ICryptoGetTextPassword
		STDMETHOD(CryptoGetTextPassword)(BSTR *password);

		CArchiveOpenCallback()
			: ref_count_(1), password_is_defined_(false)
		{
		}

		void Init(std::string const & pw);

	private:
		atomic<int32_t> ref_count_;

		bool password_is_defined_;
		std::wstring password_;
	};
}

#endif		// _ARCHIVE_OPEN_CALLBACK_HPP
