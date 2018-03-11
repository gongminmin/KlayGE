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

#ifndef KLAYGE_CORE_ARCHIVE_OPEN_CALLBACK_HPP
#define KLAYGE_CORE_ARCHIVE_OPEN_CALLBACK_HPP

#pragma once

#include <atomic>
#include <string>

#include <CPP/7zip/Archive/IArchive.h>
#include <CPP/7zip/IPassword.h>

namespace KlayGE
{
	class ArchiveOpenCallback : boost::noncopyable, public IArchiveOpenCallback, public ICryptoGetTextPassword
	{
	public:
		// IUnknown
		STDMETHOD_(ULONG, AddRef)();
		STDMETHOD_(ULONG, Release)();
		STDMETHOD(QueryInterface)(REFGUID iid, void** out_object);

		// IArchiveOpenCallback
		STDMETHOD(SetTotal)(UInt64 const * files, UInt64 const * bytes);
		STDMETHOD(SetCompleted)(UInt64 const * files, UInt64 const * bytes);

		// ICryptoGetTextPassword
		STDMETHOD(CryptoGetTextPassword)(BSTR *password);

	public:
		explicit ArchiveOpenCallback(std::string_view pw);
		virtual ~ArchiveOpenCallback() = default;

	private:
		std::atomic<int32_t> ref_count_ = 1;

		bool password_is_defined_ = false;
		std::wstring password_;
	};
}

#endif		// KLAYGE_CORE_ARCHIVE_OPEN_CALLBACK_HPP
