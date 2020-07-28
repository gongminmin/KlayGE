/**
 * @file CustomizedStreamBuf.cpp
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

#include <KFL/KFL.hpp>

#include <cstring>

#include <boost/assert.hpp>

#include <KFL/CustomizedStreamBuf.hpp>

namespace KlayGE
{
	MemInputStreamBuf::MemInputStreamBuf(void const * p, std::streamsize num_bytes)
		: MemInputStreamBuf(p, static_cast<uint8_t const *>(p) + num_bytes)
	{
	}

	MemInputStreamBuf::MemInputStreamBuf(void const * begin, void const * end)
		: begin_(static_cast<char_type const *>(begin)), end_(static_cast<char_type const *>(end)),
			current_(begin_)
	{
		BOOST_ASSERT(begin_ <= end_);
	}

	MemInputStreamBuf::int_type MemInputStreamBuf::uflow()
	{
		if (current_ == end_)
		{
			return traits_type::eof();
		}

		char_type const * c = current_;
		++ current_;
		return traits_type::to_int_type(*c);
	}

	MemInputStreamBuf::int_type MemInputStreamBuf::underflow()
	{
		if (current_ == end_)
		{
			return traits_type::eof();
		}

		return traits_type::to_int_type(*current_);
	}

	std::streamsize MemInputStreamBuf::xsgetn(char_type* s, std::streamsize count)
	{
		if (current_ + count >= end_)
		{
			count = end_ - current_;
		}
		memcpy(s, current_, static_cast<size_t>(count * sizeof(char_type)));
		current_ += count;
		return count;
	}

	MemInputStreamBuf::int_type MemInputStreamBuf::pbackfail(int_type ch)
	{
		if ((current_ == begin_) || ((ch != traits_type::eof()) && (ch != current_[-1])))
		{
			return traits_type::eof();
		}

		-- current_;
		return traits_type::to_int_type(*current_);
	}

	std::streamsize MemInputStreamBuf::showmanyc()
	{
		BOOST_ASSERT(current_ <= end_);
		return end_ - current_;
	}

	MemInputStreamBuf::pos_type MemInputStreamBuf::seekoff(off_type off, std::ios_base::seekdir way,
			std::ios_base::openmode which)
	{
		BOOST_ASSERT(which == std::ios_base::in);
		KFL_UNUSED(which);

		switch (way)
		{
		case std::ios_base::beg:
			if (off <= end_ - begin_)
			{
				current_ = begin_ + off;
			}
			else
			{
				off = -1;
			}
			break;

		case std::ios_base::end:
			if (end_ - off >= begin_)
			{
				current_ = end_ - off;
				off = current_ - begin_;
			}
			else
			{
				off = -1;
			}
			break;

		case std::ios_base::cur:
		default:
			if (current_ + off <= end_)
			{
				current_ += off;
				off = current_ - begin_;
			}
			else
			{
				off = -1;
			}
			break;
		}

		return off;
	}

	MemInputStreamBuf::pos_type MemInputStreamBuf::seekpos(pos_type sp, std::ios_base::openmode which)
	{
		BOOST_ASSERT(which == std::ios_base::in);
		KFL_UNUSED(which);

		if (sp < end_ - begin_)
		{
			current_ = begin_ + static_cast<int>(sp);
		}
		else
		{
			sp = -1;
		}

		return sp;
	}


	VectorStreamCallback::VectorStreamCallback(std::vector<std::streambuf::char_type>& data)
		: data_(data)
	{
	}

	VectorStreamCallback::VectorStreamCallback(VectorStreamCallback&& rhs) noexcept
		: data_(rhs.data_)
	{
	}

	std::streambuf::int_type VectorStreamCallback::operator()(void const * buff, std::streamsize count)
	{
		auto const * p = static_cast<std::streambuf::char_type const *>(buff);
		data_.insert(data_.end(), p, p + count);
		return static_cast<std::streambuf::int_type>(count);
	}


	VectorOutputStreamBuf::VectorOutputStreamBuf(std::vector<char_type>& data)
		: CallbackOutputStreamBuf<VectorStreamCallback>(VectorStreamCallback(data))
	{
	}
	
	
	StringStreamCallback::StringStreamCallback(std::basic_string<std::streambuf::char_type>& data)
		: data_(data)
	{
	}

	StringStreamCallback::StringStreamCallback(StringStreamCallback&& rhs) noexcept
		: data_(rhs.data_)
	{
	}

	std::streambuf::int_type StringStreamCallback::operator()(void const * buff, std::streamsize count)
	{
		auto const * p = static_cast<std::streambuf::char_type const *>(buff);
		data_.insert(data_.end(), p, p + count);
		return static_cast<std::streambuf::int_type>(count);
	}


	StringOutputStreamBuf::StringOutputStreamBuf(std::basic_string<char_type>& data)
		: CallbackOutputStreamBuf<StringStreamCallback>(StringStreamCallback(data))
	{
	}
}
