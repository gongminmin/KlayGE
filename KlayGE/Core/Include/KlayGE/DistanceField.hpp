/**
 * @file DistanceField.hpp
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

#ifndef _KLAYGE_DISTANCE_FIELD_HPP
#define _KLAYGE_DISTANCE_FIELD_HPP

#pragma once

#include <KlayGE/PreDeclare.hpp>

#include <vector>

namespace KlayGE
{
	template <typename T>
	KLAYGE_CORE_API void Downsample2x(std::vector<T> const & input_data, uint32_t input_width, uint32_t input_height,
		std::vector<T>& output_data);

	KLAYGE_CORE_API void ComputeDistance(std::vector<float> const & aa_2x_data, uint32_t input_width, uint32_t input_height,
		std::vector<float>& dist_data);
}

#endif		// _KLAYGE_DISTANCE_FIELD_HPP
