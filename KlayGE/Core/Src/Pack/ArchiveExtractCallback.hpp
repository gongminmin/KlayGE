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

#ifndef KLAYGE_CORE_ARCHIVE_EXTRACT_CALLBACK_HPP
#define KLAYGE_CORE_ARCHIVE_EXTRACT_CALLBACK_HPP

#pragma once

#include <atomic>
#include <string>

#include <CPP/7zip/Archive/IArchive.h>
#include <CPP/7zip/IPassword.h>

namespace KlayGE
{
	class ArchiveExtractCallback : boost::noncopyable, public IArchiveExtractCallback, public ICryptoGetTextPassword
	{
	public:
		// IUnknown
		STDMETHOD_(ULONG, AddRef)();
		STDMETHOD_(ULONG, Release)();
		STDMETHOD(QueryInterface)(REFGUID iid, void** out_object);

		// IProgress
		STDMETHOD(SetTotal)(UInt64 size);
		STDMETHOD(SetCompleted)(UInt64 const * complete_value);

		// IExtractCallBack
		STDMETHOD(GetStream)(UInt32 index, ISequentialOutStream** out_stream, Int32 ask_extract_mode);
		STDMETHOD(PrepareOperation)(Int32 ask_extract_mode);
		STDMETHOD(SetOperationResult)(Int32 operation_result);

		// ICryptoGetTextPassword
		STDMETHOD(CryptoGetTextPassword)(BSTR* password);

	public:
		ArchiveExtractCallback(std::string_view pw, std::shared_ptr<ISequentialOutStream> const & out_file_stream);
		virtual ~ArchiveExtractCallback() = default;

	private:
		std::atomic<int32_t> ref_count_ = 1;

		bool password_is_defined_;
		std::wstring password_;

		std::shared_ptr<ISequentialOutStream> out_file_stream_;
	};
}

#endif		// KLAYGE_CORE_ARCHIVE_EXTRACT_CALLBACK_HPP
