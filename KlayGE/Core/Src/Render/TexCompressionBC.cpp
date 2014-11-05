/**
* @file TexCompressionBC.cpp
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
#include <KlayGE/Context.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KFL/Color.hpp>
#include <KlayGE/Texture.hpp>
#include <KFL/Thread.hpp>
#include <KFL/Half.hpp>

#include <vector>
#include <cstring>
#include <boost/assert.hpp>

#include <KlayGE/TexCompressionBC.hpp>

namespace
{
	using namespace KlayGE;

	mutex singleton_mutex;

	int const bc67_prec_weights[][16] =
	{
		{ 0, 21, 43, 64 },
		{ 0, 9, 18, 27, 37, 46, 55, 64 },
		{ 0, 4, 9, 13, 17, 21, 26, 30, 34, 38, 43, 47, 51, 55, 60, 64 }
	};

	// Partition, Shape, Pixel (index into 4x4 block)
	uint8_t const bc67_partition_table[3][64][16] =
	{
		{   // 1 Region case has no subsets (all 0)
			{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
			{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
			{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
			{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
			{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
			{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
			{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
			{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
			{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
			{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
			{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
			{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
			{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
			{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
			{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
			{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
			{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
			{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
			{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
			{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
			{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
			{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
			{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
			{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
			{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
			{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
			{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
			{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
			{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
			{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
			{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
			{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
			{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
			{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
			{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
			{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
			{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
			{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
			{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
			{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
			{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
			{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
			{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
			{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
			{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
			{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
			{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
			{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
			{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
			{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
			{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
			{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
			{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
			{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
			{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
			{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
			{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
			{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
			{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
			{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
			{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
			{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
			{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
			{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
		},

		{   // BC6H/BC7 Partition Set for 2 Subsets
			{ 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1 }, // Shape 0
			{ 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1 }, // Shape 1
			{ 0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1 }, // Shape 2
			{ 0, 0, 0, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 1, 1, 1 }, // Shape 3
			{ 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 1, 1 }, // Shape 4
			{ 0, 0, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1 }, // Shape 5
			{ 0, 0, 0, 1, 0, 0, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1 }, // Shape 6
			{ 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 1, 0, 1, 1, 1 }, // Shape 7
			{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 1 }, // Shape 8
			{ 0, 0, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 }, // Shape 9
			{ 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 1, 1, 1, 1, 1, 1 }, // Shape 10
			{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 1, 1 }, // Shape 11
			{ 0, 0, 0, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 }, // Shape 12
			{ 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1 }, // Shape 13
			{ 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 }, // Shape 14
			{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1 }, // Shape 15
			{ 0, 0, 0, 0, 1, 0, 0, 0, 1, 1, 1, 0, 1, 1, 1, 1 }, // Shape 16
			{ 0, 1, 1, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0 }, // Shape 17
			{ 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 1, 1, 0 }, // Shape 18
			{ 0, 1, 1, 1, 0, 0, 1, 1, 0, 0, 0, 1, 0, 0, 0, 0 }, // Shape 19
			{ 0, 0, 1, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0 }, // Shape 20
			{ 0, 0, 0, 0, 1, 0, 0, 0, 1, 1, 0, 0, 1, 1, 1, 0 }, // Shape 21
			{ 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 1, 0, 0 }, // Shape 22
			{ 0, 1, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0, 1 }, // Shape 23
			{ 0, 0, 1, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0 }, // Shape 24
			{ 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 1, 0, 0 }, // Shape 25
			{ 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0 }, // Shape 26
			{ 0, 0, 1, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 1, 0, 0 }, // Shape 27
			{ 0, 0, 0, 1, 0, 1, 1, 1, 1, 1, 1, 0, 1, 0, 0, 0 }, // Shape 28
			{ 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0 }, // Shape 29
			{ 0, 1, 1, 1, 0, 0, 0, 1, 1, 0, 0, 0, 1, 1, 1, 0 }, // Shape 30
			{ 0, 0, 1, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 1, 0, 0 }, // Shape 31

			// BC7 Partition Set for 2 Subsets (second-half)
			{ 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1 }, // Shape 32
			{ 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1 }, // Shape 33
			{ 0, 1, 0, 1, 1, 0, 1, 0, 0, 1, 0, 1, 1, 0, 1, 0 }, // Shape 34
			{ 0, 0, 1, 1, 0, 0, 1, 1, 1, 1, 0, 0, 1, 1, 0, 0 }, // Shape 35
			{ 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0 }, // Shape 36
			{ 0, 1, 0, 1, 0, 1, 0, 1, 1, 0, 1, 0, 1, 0, 1, 0 }, // Shape 37
			{ 0, 1, 1, 0, 1, 0, 0, 1, 0, 1, 1, 0, 1, 0, 0, 1 }, // Shape 38
			{ 0, 1, 0, 1, 1, 0, 1, 0, 1, 0, 1, 0, 0, 1, 0, 1 }, // Shape 39
			{ 0, 1, 1, 1, 0, 0, 1, 1, 1, 1, 0, 0, 1, 1, 1, 0 }, // Shape 40
			{ 0, 0, 0, 1, 0, 0, 1, 1, 1, 1, 0, 0, 1, 0, 0, 0 }, // Shape 41
			{ 0, 0, 1, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 1, 0, 0 }, // Shape 42
			{ 0, 0, 1, 1, 1, 0, 1, 1, 1, 1, 0, 1, 1, 1, 0, 0 }, // Shape 43
			{ 0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0 }, // Shape 44
			{ 0, 0, 1, 1, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0, 1, 1 }, // Shape 45
			{ 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1 }, // Shape 46
			{ 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0, 0 }, // Shape 47
			{ 0, 1, 0, 0, 1, 1, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0 }, // Shape 48
			{ 0, 0, 1, 0, 0, 1, 1, 1, 0, 0, 1, 0, 0, 0, 0, 0 }, // Shape 49
			{ 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 1, 1, 0, 0, 1, 0 }, // Shape 50
			{ 0, 0, 0, 0, 0, 1, 0, 0, 1, 1, 1, 0, 0, 1, 0, 0 }, // Shape 51
			{ 0, 1, 1, 0, 1, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 1 }, // Shape 52
			{ 0, 0, 1, 1, 0, 1, 1, 0, 1, 1, 0, 0, 1, 0, 0, 1 }, // Shape 53
			{ 0, 1, 1, 0, 0, 0, 1, 1, 1, 0, 0, 1, 1, 1, 0, 0 }, // Shape 54
			{ 0, 0, 1, 1, 1, 0, 0, 1, 1, 1, 0, 0, 0, 1, 1, 0 }, // Shape 55
			{ 0, 1, 1, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 0, 0, 1 }, // Shape 56
			{ 0, 1, 1, 0, 0, 0, 1, 1, 0, 0, 1, 1, 1, 0, 0, 1 }, // Shape 57
			{ 0, 1, 1, 1, 1, 1, 1, 0, 1, 0, 0, 0, 0, 0, 0, 1 }, // Shape 58
			{ 0, 0, 0, 1, 1, 0, 0, 0, 1, 1, 1, 0, 0, 1, 1, 1 }, // Shape 59
			{ 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1 }, // Shape 60
			{ 0, 0, 1, 1, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0 }, // Shape 61
			{ 0, 0, 1, 0, 0, 0, 1, 0, 1, 1, 1, 0, 1, 1, 1, 0 }, // Shape 62
			{ 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 1, 1, 0, 1, 1, 1 }  // Shape 63
		},

		{   // BC7 Partition Set for 3 Subsets
			{ 0, 0, 1, 1, 0, 0, 1, 1, 0, 2, 2, 1, 2, 2, 2, 2 }, // Shape 0
			{ 0, 0, 0, 1, 0, 0, 1, 1, 2, 2, 1, 1, 2, 2, 2, 1 }, // Shape 1
			{ 0, 0, 0, 0, 2, 0, 0, 1, 2, 2, 1, 1, 2, 2, 1, 1 }, // Shape 2
			{ 0, 2, 2, 2, 0, 0, 2, 2, 0, 0, 1, 1, 0, 1, 1, 1 }, // Shape 3
			{ 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 2, 2, 1, 1, 2, 2 }, // Shape 4
			{ 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 2, 2, 0, 0, 2, 2 }, // Shape 5
			{ 0, 0, 2, 2, 0, 0, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1 }, // Shape 6
			{ 0, 0, 1, 1, 0, 0, 1, 1, 2, 2, 1, 1, 2, 2, 1, 1 }, // Shape 7
			{ 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2 }, // Shape 8
			{ 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2 }, // Shape 9
			{ 0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2 }, // Shape 10
			{ 0, 0, 1, 2, 0, 0, 1, 2, 0, 0, 1, 2, 0, 0, 1, 2 }, // Shape 11
			{ 0, 1, 1, 2, 0, 1, 1, 2, 0, 1, 1, 2, 0, 1, 1, 2 }, // Shape 12
			{ 0, 1, 2, 2, 0, 1, 2, 2, 0, 1, 2, 2, 0, 1, 2, 2 }, // Shape 13
			{ 0, 0, 1, 1, 0, 1, 1, 2, 1, 1, 2, 2, 1, 2, 2, 2 }, // Shape 14
			{ 0, 0, 1, 1, 2, 0, 0, 1, 2, 2, 0, 0, 2, 2, 2, 0 }, // Shape 15
			{ 0, 0, 0, 1, 0, 0, 1, 1, 0, 1, 1, 2, 1, 1, 2, 2 }, // Shape 16
			{ 0, 1, 1, 1, 0, 0, 1, 1, 2, 0, 0, 1, 2, 2, 0, 0 }, // Shape 17
			{ 0, 0, 0, 0, 1, 1, 2, 2, 1, 1, 2, 2, 1, 1, 2, 2 }, // Shape 18
			{ 0, 0, 2, 2, 0, 0, 2, 2, 0, 0, 2, 2, 1, 1, 1, 1 }, // Shape 19
			{ 0, 1, 1, 1, 0, 1, 1, 1, 0, 2, 2, 2, 0, 2, 2, 2 }, // Shape 20
			{ 0, 0, 0, 1, 0, 0, 0, 1, 2, 2, 2, 1, 2, 2, 2, 1 }, // Shape 21
			{ 0, 0, 0, 0, 0, 0, 1, 1, 0, 1, 2, 2, 0, 1, 2, 2 }, // Shape 22
			{ 0, 0, 0, 0, 1, 1, 0, 0, 2, 2, 1, 0, 2, 2, 1, 0 }, // Shape 23
			{ 0, 1, 2, 2, 0, 1, 2, 2, 0, 0, 1, 1, 0, 0, 0, 0 }, // Shape 24
			{ 0, 0, 1, 2, 0, 0, 1, 2, 1, 1, 2, 2, 2, 2, 2, 2 }, // Shape 25
			{ 0, 1, 1, 0, 1, 2, 2, 1, 1, 2, 2, 1, 0, 1, 1, 0 }, // Shape 26
			{ 0, 0, 0, 0, 0, 1, 1, 0, 1, 2, 2, 1, 1, 2, 2, 1 }, // Shape 27
			{ 0, 0, 2, 2, 1, 1, 0, 2, 1, 1, 0, 2, 0, 0, 2, 2 }, // Shape 28
			{ 0, 1, 1, 0, 0, 1, 1, 0, 2, 0, 0, 2, 2, 2, 2, 2 }, // Shape 29
			{ 0, 0, 1, 1, 0, 1, 2, 2, 0, 1, 2, 2, 0, 0, 1, 1 }, // Shape 30
			{ 0, 0, 0, 0, 2, 0, 0, 0, 2, 2, 1, 1, 2, 2, 2, 1 }, // Shape 31
			{ 0, 0, 0, 0, 0, 0, 0, 2, 1, 1, 2, 2, 1, 2, 2, 2 }, // Shape 32
			{ 0, 2, 2, 2, 0, 0, 2, 2, 0, 0, 1, 2, 0, 0, 1, 1 }, // Shape 33
			{ 0, 0, 1, 1, 0, 0, 1, 2, 0, 0, 2, 2, 0, 2, 2, 2 }, // Shape 34
			{ 0, 1, 2, 0, 0, 1, 2, 0, 0, 1, 2, 0, 0, 1, 2, 0 }, // Shape 35
			{ 0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2, 0, 0, 0, 0 }, // Shape 36
			{ 0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0 }, // Shape 37
			{ 0, 1, 2, 0, 2, 0, 1, 2, 1, 2, 0, 1, 0, 1, 2, 0 }, // Shape 38
			{ 0, 0, 1, 1, 2, 2, 0, 0, 1, 1, 2, 2, 0, 0, 1, 1 }, // Shape 39
			{ 0, 0, 1, 1, 1, 1, 2, 2, 2, 2, 0, 0, 0, 0, 1, 1 }, // Shape 40
			{ 0, 1, 0, 1, 0, 1, 0, 1, 2, 2, 2, 2, 2, 2, 2, 2 }, // Shape 41
			{ 0, 0, 0, 0, 0, 0, 0, 0, 2, 1, 2, 1, 2, 1, 2, 1 }, // Shape 42
			{ 0, 0, 2, 2, 1, 1, 2, 2, 0, 0, 2, 2, 1, 1, 2, 2 }, // Shape 43
			{ 0, 0, 2, 2, 0, 0, 1, 1, 0, 0, 2, 2, 0, 0, 1, 1 }, // Shape 44
			{ 0, 2, 2, 0, 1, 2, 2, 1, 0, 2, 2, 0, 1, 2, 2, 1 }, // Shape 45
			{ 0, 1, 0, 1, 2, 2, 2, 2, 2, 2, 2, 2, 0, 1, 0, 1 }, // Shape 46
			{ 0, 0, 0, 0, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1 }, // Shape 47
			{ 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 2, 2, 2, 2 }, // Shape 48
			{ 0, 2, 2, 2, 0, 1, 1, 1, 0, 2, 2, 2, 0, 1, 1, 1 }, // Shape 49
			{ 0, 0, 0, 2, 1, 1, 1, 2, 0, 0, 0, 2, 1, 1, 1, 2 }, // Shape 50
			{ 0, 0, 0, 0, 2, 1, 1, 2, 2, 1, 1, 2, 2, 1, 1, 2 }, // Shape 51
			{ 0, 2, 2, 2, 0, 1, 1, 1, 0, 1, 1, 1, 0, 2, 2, 2 }, // Shape 52
			{ 0, 0, 0, 2, 1, 1, 1, 2, 1, 1, 1, 2, 0, 0, 0, 2 }, // Shape 53
			{ 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 2, 2, 2, 2 }, // Shape 54
			{ 0, 0, 0, 0, 0, 0, 0, 0, 2, 1, 1, 2, 2, 1, 1, 2 }, // Shape 55
			{ 0, 1, 1, 0, 0, 1, 1, 0, 2, 2, 2, 2, 2, 2, 2, 2 }, // Shape 56
			{ 0, 0, 2, 2, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 2, 2 }, // Shape 57
			{ 0, 0, 2, 2, 1, 1, 2, 2, 1, 1, 2, 2, 0, 0, 2, 2 }, // Shape 58
			{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 1, 1, 2 }, // Shape 59
			{ 0, 0, 0, 2, 0, 0, 0, 1, 0, 0, 0, 2, 0, 0, 0, 1 }, // Shape 60
			{ 0, 2, 2, 2, 1, 2, 2, 2, 0, 2, 2, 2, 1, 2, 2, 2 }, // Shape 61
			{ 0, 1, 0, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2 }, // Shape 62
			{ 0, 1, 1, 1, 2, 0, 1, 1, 2, 2, 0, 1, 2, 2, 2, 0 }  // Shape 63
		}
	};

	// Partition, Shape, Fixup
	uint8_t const fix_up_table[3][64][3] =
	{
		{   // No fix-ups for 1st subset for BC6H or BC7
			{ 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 },
			{ 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 },
			{ 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 },
			{ 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 },
			{ 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 },
			{ 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 },
			{ 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 },
			{ 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 },
			{ 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 },
			{ 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 },
			{ 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 },
			{ 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 },
			{ 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 },
			{ 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 },
			{ 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 },
			{ 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }
		},

		{   // BC6H/BC7 Partition Set Fixups for 2 Subsets
			{ 0, 15, 0 }, { 0, 15, 0 }, { 0, 15, 0 }, { 0, 15, 0 },
			{ 0, 15, 0 }, { 0, 15, 0 }, { 0, 15, 0 }, { 0, 15, 0 },
			{ 0, 15, 0 }, { 0, 15, 0 }, { 0, 15, 0 }, { 0, 15, 0 },
			{ 0, 15, 0 }, { 0, 15, 0 }, { 0, 15, 0 }, { 0, 15, 0 },
			{ 0, 15, 0 }, { 0, 2, 0 }, { 0, 8, 0 }, { 0, 2, 0 },
			{ 0, 2, 0 }, { 0, 8, 0 }, { 0, 8, 0 }, { 0, 15, 0 },
			{ 0, 2, 0 }, { 0, 8, 0 }, { 0, 2, 0 }, { 0, 2, 0 },
			{ 0, 8, 0 }, { 0, 8, 0 }, { 0, 2, 0 }, { 0, 2, 0 },

			// BC7 Partition Set Fixups for 2 Subsets (second-half)
			{ 0, 15, 0 }, { 0, 15, 0 }, { 0, 6, 0 }, { 0, 8, 0 },
			{ 0, 2, 0 }, { 0, 8, 0 }, { 0, 15, 0 }, { 0, 15, 0 },
			{ 0, 2, 0 }, { 0, 8, 0 }, { 0, 2, 0 }, { 0, 2, 0 },
			{ 0, 2, 0 }, { 0, 15, 0 }, { 0, 15, 0 }, { 0, 6, 0 },
			{ 0, 6, 0 }, { 0, 2, 0 }, { 0, 6, 0 }, { 0, 8, 0 },
			{ 0, 15, 0 }, { 0, 15, 0 }, { 0, 2, 0 }, { 0, 2, 0 },
			{ 0, 15, 0 }, { 0, 15, 0 }, { 0, 15, 0 }, { 0, 15, 0 },
			{ 0, 15, 0 }, { 0, 2, 0 }, { 0, 2, 0 }, { 0, 15, 0 }
		},

		{   // BC7 Partition Set Fixups for 3 Subsets
			{ 0, 3, 15 }, { 0, 3, 8 }, { 0, 15, 8 }, { 0, 15, 3 },
			{ 0, 8, 15 }, { 0, 3, 15 }, { 0, 15, 3 }, { 0, 15, 8 },
			{ 0, 8, 15 }, { 0, 8, 15 }, { 0, 6, 15 }, { 0, 6, 15 },
			{ 0, 6, 15 }, { 0, 5, 15 }, { 0, 3, 15 }, { 0, 3, 8 },
			{ 0, 3, 15 }, { 0, 3, 8 }, { 0, 8, 15 }, { 0, 15, 3 },
			{ 0, 3, 15 }, { 0, 3, 8 }, { 0, 6, 15 }, { 0, 10, 8 },
			{ 0, 5, 3 }, { 0, 8, 15 }, { 0, 8, 6 }, { 0, 6, 10 },
			{ 0, 8, 15 }, { 0, 5, 15 }, { 0, 15, 10 }, { 0, 15, 8 },
			{ 0, 8, 15 }, { 0, 15, 3 }, { 0, 3, 15 }, { 0, 5, 10 },
			{ 0, 6, 10 }, { 0, 10, 8 }, { 0, 8, 9 }, { 0, 15, 10 },
			{ 0, 15, 6 }, { 0, 3, 15 }, { 0, 15, 8 }, { 0, 5, 15 },
			{ 0, 15, 3 }, { 0, 15, 6 }, { 0, 15, 6 }, { 0, 15, 8 },
			{ 0, 3, 15 }, { 0, 15, 3 }, { 0, 5, 15 }, { 0, 5, 15 },
			{ 0, 5, 15 }, { 0, 8, 15 }, { 0, 5, 15 }, { 0, 10, 15 },
			{ 0, 5, 15 }, { 0, 10, 15 }, { 0, 8, 15 }, { 0, 13, 15 },
			{ 0, 15, 3 }, { 0, 12, 15 }, { 0, 3, 15 }, { 0, 3, 8 }
		}
	};

	uint8_t ReadBit(void const * input, size_t& start_bit)
	{
		BOOST_ASSERT(start_bit < 128);

		uint8_t const * bits = static_cast<uint8_t const *>(input);

		size_t index = start_bit >> 3;
		uint8_t ret = (bits[index] >> (start_bit - (index << 3))) & 0x01;
		++ start_bit;

		return ret;
	}

	uint8_t ReadBits(void const * input, size_t& start_bit, size_t num_bits)
	{
		if (0 == num_bits)
		{
			return 0;
		}

		uint8_t const * bits = static_cast<uint8_t const *>(input);

		BOOST_ASSERT((start_bit + num_bits <= 128) && (num_bits <= 8));

		uint8_t ret;
		size_t index = start_bit / 8;
		size_t base = start_bit - index * 8;
		if (base + num_bits > 8)
		{
			size_t first_index_bits = 8 - base;
			size_t next_index_bits = num_bits - first_index_bits;
			ret = (bits[index] >> base) | ((bits[index + 1] & ((1 << next_index_bits) - 1)) << first_index_bits);
		}
		else
		{
			ret = (bits[index] >> base) & ((1 << num_bits) - 1);
		}
		BOOST_ASSERT(ret < (1 << num_bits));
		start_bit += num_bits;

		return ret;
	}

	bool IsFixUpOffset(size_t partitions, size_t shape, size_t offset)
	{
		BOOST_ASSERT((partitions < 3) && (shape < 64) && (offset < 16));
		for (size_t p = 0; p <= partitions; ++ p)
		{
			if (offset == fix_up_table[partitions][shape][p])
			{
				return true;
			}
		}
		return false;
	}

	void SignExtend(int3& clr, ARGBColor32 const & prec)
	{
		if (clr.x() & (1UL << (prec.r() - 1)))
		{
			clr.x() |= (0xFFFFFFFF << prec.r());
		}
		if (clr.y() & (1UL << (prec.g() - 1)))
		{
			clr.y() |= (0xFFFFFFFF << prec.g());
		}
		if (clr.z() & (1UL << (prec.b() - 1)))
		{
			clr.z() |= (0xFFFFFFFF << prec.b());
		}
	}

	half INT2F16(int input, bool signed_fmt)
	{
		half h;
		uint16_t out;
		if (signed_fmt)
		{
			int s = 0;
			if (input < 0)
			{
				s = 0x8000;
				input = -input;
			}
			out = static_cast<uint16_t>(s | input);
		}
		else
		{
			BOOST_ASSERT((input >= 0) && (input <= 0x7BFF));
			out = static_cast<uint16_t>(input);
		}

		*(reinterpret_cast<uint16_t*>(&h)) = out;
		return h;
	}

	void ToF16(Vector_T<half, 4>& f16, int3 const & clr, bool signed_fmt)
	{
		f16.x() = INT2F16(clr.x(), signed_fmt);
		f16.y() = INT2F16(clr.y(), signed_fmt);
		f16.z() = INT2F16(clr.z(), signed_fmt);
	}

	void TransformInverse(std::pair<int3, int3>* end_pts, ARGBColor32 const & prec, bool signed_fmt)
	{
		int3 wrap_mask((1 << prec.r()) - 1, (1 << prec.g()) - 1, (1 << prec.b()) - 1);
		end_pts[0].second += end_pts[0].first;
		end_pts[0].second.x() &= wrap_mask.x();
		end_pts[0].second.y() &= wrap_mask.y();
		end_pts[0].second.z() &= wrap_mask.z();
		end_pts[1].first += end_pts[0].first;
		end_pts[1].first.x() &= wrap_mask.x();
		end_pts[1].first.y() &= wrap_mask.y();
		end_pts[1].first.z() &= wrap_mask.z();
		end_pts[1].second += end_pts[0].first;
		end_pts[1].second.x() &= wrap_mask.x();
		end_pts[1].second.y() &= wrap_mask.y();
		end_pts[1].second.z() &= wrap_mask.z();
		if (signed_fmt)
		{
			SignExtend(end_pts[0].second, prec);
			SignExtend(end_pts[1].first, prec);
			SignExtend(end_pts[1].second, prec);
		}
	}
}

namespace KlayGE
{
	uint8_t TexCompressionBC1::expand5_[32];
	uint8_t TexCompressionBC1::expand6_[64];
	uint8_t TexCompressionBC1::o_match5_[256][2];
	uint8_t TexCompressionBC1::o_match6_[256][2];
	uint8_t TexCompressionBC1::quant_rb_tab_[256 + 16];
	uint8_t TexCompressionBC1::quant_g_tab_[256 + 16];
	bool TexCompressionBC1::lut_inited_ = false;

	TexCompressionBC1::TexCompressionBC1()
	{
		block_width_ = block_height_ = 4;
		block_depth_ = 1;
		block_bytes_ = NumFormatBytes(EF_BC1) * 4;
		decoded_fmt_ = EF_ARGB8;

		if (!lut_inited_)
		{
			unique_lock<mutex> lock(singleton_mutex);
			if (!lut_inited_)
			{
				for (int i = 0; i < 32; ++ i)
				{
					expand5_[i] = Extend5To8Bits(i);
				}

				for (int i = 0; i < 64; ++ i)
				{
					expand6_[i] = Extend6To8Bits(i);
				}

				for (int i = 0; i < 256 + 16; ++ i)
				{
					int v = MathLib::clamp(i - 8, 0, 255);
					quant_rb_tab_[i] = expand5_[Mul8Bit(v, 31)];
					quant_g_tab_[i] = expand6_[Mul8Bit(v, 63)];
				}

				this->PrepareOptTable(&o_match5_[0][0], expand5_, 32);
				this->PrepareOptTable(&o_match6_[0][0], expand6_, 64);

				lut_inited_ = true;
			}
		}
	}

	void TexCompressionBC1::EncodeBlock(void* output, void const * input, TexCompressionMethod method)
	{
		BOOST_ASSERT(output);
		BOOST_ASSERT(input);

		BC1Block& bc1 = *static_cast<BC1Block*>(output);
		ARGBColor32 const * argb = static_cast<ARGBColor32 const *>(input);

		array<ARGBColor32, 16> tmp_argb;
		bool alpha = false;
		for (size_t i = 0; i < tmp_argb.size(); ++ i)
		{
			if (argb[i].a() < 0x80)
			{
				tmp_argb[i] = ARGBColor32(0, 0, 0, 0);
				alpha = true;
			}
			else
			{
				tmp_argb[i] = argb[i];
			}
		}

		this->EncodeBC1Internal(bc1, argb, alpha, method);
	}

	void TexCompressionBC1::DecodeBlock(void* output, void const * input)
	{
		BOOST_ASSERT(output);
		BOOST_ASSERT(input);

		ARGBColor32* argb = static_cast<ARGBColor32*>(output);
		BC1Block const & bc1 = *static_cast<BC1Block const *>(input);

		ARGBColor32 max_clr = this->RGB565To888(bc1.clr_0);
		ARGBColor32 min_clr = this->RGB565To888(bc1.clr_1);

		array<ARGBColor32, 4> clr;
		clr[0] = max_clr;
		clr[1] = min_clr;
		if (bc1.clr_0 > bc1.clr_1)
		{
			clr[2].r() = (max_clr.r() * 2 + min_clr.r()) / 3;
			clr[2].g() = (max_clr.g() * 2 + min_clr.g()) / 3;
			clr[2].b() = (max_clr.b() * 2 + min_clr.b()) / 3;
			clr[2].a() = 255;
			clr[3].r() = (max_clr.r() + min_clr.r() * 2) / 3;
			clr[3].g() = (max_clr.g() + min_clr.g() * 2) / 3;
			clr[3].b() = (max_clr.b() + min_clr.b() * 2) / 3;
			clr[3].a() = 255;
		}
		else
		{
			clr[2].r() = (max_clr.r() + min_clr.r()) / 2;
			clr[2].g() = (max_clr.g() + min_clr.g()) / 2;
			clr[2].b() = (max_clr.b() + min_clr.b()) / 2;
			clr[2].a() = 255;
			clr[3] = ARGBColor32(0, 0, 0, 0);
		}

		for (int i = 0; i < 2; ++ i)
		{
			for (int j = 0; j < 8; ++ j)
			{
				argb[i * 8 + j] = clr[(bc1.bitmap[i] >> (j * 2)) & 0x3];
			}
		}
	}

	void TexCompressionBC1::PrepareOptTable(uint8_t* table, uint8_t const * expand, int size) const
	{
		for (int i = 0; i < 256; ++ i)
		{
			int best_err = 256;

			for (int min = 0; min < size; ++ min)
			{
				for (int max = 0; max < size; ++ max)
				{
					int min_e = expand[min];
					int max_e = expand[max];
					int err = abs(max_e + Mul8Bit(min_e - max_e, 0x55) - i);
					if (err < best_err)
					{
						table[i * 2 + 0] = static_cast<uint8_t>(max);
						table[i * 2 + 1] = static_cast<uint8_t>(min);
						best_err = err;
					}
				}
			}
		}
	}

	ARGBColor32 TexCompressionBC1::RGB565To888(uint16_t rgb) const
	{
		return ARGBColor32(255, expand5_[(rgb >> 11) & 0x1F], expand6_[(rgb >> 5) & 0x3F],
			expand5_[(rgb >> 0) & 0x1F]);
	}

	uint16_t TexCompressionBC1::RGB888To565(ARGBColor32 const & rgb) const
	{
		return ((rgb.r() >> 3) << 11) | ((rgb.g() >> 2) << 5) | ((rgb.b() >> 3) << 0);
	}

	// The color matching function
	uint32_t TexCompressionBC1::MatchColorsBlock(ARGBColor32 const * argb,
			ARGBColor32 const & min_clr, ARGBColor32 const & max_clr, bool alpha) const
	{
		array<ARGBColor32, 4> color;
		color[0] = max_clr;
		color[1] = min_clr;
		if (!alpha)
		{
			color[2].r() = (max_clr.r() * 2 + min_clr.r()) / 3;
			color[2].g() = (max_clr.g() * 2 + min_clr.g()) / 3;
			color[2].b() = (max_clr.b() * 2 + min_clr.b()) / 3;
			color[2].a() = 255;
			color[3].r() = (max_clr.r() + min_clr.r() * 2) / 3;
			color[3].g() = (max_clr.g() + min_clr.g() * 2) / 3;
			color[3].b() = (max_clr.b() + min_clr.b() * 2) / 3;
			color[3].a() = 255;
		}

		uint32_t mask = 0;
		int dirr = color[0].r() - color[1].r();
		int dirg = color[0].g() - color[1].g();
		int dirb = color[0].b() - color[1].b();

		int dots[16];
		for (int i = 0; i < 16; ++ i)
		{
			dots[i] = argb[i].r() * dirr + argb[i].g() * dirg + argb[i].b() * dirb;
		}

		if (alpha)
		{
			array<int, 2> stops;
			for (int i = 0; i < 2; ++ i)
			{
				stops[i] = color[i].r() * dirr + color[i].g() * dirg + color[i].b() * dirb;
			}

			int c0_point = (stops[0] + stops[1] * 2) / 3;
			int c3_point = (stops[0] * 2 + stops[1]) / 3;

			for (int i = 15; i >= 0; -- i)
			{
				mask <<= 2;
				int dot = dots[i];
				if (0 == argb[i].a())
				{
					mask |= 3;
				}
				else
				{
					if (dot >= c0_point)
					{
						mask |= (dot < c3_point) ? 2 : 1;
					}
				}
			}
		}
		else
		{
			array<int, 4> stops;
			for (int i = 0; i < 4; ++ i)
			{
				stops[i] = color[i].r() * dirr + color[i].g() * dirg + color[i].b() * dirb;
			}

			int c0_point = (stops[1] + stops[3]) >> 1;
			int half_point = (stops[3] + stops[2]) >> 1;
			int c3_point = (stops[2] + stops[0]) >> 1;

			for (int i = 15; i >= 0; -- i)
			{
				mask <<= 2;
				int dot = dots[i];

				if (dot < half_point)
				{
					mask |= (dot < c0_point) ? 1 : 3;
				}
				else
				{
					mask |= (dot < c3_point) ? 2 : 0;
				}
			}
		}

		return mask;
	}

	// The color optimization function. (Clever code, part 1)
	void TexCompressionBC1::OptimizeColorsBlock(ARGBColor32 const * argb,
			ARGBColor32& min_clr, ARGBColor32& max_clr, TexCompressionMethod method) const
	{
		if (method != TCM_Quality)
		{
			Color const LUM_WEIGHT(0.2126f, 0.7152f, 0.0722f, 0);

			max_clr = min_clr = argb[0];
			float min_lum = MathLib::dot(Color(min_clr.ARGB()), LUM_WEIGHT);
			float max_lum = min_lum;
			for (size_t i = 1; i < 16; ++ i)
			{
				float lum = MathLib::dot(Color(argb[i].ARGB()), LUM_WEIGHT);
				if (lum < min_lum)
				{
					min_lum = lum;
					min_clr = argb[i];
				}
				if (lum > max_lum)
				{
					max_lum = lum;
					max_clr = argb[i];
				}
			}
		}
		else
		{
			static int const ITER_POWER = 4;

			// determine color distribution
			int mu[3], min[3], max[3];

			for (int ch = 0; ch < 3; ++ ch)
			{
				int muv, minv, maxv;

				muv = minv = maxv = argb[0][ch];
				for (int i = 1; i < 16; ++ i)
				{
					muv += argb[i][ch];
					minv = std::min<int>(minv, argb[i][ch]);
					maxv = std::max<int>(maxv, argb[i][ch]);
				}

				mu[ch] = (muv + 8) >> 4;
				min[ch] = minv;
				max[ch] = maxv;
			}

			// determine covariance matrix
			int cov[6];
			for (int i = 0; i < 6; ++ i)
			{
				cov[i] = 0;
			}

			for (int i = 0; i < 16; ++ i)
			{
				int r = argb[i].r() - mu[ARGBColor32::RChannel];
				int g = argb[i].g() - mu[ARGBColor32::GChannel];
				int b = argb[i].b() - mu[ARGBColor32::BChannel];

				cov[0] += r * r;
				cov[1] += r * g;
				cov[2] += r * b;
				cov[3] += g * g;
				cov[4] += g * b;
				cov[5] += b * b;
			}

			// convert covariance matrix to float, find principal axis via power iter
			float covf[6], vfr, vfg, vfb;
			for (int i = 0; i < 6; ++ i)
			{
				covf[i] = cov[i] / 255.0f;
			}

			vfr = static_cast<float>(max[ARGBColor32::RChannel] - min[ARGBColor32::RChannel]);
			vfg = static_cast<float>(max[ARGBColor32::GChannel] - min[ARGBColor32::GChannel]);
			vfb = static_cast<float>(max[ARGBColor32::BChannel] - min[ARGBColor32::BChannel]);

			for (int iter = 0; iter < ITER_POWER; ++ iter)
			{
				float r = vfr * covf[0] + vfg * covf[1] + vfb * covf[2];
				float g = vfr * covf[1] + vfg * covf[3] + vfb * covf[4];
				float b = vfr * covf[2] + vfg * covf[4] + vfb * covf[5];

				vfr = r;
				vfg = g;
				vfb = b;
			}

			float magn = std::max(std::max(abs(vfr), abs(vfg)), abs(vfb));
			int v_r, v_g, v_b;

			if (magn < 4.0f) // too small, default to luminance
			{
				v_r = 148;
				v_g = 300;
				v_b = 58;
			}
			else
			{
				magn = 512.0f / magn;
				v_r = static_cast<int>(vfr * magn);
				v_g = static_cast<int>(vfg * magn);
				v_b = static_cast<int>(vfb * magn);
			}

			// Pick colors at extreme points
			int min_d = 0x7FFFFFFF, max_d = -min_d;
			min_clr = max_clr = ARGBColor32(0, 0, 0, 0);
			for (int i = 0; i < 16; ++ i)
			{
				int dot = argb[i].r() * v_r + argb[i].g() * v_g + argb[i].b() * v_b;
				if (dot < min_d)
				{
					min_d = dot;
					min_clr = argb[i];
				}
				if (dot > max_d)
				{
					max_d = dot;
					max_clr = argb[i];
				}
			}
		}
	}

	// The refinement function. (Clever code, part 2)
	// Tries to optimize colors to suit block contents better.
	// (By solving a least squares system via normal equations+Cramer's rule)
	bool TexCompressionBC1::RefineBlock(ARGBColor32 const * argb,
			ARGBColor32& min_clr, ARGBColor32& max_clr, uint32_t mask) const
	{
		static int const w1Tab[4] = { 3, 0, 2, 1 };
		static int const prods[4] = { 0x090000, 0x000900, 0x040102, 0x010402 };
		// ^some magic to save a lot of multiplies in the accumulating loop...

		int akku = 0;
		int At1_r, At1_g, At1_b;
		int At2_r, At2_g, At2_b;

		At1_r = At1_g = At1_b = 0;
		At2_r = At2_g = At2_b = 0;
		for (int i = 0; i < 16; ++ i, mask >>= 2)
		{
			int step = mask & 3;
			int w1 = w1Tab[step];
			int r = argb[i].r();
			int g = argb[i].g();
			int b = argb[i].b();

			akku += prods[step];
			At1_r += w1 * r;
			At1_g += w1 * g;
			At1_b += w1 * b;
			At2_r += r;
			At2_g += g;
			At2_b += b;
		}

		At2_r = 3 * At2_r - At1_r;
		At2_g = 3 * At2_g - At1_g;
		At2_b = 3 * At2_b - At1_b;

		// extract solutions and decide solvability
		int xx = akku >> 16;
		int yy = (akku >> 8) & 0xFF;
		int xy = (akku >> 0) & 0xFF;

		if (!yy || !xx || (xx * yy == xy * xy))
		{
			return false;
		}

		float const f = 3.0f / 255.0f / (xx * yy - xy * xy);
		float const frb = f * 31.0f;
		float const fg = f * 63.0f;

		uint16_t old_min = this->RGB888To565(min_clr);
		uint16_t old_max = this->RGB888To565(max_clr);

		// solve.
		int max_r = MathLib::clamp<int>(static_cast<int>((At1_r * yy - At2_r * xy) * frb + 0.5f), 0, 31);
		int max_g = MathLib::clamp<int>(static_cast<int>((At1_g * yy - At2_g * xy) * fg + 0.5f), 0, 63);
		int max_b = MathLib::clamp<int>(static_cast<int>((At1_b * yy - At2_b * xy) * frb + 0.5f), 0, 31);

		int min_r = MathLib::clamp<int>(static_cast<int>((At2_r * xx - At1_r * xy) * frb + 0.5f), 0, 31);
		int min_g = MathLib::clamp<int>(static_cast<int>((At2_g * xx - At1_g * xy) * fg + 0.5f), 0, 63);
		int min_b = MathLib::clamp<int>(static_cast<int>((At2_b * xx - At1_b * xy) * frb + 0.5f), 0, 31);

		uint16_t max16 = static_cast<uint16_t>((max_r << 11) | (max_g << 5) | (max_b << 0));
		uint16_t min16 = static_cast<uint16_t>((min_r << 11) | (min_g << 5) | (min_b << 0));

		if ((old_min != min16) || (old_max != max16))
		{
			min_clr = this->RGB565To888(min16);
			max_clr = this->RGB565To888(max16);

			return true;
		}
		else
		{
			return false;
		}
	}

	void TexCompressionBC1::EncodeBC1Internal(BC1Block& bc1, ARGBColor32 const * argb,
			bool alpha, TexCompressionMethod method) const
	{
		BOOST_ASSERT(argb);

		// check if block is constant
		uint32_t min32, max32;
		min32 = max32 = argb[0].ARGB();
		for (int i = 1; i < 16; ++ i)
		{
			min32 = std::min(min32, argb[i].ARGB());
			max32 = std::max(max32, argb[i].ARGB());
		}

		uint32_t mask;
		uint16_t max16, min16;
		if (min32 != max32) // no constant color
		{
			ARGBColor32 max_clr, min_clr;
			this->OptimizeColorsBlock(argb, min_clr, max_clr, method);
			max16 = this->RGB888To565(max_clr);
			min16 = this->RGB888To565(min_clr);
			if (max16 != min16)
			{
				mask = this->MatchColorsBlock(argb, min_clr, max_clr, alpha);
			}
			else
			{
				mask = 0;
			}
			if (!alpha && (method != TCM_Speed))
			{
				if (this->RefineBlock(argb, min_clr, max_clr, mask))
				{
					max16 = this->RGB888To565(max_clr);
					min16 = this->RGB888To565(min_clr);
					if (max16 != min16)
					{
						mask = this->MatchColorsBlock(argb, min_clr, max_clr, alpha);
					}
					else
					{
						mask = 0;
					}
				}
			}
		}
		else // constant color
		{
			if (alpha && (0 == argb[0].ARGB()))
			{
				mask = 0xFFFFFFFF;
				max16 = min16 = 0;
			}
			else
			{
				int const r = argb[0].r();
				int const g = argb[0].g();
				int const b = argb[0].b();

				mask = 0xAAAAAAAA;
				max16 = (o_match5_[r][0] << 11) | (o_match6_[g][0] << 5) | o_match5_[b][0];
				min16 = (o_match5_[r][1] << 11) | (o_match6_[g][1] << 5) | o_match5_[b][1];
			}
		}

		if (alpha)
		{
			if (max16 < min16)
			{
				std::swap(max16, min16);

				uint32_t xor_mask = 0;
				for (int i = 15; i >= 0; -- i)
				{
					xor_mask <<= 2;

					uint32_t pixel_mask = (mask >> (i * 2)) & 0x3;
					if (pixel_mask <= 1)
					{
						xor_mask |= 0x1;
					}
				}
				mask ^= xor_mask;
			}

			bc1.clr_0 = min16;
			bc1.clr_1 = max16;
		}
		else
		{
			if (max16 < min16)
			{
				std::swap(max16, min16);
				mask ^= 0x55555555;
			}

			bc1.clr_0 = max16;
			bc1.clr_1 = min16;
		}
		std::memcpy(bc1.bitmap, &mask, sizeof(mask));
	}


	TexCompressionBC2::TexCompressionBC2()
	{
		block_width_ = block_height_ = 4;
		block_depth_ = 1;
		block_bytes_ = NumFormatBytes(EF_BC2) * 4;
		decoded_fmt_ = EF_ARGB8;

		bc1_codec_ = MakeSharedPtr<TexCompressionBC1>();
	}

	void TexCompressionBC2::EncodeBlock(void* output, void const * input, TexCompressionMethod method)
	{
		BOOST_ASSERT(output);
		BOOST_ASSERT(input);

		BC2Block& bc2 = *static_cast<BC2Block*>(output);
		ARGBColor32 const * argb = static_cast<ARGBColor32 const *>(input);

		array<uint8_t, 16> alpha;
		array<ARGBColor32, 16> xrgb;
		for (size_t i = 0; i < xrgb.size(); ++ i)
		{
			xrgb[i] = argb[i];
			xrgb[i].a() = 255;
			alpha[i] = static_cast<uint8_t>(argb[i].a() >> 4);
		}

		bc1_codec_->EncodeBC1Internal(bc2.bc1, &xrgb[0], false, method);
		
		for (int i = 0; i < 4; ++ i)
		{
			bc2.alpha[i] = (alpha[i * 4 + 0] << 0) | (alpha[i * 4 + 1] << 4)
				| (alpha[i * 4 + 2] << 8) | (alpha[i * 4 + 3] << 12);
		}
	}

	void TexCompressionBC2::DecodeBlock(void* output, void const * input)
	{
		BOOST_ASSERT(output);
		BOOST_ASSERT(input);

		ARGBColor32* argb = static_cast<ARGBColor32*>(output);
		BC2Block const * bc2_block = static_cast<BC2Block const *>(input);

		bc1_codec_->DecodeBlock(argb, &bc2_block->bc1);

		for (int i = 0; i < 4; ++ i)
		{
			for (int j = 0; j < 4; ++ j)
			{
				argb[i * 4 + j].a() = ((bc2_block->alpha[i] >> (4 * j)) & 0xF) << 4;
			}
		}
	}


	TexCompressionBC3::TexCompressionBC3()
	{
		block_width_ = block_height_ = 4;
		block_depth_ = 1;
		block_bytes_ = NumFormatBytes(EF_BC3) * 4;
		decoded_fmt_ = EF_ARGB8;

		bc1_codec_ = MakeSharedPtr<TexCompressionBC1>();
		bc4_codec_ = MakeSharedPtr<TexCompressionBC4>();
	}

	void TexCompressionBC3::EncodeBlock(void* output, void const * input, TexCompressionMethod method)
	{
		BOOST_ASSERT(output);
		BOOST_ASSERT(input);

		BC3Block& bc3 = *static_cast<BC3Block*>(output);
		ARGBColor32 const * argb = static_cast<ARGBColor32 const *>(input);

		array<uint8_t, 16> alpha;
		array<ARGBColor32, 16> xrgb;
		for (size_t i = 0; i < xrgb.size(); ++ i)
		{
			xrgb[i] = argb[i];
			xrgb[i].a() = 255;
			alpha[i] = static_cast<uint8_t>(argb[i].a());
		}

		bc1_codec_->EncodeBC1Internal(bc3.bc1, &xrgb[0], false, method);
		bc4_codec_->EncodeBlock(&bc3.alpha, &alpha[0], method);
	}

	void TexCompressionBC3::DecodeBlock(void* output, void const * input)
	{
		BOOST_ASSERT(output);
		BOOST_ASSERT(input);

		ARGBColor32* argb = static_cast<ARGBColor32*>(output);
		BC3Block const * bc3_block = static_cast<BC3Block const *>(input);

		bc1_codec_->DecodeBlock(argb, &bc3_block->bc1);

		array<uint8_t, 16> alpha_block;
		bc4_codec_->DecodeBlock(&alpha_block[0], &bc3_block->alpha);

		for (size_t i = 0; i < alpha_block.size(); ++ i)
		{
			argb[i].a() = alpha_block[i];
		}
	}


	TexCompressionBC4::TexCompressionBC4()
	{
		block_width_ = block_height_ = 4;
		block_depth_ = 1;
		block_bytes_ = NumFormatBytes(EF_BC4) * 4;
		decoded_fmt_ = EF_R8;
	}

	// Alpha block compression (this is easy for a change)
	void TexCompressionBC4::EncodeBlock(void* output, void const * input, TexCompressionMethod method)
	{
		BOOST_ASSERT(output);
		BOOST_ASSERT(input);

		UNREF_PARAM(method);

		BC4Block& bc4 = *static_cast<BC4Block*>(output);
		uint8_t const * r = static_cast<uint8_t const *>(input);

		// find min/max color
		int min, max;
		min = max = r[0];

		for (int i = 1; i < 16; ++ i)
		{
			min = std::min<int>(min, r[i]);
			max = std::max<int>(max, r[i]);
		}

		// encode them
		bc4.alpha_0 = static_cast<uint8_t>(max);
		bc4.alpha_1 = static_cast<uint8_t>(min);

		// determine bias and emit color indices
		int dist = max - min;
		int bias = min * 7 - (dist >> 1);
		int dist4 = dist * 4;
		int dist2 = dist * 2;
		int bits = 0, mask = 0;

		int dest = 0;
		for (int i = 0; i < 16; ++ i)
		{
			int a = r[i] * 7 - bias;
			int ind, t;

			// select index (hooray for bit magic)
			t = (dist4 - a) >> 31;  ind = t & 4; a -= dist4 & t;
			t = (dist2 - a) >> 31;  ind += t & 2; a -= dist2 & t;
			t = (dist - a) >> 31;   ind += t & 1;

			ind = -ind & 7;
			ind ^= (2 > ind);

			// write index
			mask |= ind << bits;
			if ((bits += 3) >= 8)
			{
				bc4.bitmap[dest] = static_cast<uint8_t>(mask);
				++ dest;
				mask >>= 8;
				bits -= 8;
			}
		}
	}

	void TexCompressionBC4::DecodeBlock(void* output, void const * input)
	{
		BOOST_ASSERT(output);
		BOOST_ASSERT(input);

		uint8_t* alpha_block = static_cast<uint8_t*>(output);
		BC4Block const & bc4 = *static_cast<BC4Block const *>(input);

		array<uint8_t, 8> alpha;
		float falpha0 = bc4.alpha_0 / 255.0f;
		float falpha1 = bc4.alpha_1 / 255.0f;
		alpha[0] = bc4.alpha_0;
		alpha[1] = bc4.alpha_1;
		if (alpha[0] > alpha[1])
		{
			alpha[2] = static_cast<uint8_t>(MathLib::clamp(static_cast<int>(MathLib::lerp(falpha0, falpha1, 1 / 7.0f) * 255 + 0.5f), 0, 255));
			alpha[3] = static_cast<uint8_t>(MathLib::clamp(static_cast<int>(MathLib::lerp(falpha0, falpha1, 2 / 7.0f) * 255 + 0.5f), 0, 255));
			alpha[4] = static_cast<uint8_t>(MathLib::clamp(static_cast<int>(MathLib::lerp(falpha0, falpha1, 3 / 7.0f) * 255 + 0.5f), 0, 255));
			alpha[5] = static_cast<uint8_t>(MathLib::clamp(static_cast<int>(MathLib::lerp(falpha0, falpha1, 4 / 7.0f) * 255 + 0.5f), 0, 255));
			alpha[6] = static_cast<uint8_t>(MathLib::clamp(static_cast<int>(MathLib::lerp(falpha0, falpha1, 5 / 7.0f) * 255 + 0.5f), 0, 255));
			alpha[7] = static_cast<uint8_t>(MathLib::clamp(static_cast<int>(MathLib::lerp(falpha0, falpha1, 6 / 7.0f) * 255 + 0.5f), 0, 255));
		}
		else
		{
			alpha[2] = static_cast<uint8_t>(MathLib::clamp(static_cast<int>(MathLib::lerp(falpha0, falpha1, 1 / 5.0f) * 255 + 0.5f), 0, 255));
			alpha[3] = static_cast<uint8_t>(MathLib::clamp(static_cast<int>(MathLib::lerp(falpha0, falpha1, 2 / 5.0f) * 255 + 0.5f), 0, 255));
			alpha[4] = static_cast<uint8_t>(MathLib::clamp(static_cast<int>(MathLib::lerp(falpha0, falpha1, 3 / 5.0f) * 255 + 0.5f), 0, 255));
			alpha[5] = static_cast<uint8_t>(MathLib::clamp(static_cast<int>(MathLib::lerp(falpha0, falpha1, 4 / 5.0f) * 255 + 0.5f), 0, 255));
			alpha[6] = 0;
			alpha[7] = 255;
		}

		for (int i = 0; i < 2; ++ i)
		{
			uint32_t alpha32 = (bc4.bitmap[i * 3 + 2] << 16) | (bc4.bitmap[i * 3 + 1] << 8) | (bc4.bitmap[i * 3 + 0] << 0);
			for (int j = 0; j < 8; ++ j)
			{
				alpha_block[i * 8 + j] = alpha[(alpha32 >> (j * 3)) & 0x7];
			}
		}
	}


	TexCompressionBC5::TexCompressionBC5()
	{
		block_width_ = block_height_ = 4;
		block_depth_ = 1;
		block_bytes_ = NumFormatBytes(EF_BC5) * 4;
		decoded_fmt_ = EF_GR8;

		bc4_codec_ = MakeSharedPtr<TexCompressionBC4>();
	}

	void TexCompressionBC5::EncodeBlock(void* output, void const * input, TexCompressionMethod method)
	{
		BOOST_ASSERT(output);
		BOOST_ASSERT(input);

		BC5Block& bc5 = *static_cast<BC5Block*>(output);
		uint16_t const * gr = static_cast<uint16_t const *>(input);

		array<uint8_t, 16> r;
		array<uint8_t, 16> g;
		for (size_t i = 0; i < r.size(); ++ i)
		{
			r[i] = gr[i] & 0xFF;
			g[i] = gr[i] >> 8;
		}

		bc4_codec_->EncodeBlock(&bc5.red, &r[0], method);
		bc4_codec_->EncodeBlock(&bc5.green, &g[0], method);
	}

	void TexCompressionBC5::DecodeBlock(void* output, void const * input)
	{
		BOOST_ASSERT(output);
		BOOST_ASSERT(input);

		uint16_t* gr = static_cast<uint16_t*>(output);
		BC5Block const * bc5_block = static_cast<BC5Block const *>(input);

		array<uint8_t, 16> r;
		bc4_codec_->DecodeBlock(&r[0], &bc5_block->red);
		array<uint8_t, 16> g;
		bc4_codec_->DecodeBlock(&g[0], &bc5_block->green);

		for (size_t i = 0; i < r.size(); ++ i)
		{
			gr[i] = r[i] | (g[i] << 8);
		}
	}


	// BC6H Compression
	TexCompressionBC6U::ModeDescriptor const TexCompressionBC6U::mode_desc_[14][82] =
	{
		{   // Mode 1 (0x00) - 10 5 5 5
			{ M, 0 }, { M, 1 }, { GY, 4 }, { BY, 4 }, { BZ, 4 }, { RW, 0 }, { RW, 1 }, { RW, 2 }, { RW, 3 },
			{ RW, 4 }, { RW, 5 }, { RW, 6 }, { RW, 7 }, { RW, 8 }, { RW, 9 }, { GW, 0 }, { GW, 1 }, { GW, 2 },
			{ GW, 3 }, { GW, 4 }, { GW, 5 }, { GW, 6 }, { GW, 7 }, { GW, 8 }, { GW, 9 }, { BW, 0 }, { BW, 1 },
			{ BW, 2 }, { BW, 3 }, { BW, 4 }, { BW, 5 }, { BW, 6 }, { BW, 7 }, { BW, 8 }, { BW, 9 }, { RX, 0 },
			{ RX, 1 }, { RX, 2 }, { RX, 3 }, { RX, 4 }, { GZ, 4 }, { GY, 0 }, { GY, 1 }, { GY, 2 }, { GY, 3 },
			{ GX, 0 }, { GX, 1 }, { GX, 2 }, { GX, 3 }, { GX, 4 }, { BZ, 0 }, { GZ, 0 }, { GZ, 1 }, { GZ, 2 },
			{ GZ, 3 }, { BX, 0 }, { BX, 1 }, { BX, 2 }, { BX, 3 }, { BX, 4 }, { BZ, 1 }, { BY, 0 }, { BY, 1 },
			{ BY, 2 }, { BY, 3 }, { RY, 0 }, { RY, 1 }, { RY, 2 }, { RY, 3 }, { RY, 4 }, { BZ, 2 }, { RZ, 0 },
			{ RZ, 1 }, { RZ, 2 }, { RZ, 3 }, { RZ, 4 }, { BZ, 3 }, { D, 0 }, { D, 1 }, { D, 2 }, { D, 3 },
			{ D, 4 }
		},

		{   // Mode 2 (0x01) - 7 6 6 6
			{ M, 0 }, { M, 1 }, { GY, 5 }, { GZ, 4 }, { GZ, 5 }, { RW, 0 }, { RW, 1 }, { RW, 2 }, { RW, 3 },
			{ RW, 4 }, { RW, 5 }, { RW, 6 }, { BZ, 0 }, { BZ, 1 }, { BY, 4 }, { GW, 0 }, { GW, 1 }, { GW, 2 },
			{ GW, 3 }, { GW, 4 }, { GW, 5 }, { GW, 6 }, { BY, 5 }, { BZ, 2 }, { GY, 4 }, { BW, 0 }, { BW, 1 },
			{ BW, 2 }, { BW, 3 }, { BW, 4 }, { BW, 5 }, { BW, 6 }, { BZ, 3 }, { BZ, 5 }, { BZ, 4 }, { RX, 0 },
			{ RX, 1 }, { RX, 2 }, { RX, 3 }, { RX, 4 }, { RX, 5 }, { GY, 0 }, { GY, 1 }, { GY, 2 }, { GY, 3 },
			{ GX, 0 }, { GX, 1 }, { GX, 2 }, { GX, 3 }, { GX, 4 }, { GX, 5 }, { GZ, 0 }, { GZ, 1 }, { GZ, 2 },
			{ GZ, 3 }, { BX, 0 }, { BX, 1 }, { BX, 2 }, { BX, 3 }, { BX, 4 }, { BX, 5 }, { BY, 0 }, { BY, 1 },
			{ BY, 2 }, { BY, 3 }, { RY, 0 }, { RY, 1 }, { RY, 2 }, { RY, 3 }, { RY, 4 }, { RY, 5 }, { RZ, 0 },
			{ RZ, 1 }, { RZ, 2 }, { RZ, 3 }, { RZ, 4 }, { RZ, 5 }, { D, 0 }, { D, 1 }, { D, 2 }, { D, 3 },
			{ D, 4 }
		},

		{   // Mode 3 (0x02) - 11 5 4 4
			{ M, 0 }, { M, 1 }, { M, 2 }, { M, 3 }, { M, 4 }, { RW, 0 }, { RW, 1 }, { RW, 2 }, { RW, 3 },
			{ RW, 4 }, { RW, 5 }, { RW, 6 }, { RW, 7 }, { RW, 8 }, { RW, 9 }, { GW, 0 }, { GW, 1 }, { GW, 2 },
			{ GW, 3 }, { GW, 4 }, { GW, 5 }, { GW, 6 }, { GW, 7 }, { GW, 8 }, { GW, 9 }, { BW, 0 }, { BW, 1 },
			{ BW, 2 }, { BW, 3 }, { BW, 4 }, { BW, 5 }, { BW, 6 }, { BW, 7 }, { BW, 8 }, { BW, 9 }, { RX, 0 },
			{ RX, 1 }, { RX, 2 }, { RX, 3 }, { RX, 4 }, { RW, 10 }, { GY, 0 }, { GY, 1 }, { GY, 2 }, { GY, 3 },
			{ GX, 0 }, { GX, 1 }, { GX, 2 }, { GX, 3 }, { GW, 10 }, { BZ, 0 }, { GZ, 0 }, { GZ, 1 }, { GZ, 2 },
			{ GZ, 3 }, { BX, 0 }, { BX, 1 }, { BX, 2 }, { BX, 3 }, { BW, 10 }, { BZ, 1 }, { BY, 0 }, { BY, 1 },
			{ BY, 2 }, { BY, 3 }, { RY, 0 }, { RY, 1 }, { RY, 2 }, { RY, 3 }, { RY, 4 }, { BZ, 2 }, { RZ, 0 },
			{ RZ, 1 }, { RZ, 2 }, { RZ, 3 }, { RZ, 4 }, { BZ, 3 }, { D, 0 }, { D, 1 }, { D, 2 }, { D, 3 },
			{ D, 4 }
		},

		{   // Mode 4 (0x06) - 11 4 5 4
			{ M, 0 }, { M, 1 }, { M, 2 }, { M, 3 }, { M, 4 }, { RW, 0 }, { RW, 1 }, { RW, 2 }, { RW, 3 },
			{ RW, 4 }, { RW, 5 }, { RW, 6 }, { RW, 7 }, { RW, 8 }, { RW, 9 }, { GW, 0 }, { GW, 1 }, { GW, 2 },
			{ GW, 3 }, { GW, 4 }, { GW, 5 }, { GW, 6 }, { GW, 7 }, { GW, 8 }, { GW, 9 }, { BW, 0 }, { BW, 1 },
			{ BW, 2 }, { BW, 3 }, { BW, 4 }, { BW, 5 }, { BW, 6 }, { BW, 7 }, { BW, 8 }, { BW, 9 }, { RX, 0 },
			{ RX, 1 }, { RX, 2 }, { RX, 3 }, { RW, 10 }, { GZ, 4 }, { GY, 0 }, { GY, 1 }, { GY, 2 }, { GY, 3 },
			{ GX, 0 }, { GX, 1 }, { GX, 2 }, { GX, 3 }, { GX, 4 }, { GW, 10 }, { GZ, 0 }, { GZ, 1 }, { GZ, 2 },
			{ GZ, 3 }, { BX, 0 }, { BX, 1 }, { BX, 2 }, { BX, 3 }, { BW, 10 }, { BZ, 1 }, { BY, 0 }, { BY, 1 },
			{ BY, 2 }, { BY, 3 }, { RY, 0 }, { RY, 1 }, { RY, 2 }, { RY, 3 }, { BZ, 0 }, { BZ, 2 }, { RZ, 0 },
			{ RZ, 1 }, { RZ, 2 }, { RZ, 3 }, { GY, 4 }, { BZ, 3 }, { D, 0 }, { D, 1 }, { D, 2 }, { D, 3 },
			{ D, 4 }
		},

		{   // Mode 5 (0x0A) - 11 4 4 5
			{ M, 0 }, { M, 1 }, { M, 2 }, { M, 3 }, { M, 4 }, { RW, 0 }, { RW, 1 }, { RW, 2 }, { RW, 3 },
			{ RW, 4 }, { RW, 5 }, { RW, 6 }, { RW, 7 }, { RW, 8 }, { RW, 9 }, { GW, 0 }, { GW, 1 }, { GW, 2 },
			{ GW, 3 }, { GW, 4 }, { GW, 5 }, { GW, 6 }, { GW, 7 }, { GW, 8 }, { GW, 9 }, { BW, 0 }, { BW, 1 },
			{ BW, 2 }, { BW, 3 }, { BW, 4 }, { BW, 5 }, { BW, 6 }, { BW, 7 }, { BW, 8 }, { BW, 9 }, { RX, 0 },
			{ RX, 1 }, { RX, 2 }, { RX, 3 }, { RW, 10 }, { BY, 4 }, { GY, 0 }, { GY, 1 }, { GY, 2 }, { GY, 3 },
			{ GX, 0 }, { GX, 1 }, { GX, 2 }, { GX, 3 }, { GW, 10 }, { BZ, 0 }, { GZ, 0 }, { GZ, 1 }, { GZ, 2 },
			{ GZ, 3 }, { BX, 0 }, { BX, 1 }, { BX, 2 }, { BX, 3 }, { BX, 4 }, { BW, 10 }, { BY, 0 }, { BY, 1 },
			{ BY, 2 }, { BY, 3 }, { RY, 0 }, { RY, 1 }, { RY, 2 }, { RY, 3 }, { BZ, 1 }, { BZ, 2 }, { RZ, 0 },
			{ RZ, 1 }, { RZ, 2 }, { RZ, 3 }, { BZ, 4 }, { BZ, 3 }, { D, 0 }, { D, 1 }, { D, 2 }, { D, 3 },
			{ D, 4 }
		},

		{   // Mode 6 (0x0E) - 9 5 5 5
			{ M, 0 }, { M, 1 }, { M, 2 }, { M, 3 }, { M, 4 }, { RW, 0 }, { RW, 1 }, { RW, 2 }, { RW, 3 },
			{ RW, 4 }, { RW, 5 }, { RW, 6 }, { RW, 7 }, { RW, 8 }, { BY, 4 }, { GW, 0 }, { GW, 1 }, { GW, 2 },
			{ GW, 3 }, { GW, 4 }, { GW, 5 }, { GW, 6 }, { GW, 7 }, { GW, 8 }, { GY, 4 }, { BW, 0 }, { BW, 1 },
			{ BW, 2 }, { BW, 3 }, { BW, 4 }, { BW, 5 }, { BW, 6 }, { BW, 7 }, { BW, 8 }, { BZ, 4 }, { RX, 0 },
			{ RX, 1 }, { RX, 2 }, { RX, 3 }, { RX, 4 }, { GZ, 4 }, { GY, 0 }, { GY, 1 }, { GY, 2 }, { GY, 3 },
			{ GX, 0 }, { GX, 1 }, { GX, 2 }, { GX, 3 }, { GX, 4 }, { BZ, 0 }, { GZ, 0 }, { GZ, 1 }, { GZ, 2 },
			{ GZ, 3 }, { BX, 0 }, { BX, 1 }, { BX, 2 }, { BX, 3 }, { BX, 4 }, { BZ, 1 }, { BY, 0 }, { BY, 1 },
			{ BY, 2 }, { BY, 3 }, { RY, 0 }, { RY, 1 }, { RY, 2 }, { RY, 3 }, { RY, 4 }, { BZ, 2 }, { RZ, 0 },
			{ RZ, 1 }, { RZ, 2 }, { RZ, 3 }, { RZ, 4 }, { BZ, 3 }, { D, 0 }, { D, 1 }, { D, 2 }, { D, 3 },
			{ D, 4 }
		},

		{   // Mode 7 (0x12) - 8 6 5 5
			{ M, 0 }, { M, 1 }, { M, 2 }, { M, 3 }, { M, 4 }, { RW, 0 }, { RW, 1 }, { RW, 2 }, { RW, 3 },
			{ RW, 4 }, { RW, 5 }, { RW, 6 }, { RW, 7 }, { GZ, 4 }, { BY, 4 }, { GW, 0 }, { GW, 1 }, { GW, 2 },
			{ GW, 3 }, { GW, 4 }, { GW, 5 }, { GW, 6 }, { GW, 7 }, { BZ, 2 }, { GY, 4 }, { BW, 0 }, { BW, 1 },
			{ BW, 2 }, { BW, 3 }, { BW, 4 }, { BW, 5 }, { BW, 6 }, { BW, 7 }, { BZ, 3 }, { BZ, 4 }, { RX, 0 },
			{ RX, 1 }, { RX, 2 }, { RX, 3 }, { RX, 4 }, { RX, 5 }, { GY, 0 }, { GY, 1 }, { GY, 2 }, { GY, 3 },
			{ GX, 0 }, { GX, 1 }, { GX, 2 }, { GX, 3 }, { GX, 4 }, { BZ, 0 }, { GZ, 0 }, { GZ, 1 }, { GZ, 2 },
			{ GZ, 3 }, { BX, 0 }, { BX, 1 }, { BX, 2 }, { BX, 3 }, { BX, 4 }, { BZ, 1 }, { BY, 0 }, { BY, 1 },
			{ BY, 2 }, { BY, 3 }, { RY, 0 }, { RY, 1 }, { RY, 2 }, { RY, 3 }, { RY, 4 }, { RY, 5 }, { RZ, 0 },
			{ RZ, 1 }, { RZ, 2 }, { RZ, 3 }, { RZ, 4 }, { RZ, 5 }, { D, 0 }, { D, 1 }, { D, 2 }, { D, 3 },
			{ D, 4 }
		},

		{   // Mode 8 (0x16) - 8 5 6 5
			{ M, 0 }, { M, 1 }, { M, 2 }, { M, 3 }, { M, 4 }, { RW, 0 }, { RW, 1 }, { RW, 2 }, { RW, 3 },
			{ RW, 4 }, { RW, 5 }, { RW, 6 }, { RW, 7 }, { BZ, 0 }, { BY, 4 }, { GW, 0 }, { GW, 1 }, { GW, 2 },
			{ GW, 3 }, { GW, 4 }, { GW, 5 }, { GW, 6 }, { GW, 7 }, { GY, 5 }, { GY, 4 }, { BW, 0 }, { BW, 1 },
			{ BW, 2 }, { BW, 3 }, { BW, 4 }, { BW, 5 }, { BW, 6 }, { BW, 7 }, { GZ, 5 }, { BZ, 4 }, { RX, 0 },
			{ RX, 1 }, { RX, 2 }, { RX, 3 }, { RX, 4 }, { GZ, 4 }, { GY, 0 }, { GY, 1 }, { GY, 2 }, { GY, 3 },
			{ GX, 0 }, { GX, 1 }, { GX, 2 }, { GX, 3 }, { GX, 4 }, { GX, 5 }, { GZ, 0 }, { GZ, 1 }, { GZ, 2 },
			{ GZ, 3 }, { BX, 0 }, { BX, 1 }, { BX, 2 }, { BX, 3 }, { BX, 4 }, { BZ, 1 }, { BY, 0 }, { BY, 1 },
			{ BY, 2 }, { BY, 3 }, { RY, 0 }, { RY, 1 }, { RY, 2 }, { RY, 3 }, { RY, 4 }, { BZ, 2 }, { RZ, 0 },
			{ RZ, 1 }, { RZ, 2 }, { RZ, 3 }, { RZ, 4 }, { BZ, 3 }, { D, 0 }, { D, 1 }, { D, 2 }, { D, 3 },
			{ D, 4 }
		},

		{   // Mode 9 (0x1A) - 8 5 5 6
			{ M, 0 }, { M, 1 }, { M, 2 }, { M, 3 }, { M, 4 }, { RW, 0 }, { RW, 1 }, { RW, 2 }, { RW, 3 },
			{ RW, 4 }, { RW, 5 }, { RW, 6 }, { RW, 7 }, { BZ, 1 }, { BY, 4 }, { GW, 0 }, { GW, 1 }, { GW, 2 },
			{ GW, 3 }, { GW, 4 }, { GW, 5 }, { GW, 6 }, { GW, 7 }, { BY, 5 }, { GY, 4 }, { BW, 0 }, { BW, 1 },
			{ BW, 2 }, { BW, 3 }, { BW, 4 }, { BW, 5 }, { BW, 6 }, { BW, 7 }, { BZ, 5 }, { BZ, 4 }, { RX, 0 },
			{ RX, 1 }, { RX, 2 }, { RX, 3 }, { RX, 4 }, { GZ, 4 }, { GY, 0 }, { GY, 1 }, { GY, 2 }, { GY, 3 },
			{ GX, 0 }, { GX, 1 }, { GX, 2 }, { GX, 3 }, { GX, 4 }, { BZ, 0 }, { GZ, 0 }, { GZ, 1 }, { GZ, 2 },
			{ GZ, 3 }, { BX, 0 }, { BX, 1 }, { BX, 2 }, { BX, 3 }, { BX, 4 }, { BX, 5 }, { BY, 0 }, { BY, 1 },
			{ BY, 2 }, { BY, 3 }, { RY, 0 }, { RY, 1 }, { RY, 2 }, { RY, 3 }, { RY, 4 }, { BZ, 2 }, { RZ, 0 },
			{ RZ, 1 }, { RZ, 2 }, { RZ, 3 }, { RZ, 4 }, { BZ, 3 }, { D, 0 }, { D, 1 }, { D, 2 }, { D, 3 },
			{ D, 4 }
		},

		{   // Mode 10 (0x1E) - 6 6 6 6
			{ M, 0 }, { M, 1 }, { M, 2 }, { M, 3 }, { M, 4 }, { RW, 0 }, { RW, 1 }, { RW, 2 }, { RW, 3 },
			{ RW, 4 }, { RW, 5 }, { GZ, 4 }, { BZ, 0 }, { BZ, 1 }, { BY, 4 }, { GW, 0 }, { GW, 1 }, { GW, 2 },
			{ GW, 3 }, { GW, 4 }, { GW, 5 }, { GY, 5 }, { BY, 5 }, { BZ, 2 }, { GY, 4 }, { BW, 0 }, { BW, 1 },
			{ BW, 2 }, { BW, 3 }, { BW, 4 }, { BW, 5 }, { GZ, 5 }, { BZ, 3 }, { BZ, 5 }, { BZ, 4 }, { RX, 0 },
			{ RX, 1 }, { RX, 2 }, { RX, 3 }, { RX, 4 }, { RX, 5 }, { GY, 0 }, { GY, 1 }, { GY, 2 }, { GY, 3 },
			{ GX, 0 }, { GX, 1 }, { GX, 2 }, { GX, 3 }, { GX, 4 }, { GX, 5 }, { GZ, 0 }, { GZ, 1 }, { GZ, 2 },
			{ GZ, 3 }, { BX, 0 }, { BX, 1 }, { BX, 2 }, { BX, 3 }, { BX, 4 }, { BX, 5 }, { BY, 0 }, { BY, 1 },
			{ BY, 2 }, { BY, 3 }, { RY, 0 }, { RY, 1 }, { RY, 2 }, { RY, 3 }, { RY, 4 }, { RY, 5 }, { RZ, 0 },
			{ RZ, 1 }, { RZ, 2 }, { RZ, 3 }, { RZ, 4 }, { RZ, 5 }, { D, 0 }, { D, 1 }, { D, 2 }, { D, 3 },
			{ D, 4 }
		},

		{   // Mode 11 (0x03) - 10 10
			{ M, 0 }, { M, 1 }, { M, 2 }, { M, 3 }, { M, 4 }, { RW, 0 }, { RW, 1 }, { RW, 2 }, { RW, 3 },
			{ RW, 4 }, { RW, 5 }, { RW, 6 }, { RW, 7 }, { RW, 8 }, { RW, 9 }, { GW, 0 }, { GW, 1 }, { GW, 2 },
			{ GW, 3 }, { GW, 4 }, { GW, 5 }, { GW, 6 }, { GW, 7 }, { GW, 8 }, { GW, 9 }, { BW, 0 }, { BW, 1 },
			{ BW, 2 }, { BW, 3 }, { BW, 4 }, { BW, 5 }, { BW, 6 }, { BW, 7 }, { BW, 8 }, { BW, 9 }, { RX, 0 },
			{ RX, 1 }, { RX, 2 }, { RX, 3 }, { RX, 4 }, { RX, 5 }, { RX, 6 }, { RX, 7 }, { RX, 8 }, { RX, 9 },
			{ GX, 0 }, { GX, 1 }, { GX, 2 }, { GX, 3 }, { GX, 4 }, { GX, 5 }, { GX, 6 }, { GX, 7 }, { GX, 8 },
			{ GX, 9 }, { BX, 0 }, { BX, 1 }, { BX, 2 }, { BX, 3 }, { BX, 4 }, { BX, 5 }, { BX, 6 }, { BX, 7 },
			{ BX, 8 }, { BX, 9 }, { NA, 0 }, { NA, 0 }, { NA, 0 }, { NA, 0 }, { NA, 0 }, { NA, 0 }, { NA, 0 },
			{ NA, 0 }, { NA, 0 }, { NA, 0 }, { NA, 0 }, { NA, 0 }, { NA, 0 }, { NA, 0 }, { NA, 0 }, { NA, 0 },
			{ NA, 0 }
		},

		{   // Mode 12 (0x07) - 11 9
			{ M, 0 }, { M, 1 }, { M, 2 }, { M, 3 }, { M, 4 }, { RW, 0 }, { RW, 1 }, { RW, 2 }, { RW, 3 },
			{ RW, 4 }, { RW, 5 }, { RW, 6 }, { RW, 7 }, { RW, 8 }, { RW, 9 }, { GW, 0 }, { GW, 1 }, { GW, 2 },
			{ GW, 3 }, { GW, 4 }, { GW, 5 }, { GW, 6 }, { GW, 7 }, { GW, 8 }, { GW, 9 }, { BW, 0 }, { BW, 1 },
			{ BW, 2 }, { BW, 3 }, { BW, 4 }, { BW, 5 }, { BW, 6 }, { BW, 7 }, { BW, 8 }, { BW, 9 }, { RX, 0 },
			{ RX, 1 }, { RX, 2 }, { RX, 3 }, { RX, 4 }, { RX, 5 }, { RX, 6 }, { RX, 7 }, { RX, 8 }, { RW, 10 },
			{ GX, 0 }, { GX, 1 }, { GX, 2 }, { GX, 3 }, { GX, 4 }, { GX, 5 }, { GX, 6 }, { GX, 7 }, { GX, 8 },
			{ GW, 10 }, { BX, 0 }, { BX, 1 }, { BX, 2 }, { BX, 3 }, { BX, 4 }, { BX, 5 }, { BX, 6 }, { BX, 7 },
			{ BX, 8 }, { BW, 10 }, { NA, 0 }, { NA, 0 }, { NA, 0 }, { NA, 0 }, { NA, 0 }, { NA, 0 }, { NA, 0 },
			{ NA, 0 }, { NA, 0 }, { NA, 0 }, { NA, 0 }, { NA, 0 }, { NA, 0 }, { NA, 0 }, { NA, 0 }, { NA, 0 },
			{ NA, 0 }
		},

		{   // Mode 13 (0x0B) - 12 8
			{ M, 0 }, { M, 1 }, { M, 2 }, { M, 3 }, { M, 4 }, { RW, 0 }, { RW, 1 }, { RW, 2 }, { RW, 3 },
			{ RW, 4 }, { RW, 5 }, { RW, 6 }, { RW, 7 }, { RW, 8 }, { RW, 9 }, { GW, 0 }, { GW, 1 }, { GW, 2 },
			{ GW, 3 }, { GW, 4 }, { GW, 5 }, { GW, 6 }, { GW, 7 }, { GW, 8 }, { GW, 9 }, { BW, 0 }, { BW, 1 },
			{ BW, 2 }, { BW, 3 }, { BW, 4 }, { BW, 5 }, { BW, 6 }, { BW, 7 }, { BW, 8 }, { BW, 9 }, { RX, 0 },
			{ RX, 1 }, { RX, 2 }, { RX, 3 }, { RX, 4 }, { RX, 5 }, { RX, 6 }, { RX, 7 }, { RW, 11 }, { RW, 10 },
			{ GX, 0 }, { GX, 1 }, { GX, 2 }, { GX, 3 }, { GX, 4 }, { GX, 5 }, { GX, 6 }, { GX, 7 }, { GW, 11 },
			{ GW, 10 }, { BX, 0 }, { BX, 1 }, { BX, 2 }, { BX, 3 }, { BX, 4 }, { BX, 5 }, { BX, 6 }, { BX, 7 },
			{ BW, 11 }, { BW, 10 }, { NA, 0 }, { NA, 0 }, { NA, 0 }, { NA, 0 }, { NA, 0 }, { NA, 0 }, { NA, 0 },
			{ NA, 0 }, { NA, 0 }, { NA, 0 }, { NA, 0 }, { NA, 0 }, { NA, 0 }, { NA, 0 }, { NA, 0 }, { NA, 0 },
			{ NA, 0 }
		},

		{   // Mode 14 (0x0F) - 16 4
			{ M, 0 }, { M, 1 }, { M, 2 }, { M, 3 }, { M, 4 }, { RW, 0 }, { RW, 1 }, { RW, 2 }, { RW, 3 },
			{ RW, 4 }, { RW, 5 }, { RW, 6 }, { RW, 7 }, { RW, 8 }, { RW, 9 }, { GW, 0 }, { GW, 1 }, { GW, 2 },
			{ GW, 3 }, { GW, 4 }, { GW, 5 }, { GW, 6 }, { GW, 7 }, { GW, 8 }, { GW, 9 }, { BW, 0 }, { BW, 1 },
			{ BW, 2 }, { BW, 3 }, { BW, 4 }, { BW, 5 }, { BW, 6 }, { BW, 7 }, { BW, 8 }, { BW, 9 }, { RX, 0 },
			{ RX, 1 }, { RX, 2 }, { RX, 3 }, { RW, 15 }, { RW, 14 }, { RW, 13 }, { RW, 12 }, { RW, 11 }, { RW, 10 },
			{ GX, 0 }, { GX, 1 }, { GX, 2 }, { GX, 3 }, { GW, 15 }, { GW, 14 }, { GW, 13 }, { GW, 12 }, { GW, 11 },
			{ GW, 10 }, { BX, 0 }, { BX, 1 }, { BX, 2 }, { BX, 3 }, { BW, 15 }, { BW, 14 }, { BW, 13 }, { BW, 12 },
			{ BW, 11 }, { BW, 10 }, { NA, 0 }, { NA, 0 }, { NA, 0 }, { NA, 0 }, { NA, 0 }, { NA, 0 }, { NA, 0 },
			{ NA, 0 }, { NA, 0 }, { NA, 0 }, { NA, 0 }, { NA, 0 }, { NA, 0 }, { NA, 0 }, { NA, 0 }, { NA, 0 },
			{ NA, 0 }
		}
	};

	// Mode, Partitions, Transformed, IndexPrec, RGBAPrec
	TexCompressionBC6U::ModeInfo const TexCompressionBC6U::mode_info_[] =
	{
		{ 0x00, 1, true, 3, { { ARGBColor32(0, 10, 10, 10), ARGBColor32(0, 5, 5, 5) },
			{ ARGBColor32(0, 5, 5, 5), ARGBColor32(0, 5, 5, 5) } } },
		{ 0x01, 1, true, 3, { { ARGBColor32(0, 7, 7, 7), ARGBColor32(0, 6, 6, 6) },
			{ ARGBColor32(0, 6, 6, 6), ARGBColor32(0, 6, 6, 6) } } },
		{ 0x02, 1, true, 3, { { ARGBColor32(0, 11, 11, 11), ARGBColor32(0, 5, 4, 4) },
			{ ARGBColor32(0, 5, 4, 4), ARGBColor32(0, 5, 4, 4) } } },
		{ 0x06, 1, true, 3, { { ARGBColor32(0, 11, 11, 11), ARGBColor32(0, 4, 5, 4) },
			{ ARGBColor32(0, 4, 5, 4), ARGBColor32(0, 4, 5, 4) } } },
		{ 0x0A, 1, true, 3, { { ARGBColor32(0, 11, 11, 11), ARGBColor32(0, 4, 4, 5) },
			{ ARGBColor32(0, 4, 4, 5), ARGBColor32(0, 4, 4, 5) } } },
		{ 0x0E, 1, true, 3, { { ARGBColor32(0, 9, 9, 9), ARGBColor32(0, 5, 5, 5) },
			{ ARGBColor32(0, 5, 5, 5), ARGBColor32(0, 5, 5, 5) } } },
		{ 0x12, 1, true, 3, { { ARGBColor32(0, 8, 8, 8), ARGBColor32(0, 6, 5, 5) },
			{ ARGBColor32(0, 6, 5, 5), ARGBColor32(0, 6, 5, 5) } } },
		{ 0x16, 1, true, 3, { { ARGBColor32(0, 8, 8, 8), ARGBColor32(0, 5, 6, 5) },
			{ ARGBColor32(0, 5, 6, 5), ARGBColor32(0, 5, 6, 5) } } },
		{ 0x1A, 1, true, 3, { { ARGBColor32(0, 8, 8, 8), ARGBColor32(0, 5, 5, 6) },
			{ ARGBColor32(0, 5, 5, 6), ARGBColor32(0, 5, 5, 6) } } },
		{ 0x1E, 1, false, 3, { { ARGBColor32(0, 6, 6, 6), ARGBColor32(0, 6, 6, 6) },
			{ ARGBColor32(0, 6, 6, 6), ARGBColor32(0, 6, 6, 6) } } },
		{ 0x03, 0, false, 4, { { ARGBColor32(0, 10, 10, 10), ARGBColor32(0, 10, 10, 10) },
			{ ARGBColor32(0, 0, 0, 0), ARGBColor32(0, 0, 0, 0) } } },
		{ 0x07, 0, true, 4, { { ARGBColor32(0, 11, 11, 11), ARGBColor32(0, 9, 9, 9) },
			{ ARGBColor32(0, 0, 0, 0), ARGBColor32(0, 0, 0, 0) } } },
		{ 0x0B, 0, true, 4, { { ARGBColor32(0, 12, 12, 12), ARGBColor32(0, 8, 8, 8) },
			{ ARGBColor32(0, 0, 0, 0), ARGBColor32(0, 0, 0, 0) } } },
		{ 0x0F, 0, true, 4, { { ARGBColor32(0, 16, 16, 16), ARGBColor32(0, 4, 4, 4) },
			{ ARGBColor32(0, 0, 0, 0), ARGBColor32(0, 0, 0, 0) } } }
	};

	int const TexCompressionBC6U::mode_to_info_[] =
	{
		0, // Mode 1   - 0x00
		1, // Mode 2   - 0x01
		2, // Mode 3   - 0x02
		10, // Mode 11  - 0x03
		-1, // Invalid  - 0x04
		-1, // Invalid  - 0x05
		3, // Mode 4   - 0x06
		11, // Mode 12  - 0x07
		-1, // Invalid  - 0x08
		-1, // Invalid  - 0x09
		4, // Mode 5   - 0x0a
		12, // Mode 13  - 0x0b
		-1, // Invalid  - 0x0c
		-1, // Invalid  - 0x0d
		5, // Mode 6   - 0x0e
		13, // Mode 14  - 0x0f
		-1, // Invalid  - 0x10
		-1, // Invalid  - 0x11
		6, // Mode 7   - 0x12
		-1, // Reserved - 0x13
		-1, // Invalid  - 0x14
		-1, // Invalid  - 0x15
		7, // Mode 8   - 0x16
		-1, // Reserved - 0x17
		-1, // Invalid  - 0x18
		-1, // Invalid  - 0x19
		8, // Mode 9   - 0x1a
		-1, // Reserved - 0x1b
		-1, // Invalid  - 0x1c
		-1, // Invalid  - 0x1d
		9, // Mode 10  - 0x1e
		-1, // Resreved - 0x1f
	};

	TexCompressionBC6U::TexCompressionBC6U()
	{
		block_width_ = block_height_ = 4;
		block_depth_ = 1;
		block_bytes_ = NumFormatBytes(EF_BC6) * 4;
		decoded_fmt_ = EF_ABGR16F;
	}

	void TexCompressionBC6U::EncodeBlock(void* output, void const * input, TexCompressionMethod method)
	{
		UNREF_PARAM(output);
		UNREF_PARAM(input);
		UNREF_PARAM(method);

		// TODO
	}

	void TexCompressionBC6U::DecodeBlock(void* output, void const * input)
	{
		this->DecodeBC6Internal(output, input, false);
	}

	void TexCompressionBC6U::DecodeBC6Internal(void* output, void const * input, bool signed_fmt)
	{
		BOOST_ASSERT(output);
		BOOST_ASSERT(input);

		Vector_T<half, 4>* abgr = static_cast<Vector_T<half, 4>*>(output);

		size_t start_bit = 0;
		uint8_t mode = ReadBits(input, start_bit, 2);
		if (mode > 1)
		{
			mode = (ReadBits(input, start_bit, 3) << 2) | mode;
		}
		BOOST_ASSERT(mode < 32);

		if (mode_to_info_[mode] >= 0)
		{
			BOOST_ASSERT(mode_to_info_[mode] < static_cast<int>(sizeof(mode_info_) / sizeof(mode_info_[0])));
			ModeDescriptor const * desc = mode_desc_[mode_to_info_[mode]];

			BOOST_ASSERT(mode_to_info_[mode] < static_cast<int>(sizeof(mode_desc_) / sizeof(mode_desc_[0])));
			ModeInfo const & info = mode_info_[mode_to_info_[mode]];

			array<std::pair<int3, int3>, BC6_MAX_REGIONS> end_pts;
			memset(&end_pts[0], 0, BC6_MAX_REGIONS * sizeof(end_pts[0]));
			uint32_t shape = 0;

			// Read header
			size_t const header_bits = info.partitions > 0 ? 82 : 65;
			while (start_bit < header_bits)
			{
				size_t curr_bit = start_bit;
				if (ReadBit(input, start_bit))
				{
					size_t val = 1UL << desc[curr_bit].bit;
					switch (desc[curr_bit].field)
					{
					case D:
						shape |= val;
						break;
					case RW:
						end_pts[0].first.x() |= val;
						break;
					case RX:
						end_pts[0].second.x() |= val;
						break;
					case RY:
						end_pts[1].first.x() |= val;
						break;
					case RZ:
						end_pts[1].second.x() |= val;
						break;
					case GW:
						end_pts[0].first.y() |= val;
						break;
					case GX:
						end_pts[0].second.y() |= val;
						break;
					case GY:
						end_pts[1].first.y() |= val;
						break;
					case GZ:
						end_pts[1].second.y() |= val;
						break;
					case BW:
						end_pts[0].first.z() |= val;
						break;
					case BX:
						end_pts[0].second.z() |= val;
						break;
					case BY:
						end_pts[1].first.z() |= val;
						break;
					case BZ:
						end_pts[1].second.z() |= val;
						break;

					default:
						memset(abgr, 0, 16 * sizeof(abgr[0]));
						return;
					}
				}
			}

			BOOST_ASSERT(shape < 64);

			// Sign extend necessary end points
			if (signed_fmt)
			{
				SignExtend(end_pts[0].first, info.rgba_prec[0][0]);
			}
			if (signed_fmt || info.transformed)
			{
				BOOST_ASSERT(info.partitions < BC6_MAX_REGIONS);
				for (size_t p = 0; p <= info.partitions; ++ p)
				{
					if (p > 0)
					{
						SignExtend(end_pts[p].first, info.rgba_prec[p][0]);
					}
					SignExtend(end_pts[p].second, info.rgba_prec[p][1]);
				}
			}

			// Inverse transform the end points
			if (info.transformed)
			{
				TransformInverse(&end_pts[0], info.rgba_prec[0][0], signed_fmt);
			}

			// Read indices
			for (size_t i = 0; i < 16; ++ i)
			{
				size_t num_bits = IsFixUpOffset(info.partitions, shape, i) ? info.index_prec - 1 : info.index_prec;
				if (start_bit + num_bits > 128)
				{
					memset(abgr, 0, 16 * sizeof(abgr[0]));
					return;
				}

				uint8_t index = ReadBits(input, start_bit, num_bits);
				if (index >= ((info.partitions > 0) ? 8 : 16))
				{
					memset(abgr, 0, 16 * sizeof(abgr[0]));
					return;
				}

				size_t region = bc67_partition_table[info.partitions][shape][i];
				BOOST_ASSERT(region < BC6_MAX_REGIONS);

				// Unquantize endpoints and interpolate
				int r1 = this->Unquantize(end_pts[region].first.x(), info.rgba_prec[0][0].r(), signed_fmt);
				int g1 = this->Unquantize(end_pts[region].first.y(), info.rgba_prec[0][0].g(), signed_fmt);
				int b1 = this->Unquantize(end_pts[region].first.z(), info.rgba_prec[0][0].b(), signed_fmt);
				int r2 = this->Unquantize(end_pts[region].second.x(), info.rgba_prec[0][0].r(), signed_fmt);
				int g2 = this->Unquantize(end_pts[region].second.y(), info.rgba_prec[0][0].g(), signed_fmt);
				int b2 = this->Unquantize(end_pts[region].second.z(), info.rgba_prec[0][0].b(), signed_fmt);
				int const * weights = bc67_prec_weights[1 + (0 == info.partitions)];
				int3 fc;
				fc.x() = this->FinishUnquantize((r1 * (BC6_WEIGHT_MAX - weights[index])
					+ r2 * weights[index] + BC6_WEIGHT_ROUND) >> BC6_WEIGHT_SHIFT, signed_fmt);
				fc.y() = this->FinishUnquantize((g1 * (BC6_WEIGHT_MAX - weights[index])
					+ g2 * weights[index] + BC6_WEIGHT_ROUND) >> BC6_WEIGHT_SHIFT, signed_fmt);
				fc.z() = this->FinishUnquantize((b1 * (BC6_WEIGHT_MAX - weights[index])
					+ b2 * weights[index] + BC6_WEIGHT_ROUND) >> BC6_WEIGHT_SHIFT, signed_fmt);
				ToF16(abgr[i], fc, signed_fmt);
				abgr[i].w() = half(1.0f);
			}
		}
		else
		{
			// Per the BC6H format spec, we must return opaque black
			for (size_t i = 0; i < 16; ++ i)
			{
				abgr[i] = Vector_T<half, 4>(half(0.0f), half(0.0f), half(0.0f), half(1.0f));
			}
		}
	}

	int TexCompressionBC6U::Unquantize(int comp, uint8_t bits_per_comp, bool signed_fmt)
	{
		int unq = 0, s = 0;
		if (signed_fmt)
		{
			if (bits_per_comp >= 16)
			{
				unq = comp;
			}
			else
			{
				if (comp < 0)
				{
					s = 1;
					comp = -comp;
				}

				if (0 == comp)
				{
					unq = 0;
				}
				else if (comp >= ((1 << (bits_per_comp - 1)) - 1))
				{
					unq = 0x7FFF;
				}
				else
				{
					unq = ((comp << 15) + 0x4000) >> (bits_per_comp - 1);
				}

				if (s)
				{
					unq = -unq;
				}
			}
		}
		else
		{
			if (bits_per_comp >= 15)
			{
				unq = comp;
			}
			else if (0 == comp)
			{
				unq = 0;
			}
			else if (comp == ((1 << bits_per_comp) - 1))
			{
				unq = 0xFFFF;
			}
			else
			{
				unq = ((comp << 16) + 0x8000) >> bits_per_comp;
			}
		}

		return unq;
	}

	int TexCompressionBC6U::FinishUnquantize(int comp, bool signed_fmt)
	{
		if (signed_fmt)
		{
			return (comp < 0) ? -(((-comp) * 31) >> 5) : (comp * 31) >> 5;  // scale the magnitude by 31/32
		}
		else
		{
			return (comp * 31) >> 6;                                        // scale the magnitude by 31/64
		}
	}


	TexCompressionBC6S::TexCompressionBC6S()
	{
		block_width_ = block_height_ = 4;
		block_depth_ = 1;
		block_bytes_ = NumFormatBytes(EF_SIGNED_BC6) * 4;
		decoded_fmt_ = EF_ABGR16F;
	}

	void TexCompressionBC6S::EncodeBlock(void* output, void const * input, TexCompressionMethod method)
	{
		UNREF_PARAM(output);
		UNREF_PARAM(input);
		UNREF_PARAM(method);

		// TODO
	}

	void TexCompressionBC6S::DecodeBlock(void* output, void const * input)
	{
		bc6u_codec_->DecodeBC6Internal(output, input, true);
	}


	// BC7 compression: partitions, partition_bits, p_bits, rotation_bits, index_mode_bits, index_prec, index_prec_2, rgba_prec, rgba_prec_with_p
	TexCompressionBC7::ModeInfo const TexCompressionBC7::mode_info_[] =
	{
		{ 2, 4, 6, 0, 0, 3, 0, ARGBColor32(0, 4, 4, 4), ARGBColor32(0, 5, 5, 5) },
		// Mode 0: Color only, 3 Subsets, RGBP 4441 (unique P-bit), 3-bit indecies, 16 partitions
		{ 1, 6, 2, 0, 0, 3, 0, ARGBColor32(0, 6, 6, 6), ARGBColor32(0, 7, 7, 7) },
		// Mode 1: Color only, 2 Subsets, RGBP 6661 (shared P-bit), 3-bit indecies, 64 partitions
		{ 2, 6, 0, 0, 0, 2, 0, ARGBColor32(0, 5, 5, 5), ARGBColor32(0, 5, 5, 5) },
		// Mode 2: Color only, 3 Subsets, RGB 555, 2-bit indecies, 64 partitions
		{ 1, 6, 4, 0, 0, 2, 0, ARGBColor32(0, 7, 7, 7), ARGBColor32(0, 8, 8, 8) },
		// Mode 3: Color only, 2 Subsets, RGBP 7771 (unique P-bit), 2-bits indecies, 64 partitions
		{ 0, 0, 0, 2, 1, 2, 3, ARGBColor32(6, 5, 5, 5), ARGBColor32(6, 5, 5, 5) },
		// Mode 4: Color w/ Separate Alpha, 1 Subset, RGB 555, A6, 16x2/16x3-bit indices, 2-bit rotation, 1-bit index selector
		{ 0, 0, 0, 2, 0, 2, 2, ARGBColor32(8, 7, 7, 7), ARGBColor32(8, 7, 7, 7) },
		// Mode 5: Color w/ Separate Alpha, 1 Subset, RGB 777, A8, 16x2/16x2-bit indices, 2-bit rotation
		{ 0, 0, 2, 0, 0, 4, 0, ARGBColor32(7, 7, 7, 7), ARGBColor32(8, 8, 8, 8) },
		// Mode 6: Color+Alpha, 1 Subset, RGBAP 77771 (unique P-bit), 16x4-bit indecies
		{ 1, 6, 4, 0, 0, 2, 0, ARGBColor32(5, 5, 5, 5), ARGBColor32(6, 6, 6, 6) }
		// Mode 7: Color+Alpha, 2 Subsets, RGBAP 55551 (unique P-bit), 2-bit indices, 64 partitions
	};

	TexCompressionBC7::TexCompressionBC7()
	{
		block_width_ = block_height_ = 4;
		block_depth_ = 1;
		block_bytes_ = NumFormatBytes(EF_BC7) * 4;
		decoded_fmt_ = EF_ARGB8;
	}

	void TexCompressionBC7::EncodeBlock(void* output, void const * input, TexCompressionMethod method)
	{
		UNREF_PARAM(output);
		UNREF_PARAM(input);
		UNREF_PARAM(method);

		// TODO
	}

	void TexCompressionBC7::DecodeBlock(void* output, void const * input)
	{
		BOOST_ASSERT(output);
		BOOST_ASSERT(input);

		ARGBColor32* argb = static_cast<ARGBColor32*>(output);

		size_t first = 0;
		while ((first < 128) && !ReadBit(input, first));
		size_t mode = first - 1;
		if (mode < 8)
		{
			uint8_t const partitions = mode_info_[mode].partitions;
			BOOST_ASSERT(partitions < BC7_MAX_REGIONS);

			uint8_t const num_end_pts = (partitions + 1) << 1;
			uint8_t const index_prec_1 = mode_info_[mode].index_prec_1;
			uint8_t const index_prec_2 = mode_info_[mode].index_prec_2;
			size_t start_bit = mode + 1;
			array<uint8_t, 6> p;
			uint8_t shape = ReadBits(input, start_bit, mode_info_[mode].partition_bits);
			BOOST_ASSERT(shape < BC7_MAX_SHAPES);

			uint8_t rotation = ReadBits(input, start_bit, mode_info_[mode].rotation_bits);
			BOOST_ASSERT(rotation < 4);

			uint8_t index_mode = ReadBits(input, start_bit, mode_info_[mode].index_mode_bits);
			BOOST_ASSERT(index_mode < 2);

			array<ARGBColor32, BC7_MAX_REGIONS << 1> c;
			ARGBColor32 const & rgba_prec = mode_info_[mode].rgba_prec;
			ARGBColor32 const & rgba_prec_with_p = mode_info_[mode].rgba_prec_with_p;

			BOOST_ASSERT(num_end_pts <= (BC7_MAX_REGIONS << 1));

			static int const comps[] = { ARGBColor32::RChannel, ARGBColor32::GChannel,
				ARGBColor32::BChannel, ARGBColor32::AChannel };

			for (int i = 0; i < 4; ++ i)
			{
				int ch = comps[i];
				for (uint32_t j = 0; j < num_end_pts; ++ j)
				{
					if (start_bit + rgba_prec[ch] > 128)
					{
						memset(argb, 0, 16 * sizeof(argb[0]));
						return;
					}

					c[j][ch] = ((ch != ARGBColor32::AChannel) || rgba_prec.a())
						? ReadBits(input, start_bit, rgba_prec[ch]) : 255;
				}
			}

			BOOST_ASSERT(mode_info_[mode].p_bits <= 6);
			for (uint32_t i = 0; i < mode_info_[mode].p_bits; ++ i)
			{
				if (start_bit > 127)
				{
					memset(argb, 0, 16 * sizeof(argb[0]));
					return;
				}

				p[i] = ReadBit(input, start_bit);
			}

			if (mode_info_[mode].p_bits)
			{
				for (uint32_t i = 0; i < num_end_pts; ++ i)
				{
					size_t pi = i * mode_info_[mode].p_bits / num_end_pts;
					for (uint8_t ch = 0; ch < BC7_NUM_CHANNELS; ++ ch)
					{
						if (rgba_prec[ch] != rgba_prec_with_p[ch])
						{
							c[i][ch] = (c[i][ch] << 1) | p[pi];
						}
					}
				}
			}

			for (uint32_t i = 0; i < num_end_pts; ++ i)
			{
				c[i] = this->Unquantize(c[i], rgba_prec_with_p);
			}

			array<uint8_t, 16> w1;
			array<uint8_t, 16> w2;

			for (uint32_t i = 0; i < 16; ++ i)
			{
				size_t num_bits = IsFixUpOffset(mode_info_[mode].partitions, shape, i)
					? index_prec_1 - 1 : index_prec_1;
				if (start_bit + num_bits > 128)
				{
					memset(argb, 0, 16 * sizeof(argb[0]));
					return;
				}
				w1[i] = ReadBits(input, start_bit, num_bits);
			}

			if (index_prec_2)
			{
				for (uint32_t i = 0; i < 16; ++ i)
				{
					size_t num_bits = i ? index_prec_2 : index_prec_2 - 1;
					if (start_bit + num_bits > 128)
					{
						memset(argb, 0, 16 * sizeof(argb[0]));
						return;
					}
					w2[i] = ReadBits(input, start_bit, num_bits);
				}
			}

			for (uint32_t i = 0; i < 16; ++ i)
			{
				uint8_t region = bc67_partition_table[partitions][shape][i];
				ARGBColor32 out_pixel;
				if (0 == index_prec_2)
				{
					out_pixel = this->Interpolate(c[region << 1], c[(region << 1) + 1],
						w1[i], w1[i], index_prec_1, index_prec_1);
				}
				else if (0 == index_mode)
				{
					out_pixel = this->Interpolate(c[region << 1], c[(region << 1) + 1],
						w1[i], w2[i], index_prec_1, index_prec_2);
				}
				else
				{
					out_pixel = this->Interpolate(c[region << 1], c[(region << 1) + 1],
						w2[i], w1[i], index_prec_2, index_prec_1);
				}

				switch (rotation)
				{
				case 1:
					std::swap(out_pixel.r(), out_pixel.a());
					break;
				case 2:
					std::swap(out_pixel.g(), out_pixel.a());
					break;
				case 3:
					std::swap(out_pixel.b(), out_pixel.a());
					break;
				}

				argb[i] = out_pixel;
			}
		}
		else
		{
			memset(argb, 0, 16 * sizeof(argb[0]));
		}
	}

	uint8_t TexCompressionBC7::Unquantize(uint8_t comp, size_t prec) const
	{
		BOOST_ASSERT((0 < prec) && (prec <= 8));
		comp = comp << (8 - prec);
		return comp | (comp >> prec);
	}

	ARGBColor32 TexCompressionBC7::Unquantize(ARGBColor32 const & c, ARGBColor32 const & rgba_prec) const
	{
		ARGBColor32 q;
		q.r() = this->Unquantize(c.r(), rgba_prec.r());
		q.g() = this->Unquantize(c.g(), rgba_prec.g());
		q.b() = this->Unquantize(c.b(), rgba_prec.b());
		q.a() = rgba_prec.a() ? this->Unquantize(c.a(), rgba_prec.a()) : 255;
		return q;
	}

	ARGBColor32 TexCompressionBC7::Interpolate(ARGBColor32 const & c0, ARGBColor32 const & c1, size_t wc, size_t wa,
			size_t wc_prec, size_t wa_prec) const
	{
		BOOST_ASSERT((wc_prec >= 2) && (wc_prec <= 4));
		BOOST_ASSERT((wa_prec >= 2) && (wa_prec <= 4));
		BOOST_ASSERT(wc < (static_cast<size_t>(1) << wc_prec));
		BOOST_ASSERT(wa < (static_cast<size_t>(1) << wa_prec));

		ARGBColor32 out;

		int const * weights = bc67_prec_weights[wc_prec - 2];
		out.r() = static_cast<uint8_t>((c0.r() * (BC7_WEIGHT_MAX - weights[wc])
			+ c1.r() * weights[wc] + BC7_WEIGHT_ROUND) >> BC7_WEIGHT_SHIFT);
		out.g() = static_cast<uint8_t>((c0.g() * (BC7_WEIGHT_MAX - weights[wc])
			+ c1.g() * weights[wc] + BC7_WEIGHT_ROUND) >> BC7_WEIGHT_SHIFT);
		out.b() = static_cast<uint8_t>((c0.b() * (BC7_WEIGHT_MAX - weights[wc])
			+ c1.b() * weights[wc] + BC7_WEIGHT_ROUND) >> BC7_WEIGHT_SHIFT);

		weights = bc67_prec_weights[wa_prec - 2];
		out.a() = static_cast<uint8_t>((c0.a() * (BC7_WEIGHT_MAX - weights[wa])
			+ c1.a() * weights[wa] + BC7_WEIGHT_ROUND) >> BC7_WEIGHT_SHIFT);

		return out;
	}


	void BC4ToBC1G(BC1Block& bc1, BC4Block const & bc4)
	{
		bc1.clr_0 = (bc4.alpha_0 >> 2) << 5;
		bc1.clr_1 = (bc4.alpha_1 >> 2) << 5;
		bool swap_clr = false;
		if (bc4.alpha_0 < bc4.alpha_1)
		{
			swap_clr = true;
		}
		for (int i = 0; i < 2; ++ i)
		{
			uint32_t alpha32 = (bc4.bitmap[i * 3 + 2] << 16) | (bc4.bitmap[i * 3 + 1] << 8) | (bc4.bitmap[i * 3 + 0] << 0);
			uint16_t mask = 0;
			for (int j = 0; j < 8; ++ j)
			{
				uint16_t bit = (alpha32 >> (j * 3)) & 0x7;
				if (swap_clr)
				{
					switch (bit)
					{
					case 0:
					case 6:
						bit = 0;
						break;

					case 1:
					case 7:
						bit = 1;
						break;

					case 2:
					case 3:
						bit = 2;
						break;

					case 4:
					case 5:
						bit = 3;
						break;
					}
				}
				else
				{
					switch (bit)
					{
					case 0:
					case 2:
						bit = 0;
						break;

					case 1:
					case 5:
					case 7:
						bit = 1;
						break;

					case 3:
					case 4:
					case 6:
						bit = 2;
						break;
					}
				}

				mask |= bit << (j * 2);
			}

			bc1.bitmap[i] = mask;
		}
	}
}
