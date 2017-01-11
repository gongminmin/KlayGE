/**
 * @file ArchiveOpenCallback.hpp
 * @author Minmin Gong
 *
 * @section DESCRIPTION
 *
 * This source file is part of KlayGE
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

#ifndef _KFL_ARCHIVEOPENCALLBACK_HPP
#define _KFL_ARCHIVEOPENCALLBACK_HPP

#pragma once

#include <string>
#include <atomic>

#include <CPP/7zip/Archive/IArchive.h>
#include <CPP/7zip/IPassword.h>

namespace KlayGE
{
	class CArchiveOpenCallback : boost::noncopyable, public IArchiveOpenCallback, public ICryptoGetTextPassword
	{
	public:
		STDMETHOD_(ULONG, AddRef)()
		{
			++ ref_count_;
			return ref_count_;
		}
		STDMETHOD_(ULONG, Release)()
		{
			-- ref_count_;
			if (0 == ref_count_)
			{
				delete this;
				return 0;
			}
			return ref_count_;
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

		STDMETHOD(SetTotal)(const UInt64* files, const UInt64* bytes);
		STDMETHOD(SetCompleted)(const UInt64* files, const UInt64* bytes);

		// ICryptoGetTextPassword
		STDMETHOD(CryptoGetTextPassword)(BSTR *password);

		CArchiveOpenCallback()
			: ref_count_(1), password_is_defined_(false)
		{
		}
		
		virtual ~CArchiveOpenCallback()
		{
		}

		void Init(std::string const & pw);

	private:
		std::atomic<int32_t> ref_count_;

		bool password_is_defined_;
		std::wstring password_;
	};
}

#endif		// _KFL_ARCHIVEOPENCALLBACK_HPP
