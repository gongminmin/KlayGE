/**
 * @file Streams.hpp
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

#ifndef _KFL_STREAMS_HPP
#define _KFL_STREAMS_HPP

#pragma once

#include <KlayGE/PreDeclare.hpp>

#include <fstream>
#include <string>
#include <atomic>

#include <CPP/7zip/IStream.h>

namespace KlayGE
{
	class CInStream : boost::noncopyable, public IInStream, public IStreamGetSize
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
			if (IID_IInStream == iid)
			{
				*outObject = static_cast<IInStream*>(this);
				this->AddRef();
				return S_OK;
			}
			else
			{
				if (IID_IStreamGetSize == iid)
				{
					*outObject = static_cast<IStreamGetSize*>(this);
					this->AddRef();
					return S_OK;
				}
				else
				{
					return E_NOINTERFACE;
				}
			}
		}


		CInStream()
			: ref_count_(1),
				stream_size_(0)
		{
		}
		virtual ~CInStream()
		{
		}

		void Attach(ResIdentifierPtr const & is);

		STDMETHOD(Read)(void* data, UInt32 size, UInt32* processedSize);
		STDMETHOD(Seek)(Int64 offset, UInt32 seekOrigin, UInt64* newPosition);

		STDMETHOD(GetSize)(UInt64* size);

	private:
		std::atomic<int32_t> ref_count_;

		ResIdentifierPtr is_;
		uint64_t stream_size_;
	};

	class COutStream : boost::noncopyable, public IOutStream
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
			if (IID_IOutStream == iid)
			{
				*outObject = static_cast<void*>(this);
				this->AddRef();
				return S_OK;
			}
			else
			{
				return E_NOINTERFACE;
			}
		}

		COutStream()
			: ref_count_(1)
		{
		}
		virtual ~COutStream()
		{
		}

		void Attach(std::shared_ptr<std::ostream> const & os);

		STDMETHOD(Write)(const void* data, UInt32 size, UInt32* processedSize);
		STDMETHOD(Seek)(Int64 offset, UInt32 seekOrigin, UInt64* newPosition);
		STDMETHOD(SetSize)(UInt64 newSize);

	private:
		std::atomic<int32_t> ref_count_;

		std::shared_ptr<std::ostream> os_;
	};
}

#endif		// _KFL_STREAMS_HPP
