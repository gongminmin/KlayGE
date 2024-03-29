/**
 * @file NullSource.cpp
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

#include "NullSource.hpp"

namespace KlayGE
{
	NullSource::NullSource()
	{
	}

	NullSource::~NullSource()
	{
	}

	void NullSource::Open([[maybe_unused]] ResIdentifierPtr const & file)
	{
		format_ = AF_Mono16;
		freq_ = 22050;

		this->Reset();
	}

	void NullSource::Close()
	{
		this->Reset();
	}

	size_t NullSource::Read([[maybe_unused]] void* data, [[maybe_unused]] size_t size)
	{
		return 0;
	}

	size_t NullSource::Size()
	{
		return 0;
	}

	void NullSource::Reset()
	{
	}
}
