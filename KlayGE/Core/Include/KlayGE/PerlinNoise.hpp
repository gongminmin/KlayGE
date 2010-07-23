// PerlinNoise.hpp
// KlayGE 数学函数库 头文件
// Ver 3.4.0
// 版权所有(C) 龚敏敏, 2005-2006
// Homepage: http://www.klayge.org
// This algorithm is based on the great work done by Ken Perlin.
// http://mrl.nyu.edu/~perlin/doc/oscar.html
//
// 3.4.0
// 使用了Improving Noise (2006.8.6)
//
// 2.5.0
// 初次建立 (2005.4.11)
//
// 修改记录
///////////////////////////////////////////////////////////////////////////////

#ifndef _PERLINNOISE_HPP
#define _PERLINNOISE_HPP

#include <KlayGE/Math.hpp>

#pragma once

namespace KlayGE
{
	namespace MathLib
	{
		// noise functions over 1, 2, and 3 dimensions

		template <typename T>
		class PerlinNoise
		{
		public:
			static PerlinNoise& Instance()
			{
				static PerlinNoise instance;
				return instance;
			}

			T noise(T const & x)
			{
				int bx;
				Vector_T<T, 1> r;

				this->setup(x, bx, r.x());

				T u = this->fade(r.x());

				int A  = p_[bx];
				int AA = p_[A];
				int B  = p_[bx + 1];
				int BA = p_[B];

				return lerp(this->grad(p_[AA], r),
								this->grad(p_[BA], r + Vector_T<T, 1>(-1)), u);
			}

			T noise(T const & x, T const & y)
			{
				int bx, by;
				Vector_T<T, 2> r;

				this->setup(x, bx, r.x());
				this->setup(y, by, r.y());

				T u = this->fade(r.x());
				T v = this->fade(r.y());

				int A  = p_[bx] + by;
				int AA = p_[A];
				int AB = p_[A + 1];
				int B  = p_[bx + 1] + by;
				int BA = p_[B];
				int BB = p_[B + 1];

				return lerp(lerp(this->grad(p_[AA], r),
									this->grad(p_[BA], r + Vector_T<T, 2>(-1, 0)), u),
								lerp(this->grad(p_[AB], r + Vector_T<T, 2>(0, -1)),
									this->grad(p_[BB], r +  + Vector_T<T, 2>(-1, -1)), u), v);
			}

			T noise(T const & x, T const & y, T const & z)
			{
				int bx, by, bz;
				Vector_T<T, 3> r;

				this->setup(x, bx, r.x());
				this->setup(y, by, r.y());
				this->setup(z, bz, r.z());

				T u = this->fade(r.x());
				T v = this->fade(r.y());
				T w = this->fade(r.z());

				// HASH COORDINATES OF THE 8 CUBE CORNERS,
				int A  = p_[bx] + by;
				int AA = p_[A] + bz;
				int AB = p_[A + 1] + bz;
				int B  = p_[bx + 1] + by;
				int BA = p_[B] + bz;
				int BB = p_[B + 1] + bz;

				return lerp(lerp(lerp(this->grad(p_[AA], r),	// AND ADD
									this->grad(p_[BA], r + Vector_T<T, 3>(-1, 0, 0)), u),			// BLENDED
								lerp(this->grad(p_[AB], r + Vector_T<T, 3>(0, -1, 0)),				// RESULTS
									this->grad(p_[BB], r + Vector_T<T, 3>(-1, -1, 0)), u), v),		// FROM  8
							lerp(lerp(this->grad(p_[AA + 1], r + Vector_T<T, 3>(0, 0, -1)),			// CORNERS
									this->grad(p_[BA + 1], r + Vector_T<T, 3>(-1, 0, -1)), u),		// OF CUBE
								lerp(this->grad(p_[AB + 1], r + Vector_T<T, 3>(0, -1, -1)),
									this->grad(p_[BB + 1], r + Vector_T<T, 3>(-1, -1, -1)), u), v), w);
			}

			T fBm(T const & x, T const & y, int octaves, T const & lacunarity = T(2), T const & gain = T(0.5))
			{
				T sum(0);
				T freq(1), amp(0.5);
				for (int i = 0; i < octaves; ++ i)
				{
					sum += this->noise(freq * x, freq * y) * amp;
					freq *= lacunarity;
					amp *= gain;
				}
				return sum;
			}

			T fBm(T const & x, T const & y, T const & z, int octaves, T const & lacunarity = T(2), T const & gain = T(0.5))
			{
				T sum(0);
				T freq(1), amp(0.5);
				for (int i = 0; i < octaves; ++ i)
				{
					sum += this->noise(freq * x, freq * y, freq * z) * amp;
					freq *= lacunarity;
					amp *= gain;
				}
				return sum;
			}

			T turbulence(T const & x, T const & y, int octaves, T const & lacunarity = T(2), T const & gain = T(0.5))
			{
				T sum(0);
				T freq(1), amp(1);
				for (int i = 0; i < octaves; ++ i)
				{
					sum += MathLib::abs(this->noise(freq * x, freq * y)) * amp;
					freq *= lacunarity;
					amp *= gain;
				}
				return sum;
			}

			T turbulence(T const & x, T const & y, T const & z, int octaves, T const & lacunarity = T(2), T const & gain = T(0.5))
			{
				T sum(0);
				T freq(1), amp(1);
				for (int i = 0; i < octaves; ++ i)
				{
					sum += MathLib::abs(this->noise(freq * x, freq * y, freq * z)) * amp;
					freq *= lacunarity;
					amp *= gain;
				}
				return sum;
			}

			T tileable_noise(T const & x, T const & w)
			{
				return (this->noise(x + 0) * (w - x)
					+ this->noise(x - w) * (0 + x)) / w;
			}

			T tileable_noise(T const & x, T const & y,
				T const & w, T const & h)
			{
				return (this->noise(x + 0, y + 0) * (w - x) * (h - y)
					+ this->noise(x - w, y + 0) * (0 + x) * (h - y)
					+ this->noise(x + 0, y - h) * (w - x) * (0 + y)
					+ this->noise(x - w, y - h) * (0 + x) * (0 + y)) / (w * h);
			}

			T tileable_noise(T const & x, T const & y, T const & z,
				T const & w, T const & h, T const & d)
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

			T tileable_fBm(T const & x, T const & y, T const & w, T const & h,
				int octaves, T const & lacunarity = T(2), T const & gain = T(0.5))
			{
				T sum(0);
				T freq(1), amp(0.5);
				for (int i = 0; i < octaves; ++ i)
				{
					sum += this->tileable_noise(freq * x, freq * y, freq * w, freq * h) * amp;
					freq *= lacunarity;
					amp *= gain;
				}
				return sum;
			}

			T tileable_fBm(T const & x, T const & y, T const & z, T const & w, T const & h, T const & d,
				int octaves, T const & lacunarity = T(2), T const & gain = T(0.5))
			{
				T sum(0);
				T freq(1), amp(0.5);
				for (int i = 0; i < octaves; ++ i)
				{
					sum += this->tileable_noise(freq * x, freq * y, freq * z, freq * w, freq * h, freq * d) * amp;
					freq *= lacunarity;
					amp *= gain;
				}
				return sum;
			}

			T tileable_turbulence(T const & x, T const & y,
				T const & w, T const & h, int octaves, T const & lacunarity = T(2), T const & gain = T(0.5))
			{
				T sum(0);
				T freq(1), amp(1);
				for (int i = 0; i < octaves; ++ i)
				{
					sum += MathLib::abs(this->tileable_noise(freq * x, freq * y, w * freq, h * freq)) * amp;
					freq *= lacunarity;
					amp *= gain;
				}
				return sum;
			}

			T tileable_turbulence(T const & x, T const & y, T const & z,
				T const & w, T const & h, T const & d, int octaves, T const & lacunarity = T(2), T const & gain = T(0.5))
			{
				T sum(0);
				T freq(1), amp(1);
				for (int i = 0; i < octaves; ++ i)
				{
					sum += MathLib::abs(this->tileable_noise(freq * x, freq * y, freq * z, w * freq, h * freq, d * freq)) * amp;
					freq *= lacunarity;
					amp *= gain;
				}
				return sum;
			}

		private:
			PerlinNoise()
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
				g_[12] = Vector_T<T, 3>(1, 1, 0);
				g_[13] = Vector_T<T, 3>(0, -1, 1);
				g_[14] = Vector_T<T, 3>(-1, 1, 0);
				g_[15] = Vector_T<T, 3>(0, -1, -1);
			}

			void setup(T const & i, int& b, T& r)
			{
				T x = i + 0x1000;
				b = (static_cast<int>(x)) & 255;
				r = frac(x);
			}

			T grad(int hash, Vector_T<T, 1> const & p)
			{
				return this->grad(hash, Vector_T<T, 2>(p.x(), 0));
			}

			T grad(int hash, Vector_T<T, 2> const & p)
			{
				return this->grad(hash, Vector_T<T, 3>(p.x(), p.y(), 0));
			}

			T grad(int hash, Vector_T<T, 3> const & p)
			{
				return dot(g_[hash & 15], p);
			}

			T fade(T const & t)
			{
				return t * t * t * (t * (t * 6 - 15) + 10);
			}

		private:
			int p_[512];
			Vector_T<T, 3> g_[16];
		};
	}
}

#endif		// _PERLINNOISE_HPP
