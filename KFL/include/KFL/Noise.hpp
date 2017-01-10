/**
 * @file Noise.hpp
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

#ifndef _KFL_NOISE_HPP
#define _KFL_NOISE_HPP

#pragma once

#include <KFL/Math.hpp>

namespace KlayGE
{
	namespace MathLib
	{
		template <typename T>
		class SimplexNoise final
		{
		public:
			static SimplexNoise& Instance();

			T noise(T x, T y) noexcept;
			T noise(T x, T y, T z) noexcept;

			T fBm(T x, T y, int octaves, T lacunarity = T(2), T gain = T(0.5)) noexcept;
			T fBm(T x, T y, T z, int octaves, T lacunarity = T(2), T gain = T(0.5)) noexcept;

			T turbulence(T x, T y, int octaves, T lacunarity = T(2), T gain = T(0.5)) noexcept;
			T turbulence(T x, T y, T z, int octaves, T lacunarity = T(2), T gain = T(0.5)) noexcept;

			T tileable_noise(T x, T y, T w, T h) noexcept;
			T tileable_noise(T x, T y, T z, T w, T h, T d) noexcept;

			T tileable_fBm(T x, T y, T w, T h,
				int octaves, T lacunarity = T(2), T gain = T(0.5)) noexcept;
			T tileable_fBm(T x, T y, T z, T w, T h, T d,
				int octaves, T lacunarity = T(2), T gain = T(0.5)) noexcept;

			T tileable_turbulence(T x, T y,
				T w, T h, int octaves, T lacunarity = T(2), T gain = T(0.5)) noexcept;
			T tileable_turbulence(T x, T y, T z,
				T w, T h, T d, int octaves, T lacunarity = T(2), T gain = T(0.5)) noexcept;

		private:
			SimplexNoise() noexcept;

		private:
			int p_[512];
			Vector_T<T, 3> g_[12];
		};
	}
}

#endif		// _KFL_NOISE_HPP
