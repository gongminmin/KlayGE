/**
 * @file Streams.cpp
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
#include <KlayGE/ResLoader.hpp>

#include <boost/assert.hpp>

#include <CPP/Common/MyWindows.h>

#include "Streams.hpp"

namespace KlayGE
{
	InStream::InStream(ResIdentifierPtr const & is)
		: is_(is)
	{
		BOOST_ASSERT(*is);

		is_->seekg(0, std::ios_base::end);
		stream_size_ = is_->tellg();
		is_->seekg(0, std::ios_base::beg);
	}

	STDMETHODIMP_(ULONG) InStream::AddRef()
	{
		++ ref_count_;
		return ref_count_;
	}

	STDMETHODIMP_(ULONG) InStream::Release()
	{
		-- ref_count_;
		if (0 == ref_count_)
		{
			delete this;
			return 0;
		}
		return ref_count_;
	}

	STDMETHODIMP InStream::QueryInterface(REFGUID iid, void** out_object)
	{
		if (IID_IInStream == iid)
		{
			*out_object = static_cast<IInStream*>(this);
			this->AddRef();
			return S_OK;
		}
		else if (IID_IStreamGetSize == iid)
		{
			*out_object = static_cast<IStreamGetSize*>(this);
			this->AddRef();
			return S_OK;
		}
		else
		{
			return E_NOINTERFACE;
		}
	}

	STDMETHODIMP InStream::Read(void *data, UInt32 size, UInt32* processedSize)
	{
		is_->read(data, size);
		if (processedSize)
		{
			*processedSize = static_cast<uint32_t>(is_->gcount());
		}

		return *is_ ? S_OK: E_FAIL;
	}

	STDMETHODIMP InStream::Seek(Int64 offset, uint32_t seekOrigin, UInt64* newPosition)
	{
		std::ios_base::seekdir way;
		switch (seekOrigin)
		{
		case 0:
			way = std::ios_base::beg;
			break;

		case 1:
			way = std::ios_base::cur;
			break;

		case 2:
			way = std::ios_base::end;
			break;

		default:
			return STG_E_INVALIDFUNCTION;
		}

		is_->seekg(static_cast<std::istream::off_type>(offset), way);
		if (newPosition)
		{
			*newPosition = is_->tellg();
		}

		return *is_ ? S_OK: E_FAIL;
	}

	STDMETHODIMP InStream::GetSize(UInt64* size)
	{
		*size = stream_size_;
		return S_OK;
	}


	OutStream::OutStream(std::shared_ptr<std::ostream> const & os)
		: os_(os)
	{
	}

	STDMETHODIMP_(ULONG) OutStream::AddRef()
	{
		++ ref_count_;
		return ref_count_;
	}

	STDMETHODIMP_(ULONG) OutStream::Release()
	{
		-- ref_count_;
		if (0 == ref_count_)
		{
			delete this;
			return 0;
		}
		return ref_count_;
	}

	STDMETHODIMP OutStream::QueryInterface(REFGUID iid, void** out_object)
	{
		if (IID_IOutStream == iid)
		{
			*out_object = static_cast<void*>(this);
			this->AddRef();
			return S_OK;
		}
		else
		{
			return E_NOINTERFACE;
		}
	}

	STDMETHODIMP OutStream::Write(void const * data, UInt32 size, UInt32* processed_size)
	{
		os_->write(static_cast<char const *>(data), size);
		if (processed_size)
		{
			*processed_size = size;
		}

		return *os_ ? S_OK: E_FAIL;
	}

	STDMETHODIMP OutStream::Seek(Int64 offset, UInt32 seek_origin, UInt64* new_position)
	{
		std::ios_base::seekdir way;
		switch (seek_origin)
		{
		case 0:
			way = std::ios_base::beg;
			break;

		case 1:
			way = std::ios_base::cur;
			break;

		case 2:
			way = std::ios_base::end;
			break;

		default:
			return STG_E_INVALIDFUNCTION;
		}

		os_->seekp(static_cast<std::ofstream::off_type>(offset), way);
		if (new_position)
		{
			*new_position = os_->tellp();
		}

		return *os_ ? S_OK: E_FAIL;
	}

	STDMETHODIMP OutStream::SetSize(UInt64 new_size)
	{
		KFL_UNUSED(new_size);
		return E_NOTIMPL;
	}
}
