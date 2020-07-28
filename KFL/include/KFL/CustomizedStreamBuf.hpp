/**
 * @file CustomizedStreamBuf.hpp
 * @author Minmin Gong
 *
 * @section DESCRIPTION
 *
 * This source file is part of KFL, a subproject of KlayGE
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

#ifndef _KFL_CUSTOMIZEDSTREAMBUF_HPP
#define _KFL_CUSTOMIZEDSTREAMBUF_HPP

#pragma once

#include <streambuf>
#include <vector>
#include <string>
#include <boost/noncopyable.hpp>

namespace KlayGE
{
	class MemInputStreamBuf : public std::streambuf, boost::noncopyable
	{
	public:
		MemInputStreamBuf(void const * p, std::streamsize num_bytes);
		MemInputStreamBuf(void const * begin, void const * end);

	protected:
		int_type uflow() override;
		int_type underflow() override;

		std::streamsize xsgetn(char_type* s, std::streamsize count) override;

		int_type pbackfail(int_type ch) override;
		std::streamsize showmanyc() override;

		pos_type seekoff(off_type off, std::ios_base::seekdir way, std::ios_base::openmode which) override;
		pos_type seekpos(pos_type sp, std::ios_base::openmode which) override;

	private:
		char_type const * const begin_;
		char_type const * const end_;
		char_type const * current_;
	};

	template <typename Callback>
	class CallbackOutputStreamBuf : public std::streambuf, boost::noncopyable
	{
	public:
		explicit CallbackOutputStreamBuf(Callback const & cb)
			: cb_(cb)
		{
		}

		explicit CallbackOutputStreamBuf(Callback&& cb)
			: cb_(std::move(cb))
		{
		}

	protected:
		std::streamsize xsputn(char_type const * s, std::streamsize count) override
		{
			return cb_(s, count);
		}

		int_type overflow(int_type ch = traits_type::eof()) override
		{
			return cb_(&ch, 1);
		}

	private:
		Callback cb_;
	};


	class VectorStreamCallback final : boost::noncopyable
	{
	public:
		explicit VectorStreamCallback(std::vector<std::streambuf::char_type>& data);
		VectorStreamCallback(VectorStreamCallback&& rhs) noexcept;

		std::streambuf::int_type operator()(void const * buff, std::streamsize count);

	private:
		std::vector<std::streambuf::char_type>& data_;
	};

	class VectorOutputStreamBuf final : public CallbackOutputStreamBuf<VectorStreamCallback>
	{
	public:
		explicit VectorOutputStreamBuf(std::vector<char_type>& data);
	};

	class StringStreamCallback final : boost::noncopyable
	{
	public:
		explicit StringStreamCallback(std::basic_string<std::streambuf::char_type>& data);
		StringStreamCallback(StringStreamCallback&& rhs) noexcept;

		std::streambuf::int_type operator()(void const * buff, std::streamsize count);

	private:
		std::basic_string<std::streambuf::char_type>& data_;
	};

	class StringOutputStreamBuf final : public CallbackOutputStreamBuf<StringStreamCallback>
	{
	public:
		explicit StringOutputStreamBuf(std::basic_string<char_type>& data);
	};
}

#endif		// _KFL_CUSTOMIZEDSTREAMBUF_HPP
