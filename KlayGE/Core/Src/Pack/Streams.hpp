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

#ifndef KLAYGE_CORE_STREAMS_HPP
#define KLAYGE_CORE_STREAMS_HPP

#pragma once

#include <KlayGE/PreDeclare.hpp>

#include <atomic>
#include <fstream>
#include <string>

#include <CPP/7zip/IStream.h>

namespace KlayGE
{
	class InStream : boost::noncopyable, public IInStream, public IStreamGetSize
	{
	public:
		// IUnknown
		STDMETHOD_(ULONG, AddRef)();
		STDMETHOD_(ULONG, Release)();
		STDMETHOD(QueryInterface)(REFGUID iid, void** out_object);

		// IInStream
		STDMETHOD(Read)(void* data, UInt32 size, UInt32* processed_size);
		STDMETHOD(Seek)(Int64 offset, UInt32 seek_origin, UInt64* new_position);

		// IStreamGetSize
		STDMETHOD(GetSize)(UInt64* size);

	public:
		explicit InStream(ResIdentifierPtr const & is);
		virtual ~InStream() = default;

	private:
		std::atomic<int32_t> ref_count_ = 1;

		ResIdentifierPtr is_;
		uint64_t stream_size_ = 0;
	};

	class OutStream : boost::noncopyable, public IOutStream
	{
	public:
		// IUnknown
		STDMETHOD_(ULONG, AddRef)();
		STDMETHOD_(ULONG, Release)();
		STDMETHOD(QueryInterface)(REFGUID iid, void** out_object);

		// IOutStream
		STDMETHOD(Write)(void const * data, UInt32 size, UInt32* processed_size);
		STDMETHOD(Seek)(Int64 offset, UInt32 seek_origin, UInt64* new_position);
		STDMETHOD(SetSize)(UInt64 new_size);

	public:
		explicit OutStream(std::shared_ptr<std::ostream> const & os);
		virtual ~OutStream() = default;

	private:
		std::atomic<int32_t> ref_count_ = 1;

		std::shared_ptr<std::ostream> os_;
	};
}

#endif		// KLAYGE_CORE_STREAMS_HPP
