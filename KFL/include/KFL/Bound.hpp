/**
 * @file Bound.hpp
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

#ifndef _KFL_BOUND_HPP
#define _KFL_BOUND_HPP

#pragma once

namespace KlayGE
{
	template <typename T>
	class Bound_T
	{
	public:
		virtual ~Bound_T() noexcept = default;

		virtual bool IsEmpty() const noexcept = 0;

		virtual bool VecInBound(Vector_T<T, 3> const & v) const noexcept = 0;
		virtual T MaxRadiusSq() const noexcept = 0;
	};

	using Bound = Bound_T<float>;
}

#endif			// _KFL_BOUND_HPP
