/**
 * @file ResIdentifier.hpp
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

#ifndef _KFL_RESIDENTIFIER_HPP
#define _KFL_RESIDENTIFIER_HPP

#pragma once

#include <KFL/PreDeclare.hpp>
#include <istream>
#include <string>
#include <string_view>
#include <vector>

namespace KlayGE
{
	class ResIdentifier final
	{
	public:
		ResIdentifier(std::string_view name, uint64_t timestamp,
				std::shared_ptr<std::istream> const & is)
			: ResIdentifier(name, timestamp, is, std::shared_ptr<std::streambuf>())
		{
		}
		ResIdentifier(std::string_view name, uint64_t timestamp,
				std::shared_ptr<std::istream> const & is, std::shared_ptr<std::streambuf> const & streambuf)
			: res_name_(name), timestamp_(timestamp), istream_(is), streambuf_(streambuf)
		{
		}

		void ResName(std::string_view name)
		{
			res_name_ = std::string(name);
		}
		std::string const & ResName() const
		{
			return res_name_;
		}

		void Timestamp(uint64_t ts)
		{
			timestamp_ = ts;
		}
		uint64_t Timestamp() const
		{
			return timestamp_;
		}

		void read(void* p, size_t size)
		{
			istream_->read(static_cast<char*>(p), static_cast<std::streamsize>(size));
		}

		int64_t gcount() const
		{
			return static_cast<int64_t>(istream_->gcount());
		}

		void seekg(int64_t offset, std::ios_base::seekdir way)
		{
			istream_->seekg(static_cast<std::istream::off_type>(offset), way);
		}

		int64_t tellg()
		{
			return static_cast<int64_t>(istream_->tellg());
		}

		void clear()
		{
			istream_->clear();
		}

		operator bool() const
		{
			return !istream_->fail();
		}

		bool operator!() const
		{
			return istream_->operator!();
		}

		std::istream& input_stream()
		{
			return *istream_;
		}

	private:
		std::string res_name_;
		uint64_t timestamp_;
		std::shared_ptr<std::istream> istream_;
		std::shared_ptr<std::streambuf> streambuf_;
	};
}

#endif			// _KFL_RESIDENTIFIER_HPP
