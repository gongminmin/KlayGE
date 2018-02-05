/**
 * @file Noise.cpp
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

#include <KFL/Noise.hpp>

namespace KlayGE
{
	namespace MathLib
	{
		template <typename T>
		SimplexNoise<T>::SimplexNoise() noexcept
		{
			int const permutation[] =
			{
				151, 160, 137, 91, 90, 15,
				131, 13, 201, 95, 96, 53, 194, 233, 7, 225, 140, 36, 103, 30, 69, 142, 8, 99, 37, 240, 21, 10, 23,
				190, 6, 148, 247, 120, 234, 75, 0, 26, 197, 62, 94, 252, 219, 203, 117, 35, 11, 32, 57, 177, 33,
				88, 237, 149, 56, 87, 174, 20, 125, 136, 171, 168, 68, 175, 74, 165, 71, 134, 139, 48, 27, 166,
				77, 146, 158, 231, 83, 111, 229, 122, 60, 211, 133, 230, 220, 105, 92, 41, 55, 46, 245, 40, 244,
				102, 143, 54, 65, 25, 63, 161, 1, 216, 80, 73, 209, 76, 132, 187, 208, 89, 18, 169, 200, 196,
				135, 130, 116, 188, 159, 86, 164, 100, 109, 198, 173, 186, 3, 64, 52, 217, 226, 250, 124, 123,
				5, 202, 38, 147, 118, 126, 255, 82, 85, 212, 207, 206, 59, 227, 47, 16, 58, 17, 182, 189, 28, 42,
				223, 183, 170, 213, 119, 248, 152, 2, 44, 154, 163, 70, 221, 153, 101, 155, 167, 43, 172, 9,
				129, 22, 39, 253, 19, 98, 108, 110, 79, 113, 224, 232, 178, 185, 112, 104, 218, 246, 97, 228,
				251, 34, 242, 193, 238, 210, 144, 12, 191, 179, 162, 241, 81, 51, 145, 235, 249, 14, 239, 107,
				49, 192, 214, 31, 181, 199, 106, 157, 184, 84, 204, 176, 115, 121, 50, 45, 127, 4, 150, 254,
				138, 236, 205, 93, 222, 114, 67, 29, 24, 72, 243, 141, 128, 195, 78, 66, 215, 61, 156, 180
			};
			for (int i = 0; i < 256; ++ i)
			{
				p_[256 + i] = p_[i] = permutation[i];
			}

			g_[0] = Vector_T<T, 3>(1, 1, 0);
			g_[1] = Vector_T<T, 3>(-1, 1, 0);
			g_[2] = Vector_T<T, 3>(1, -1, 0);
			g_[3] = Vector_T<T, 3>(-1, -1, 0);
			g_[4] = Vector_T<T, 3>(1, 0, 1);
			g_[5] = Vector_T<T, 3>(-1, 0, 1);
			g_[6] = Vector_T<T, 3>(1, 0, -1);
			g_[7] = Vector_T<T, 3>(-1, 0, -1);
			g_[8] = Vector_T<T, 3>(0, 1, 1);
			g_[9] = Vector_T<T, 3>(0, -1, 1);
			g_[10] = Vector_T<T, 3>(0, 1, -1);
			g_[11] = Vector_T<T, 3>(0, -1, -1);
		}

		template <typename T>
		SimplexNoise<T>& SimplexNoise<T>::Instance()
		{
			static SimplexNoise<T> instance;
			return instance;
		}

		template <typename T>
		T SimplexNoise<T>::noise(T x, T y) noexcept
		{
			T const F2 = T(0.366025403784);//(sqrt(3) - 1) / 2
			T const G2 = T(0.211324865405);//(3 - sqrt(3)) / 6

			T s = (x + y) * F2;
			int i = static_cast<int>(floor(x + s));
			int j = static_cast<int>(floor(y + s));
			T t = (i + j) * G2;
			T X0 = i - t;
			T Y0 = j - t;
			T x0 = x - X0;
			T y0 = y - Y0;

			int i1, j1;
			if (x0 > y0)
			{
				i1 = 1;
				j1 = 0;
			}
			else
			{
				i1 = 0;
				j1 = 1;
			}

			T x1 = x0 - i1 + G2;
			T y1 = y0 - j1 + G2;
			T x2 = x0 - 1 + 2 * G2;
			T y2 = y0 - 1 + 2 * G2;

			int ii = i & 255;
			int jj = j & 255;

			T n = 0;

			T t0 = T(0.5) - x0 * x0 - y0 * y0;
			if (t0 > 0)
			{
				t0 *= t0;
				n += t0 * t0 * dot(g_[p_[ii + p_[jj]] % 12], Vector_T<T, 3>(x0, y0, T(0)));
			}
			T t1 = T(0.5) - x1 * x1 - y1 * y1;
			if (t1 > 0)
			{
				t1 *= t1;
				n += t1 * t1 * dot(g_[p_[ii + i1 + p_[jj + j1]] % 12], Vector_T<T, 3>(x1, y1, T(0)));
			}
			T t2 = T(0.5) - x2 * x2 - y2 * y2;
			if (t2 > 0)
			{
				t2 *= t2;
				n += t2 * t2 * dot(g_[p_[ii + 1 + p_[jj + 1]] % 12], Vector_T<T, 3>(x2, y2, T(0)));
			}

			return 70 * n;
		}

		template <typename T>
		T SimplexNoise<T>::noise(T x, T y, T z) noexcept
		{
			T const F3 = 1 / T(3);
			T const G3 = 1 / T(6);

			T s = (x + y + z) * F3;
			int i = static_cast<int>(floor(x + s));
			int j = static_cast<int>(floor(y + s));
			int k = static_cast<int>(floor(z + s));
			T t = (i + j + k) * G3; 
			T X0 = i - t;
			T Y0 = j - t;
			T Z0 = k - t;
			T x0 = x - X0;
			T y0 = y - Y0;
			T z0 = z - Z0;

			int i1, j1, k1;
			int i2, j2, k2;
			if (x0 >= y0)
			{
				if (y0 >= z0)
				{
					// X Y Z order
					i1 = 1;
					j1 = 0;
					k1 = 0;
					i2 = 1;
					j2 = 1;
					k2 = 0;
				}
				else if (x0 >= z0)
				{
					// X Z Y order
					i1 = 1;
					j1 = 0;
					k1 = 0;
					i2 = 1;
					j2 = 0;
					k2 = 1;
				}
				else
				{
					// Z X Y order
					i1 = 0;
					j1 = 0;
					k1 = 1;
					i2 = 1;
					j2 = 0;
					k2 = 1;
				}
			}
			else
			{
				if (y0 < z0)
				{
					// Z Y X order
					i1 = 0;
					j1 = 0;
					k1 = 1;
					i2 = 0;
					j2 = 1;
					k2 = 1;
				}
				else if (x0 < z0)
				{
					// Y Z X order
					i1 = 0;
					j1 = 1;
					k1 = 0;
					i2 = 0;
					j2 = 1;
					k2 = 1;
				}
				else
				{
					// Y X Z order
					i1 = 0;
					j1 = 1;
					k1 = 0;
					i2 = 1;
					j2 = 1;
					k2 = 0;
				}
			}

			T x1 = x0 - i1 + G3;
			T y1 = y0 - j1 + G3;
			T z1 = z0 - k1 + G3;
			T x2 = x0 - i2 + 2 * G3;
			T y2 = y0 - j2 + 2 * G3;
			T z2 = z0 - k2 + 2 * G3;
			T x3 = x0 - 1 + 3 * G3;
			T y3 = y0 - 1 + 3 * G3;
			T z3 = z0 - 1 + 3 * G3;

			int ii = i & 255;
			int jj = j & 255;
			int kk = k & 255;

			T n = 0;

			T t0 = T(0.6) - x0 * x0 - y0 * y0 - z0 * z0;
			if (t0 > 0)
			{
				t0 *= t0;
				n += t0 * t0 * dot(g_[p_[ii + p_[jj + p_[kk]]] % 12], Vector_T<T, 3>(x0, y0, z0));
			}
			T t1 = T(0.6) - x1 * x1 - y1 * y1 - z1 * z1;
			if (t1 > 0)
			{
				t1 *= t1;
				n += t1 * t1 * dot(g_[p_[ii + i1 + p_[jj + j1 + p_[kk + k1]]] % 12], Vector_T<T, 3>(x1, y1, z1));
			}
			T t2 = T(0.6) - x2 * x2 - y2 * y2 - z2 * z2;
			if (t2 > 0)
			{
				t2 *= t2;
				n += t2 * t2 * dot(g_[p_[ii + i2 + p_[jj + j2 + p_[kk + k2]]] % 12], Vector_T<T, 3>(x2, y2, z2));
			}
			T t3 = T(0.6) - x3 * x3 - y3 * y3 - z3 * z3;
			if (t3 > 0)
			{
				t3 *= t3;
				n += t3 * t3 * dot(g_[p_[ii + 1 + p_[jj + 1 + p_[kk + 1]]] % 12], Vector_T<T, 3>(x3, y3, z3));
			}

			return 32 * n;
		}

		template <typename T>
		T SimplexNoise<T>::fBm(T x, T y, int octaves, T lacunarity, T gain) noexcept
		{
			T sum = 0;
			T amp = 1;
			T amp_sum = 0;
			for (int i = 0; i < octaves; ++ i)
			{
				sum += this->noise(x, y) * amp;
				amp_sum += amp;
				x *= lacunarity;
				y *= lacunarity;
				amp *= gain;
			}
			return sum / amp_sum;
		}

		template <typename T>
		T SimplexNoise<T>::fBm(T x, T y, T z, int octaves, T lacunarity, T gain) noexcept
		{
			T sum = 0;
			T amp = 1;
			T amp_sum = 0;
			for (int i = 0; i < octaves; ++ i)
			{
				sum += this->noise(x, y, z) * amp;
				amp_sum += amp;
				x *= lacunarity;
				y *= lacunarity;
				z *= lacunarity;
				amp *= gain;
			}
			return sum / amp_sum;
		}

		template <typename T>
		T SimplexNoise<T>::turbulence(T x, T y, int octaves, T lacunarity, T gain) noexcept
		{
			T sum = 0;
			T amp = 1;
			T amp_sum = 0;
			for (int i = 0; i < octaves; ++ i)
			{
				sum += MathLib::abs(this->noise(x, y)) * amp;
				amp_sum += amp;
				x *= lacunarity;
				y *= lacunarity;
				amp *= gain;
			}
			return sum / amp_sum;
		}

		template <typename T>
		T SimplexNoise<T>::turbulence(T x, T y, T z, int octaves, T lacunarity, T gain) noexcept
		{
			T sum = 0;
			T amp = 1;
			T amp_sum = 0;
			for (int i = 0; i < octaves; ++ i)
			{
				sum += MathLib::abs(this->noise(x, y, z)) * amp;
				amp_sum += amp;
				x *= lacunarity;
				y *= lacunarity;
				z *= lacunarity;
				amp *= gain;
			}
			return sum / amp_sum;
		}

		template <typename T>
		T SimplexNoise<T>::tileable_noise(T x, T y, T w, T h) noexcept
		{
			return (this->noise(x + 0, y + 0) * (w - x) * (h - y)
				+ this->noise(x - w, y + 0) * (0 + x) * (h - y)
				+ this->noise(x + 0, y - h) * (w - x) * (0 + y)
				+ this->noise(x - w, y - h) * (0 + x) * (0 + y)) / (w * h);
		}

		template <typename T>
		T SimplexNoise<T>::tileable_noise(T x, T y, T z, T w, T h, T d) noexcept
		{
			return (this->noise(x + 0, y + 0, z + 0) * (w - x) * (h - y) * (d - z)
				+ this->noise(x - w, y + 0, z + 0) * (0 + x) * (h - y) * (d - z)
				+ this->noise(x + 0, y - h, z + 0) * (w - x) * (0 + y) * (d - z)
				+ this->noise(x - w, y - h, z + 0) * (0 + x) * (0 + y) * (d - z)
				+ this->noise(x + 0, y + 0, z - d) * (w - x) * (h - y) * (0 + z)
				+ this->noise(x - w, y + 0, z - d) * (0 + x) * (h - y) * (0 + z)
				+ this->noise(x + 0, y - h, z - d) * (w - x) * (0 + y) * (0 + z)
				+ this->noise(x - w, y - h, z - d) * (0 + x) * (0 + y) * (0 + z)) / (w * h * d);
		}

		template <typename T>
		T SimplexNoise<T>::tileable_fBm(T x, T y, T w, T h,
			int octaves, T lacunarity, T gain) noexcept
		{
			T sum = 0;
			T amp = 1;
			T amp_sum = 0;
			for (int i = 0; i < octaves; ++ i)
			{
				sum += this->tileable_noise(x, y, w, h) * amp;
				amp_sum += amp;
				x *= lacunarity;
				y *= lacunarity;
				w *= lacunarity;
				h *= lacunarity;
				amp *= gain;
			}
			return sum / amp_sum;
		}

		template <typename T>
		T SimplexNoise<T>::tileable_fBm(T x, T y, T z, T w, T h, T d,
			int octaves, T lacunarity, T gain) noexcept
		{
			T sum = 0;
			T amp = 1;
			T amp_sum = 0;
			for (int i = 0; i < octaves; ++ i)
			{
				sum += this->tileable_noise(x, y, z, w, h, d) * amp;
				amp_sum += amp;
				x *= lacunarity;
				y *= lacunarity;
				z *= lacunarity;
				w *= lacunarity;
				h *= lacunarity;
				d *= lacunarity;
				amp *= gain;
			}
			return sum / amp_sum;
		}

		template <typename T>
		T SimplexNoise<T>::tileable_turbulence(T x, T y,
			T w, T h, int octaves, T lacunarity, T gain) noexcept
		{
			T sum = 0;
			T amp = 1;
			T amp_sum = 0;
			for (int i = 0; i < octaves; ++ i)
			{
				sum += MathLib::abs(this->tileable_noise(x, y, w, h)) * amp;
				amp_sum += amp;
				x *= lacunarity;
				y *= lacunarity;
				w *= lacunarity;
				h *= lacunarity;
				amp *= gain;
			}
			return sum / amp_sum;
		}

		template <typename T>
		T SimplexNoise<T>::tileable_turbulence(T x, T y, T z,
			T w, T h, T d, int octaves, T lacunarity, T gain) noexcept
		{
			T sum = 0;
			T amp = 1;
			T amp_sum = 0;
			for (int i = 0; i < octaves; ++ i)
			{
				sum += MathLib::abs(this->tileable_noise(x, y, z, w, h, d)) * amp;
				amp_sum += amp;
				x *= lacunarity;
				y *= lacunarity;
				z *= lacunarity;
				w *= lacunarity;
				h *= lacunarity;
				d *= lacunarity;
				amp *= gain;
			}
			return sum / amp_sum;
		}


		template class SimplexNoise<float>;
	}
}
