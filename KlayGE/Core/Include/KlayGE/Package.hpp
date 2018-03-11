/**
 * @file Package.hpp
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

#ifndef KLAYGE_CORE_PACKAGE_HPP
#define KLAYGE_CORE_PACKAGE_HPP

#pragma once

#include <KlayGE/PreDeclare.hpp>
#include <KFL/CXX17/string_view.hpp>

struct IInArchive;

namespace KlayGE
{
	class KLAYGE_CORE_API Package
	{
	public:
		explicit Package(ResIdentifierPtr const & archive_is);
		Package(ResIdentifierPtr const & archive_is, std::string_view password);

		bool Locate(std::string_view extract_file_path);
		ResIdentifierPtr Extract(std::string_view extract_file_path, std::string_view res_name);

		ResIdentifier* ArchiveStream() const
		{
			return archive_is_.get();
		}

	private:
		uint32_t Find(std::string_view extract_file_path);

	private:
		ResIdentifierPtr archive_is_;

		std::shared_ptr<IInArchive> archive_;
		std::string password_;

		uint32_t num_items_;
	};
}

#endif		// KLAYGE_CORE_PACKAGE_HPP
