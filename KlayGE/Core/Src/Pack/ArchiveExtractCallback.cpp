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

#include <KlayGE/KlayGE.hpp>
#include <KFL/Util.hpp>

#include <CPP/Common/MyWindows.h>

#include "ArchiveExtractCallback.hpp"

namespace KlayGE
{
	ArchiveExtractCallback::ArchiveExtractCallback(std::string_view pw, std::shared_ptr<ISequentialOutStream> const & out_file_stream)
		: password_is_defined_(!pw.empty()), out_file_stream_(out_file_stream)
	{
		Convert(password_, pw);
	}

	STDMETHODIMP_(ULONG) ArchiveExtractCallback::AddRef()
	{
		++ ref_count_;
		return ref_count_;
	}

	STDMETHODIMP_(ULONG) ArchiveExtractCallback::Release()
	{
		-- ref_count_;
		if (0 == ref_count_)
		{
			delete this;
			return 0;
		}
		return ref_count_;
	}

	STDMETHODIMP ArchiveExtractCallback::QueryInterface(REFGUID iid, void** out_object)
	{
		if (IID_ICryptoGetTextPassword == iid)
		{
			*out_object = static_cast<ICryptoGetTextPassword*>(this);
			this->AddRef();
			return S_OK;
		}
		else if (IID_IArchiveExtractCallback == iid)
		{
			*out_object = static_cast<IArchiveExtractCallback*>(this);
			this->AddRef();
			return S_OK;
		}
		else
		{
			return E_NOINTERFACE;
		}
	}

	STDMETHODIMP ArchiveExtractCallback::SetTotal(UInt64 size)
	{
		KFL_UNUSED(size);
		return S_OK;
	}

	STDMETHODIMP ArchiveExtractCallback::SetCompleted(UInt64 const * complete_value)
	{
		KFL_UNUSED(complete_value);
		return S_OK;
	}

	STDMETHODIMP ArchiveExtractCallback::GetStream(UInt32 index, ISequentialOutStream** out_stream, Int32 ask_extract_mode)
	{
		KFL_UNUSED(index);

		enum
		{
			kExtract = 0,
			kTest,
			kSkip,
		};

		if (kExtract == ask_extract_mode)
		{
			out_file_stream_->AddRef();
			*out_stream = out_file_stream_.get();
		}
		else
		{
			*out_stream = nullptr;
		}
		return S_OK;
	}

	STDMETHODIMP ArchiveExtractCallback::PrepareOperation(Int32 ask_extract_mode)
	{
		KFL_UNUSED(ask_extract_mode);
		return S_OK;
	}

	STDMETHODIMP ArchiveExtractCallback::SetOperationResult(Int32 operation_result)
	{
		KFL_UNUSED(operation_result);
		return S_OK;
	}

	STDMETHODIMP ArchiveExtractCallback::CryptoGetTextPassword(BSTR* password)
	{
		if (password_is_defined_)
		{
#ifdef KLAYGE_PLATFORM_WINDOWS_DESKTOP
			*password = SysAllocString(password_.c_str());
#else
			*password = nullptr;
#endif
			return S_OK;
		}
		else
		{
			return E_ABORT;
		}
	}
}
