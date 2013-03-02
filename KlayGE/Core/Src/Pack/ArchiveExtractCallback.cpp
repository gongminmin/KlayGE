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
	STDMETHODIMP CArchiveExtractCallback::SetTotal(uint64_t /*size*/)
	{
		return S_OK;
	}

	STDMETHODIMP CArchiveExtractCallback::SetCompleted(const uint64_t* /*completeValue*/)
	{
		return S_OK;
	}

	STDMETHODIMP CArchiveExtractCallback::GetStream(uint32_t /*index*/, ISequentialOutStream** outStream, int32_t askExtractMode)
	{
		enum 
		{
			kExtract = 0,
			kTest,
			kSkip,
		};

		if (kExtract == askExtractMode)
		{
			_outFileStream->AddRef();
			*outStream = _outFileStream.get();
		}
		else
		{
			*outStream = nullptr;
		}
		return S_OK;
	}

	STDMETHODIMP CArchiveExtractCallback::PrepareOperation(int32_t /*askExtractMode*/)
	{
		return S_OK;
	}

	STDMETHODIMP CArchiveExtractCallback::SetOperationResult(int32_t /*operationResult*/)
	{
		return S_OK;
	}

	STDMETHODIMP CArchiveExtractCallback::CryptoGetTextPassword(BSTR* password)
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
			*password = nullptr;
#endif
			return S_OK;
		}
	}

	void CArchiveExtractCallback::Init(std::string const & pw, shared_ptr<ISequentialOutStream> const & outFileStream)
	{
		_outFileStream = outFileStream;

		password_is_defined_ = !pw.empty();
		Convert(password_, pw);
	}
}
