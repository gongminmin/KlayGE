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
	MemStreamBuf::MemStreamBuf(void const * begin, void const * end)
		: begin_(static_cast<char_type const *>(begin)), end_(static_cast<char_type const *>(end)),
			current_(begin_)
	{
		BOOST_ASSERT(begin_ <= end_);
	}
	
	MemStreamBuf::int_type MemStreamBuf::uflow()
	{
		if (current_ == end_)
		{
			return traits_type::eof();
		}

		char_type const * c = current_;
		++ current_;
		return *c;
	}

	MemStreamBuf::int_type MemStreamBuf::underflow()
	{
		if (current_ == end_)
		{
			return traits_type::eof();
		}

		return *current_;
	}

	std::streamsize MemStreamBuf::xsgetn(char_type* s, std::streamsize count)
	{
		if (current_ + count >= end_)
		{
			count = end_ - current_;
		}
		memcpy(s, current_, static_cast<size_t>(count * sizeof(char_type)));
		current_ += count;
		return count;
	}

	MemStreamBuf::int_type MemStreamBuf::pbackfail(int_type ch)
	{
		if ((current_ == begin_) || ((ch != traits_type::eof()) && (ch != current_[-1])))
		{
			return traits_type::eof();
		}

		-- current_;
		return *current_;
	}
	
	std::streamsize MemStreamBuf::showmanyc()
	{
		BOOST_ASSERT(current_ <= end_);
		return end_ - current_;
	}

	MemStreamBuf::pos_type MemStreamBuf::seekoff(off_type off, std::ios_base::seekdir way,
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

	MemStreamBuf::pos_type MemStreamBuf::seekpos(pos_type sp, std::ios_base::openmode which)
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
}
