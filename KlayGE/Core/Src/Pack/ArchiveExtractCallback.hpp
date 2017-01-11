/**
 * @file ArchiveExtractCallback.hpp
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

#ifndef _KFL_ARCHIVEEXTRACTCALLBACK_HPP
#define _KFL_ARCHIVEEXTRACTCALLBACK_HPP

#pragma once

#include <string>
#include <atomic>

#include <CPP/7zip/Archive/IArchive.h>
#include <CPP/7zip/IPassword.h>

namespace KlayGE
{
	class CArchiveExtractCallback : boost::noncopyable, public IArchiveExtractCallback, public ICryptoGetTextPassword
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
				if (IID_IArchiveExtractCallback == iid)
				{
					*outObject = static_cast<IArchiveExtractCallback*>(this);
					this->AddRef();
					return S_OK;
				}
				else
				{
					return E_NOINTERFACE;
				}
			}
		}

		// IProgress
		STDMETHOD(SetTotal)(UInt64 size);
		STDMETHOD(SetCompleted)(const UInt64* completeValue);

		// IExtractCallBack
		STDMETHOD(GetStream)(UInt32 index, ISequentialOutStream **outStream, Int32 askExtractMode);
		STDMETHOD(PrepareOperation)(Int32 askExtractMode);
		STDMETHOD(SetOperationResult)(Int32 resultEOperationResult);

		// ICryptoGetTextPassword
		STDMETHOD(CryptoGetTextPassword)(BSTR *aPassword);


	public:
		CArchiveExtractCallback()
			: ref_count_(1), password_is_defined_(false)
		{
		}
		
		virtual ~CArchiveExtractCallback()
		{
		}

		void Init(std::string const & pw, std::shared_ptr<ISequentialOutStream> const & outFileStream);

	private:
		std::atomic<int32_t> ref_count_;

		bool password_is_defined_;
		std::wstring password_;

		std::shared_ptr<ISequentialOutStream> _outFileStream;
	};
}

#endif		// _KFL_ARCHIVEEXTRACTCALLBACK_HPP
