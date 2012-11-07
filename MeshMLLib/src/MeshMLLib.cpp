/**
 * @file MeshMLLib.cpp
 * @author Rui Wang, Minmin Gong
 *
 * @section DESCRIPTION
 *
 * This source file is part of KlayGE's subproject MeshMLLib
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

#include <KlayGE/Types.hpp>
#include <MeshMLLib/MeshMLLib.hpp>

#include <set>
#include <sstream>
#include <algorithm>

#include <boost/assert.hpp>
#include <boost/typeof/typeof.hpp>
#include <boost/foreach.hpp>

namespace
{
	std::string RemoveQuote(std::string const & str)
	{
		std::string ret = str;
		ret.erase(std::remove(ret.begin(), ret.end(), '\"'), ret.end());
		return ret;
	}
}

namespace KlayGE
{
	namespace MathLib
	{
		template <typename T>
		T abs(T const & x)
		{
			return x < T(0) ? -x : x;
		}

		template <typename T>
		T sgn(T const & x)
		{
			return x < T(0) ? T(-1) : (x > T(0) ? T(1) : T(0));
		}

		// From Quake III. But the magic number is from http://www.lomont.org/Math/Papers/2003/InvSqrt.pdf
		float recip_sqrt(float number)
		{
			float const threehalfs = 1.5f;

			float x2 = number * 0.5f;
			union FNI
			{
				float f;
				int32_t i;
			} fni;
			fni.f = number;											// evil floating point bit level hacking
			fni.i = 0x5f375a86 - (fni.i >> 1);						// what the fuck?
			fni.f = fni.f * (threehalfs - (x2 * fni.f * fni.f));	// 1st iteration
			fni.f = fni.f * (threehalfs - (x2 * fni.f * fni.f));		// 2nd iteration, this can be removed

			return fni.f;
		}

		template <typename T>
		bool equal(T const & lhs, T const & rhs)
		{
			return (lhs == rhs);
		}
		template <>
		bool equal<float>(float const & lhs, float const & rhs)
		{
			return (std::abs(lhs - rhs)
				<= std::numeric_limits<float>::epsilon());
		}

		void sincos(float x, float& s, float& c)
		{
			s = sin(x);
			c = cos(x);
		}

		template <typename T>
		typename T::value_type dot(T const & lhs, T const & rhs)
		{
			return detail::dot_helper<typename T::value_type,
							T::elem_num>::Do(&lhs[0], &rhs[0]);
		}

		template <typename T>
		typename T::value_type length_sq(T const & rhs)
		{
			return dot(rhs, rhs);
		}

		template <typename T>
		typename T::value_type length(T const & rhs)
		{
			return sqrt(length_sq(rhs));
		}

		template <typename T>
		T lerp(T const & lhs, T const & rhs, float s)
		{
			return lhs + (rhs - lhs) * s;
		}

		template <typename T>
		T normalize(T const & rhs)
		{
			return rhs * recip_sqrt(length_sq(rhs));
		}
		
		template <typename T>
		Vector_T<T, 3> cross(Vector_T<T, 3> const & lhs, Vector_T<T, 3> const & rhs)
		{
			return Vector_T<T, 3>(lhs.y() * rhs.z() - lhs.z() * rhs.y(),
				lhs.z() * rhs.x() - lhs.x() * rhs.z(),
				lhs.x() * rhs.y() - lhs.y() * rhs.x());
		}

		template <typename T>
		Vector_T<T, 3> transform_quat(Vector_T<T, 3> const & v, Quaternion_T<T> const & quat)
		{
			return v + cross(quat.v(), cross(quat.v(), v) + quat.w() * v) * T(2);
		}

		template <typename T>
		void decompose(Vector_T<T, 3>& scale, Quaternion_T<T>& rot, Vector_T<T, 3>& trans, Matrix4_T<T> const & rhs)
		{
			scale.x() = sqrt(rhs(0, 0) * rhs(0, 0) + rhs(0, 1) * rhs(0, 1) + rhs(0, 2) * rhs(0, 2));
			scale.y() = sqrt(rhs(1, 0) * rhs(1, 0) + rhs(1, 1) * rhs(1, 1) + rhs(1, 2) * rhs(1, 2));
			scale.z() = sqrt(rhs(2, 0) * rhs(2, 0) + rhs(2, 1) * rhs(2, 1) + rhs(2, 2) * rhs(2, 2));

			trans = Vector_T<T, 3>(rhs(3, 0), rhs(3, 1), rhs(3, 2));

			Matrix4_T<T> rot_mat;
			rot_mat(0, 0) = rhs(0, 0) / scale.x();
			rot_mat(0, 1) = rhs(0, 1) / scale.x();
			rot_mat(0, 2) = rhs(0, 2) / scale.x();
			rot_mat(0, 3) = 0;
			rot_mat(1, 0) = rhs(1, 0) / scale.y();
			rot_mat(1, 1) = rhs(1, 1) / scale.y();
			rot_mat(1, 2) = rhs(1, 2) / scale.y();
			rot_mat(1, 3) = 0;
			rot_mat(2, 0) = rhs(2, 0) / scale.z();
			rot_mat(2, 1) = rhs(2, 1) / scale.z();
			rot_mat(2, 2) = rhs(2, 2) / scale.z();
			rot_mat(2, 3) = 0;
			rot_mat(3, 0) = 0;
			rot_mat(3, 1) = 0;
			rot_mat(3, 2) = 0;
			rot_mat(3, 3) = 1;
			rot = to_quaternion(rot_mat);
		}

		template <typename T>
		Matrix4_T<T> mul(Matrix4_T<T> const & lhs, Matrix4_T<T> const & rhs)
		{
			Matrix4_T<T> const tmp(transpose(rhs));

			return Matrix4_T<T>(
				lhs(0, 0) * tmp(0, 0) + lhs(0, 1) * tmp(0, 1) + lhs(0, 2) * tmp(0, 2) + lhs(0, 3) * tmp(0, 3),
				lhs(0, 0) * tmp(1, 0) + lhs(0, 1) * tmp(1, 1) + lhs(0, 2) * tmp(1, 2) + lhs(0, 3) * tmp(1, 3),
				lhs(0, 0) * tmp(2, 0) + lhs(0, 1) * tmp(2, 1) + lhs(0, 2) * tmp(2, 2) + lhs(0, 3) * tmp(2, 3),
				lhs(0, 0) * tmp(3, 0) + lhs(0, 1) * tmp(3, 1) + lhs(0, 2) * tmp(3, 2) + lhs(0, 3) * tmp(3, 3),

				lhs(1, 0) * tmp(0, 0) + lhs(1, 1) * tmp(0, 1) + lhs(1, 2) * tmp(0, 2) + lhs(1, 3) * tmp(0, 3),
				lhs(1, 0) * tmp(1, 0) + lhs(1, 1) * tmp(1, 1) + lhs(1, 2) * tmp(1, 2) + lhs(1, 3) * tmp(1, 3),
				lhs(1, 0) * tmp(2, 0) + lhs(1, 1) * tmp(2, 1) + lhs(1, 2) * tmp(2, 2) + lhs(1, 3) * tmp(2, 3),
				lhs(1, 0) * tmp(3, 0) + lhs(1, 1) * tmp(3, 1) + lhs(1, 2) * tmp(3, 2) + lhs(1, 3) * tmp(3, 3),

				lhs(2, 0) * tmp(0, 0) + lhs(2, 1) * tmp(0, 1) + lhs(2, 2) * tmp(0, 2) + lhs(2, 3) * tmp(0, 3),
				lhs(2, 0) * tmp(1, 0) + lhs(2, 1) * tmp(1, 1) + lhs(2, 2) * tmp(1, 2) + lhs(2, 3) * tmp(1, 3),
				lhs(2, 0) * tmp(2, 0) + lhs(2, 1) * tmp(2, 1) + lhs(2, 2) * tmp(2, 2) + lhs(2, 3) * tmp(2, 3),
				lhs(2, 0) * tmp(3, 0) + lhs(2, 1) * tmp(3, 1) + lhs(2, 2) * tmp(3, 2) + lhs(2, 3) * tmp(3, 3),

				lhs(3, 0) * tmp(0, 0) + lhs(3, 1) * tmp(0, 1) + lhs(3, 2) * tmp(0, 2) + lhs(3, 3) * tmp(0, 3),
				lhs(3, 0) * tmp(1, 0) + lhs(3, 1) * tmp(1, 1) + lhs(3, 2) * tmp(1, 2) + lhs(3, 3) * tmp(1, 3),
				lhs(3, 0) * tmp(2, 0) + lhs(3, 1) * tmp(2, 1) + lhs(3, 2) * tmp(2, 2) + lhs(3, 3) * tmp(2, 3),
				lhs(3, 0) * tmp(3, 0) + lhs(3, 1) * tmp(3, 1) + lhs(3, 2) * tmp(3, 2) + lhs(3, 3) * tmp(3, 3));
		}

		template <typename T>
		T determinant(Matrix4_T<T> const & rhs)
		{
			T const _3142_3241(rhs(2, 0) * rhs(3, 1) - rhs(2, 1) * rhs(3, 0));
			T const _3143_3341(rhs(2, 0) * rhs(3, 2) - rhs(2, 2) * rhs(3, 0));
			T const _3144_3441(rhs(2, 0) * rhs(3, 3) - rhs(2, 3) * rhs(3, 0));
			T const _3243_3342(rhs(2, 1) * rhs(3, 2) - rhs(2, 2) * rhs(3, 1));
			T const _3244_3442(rhs(2, 1) * rhs(3, 3) - rhs(2, 3) * rhs(3, 1));
			T const _3344_3443(rhs(2, 2) * rhs(3, 3) - rhs(2, 3) * rhs(3, 2));

			return rhs(0, 0) * (rhs(1, 1) * _3344_3443 - rhs(1, 2) * _3244_3442 + rhs(1, 3) * _3243_3342)
				- rhs(0, 1) * (rhs(1, 0) * _3344_3443 - rhs(1, 2) * _3144_3441 + rhs(1, 3) * _3143_3341)
				+ rhs(0, 2) * (rhs(1, 0) * _3244_3442 - rhs(1, 1) * _3144_3441 + rhs(1, 3) * _3142_3241)
				- rhs(0, 3) * (rhs(1, 0) * _3243_3342 - rhs(1, 1) * _3143_3341 + rhs(1, 2) * _3142_3241);
		}

		template <typename T>
		Matrix4_T<T> inverse(Matrix4_T<T> const & rhs)
		{
			T const _2132_2231(rhs(1, 0) * rhs(2, 1) - rhs(1, 1) * rhs(2, 0));
			T const _2133_2331(rhs(1, 0) * rhs(2, 2) - rhs(1, 2) * rhs(2, 0));
			T const _2134_2431(rhs(1, 0) * rhs(2, 3) - rhs(1, 3) * rhs(2, 0));
			T const _2142_2241(rhs(1, 0) * rhs(3, 1) - rhs(1, 1) * rhs(3, 0));
			T const _2143_2341(rhs(1, 0) * rhs(3, 2) - rhs(1, 2) * rhs(3, 0));
			T const _2144_2441(rhs(1, 0) * rhs(3, 3) - rhs(1, 3) * rhs(3, 0));
			T const _2233_2332(rhs(1, 1) * rhs(2, 2) - rhs(1, 2) * rhs(2, 1));
			T const _2234_2432(rhs(1, 1) * rhs(2, 3) - rhs(1, 3) * rhs(2, 1));
			T const _2243_2342(rhs(1, 1) * rhs(3, 2) - rhs(1, 2) * rhs(3, 1));
			T const _2244_2442(rhs(1, 1) * rhs(3, 3) - rhs(1, 3) * rhs(3, 1));
			T const _2334_2433(rhs(1, 2) * rhs(2, 3) - rhs(1, 3) * rhs(2, 2));
			T const _2344_2443(rhs(1, 2) * rhs(3, 3) - rhs(1, 3) * rhs(3, 2));
			T const _3142_3241(rhs(2, 0) * rhs(3, 1) - rhs(2, 1) * rhs(3, 0));
			T const _3143_3341(rhs(2, 0) * rhs(3, 2) - rhs(2, 2) * rhs(3, 0));
			T const _3144_3441(rhs(2, 0) * rhs(3, 3) - rhs(2, 3) * rhs(3, 0));
			T const _3243_3342(rhs(2, 1) * rhs(3, 2) - rhs(2, 2) * rhs(3, 1));
			T const _3244_3442(rhs(2, 1) * rhs(3, 3) - rhs(2, 3) * rhs(3, 1));
			T const _3344_3443(rhs(2, 2) * rhs(3, 3) - rhs(2, 3) * rhs(3, 2));

			// 行列式的值
			T const det(determinant(rhs));
			if (!equal<T>(det, 0))
			{
				T invDet(T(1) / det);

				return Matrix4_T<T>(
					+invDet * (rhs(1, 1) * _3344_3443 - rhs(1, 2) * _3244_3442 + rhs(1, 3) * _3243_3342),
					-invDet * (rhs(0, 1) * _3344_3443 - rhs(0, 2) * _3244_3442 + rhs(0, 3) * _3243_3342),
					+invDet * (rhs(0, 1) * _2344_2443 - rhs(0, 2) * _2244_2442 + rhs(0, 3) * _2243_2342),
					-invDet * (rhs(0, 1) * _2334_2433 - rhs(0, 2) * _2234_2432 + rhs(0, 3) * _2233_2332),

					-invDet * (rhs(1, 0) * _3344_3443 - rhs(1, 2) * _3144_3441 + rhs(1, 3) * _3143_3341),
					+invDet * (rhs(0, 0) * _3344_3443 - rhs(0, 2) * _3144_3441 + rhs(0, 3) * _3143_3341),
					-invDet * (rhs(0, 0) * _2344_2443 - rhs(0, 2) * _2144_2441 + rhs(0, 3) * _2143_2341),
					+invDet * (rhs(0, 0) * _2334_2433 - rhs(0, 2) * _2134_2431 + rhs(0, 3) * _2133_2331),

					+invDet * (rhs(1, 0) * _3244_3442 - rhs(1, 1) * _3144_3441 + rhs(1, 3) * _3142_3241),
					-invDet * (rhs(0, 0) * _3244_3442 - rhs(0, 1) * _3144_3441 + rhs(0, 3) * _3142_3241),
					+invDet * (rhs(0, 0) * _2244_2442 - rhs(0, 1) * _2144_2441 + rhs(0, 3) * _2142_2241),
					-invDet * (rhs(0, 0) * _2234_2432 - rhs(0, 1) * _2134_2431 + rhs(0, 3) * _2132_2231),

					-invDet * (rhs(1, 0) * _3243_3342 - rhs(1, 1) * _3143_3341 + rhs(1, 2) * _3142_3241),
					+invDet * (rhs(0, 0) * _3243_3342 - rhs(0, 1) * _3143_3341 + rhs(0, 2) * _3142_3241),
					-invDet * (rhs(0, 0) * _2243_2342 - rhs(0, 1) * _2143_2341 + rhs(0, 2) * _2142_2241),
					+invDet * (rhs(0, 0) * _2233_2332 - rhs(0, 1) * _2133_2331 + rhs(0, 2) * _2132_2231));
			}
			else
			{
				return rhs;
			}
		}

		template <typename T>
		Matrix4_T<T> scaling(T const & sx, T const & sy, T const & sz)
		{
			return Matrix4_T<T>(
				sx,	0,	0,	0,
				0,	sy,	0,	0,
				0,	0,	sz,	0,
				0,	0,	0,	1);
		}

		template <typename T>
		Matrix4_T<T> to_matrix(Quaternion_T<T> const & quat)
		{
			// calculate coefficients
			T const x2(quat.x() + quat.x());
			T const y2(quat.y() + quat.y());
			T const z2(quat.z() + quat.z());

			T const xx2(quat.x() * x2), xy2(quat.x() * y2), xz2(quat.x() * z2);
			T const yy2(quat.y() * y2), yz2(quat.y() * z2), zz2(quat.z() * z2);
			T const wx2(quat.w() * x2), wy2(quat.w() * y2), wz2(quat.w() * z2);

			return Matrix4_T<T>(
				1 - yy2 - zz2,	xy2 + wz2,		xz2 - wy2,		0,
				xy2 - wz2,		1 - xx2 - zz2,	yz2 + wx2,		0,
				xz2 + wy2,		yz2 - wx2,		1 - xx2 - yy2,	0,
				0,				0,				0,				1);
		}

		template <typename T>
		Matrix4_T<T> translation(T const & x, T const & y, T const & z)
		{
			return Matrix4_T<T>(
				1,	0,	0,	0,
				0,	1,	0,	0,
				0,	0,	1,	0,
				x,	y,	z,	1);
		}

		template <typename T>
		Matrix4_T<T> translation(Vector_T<T, 3> const & pos)
		{
			return translation(pos.x(), pos.y(), pos.z());
		}

		template <typename T>
		Matrix4_T<T> transpose(Matrix4_T<T> const & rhs)
		{
			return Matrix4_T<T>(
				rhs(0, 0), rhs(1, 0), rhs(2, 0), rhs(3, 0),
				rhs(0, 1), rhs(1, 1), rhs(2, 1), rhs(3, 1),
				rhs(0, 2), rhs(1, 2), rhs(2, 2), rhs(3, 2),
				rhs(0, 3), rhs(1, 3), rhs(2, 3), rhs(3, 3));
		}

		template <typename T>
		Quaternion_T<T> conjugate(Quaternion_T<T> const & rhs)
		{
			return Quaternion_T<T>(-rhs.x(), -rhs.y(), -rhs.z(), rhs.w());
		}

		template <typename T>
		Quaternion_T<T> mul(Quaternion_T<T> const & lhs, Quaternion_T<T> const & rhs)
		{
			return Quaternion_T<T>(
				lhs.x() * rhs.w() - lhs.y() * rhs.z() + lhs.z() * rhs.y() + lhs.w() * rhs.x(),
				lhs.x() * rhs.z() + lhs.y() * rhs.w() - lhs.z() * rhs.x() + lhs.w() * rhs.y(),
				lhs.y() * rhs.x() - lhs.x() * rhs.y() + lhs.z() * rhs.w() + lhs.w() * rhs.z(),
				lhs.w() * rhs.w() - lhs.x() * rhs.x() - lhs.y() * rhs.y() - lhs.z() * rhs.z());
		}

		template <typename T>
		Quaternion_T<T> to_quaternion(Matrix4_T<T> const & mat)
		{
			Quaternion_T<T> quat;
			T s;
			T const tr = mat(0, 0) + mat(1, 1) + mat(2, 2) + 1;

			// check the diagonal
			if (tr > 1)
			{
				s = sqrt(tr);
				quat.w() = s * T(0.5);
				s = T(0.5) / s;
				quat.x() = (mat(1, 2) - mat(2, 1)) * s;
				quat.y() = (mat(2, 0) - mat(0, 2)) * s;
				quat.z() = (mat(0, 1) - mat(1, 0)) * s;
			}
			else
			{
				int maxi = 0;
				T maxdiag = mat(0, 0);
				for (int i = 1; i < 3; ++ i)
				{
					if (mat(i, i) > maxdiag)
					{
						maxi = i;
						maxdiag = mat(i, i);
					}
				}

				switch (maxi)
				{
				case 0:
					s = sqrt((mat(0, 0) - (mat(1, 1) + mat(2, 2))) + 1);

					quat.x() = s * T(0.5);

					if (!equal<T>(s, 0))
					{
						s = T(0.5) / s;
					}

					quat.w() = (mat(1, 2) - mat(2, 1)) * s;
					quat.y() = (mat(1, 0) + mat(0, 1)) * s;
					quat.z() = (mat(2, 0) + mat(0, 2)) * s;
					break;

				case 1:
					s = sqrt((mat(1, 1) - (mat(2, 2) + mat(0, 0))) + 1);
					quat.y() = s * T(0.5);

					if (!equal<T>(s, 0))
					{
						s = T(0.5) / s;
					}

					quat.w() = (mat(2, 0) - mat(0, 2)) * s;
					quat.z() = (mat(2, 1) + mat(1, 2)) * s;
					quat.x() = (mat(0, 1) + mat(1, 0)) * s;
					break;

				case 2:
				default:
					s = sqrt((mat(2, 2) - (mat(0, 0) + mat(1, 1))) + 1);

					quat.z() = s * T(0.5);

					if (!equal<T>(s, 0))
					{
						s = T(0.5) / s;
					}

					quat.w() = (mat(0, 1) - mat(1, 0)) * s;
					quat.x() = (mat(0, 2) + mat(2, 0)) * s;
					quat.y() = (mat(1, 2) + mat(2, 1)) * s;
					break;
				}
			}

			return normalize(quat);
		}

		template <typename T>
		Quaternion_T<T> to_quaternion(Vector_T<T, 3> const & tangent, Vector_T<T, 3> const & binormal, Vector_T<T, 3> const & normal, int bits)
		{
			T k = 1;
			if (dot(binormal, cross(normal, tangent)) < 0)
			{
				k = -1;
			}

			Matrix4_T<T> tangent_frame(tangent.x(), tangent.y(), tangent.z(), 0,
				k * binormal.x(), k * binormal.y(), k * binormal.z(), 0,
				normal.x(), normal.y(), normal.z(), 0,
				0, 0, 0, 1);
			Quaternion_T<T> tangent_quat = to_quaternion(tangent_frame);
			if (tangent_quat.w() < 0)
			{
				tangent_quat = -tangent_quat;
			}
			T const bias = T(1) / ((1UL << (bits - 1)) - 1);
			if (tangent_quat.w() < bias)
			{
				T const factor = sqrt(1 - bias * bias);
				tangent_quat.x() *= factor;
				tangent_quat.y() *= factor;
				tangent_quat.z() *= factor;
				tangent_quat.w() = bias;
			}
			if (k < 0)
			{
				tangent_quat = -tangent_quat;
			}

			return tangent_quat;
		}

		template <typename T>
		std::pair<Quaternion_T<T>, Quaternion_T<T> > conjugate(Quaternion_T<T> const & real, Quaternion_T<T> const & dual)
		{
			return std::make_pair(MathLib::conjugate(real), MathLib::conjugate(dual));
		}

		template <typename T>
		Quaternion_T<T> quat_trans_to_udq(Quaternion_T<T> const & q, Vector_T<T, 3> const & t)
		{
			return mul(q, Quaternion_T<T>(T(0.5) * t.x(), T(0.5) * t.y(), T(0.5) * t.z(), T(0.0)));
		}

		template <typename T>
		Vector_T<T, 3> udq_to_trans(Quaternion_T<T> const & real, Quaternion_T<T> const & dual)
		{
			Quaternion_T<T> qeq0 = mul(conjugate(real), dual);
			return T(2.0) * Vector_T<T, 3>(qeq0.x(), qeq0.y(), qeq0.z());
		}

		template <typename T>
		std::pair<Quaternion_T<T>, Quaternion_T<T> > inverse(Quaternion_T<T> const & real, Quaternion_T<T> const & dual)
		{
			float sqr_len_0 = MathLib::dot(real, real);
			float sqr_len_e = 2.0f * MathLib::dot(real, dual);
			float inv_sqr_len_0 = 1.0f / sqr_len_0;
			float inv_sqr_len_e = -sqr_len_e / (sqr_len_0 * sqr_len_0);
			std::pair<Quaternion_T<T>, Quaternion_T<T> > conj = conjugate(real, dual);
			return std::make_pair(inv_sqr_len_0 * conj.first, inv_sqr_len_0 * conj.second + inv_sqr_len_e * conj.first);
		}

		template <typename T>
		Quaternion_T<T> mul_real(Quaternion_T<T> const & lhs_real, Quaternion_T<T> const & rhs_real)
		{
			return lhs_real * rhs_real;
		}

		template <typename T>
		Quaternion_T<T> mul_dual(Quaternion_T<T> const & lhs_real, Quaternion_T<T> const & lhs_dual,
			Quaternion_T<T> const & rhs_real, Quaternion_T<T> const & rhs_dual)
		{
			return lhs_real * rhs_dual + lhs_dual * rhs_real;
		}

		template <typename T>
		void udq_to_screw(T& angle, T& pitch, Vector_T<T, 3>& dir, Vector_T<T, 3>& moment,
			Quaternion_T<T> const & real, Quaternion_T<T> const & dual)
		{
			if (abs(real.w()) >= 1)
			{
				// pure translation

				angle = 0;
				dir = dual.v();

				T dir_sq_len = length_sq(dir);

				if (dir_sq_len > T(1e-6))
				{
					T dir_len = sqrt(dir_sq_len);
					pitch = 2 * dir_len;
					dir /= dir_len;
				}
				else
				{
					pitch = 0;
				}

				moment = Vector_T<T, 3>::Zero();
			}
			else
			{ 
				angle = 2 * acos(real.w());

				float s = length_sq(real.v());
				if (s < T(1e-6))
				{
					dir = Vector_T<T, 3>::Zero();
					pitch = 0;
					moment = Vector_T<T, 3>::Zero();
				}
				else
				{
					float oos = recip_sqrt(s);
					dir = real.v() * oos;

					pitch = -2 * dual.w() * oos;

					moment = (dual.v() - dir * pitch * real.w() * T(0.5)) * oos;
				}
			}
		}

		template <typename T>
		std::pair<Quaternion_T<T>, Quaternion_T<T> > udq_from_screw(T const & angle, T const & pitch, Vector_T<T, 3> const & dir, Vector_T<T, 3> const & moment)
		{
			T sa, ca;
			sincos(angle * T(0.5), sa, ca);
			return std::make_pair(Quaternion_T<T>(dir * sa, ca),
				Quaternion_T<T>(sa * moment + T(0.5) * pitch * ca * dir, -pitch * sa * T(0.5)));
		}

		template <typename T>
		std::pair<Quaternion_T<T>, Quaternion_T<T> > sclerp(Quaternion_T<T> const & lhs_real, Quaternion_T<T> const & lhs_dual,
			Quaternion_T<T> const & rhs_real, Quaternion_T<T> const & rhs_dual, float const & slerp)
		{
			// Make sure dot product is >= 0
			float quat_dot = dot(lhs_real, rhs_real);
			Quaternion to_sign_corrected_real = rhs_real;
			Quaternion to_sign_corrected_dual = rhs_dual;
			if (quat_dot < 0)
			{
				to_sign_corrected_real = -to_sign_corrected_real;
				to_sign_corrected_dual = -to_sign_corrected_dual;
			}

			std::pair<Quaternion_T<T>, Quaternion_T<T> > dif_dq = inverse(lhs_real, lhs_dual);
			dif_dq.second = mul_dual(dif_dq.first, dif_dq.second, to_sign_corrected_real, to_sign_corrected_dual);
			dif_dq.first = mul_real(dif_dq.first, to_sign_corrected_real);
	
			float angle, pitch;
			float3 direction, moment;
			udq_to_screw(angle, pitch, direction, moment, dif_dq.first, dif_dq.second);

			angle *= slerp; 
			pitch *= slerp;
			dif_dq = udq_from_screw(angle, pitch, direction, moment);

			dif_dq.second = mul_dual(lhs_real, lhs_dual, dif_dq.first, dif_dq.second);
			dif_dq.first = mul_real(lhs_real, dif_dq.first);

			return dif_dq;
		}
	}

	bool MeshMLObj::Material::operator==(MeshMLObj::Material const & rhs) const
	{
		bool same = (ambient == rhs.ambient) && (diffuse == rhs.diffuse)
			&& (specular_level == rhs.specular_level) && (emit == rhs.emit)
			&& (opacity == rhs.opacity) && (specular_level == rhs.specular_level)
			&& (shininess == rhs.shininess);
		if (same)
		{
			for (size_t i = 0; i < texture_slots.size(); ++ i)
			{
				bool found = false;
				for (size_t j = 0; j < rhs.texture_slots.size(); ++ j)
				{
					if (texture_slots[i] == rhs.texture_slots[j])
					{
						found = true;
						break;
					}
				}

				if (!found)
				{
					return false;
				}
			}

			return true;
		}
		else
		{
			return false;
		}
	}

	std::pair<std::pair<Quaternion, Quaternion>, float> MeshMLObj::Keyframes::Frame(float frame) const
	{
		frame = std::fmod(frame, static_cast<float>(frame_ids.back() + 1));

		BOOST_AUTO(iter, std::upper_bound(frame_ids.begin(), frame_ids.end(), frame));
		int index = static_cast<int>(iter - frame_ids.begin());

		int index0 = index - 1;
		int index1 = index % frame_ids.size();
		int frame0 = frame_ids[index0];
		int frame1 = frame_ids[index1];
		float factor = (frame - frame0) / (frame1 - frame0);
		std::pair<std::pair<Quaternion, Quaternion>, float> ret;
		ret.first = MathLib::sclerp(bind_reals[index0], bind_duals[index0], bind_reals[index1], bind_duals[index1], factor);
		ret.second = MathLib::lerp(bind_scales[index0], bind_scales[index1], factor);
		return ret;
	}


	MeshMLObj::MeshMLObj(float unit_scale)
		: unit_scale_(unit_scale), num_frames_(0), frame_rate_(25)
	{
	}

	int MeshMLObj::AllocJoint()
	{
		int id = static_cast<int>(joints_.size());
		joints_.insert(std::make_pair(id, Joint()));
		return id;
	}

	void MeshMLObj::SetJoint(int joint_id, std::string const & joint_name, int parent_id,
		float4x4 const & bind_mat)
	{
		Quaternion real, dual;
		this->MatrixToDQ(bind_mat, real, dual);

		this->SetJoint(joint_id, joint_name, parent_id, real, dual);
	}

	void MeshMLObj::SetJoint(int joint_id, std::string const & joint_name, int parent_id,
		Quaternion const & bind_quat, float3 const & bind_pos)
	{
		this->SetJoint(joint_id, joint_name, parent_id, bind_quat, MathLib::quat_trans_to_udq(bind_quat, bind_pos));
	}

	void MeshMLObj::SetJoint(int joint_id, std::string const & joint_name, int parent_id,
		Quaternion const & bind_real, Quaternion const & bind_dual)
	{
		float scale = MathLib::length(bind_real);
		if (bind_real.w() < 0)
		{
			scale = -scale;
		}

		Joint& joint = joints_[joint_id];
		joint.name = joint_name;
		joint.parent_id = parent_id;
		joint.bind_real = bind_real / scale;
		joint.bind_dual = bind_dual;
		joint.bind_scale = scale;
	}

	int MeshMLObj::AllocMaterial()
	{
		int id = static_cast<int>(materials_.size());
		materials_.push_back(Material());
		return id;
	}

	void MeshMLObj::SetMaterial(int mtl_id, float3 const & ambient, float3 const & diffuse,
			float3 const & specular, float3 const & emit, float opacity, float specular_level, float shininess)
	{
		BOOST_ASSERT(static_cast<int>(materials_.size()) > mtl_id);

		Material& mtl = materials_[mtl_id];
		mtl.ambient = ambient;
		mtl.diffuse = diffuse;
		mtl.specular = specular;
		mtl.emit = emit;
		mtl.opacity = opacity;
		mtl.specular_level = specular_level;
		mtl.shininess = shininess;
	}

	int MeshMLObj::AllocTextureSlot(int mtl_id)
	{
		BOOST_ASSERT(static_cast<int>(materials_.size()) > mtl_id);

		Material& mtl = materials_[mtl_id];
		int id = static_cast<int>(mtl.texture_slots.size());
		mtl.texture_slots.push_back(TextureSlot());
		return id;
	}

	void MeshMLObj::SetTextureSlot(int mtl_id, int slot_id, std::string const & type, std::string const & name)
	{
		BOOST_ASSERT(static_cast<int>(materials_.size()) > mtl_id);
		BOOST_ASSERT(static_cast<int>(materials_[mtl_id].texture_slots.size()) > slot_id);

		TextureSlot& ts = materials_[mtl_id].texture_slots[slot_id];
		ts.first = type;
		ts.second = name;
	}

	int MeshMLObj::AllocMesh()
	{
		int id = static_cast<int>(meshes_.size());
		meshes_.push_back(Mesh());
		return id;
	}

	void MeshMLObj::SetMesh(int mesh_id, int material_id, std::string const & name)
	{
		BOOST_ASSERT(static_cast<int>(meshes_.size()) > mesh_id);

		Mesh& mesh = meshes_[mesh_id];
		mesh.material_id = material_id;
		mesh.name = name;
	}

	int MeshMLObj::AllocVertex(int mesh_id)
	{
		BOOST_ASSERT(static_cast<int>(meshes_.size()) > mesh_id);

		Mesh& mesh = meshes_[mesh_id];
		int id = static_cast<int>(mesh.vertices.size());
		mesh.vertices.push_back(Vertex());
		return id;
	}

	void MeshMLObj::SetVertex(int mesh_id, int vertex_id, float3 const & pos, float3 const & normal,
		int texcoord_components, std::vector<float3> const & texcoords)
	{
		BOOST_ASSERT(static_cast<int>(meshes_.size()) > mesh_id);
		BOOST_ASSERT(static_cast<int>(meshes_[mesh_id].vertices.size()) > vertex_id);

		Vertex& vertex = meshes_[mesh_id].vertices[vertex_id];
		vertex.position = pos * unit_scale_;
		vertex.normal = normal;
		vertex.texcoord_components = texcoord_components;
		vertex.texcoords = texcoords;
	}

	void MeshMLObj::SetVertex(int mesh_id, int vertex_id, float3 const & pos,
		float3 const & tangent, float3 const & binormal, float3 const & normal,
		int texcoord_components, std::vector<float3> const & texcoords)
	{
		this->SetVertex(mesh_id, vertex_id, pos, MathLib::to_quaternion(tangent, binormal, normal, 8),
			texcoord_components, texcoords);
	}

	void MeshMLObj::SetVertex(int mesh_id, int vertex_id, float3 const & pos, Quaternion const & tangent_quat,
		int texcoord_components, std::vector<float3> const & texcoords)
	{
		BOOST_ASSERT(static_cast<int>(meshes_.size()) > mesh_id);
		BOOST_ASSERT(static_cast<int>(meshes_[mesh_id].vertices.size()) > vertex_id);

		Vertex& vertex = meshes_[mesh_id].vertices[vertex_id];
		vertex.position = pos * unit_scale_;
		vertex.tangent_quat = tangent_quat;
		vertex.texcoord_components = texcoord_components;
		vertex.texcoords = texcoords;
	}

	int MeshMLObj::AllocJointBinding(int mesh_id, int vertex_id)
	{
		BOOST_ASSERT(static_cast<int>(meshes_.size()) > mesh_id);
		BOOST_ASSERT(static_cast<int>(meshes_[mesh_id].vertices.size()) > vertex_id);

		Vertex& vertex = meshes_[mesh_id].vertices[vertex_id];
		int id = static_cast<int>(vertex.binds.size());
		vertex.binds.push_back(JointBinding());
		return id;
	}

	void MeshMLObj::SetJointBinding(int mesh_id, int vertex_id, int binding_id,
			int joint_id, float weight)
	{
		BOOST_ASSERT(static_cast<int>(meshes_.size()) > mesh_id);
		BOOST_ASSERT(static_cast<int>(meshes_[mesh_id].vertices.size()) > vertex_id);
		BOOST_ASSERT(static_cast<int>(meshes_[mesh_id].vertices[vertex_id].binds.size()) > binding_id);

		JointBinding& binding = meshes_[mesh_id].vertices[vertex_id].binds[binding_id];
		binding.first = joint_id;
		binding.second = weight;
	}

	int MeshMLObj::AllocTriangle(int mesh_id)
	{
		BOOST_ASSERT(static_cast<int>(meshes_.size()) > mesh_id);

		Mesh& mesh = meshes_[mesh_id];
		int id = static_cast<int>(mesh.triangles.size());
		mesh.triangles.push_back(Triangle());
		return id;
	}

	void MeshMLObj::SetTriangle(int mesh_id, int triangle_id, int index0, int index1, int index2)
	{
		BOOST_ASSERT(static_cast<int>(meshes_.size()) > mesh_id);
		BOOST_ASSERT(static_cast<int>(meshes_[mesh_id].triangles.size()) > triangle_id);

		Triangle& triangle = meshes_[mesh_id].triangles[triangle_id];
		triangle.vertex_index[0] = index0;
		triangle.vertex_index[1] = index1;
		triangle.vertex_index[2] = index2;
	}

	int MeshMLObj::AllocKeyframes()
	{
		int id = static_cast<int>(keyframes_.size());
		keyframes_.push_back(Keyframes());
		return id;
	}

	void MeshMLObj::SetKeyframes(int kfs_id, int joint_id)
	{
		BOOST_ASSERT(static_cast<int>(keyframes_.size()) > kfs_id);

		Keyframes& kfs = keyframes_[kfs_id];
		kfs.joint_id = joint_id;
	}

	int MeshMLObj::AllocKeyframe(int kfs_id)
	{
		BOOST_ASSERT(static_cast<int>(keyframes_.size()) > kfs_id);

		Keyframes& kfs = keyframes_[kfs_id];
		int id = static_cast<int>(kfs.frame_ids.size());
		kfs.frame_ids.push_back(id);
		kfs.bind_reals.push_back(Quaternion());
		kfs.bind_duals.push_back(Quaternion());
		kfs.bind_scales.push_back(1);
		return id;
	}

	void MeshMLObj::SetKeyframe(int kfs_id, int kf_id, float4x4 const & bind_mat)
	{
		Quaternion real, dual;
		this->MatrixToDQ(bind_mat, real, dual);

		this->SetKeyframe(kfs_id, kf_id, real, dual);
	}

	void MeshMLObj::SetKeyframe(int kfs_id, int kf_id, Quaternion const & bind_quat, float3 const & bind_pos)
	{
		this->SetKeyframe(kfs_id, kf_id, bind_quat, MathLib::quat_trans_to_udq(bind_quat, bind_pos));
	}

	void MeshMLObj::SetKeyframe(int kfs_id, int kf_id, Quaternion const & bind_real, Quaternion const & bind_dual)
	{
		BOOST_ASSERT(static_cast<int>(keyframes_.size()) > kfs_id);
		BOOST_ASSERT(static_cast<int>(keyframes_[kfs_id].bind_reals.size()) > kf_id);

		float scale = MathLib::length(bind_real);
		if (bind_real.w() < 0)
		{
			scale = -scale;
		}
		
		Keyframes& kfs = keyframes_[kfs_id];
		kfs.bind_reals[kf_id] = bind_real / scale;
		kfs.bind_duals[kf_id] = bind_dual;
		kfs.bind_scales[kf_id] = scale;
	}

	void MeshMLObj::WriteMeshML(std::ostream& os, int vertex_export_settings, int user_export_settings, std::string const & encoding)
	{
		this->OptimizeJoints();
		this->OptimizeMaterials();
		this->OptimizeMeshes(user_export_settings);

		std::map<int, int> joint_id_to_index;
		std::vector<int> joint_index_to_id;
		if (!joints_.empty())
		{
			typedef BOOST_TYPEOF(joints_) JointsType;
			BOOST_FOREACH(JointsType::const_reference joint, joints_)
			{
				joint_index_to_id.push_back(joint.first);
			}

			for (int i = 0; i < static_cast<int>(joint_index_to_id.size()); ++ i)
			{
				joint_id_to_index.insert(std::make_pair(joint_index_to_id[i], i));
			}

			// Replace parent_id
			typedef BOOST_TYPEOF(joints_) JointsType;
			BOOST_FOREACH(JointsType::reference joint, joints_)
			{
				if (joint.second.parent_id != -1)
				{
					BOOST_AUTO(fiter, joint_id_to_index.find(joint.second.parent_id));
					BOOST_ASSERT(fiter != joint_id_to_index.end());

					joint.second.parent_id = fiter->second;
				}
			}

			// Replace joint_id in weight
			typedef BOOST_TYPEOF(meshes_) MeshesType;
			BOOST_FOREACH(MeshesType::reference mesh, meshes_)
			{
				typedef BOOST_TYPEOF(mesh.vertices) VerticesType;
				BOOST_FOREACH(VerticesType::reference vertex, mesh.vertices)
				{
					typedef BOOST_TYPEOF(vertex.binds) BindsType;
					BOOST_FOREACH(BindsType::reference bind, vertex.binds)
					{
						BOOST_AUTO(fiter, joint_id_to_index.find(bind.first));
						BOOST_ASSERT(fiter != joint_id_to_index.end());

						bind.first = fiter->second;
					}
				}
			}

			// Replace joint_id in keyframes and remove unused keyframes
			for (BOOST_AUTO(iter, keyframes_.begin()); iter != keyframes_.end();)
			{
				BOOST_AUTO(fiter, joint_id_to_index.find(iter->joint_id));
				if (fiter != joint_id_to_index.end())
				{
					iter->joint_id = fiter->second;
					++ iter;
				}
				else
				{
					iter = keyframes_.erase(iter);
				}
			}
		}
		else
		{
			keyframes_.clear();
		}

		int model_ver = 6;

		// Initialize the xml document
		os << "<?xml version=\"1.0\"";
		if (!encoding.empty())
		{
			os << " encoding=\"" << encoding << "\"";
		}
		os << "?>" << std::endl << std::endl;
		os << "<model version=\"" << model_ver << "\">" << std::endl;

		if (!joints_.empty())
		{
			this->WriteJointChunk(os);
		}
		if (!materials_.empty())
		{
			this->WriteMaterialChunk(os);
		}
		if (!meshes_.empty())
		{
			this->WriteMeshChunk(os, vertex_export_settings);
		}
		if (!keyframes_.empty())
		{
			this->WriteKeyframeChunk(os);
			this->WriteAABBKeyframeChunk(os);
		}

		// Finish the writing process
		os << "</model>" << std::endl;
	}

	void MeshMLObj::WriteJointChunk(std::ostream& os)
	{
		os << "\t<bones_chunk>" << std::endl;
		typedef BOOST_TYPEOF(joints_) JointsType;
		BOOST_FOREACH(JointsType::const_reference joint, joints_)
		{
			os << "\t\t<bone name=\"" << RemoveQuote(joint.second.name);

			Quaternion const bind_real = joint.second.bind_real * joint.second.bind_scale;
			Quaternion const & bind_dual = joint.second.bind_dual;

			os << "\" parent=\"" << joint.second.parent_id << "\">" << std::endl;
			os << "\t\t\t<real v=\"" << bind_real[0]
				<< " " << bind_real[1] << " " << bind_real[2]
				<< " " << bind_real[3] << "\"/>" << std::endl;
			os << "\t\t\t<dual v=\"" << bind_dual[0]
				<< " " << bind_dual[1] << " " << bind_dual[2]
				<< " " << bind_dual[3] << "\"/>" << std::endl;
			os << "\t\t</bone>" << std::endl;
		}
		os << "\t</bones_chunk>" << std::endl;
	}

	void MeshMLObj::WriteMaterialChunk(std::ostream& os)
	{
		os << "\t<materials_chunk>" << std::endl;
		typedef BOOST_TYPEOF(materials_) MaterialsType;
		BOOST_FOREACH(MaterialsType::const_reference mtl, materials_)
		{
			os << "\t\t<material ambient=\"" << mtl.ambient[0]
				<< " " << mtl.ambient[1]
				<< " " << mtl.ambient[2]
				<< "\" diffuse=\"" << mtl.diffuse[0]
				<< " " << mtl.diffuse[1]
				<< " " << mtl.diffuse[2]
				<< "\" specular=\"" << mtl.specular[0]
				<< " " << mtl.specular[1]
				<< " " << mtl.specular[2]
				<< "\" emit=\"" << mtl.emit[0]
				<< " " << mtl.emit[1]
				<< " " << mtl.emit[2]
				<< "\" opacity=\"" << mtl.opacity
				<< "\" specular_level=\"" << mtl.specular_level
				<< "\" shininess=\"" << mtl.shininess << "\"";

			if (!mtl.texture_slots.empty())
			{
				os << ">" << std::endl;

				typedef BOOST_TYPEOF(mtl.texture_slots) TextureSlotsType;
				BOOST_FOREACH(TextureSlotsType::const_reference tl, mtl.texture_slots)
				{
					os << "\t\t\t<texture type=\"" << RemoveQuote(tl.first)
						<< "\" name=\"" << RemoveQuote(tl.second) << "\"/>" << std::endl;
				}

				os << "\t\t</material>" << std::endl;
			}
			else
			{
				os << "/>" << std::endl;
			}
		}
		os << "\t</materials_chunk>" << std::endl;
	}

	void MeshMLObj::WriteMeshChunk(std::ostream& os, int vertex_export_settings)
	{
		os << "\t<meshes_chunk>" << std::endl;
		typedef BOOST_TYPEOF(meshes_) MeshesType;
		BOOST_FOREACH(MeshesType::const_reference mesh, meshes_)
		{
			os << "\t\t<mesh name=\"" << RemoveQuote(mesh.name)
				<< "\" mtl_id=\"" << mesh.material_id << "\">" << std::endl;

			os << "\t\t\t<vertices_chunk>" << std::endl;

			typedef BOOST_TYPEOF(mesh.vertices) VerticesType;
			float3 pos_min_bb = mesh.vertices[0].position;
			float3 pos_max_bb = pos_min_bb;
			float2 tc_min_bb(-1, -1);
			float2 tc_max_bb(+1, +1);
			if (vertex_export_settings & VES_Texcoord)
			{
				tc_min_bb = tc_max_bb = mesh.vertices[0].texcoords[0];
			}
			BOOST_FOREACH(VerticesType::const_reference vertex, mesh.vertices)
			{
				pos_min_bb.x() = std::min(pos_min_bb.x(), vertex.position.x());
				pos_min_bb.y() = std::min(pos_min_bb.y(), vertex.position.y());
				pos_min_bb.z() = std::min(pos_min_bb.z(), vertex.position.z());

				pos_max_bb.x() = std::max(pos_max_bb.x(), vertex.position.x());
				pos_max_bb.y() = std::max(pos_max_bb.y(), vertex.position.y());
				pos_max_bb.z() = std::max(pos_max_bb.z(), vertex.position.z());

				if (vertex_export_settings & VES_Texcoord)
				{
					tc_min_bb.x() = std::min(tc_min_bb.x(), vertex.texcoords[0].x());
					tc_min_bb.y() = std::min(tc_min_bb.y(), vertex.texcoords[0].y());

					tc_max_bb.x() = std::max(tc_max_bb.x(), vertex.texcoords[0].x());
					tc_max_bb.y() = std::max(tc_max_bb.y(), vertex.texcoords[0].y());
				}
			}

			os << "\t\t\t\t<pos_bb min=\"" << pos_min_bb.x() << " " << pos_min_bb.y()
				<< " " << pos_min_bb.z() << "\" max=\"" << pos_max_bb.x() << " " << pos_max_bb.y()
				<< " " << pos_max_bb.z() << "\"/>" << std::endl;
			if (vertex_export_settings & VES_Texcoord)
			{
				os << "\t\t\t\t<tc_bb min=\"" << tc_min_bb.x() << " " << tc_min_bb.y()
					<< "\" max=\"" << tc_max_bb.x() << " " << tc_max_bb.y() << "\"/>" << std::endl;
			}
			os << std::endl;

			BOOST_FOREACH(VerticesType::const_reference vertex, mesh.vertices)
			{
				os << "\t\t\t\t<vertex v=\"" << vertex.position.x()
					<< " " << vertex.position.y()
					<< " " << vertex.position.z() << "\"";
				if (vertex_export_settings != VES_None)
				{
					os << ">" << std::endl;

					if (vertex_export_settings & VES_Normal)
					{
						os << "\t\t\t\t\t<normal v=\"" << vertex.normal.x()
							<< " " << vertex.normal.y()
							<< " " << vertex.normal.z() << "\"/>" << std::endl;
					}

					if (vertex_export_settings & VES_TangentQuat)
					{
						os << "\t\t\t\t\t<tangent_quat v=\"" << vertex.tangent_quat.x()
							<< " " << vertex.tangent_quat.y()
							<< " " << vertex.tangent_quat.z()
							<< " " << vertex.tangent_quat.w() << "\"/>" << std::endl;
					}

					if (vertex_export_settings & VES_Texcoord)
					{
						typedef BOOST_TYPEOF(vertex.texcoords) TexcoordsType;
						switch (vertex.texcoord_components)
						{
						case 1:
							BOOST_FOREACH(TexcoordsType::const_reference tc, vertex.texcoords)
							{
								os << "\t\t\t\t\t<tex_coord v=\"" << tc.x() << "\"/>" << std::endl;
							}
							break;

						case 2:
							BOOST_FOREACH(TexcoordsType::const_reference tc, vertex.texcoords)
							{
								os << "\t\t\t\t\t<tex_coord v=\"" << tc.x()
									<< " " << tc.y() << "\"/>" << std::endl;
							}
							break;

						case 3:
							BOOST_FOREACH(TexcoordsType::const_reference tc, vertex.texcoords)
							{
								os << "\t\t\t\t\t<tex_coord v=\"" << tc.x()
									<< " " << tc.y() << " " << tc.z() << "\"/>" << std::endl;
							}
							break;

						default:
							break;
						}
					}

					if (!vertex.binds.empty())
					{
						os << "\t\t\t\t\t<weight joint=\"";
						for (size_t i = 0; i < vertex.binds.size(); ++ i)
						{
							os << vertex.binds[i].first;
							if (i != vertex.binds.size() - 1)
							{
								os << ' ';
							}
						}
						os << "\" weight=\"";
						for (size_t i = 0; i < vertex.binds.size(); ++ i)
						{
							os << vertex.binds[i].second;
							if (i != vertex.binds.size() - 1)
							{
								os << ' ';
							}
						}
						os << "\"/>" << std::endl;
					}

					os << "\t\t\t\t</vertex>" << std::endl;
				}
				else
				{
					os << "/>" << std::endl;
				}
			}
			os << "\t\t\t</vertices_chunk>" << std::endl;

			os << "\t\t\t<triangles_chunk>" << std::endl;
			typedef BOOST_TYPEOF(mesh.triangles) TrianglesType;
			BOOST_FOREACH(TrianglesType::const_reference tri, mesh.triangles)
			{
				os << "\t\t\t\t<triangle index=\"" << tri.vertex_index[0]
					<< " " << tri.vertex_index[1]
					<< " " << tri.vertex_index[2] << "\"/>" << std::endl;
			}
			os << "\t\t\t</triangles_chunk>" << std::endl;

			os << "\t\t</mesh>" << std::endl;
		}
		os << "\t</meshes_chunk>" << std::endl;
	}

	void MeshMLObj::WriteKeyframeChunk(std::ostream& os)
	{
		float const THRESHOLD = 1e-3f;

		std::map<int, int> joint_id_to_kf;
		for (size_t i = 0; i < keyframes_.size(); ++ i)
		{
			joint_id_to_kf.insert(std::make_pair(keyframes_[i].joint_id, static_cast<int>(i)));
		}

		os << "\t<key_frames_chunk num_frames=\"" << num_frames_
			<< "\" frame_rate=\"" << frame_rate_ << "\">" << std::endl;
		typedef BOOST_TYPEOF(joints_) JointsType;
		BOOST_FOREACH(JointsType::const_reference joint, joints_)
		{
			BOOST_AUTO(iter, joint_id_to_kf.find(joint.first));
			BOOST_ASSERT(iter != joint_id_to_kf.end());

			Keyframes kf = keyframes_[iter->second];

			BOOST_ASSERT(kf.bind_reals.size() == kf.bind_duals.size());

			int base = 0;
			while (base < kf.frame_ids.size() - 2)
			{
				int const frame0 = kf.frame_ids[base + 0];
				int const frame1 = kf.frame_ids[base + 1];
				int const frame2 = kf.frame_ids[base + 2];
				float const factor = static_cast<float>(frame1 - frame0) / (frame2 - frame0);
				std::pair<Quaternion, Quaternion> interpolate = MathLib::sclerp(kf.bind_reals[base + 0], kf.bind_duals[base + 0],
					kf.bind_reals[base + 2], kf.bind_duals[base + 2], factor);
				float const scale = MathLib::lerp(kf.bind_scales[base + 0], kf.bind_scales[base + 2], factor);

				if (MathLib::dot(kf.bind_reals[base + 1], interpolate.first) < 0)
				{
					interpolate.first = -interpolate.first;
					interpolate.second = -interpolate.second;
				}

				std::pair<Quaternion, Quaternion> diff_dq = MathLib::inverse(kf.bind_reals[base + 1], kf.bind_duals[base + 1]);
				diff_dq.second = MathLib::mul_dual(diff_dq.first, diff_dq.second * scale, interpolate.first, interpolate.second);
				diff_dq.first = MathLib::mul_real(diff_dq.first, interpolate.first);
				float diff_scale = scale * kf.bind_scales[base + 1];

				if ((MathLib::abs(diff_dq.first.x()) < THRESHOLD) && (MathLib::abs(diff_dq.first.y()) < THRESHOLD)
					&& (MathLib::abs(diff_dq.first.z()) < THRESHOLD) && (MathLib::abs(diff_dq.first.w() - 1) < THRESHOLD)
					&& (MathLib::abs(diff_dq.second.x()) < THRESHOLD) && (MathLib::abs(diff_dq.second.y()) < THRESHOLD)
					&& (MathLib::abs(diff_dq.second.z()) < THRESHOLD) && (MathLib::abs(diff_dq.second.w()) < THRESHOLD)
					&& (MathLib::abs(diff_scale - 1) < THRESHOLD))
				{
					kf.frame_ids.erase(kf.frame_ids.begin() + base + 1);
					kf.bind_reals.erase(kf.bind_reals.begin() + base + 1);
					kf.bind_duals.erase(kf.bind_duals.begin() + base + 1);
					kf.bind_scales.erase(kf.bind_scales.begin() + base + 1);
				}
				else
				{
					++ base;
				}
			}

			os << "\t\t<key_frame joint=\"" << kf.joint_id << "\">" << std::endl;
			for (size_t j = 0; j < kf.frame_ids.size(); ++ j)
			{
				Quaternion const bind_real = kf.bind_reals[j] * kf.bind_scales[j];
				Quaternion const & bind_dual = kf.bind_duals[j];

				os << "\t\t\t<key id=\"" << kf.frame_ids[j] << "\">" << std::endl;
				os << "\t\t\t\t<real v=\"" << bind_real.x()
					<< " " << bind_real.y()
					<< " " << bind_real.z()
					<< " " << bind_real.w() << "\"/>" << std::endl;
				os << "\t\t\t\t<dual v=\"" << bind_dual.x()
					<< " " << bind_dual.y()
					<< " " << bind_dual.z()
					<< " " << bind_dual.w() << "\"/>" << std::endl;
				os << "\t\t\t</key>" << std::endl;
			}
			os << "\t\t</key_frame>" << std::endl;
		}
		os << "\t</key_frames_chunk>" << std::endl;
	}

	void MeshMLObj::WriteAABBKeyframeChunk(std::ostream& os)
	{
		float const THRESHOLD = 1e-3f;

		std::vector<Quaternion> bind_reals;
		std::vector<Quaternion> bind_duals;
		std::vector<std::vector<int> > frame_ids(meshes_.size());
		std::vector<std::vector<float3> > bb_min_key_frames(meshes_.size());
		std::vector<std::vector<float3> > bb_max_key_frames(meshes_.size());
		for (size_t m = 0; m < meshes_.size(); ++ m)
		{
			frame_ids[m].resize(num_frames_);
			bb_min_key_frames[m].resize(num_frames_);
			bb_max_key_frames[m].resize(num_frames_);
		}

		for (int f = 0; f < num_frames_; ++ f)
		{
			this->UpdateJoints(f, bind_reals, bind_duals);
			for (size_t m = 0; m < meshes_.size(); ++ m)
			{
				float3 bb_min(+1e10f, +1e10f, +1e10f);
				float3 bb_max(-1e10f, -1e10f, -1e10f);
				for (size_t v = 0; v < meshes_[m].vertices.size(); ++ v)
				{
					Vertex const & vertex = meshes_[m].vertices[v];

					Quaternion const & dp0 = bind_reals[vertex.binds[0].first];
	
					float3 pos_s(0, 0, 0);
					Quaternion blend_real(0, 0, 0, 0);
					Quaternion blend_dual(0, 0, 0, 0);
					for (size_t bi = 0; bi < vertex.binds.size(); ++ bi)
					{
						Quaternion joint_real = bind_reals[vertex.binds[bi].first];
						Quaternion joint_dual = bind_duals[vertex.binds[bi].first];
		
						float scale = MathLib::length(joint_real);
						joint_real /= scale;

						float weight = vertex.binds[bi].second;
		
						if (MathLib::dot(dp0, joint_real) < 0)
						{
							joint_real = -joint_real;
							joint_dual = -joint_dual;
						}

						pos_s += vertex.position * scale * weight;
						blend_real += joint_real * weight;
						blend_dual += joint_dual * weight;
					}
	
					float len = MathLib::length(blend_real);
					blend_real /= len;
					blend_dual /= len;

					Quaternion trans = MathLib::mul(Quaternion(blend_dual.x(), blend_dual.y(), blend_dual.z(), -blend_dual.w()), blend_real);
					float3 result_pos = MathLib::transform_quat(pos_s, blend_real) + 2 * float3(trans.x(), trans.y(), trans.z());

					if ((result_pos.x() == result_pos.x()) && (result_pos.y() == result_pos.y())
						&& (result_pos.z() == result_pos.z()))
					{
						bb_min.x() = std::min(bb_min.x(), result_pos.x());
						bb_min.y() = std::min(bb_min.y(), result_pos.y());
						bb_min.z() = std::min(bb_min.z(), result_pos.z());

						bb_max.x() = std::max(bb_max.x(), result_pos.x());
						bb_max.y() = std::max(bb_max.y(), result_pos.y());
						bb_max.z() = std::max(bb_max.z(), result_pos.z());
					}
				}

				frame_ids[m][f] = f;
				bb_min_key_frames[m][f] = bb_min;
				bb_max_key_frames[m][f] = bb_max;
			}
		}

		os << "\t<bb_key_frames_chunk>" << std::endl;
		for (size_t m = 0; m < meshes_.size(); ++ m)
		{
			std::vector<int>& fid = frame_ids[m];
			std::vector<float3>& min_kf = bb_min_key_frames[m];
			std::vector<float3>& max_kf = bb_max_key_frames[m];

			int base = 0;
			while (base < fid.size() - 2)
			{
				int const frame0 = fid[base + 0];
				int const frame1 = fid[base + 1];
				int const frame2 = fid[base + 2];
				float const factor = static_cast<float>(frame1 - frame0) / (frame2 - frame0);
				float3 const interpolate_min = MathLib::lerp(min_kf[base + 0], min_kf[base + 2], factor);
				float3 const interpolate_max = MathLib::lerp(max_kf[base + 0], max_kf[base + 2], factor);

				float3 const diff_min = min_kf[base + 1] - interpolate_min;
				float3 const diff_max = max_kf[base + 1] - interpolate_max;

				if ((MathLib::abs(diff_min.x()) < THRESHOLD) && (MathLib::abs(diff_min.y()) < THRESHOLD)
					&& (MathLib::abs(diff_min.z()) < THRESHOLD)
					&& (MathLib::abs(diff_max.x()) < THRESHOLD) && (MathLib::abs(diff_max.y()) < THRESHOLD)
					&& (MathLib::abs(diff_max.z()) < THRESHOLD))
				{
					fid.erase(fid.begin() + base + 1);
					min_kf.erase(min_kf.begin() + base + 1);
					max_kf.erase(max_kf.begin() + base + 1);
				}
				else
				{
					++ base;
				}
			}

			os << "\t\t<bb_key_frame mesh=\"" << m << "\">" << std::endl;
			for (size_t f = 0; f < fid.size(); ++ f)
			{
				float3 const & bb_min = min_kf[f];
				float3 const & bb_max = max_kf[f];

				os << "\t\t\t<key id=\"" << fid[f]
					<< "\" min=\"" << bb_min.x()
					<< " " << bb_min.y()
					<< " " << bb_min.z()
					<< "\" max=\"" << bb_max.x()
					<< " " << bb_max.y()
					<< " " << bb_max.z() << "\"/>" << std::endl;
			}
			os << "\t\t</bb_key_frame>" << std::endl;
		}
		os << "\t</bb_key_frames_chunk>" << std::endl;
	}

	void MeshMLObj::OptimizeJoints()
	{
		std::set<int> joints_used;

		// Find all joints used in the mesh list
		typedef BOOST_TYPEOF(meshes_) MeshesType;
		BOOST_FOREACH(MeshesType::const_reference mesh, meshes_)
		{
			typedef BOOST_TYPEOF(mesh.vertices) VerticesType;
			BOOST_FOREACH(VerticesType::const_reference vertex, mesh.vertices)
			{
				typedef BOOST_TYPEOF(vertex.binds) BindsType;
				BOOST_FOREACH(BindsType::const_reference bind, vertex.binds)
				{
					joints_used.insert(bind.first);
				}
			}
		}

		// Traverse the joint list and see if used joints' parents can be added
		std::set<int> parent_joints_used;
		typedef BOOST_TYPEOF(joints_) JointsType;
		BOOST_FOREACH(JointsType::const_reference joint, joints_)
		{
			if (joints_used.find(joint.first) != joints_used.end())
			{
				Joint const * j = &joint.second;
				while (j->parent_id != -1)
				{
					parent_joints_used.insert(j->parent_id);
					j = &joints_[j->parent_id];
				}
			}
		}

		joints_used.insert(parent_joints_used.begin(), parent_joints_used.end());

		// Traverse the joint list and erase those never recorded by joints_used
		for (BOOST_AUTO(iter, joints_.begin()); iter != joints_.end();)
		{
			if (joints_used.find(iter->first) == joints_used.end())
			{
				iter = joints_.erase(iter);
			}
			else
			{
				++ iter;
			}
		}
	}

	void MeshMLObj::OptimizeMaterials()
	{
		std::vector<int> mtl_mapping(materials_.size());
		std::vector<Material> mtls_used;

		// Traverse materials and setup IDs
		for (size_t i = 0; i < materials_.size(); ++ i)
		{
			bool found = false;
			for (size_t j = 0; j < mtls_used.size(); ++ j)
			{
				if (mtls_used[j] == materials_[i])
				{
					mtl_mapping[i] = static_cast<int>(j);
					found = true;
					break;
				}
			}

			if (!found)
			{
				mtl_mapping[i] = static_cast<int>(mtls_used.size());
				mtls_used.push_back(materials_[i]);
			}
		}

		materials_ = mtls_used;

		typedef BOOST_TYPEOF(meshes_) MeshesType;
		BOOST_FOREACH(MeshesType::reference mesh, meshes_)
		{
			mesh.material_id = mtl_mapping[mesh.material_id];
		}
	}

	void MeshMLObj::OptimizeMeshes(int user_export_settings)
	{
		if (user_export_settings & UES_CombineMeshes)
		{
			std::set<size_t> meshid_to_remove;
			std::vector<Mesh> meshes_finished;
			for (size_t i = 0; i < materials_.size(); ++ i)
			{
				// Find all meshes sharing one material
				std::vector<Mesh> meshes_to_combine;
				for (size_t j = 0; j < meshes_.size(); ++ j)
				{
					if (meshes_[j].material_id == static_cast<int>(i))
					{
						meshes_to_combine.push_back(meshes_[j]);
						meshid_to_remove.insert(j);
					}
				}

				// Combine these meshes
				if (!meshes_to_combine.empty())
				{
					std::stringstream ss;
					ss << "combined_for_mtl_" << i;

					Mesh opt_mesh;
					opt_mesh.material_id = static_cast<int>(i);
					opt_mesh.name = ss.str();

					typedef BOOST_TYPEOF(meshes_to_combine) MeshesType;
					BOOST_FOREACH(MeshesType::const_reference mesh, meshes_to_combine)
					{
						int base = static_cast<int>(opt_mesh.vertices.size());
						opt_mesh.vertices.insert(opt_mesh.vertices.end(),
							mesh.vertices.begin(), mesh.vertices.end());

						typedef BOOST_TYPEOF(mesh.triangles) TrianglesType;
						BOOST_FOREACH(TrianglesType::const_reference tri, mesh.triangles)
						{
							Triangle opt_tri;
							opt_tri.vertex_index[0] = tri.vertex_index[0] + base;
							opt_tri.vertex_index[1] = tri.vertex_index[1] + base;
							opt_tri.vertex_index[2] = tri.vertex_index[2] + base;
							opt_mesh.triangles.push_back(opt_tri);
						}
					}
					meshes_finished.push_back(opt_mesh);
				}
			}

			// Rebuild the mesh list
			for (size_t i = 0; i < meshes_.size(); ++ i)
			{
				if (meshid_to_remove.find(i) == meshid_to_remove.end())
				{
					meshes_finished.push_back(meshes_[i]);
				}
			}
			meshes_ = meshes_finished;
		}

		if (user_export_settings & UES_SortMeshes)
		{
			std::sort(meshes_.begin(), meshes_.end(), MaterialIDSortOp());
		}
	}

	void MeshMLObj::MatrixToDQ(float4x4 const & mat, Quaternion& real, Quaternion& dual) const
	{
		float4x4 tmp_mat = mat;
		float flip = 1;
		if (MathLib::dot(MathLib::cross(float3(tmp_mat(0, 0), tmp_mat(0, 1), tmp_mat(0, 2)),
			float3(tmp_mat(1, 0), tmp_mat(1, 1), tmp_mat(1, 2))),
			float3(tmp_mat(2, 0), tmp_mat(2, 1), tmp_mat(2, 2))) < 0)
		{
			tmp_mat(2, 0) = -tmp_mat(2, 0);
			tmp_mat(2, 1) = -tmp_mat(2, 1);
			tmp_mat(2, 2) = -tmp_mat(2, 2);

			flip = -1;
		}

		float3 scale;
		float3 trans;
		MathLib::decompose(scale, real, trans, tmp_mat);

		dual = MathLib::quat_trans_to_udq(real, trans * unit_scale_);

		if (flip * real.w() < 0)
		{
			real = -real;
			dual = -dual;
		}

		real *= scale.x();
	}

	void MeshMLObj::UpdateJoints(int frame, std::vector<Quaternion>& bind_reals, std::vector<Quaternion>& bind_duals) const
	{
		std::vector<Joint> bind_joints;
		typedef BOOST_TYPEOF(joints_) JointsType;
		BOOST_FOREACH(JointsType::const_reference joint, joints_)
		{
			bind_joints.push_back(joint.second);
		}

		std::vector<Joint> bind_inverse_joints(bind_joints.size());
		for (size_t i = 0; i < bind_joints.size(); ++ i)
		{
			Joint inverse_joint;

			float flip = MathLib::sgn(bind_joints[i].bind_scale);

			inverse_joint.bind_scale = 1 / bind_joints[i].bind_scale;

			if (flip > 0)
			{
				std::pair<Quaternion, Quaternion> inv = MathLib::inverse(bind_joints[i].bind_real, bind_joints[i].bind_dual);
				inverse_joint.bind_real = inv.first;
				inverse_joint.bind_dual = inv.second;
			}
			else
			{
				float4x4 tmp_mat = MathLib::scaling(abs(bind_joints[i].bind_scale), abs(bind_joints[i].bind_scale), bind_joints[i].bind_scale)
					* MathLib::to_matrix(bind_joints[i].bind_real)
					* MathLib::translation(MathLib::udq_to_trans(bind_joints[i].bind_real, bind_joints[i].bind_dual));
				tmp_mat = MathLib::inverse(tmp_mat);
				tmp_mat(2, 0) = -tmp_mat(2, 0);
				tmp_mat(2, 1) = -tmp_mat(2, 1);
				tmp_mat(2, 2) = -tmp_mat(2, 2);

				float3 scale;
				Quaternion rot;
				float3 trans;
				MathLib::decompose(scale, rot, trans, tmp_mat);

				inverse_joint.bind_real = rot;
				inverse_joint.bind_dual = MathLib::quat_trans_to_udq(rot, trans);
				inverse_joint.bind_scale = -scale.x();
			}

			bind_inverse_joints[i] = inverse_joint;
		}

		for (size_t i = 0; i < bind_joints.size(); ++ i)
		{
			size_t kf_id = 0;
			Joint& joint = bind_joints[i];
			for (size_t j = 0; j < keyframes_.size(); ++ j)
			{
				Keyframes const & kf = keyframes_[j];
				if (kf.joint_id == static_cast<int>(i))
				{
					kf_id = j;
					break;
				}
			}

			Keyframes const & kf = keyframes_[kf_id];

			std::pair<std::pair<Quaternion, Quaternion>, float> key_dq = kf.Frame(static_cast<float>(frame));

			if (joint.parent_id != -1)
			{
				Joint const & parent(bind_joints[joint.parent_id]);

				if (MathLib::dot(key_dq.first.first, parent.bind_real) < 0)
				{
					key_dq.first.first = -key_dq.first.first;
					key_dq.first.second = -key_dq.first.second;
				}

				if ((key_dq.second > 0) && (parent.bind_scale > 0))
				{
					joint.bind_real = MathLib::mul_real(key_dq.first.first, parent.bind_real);
					joint.bind_dual = MathLib::mul_dual(key_dq.first.first, key_dq.first.second * parent.bind_scale, parent.bind_real, parent.bind_dual);
					joint.bind_scale = key_dq.second * parent.bind_scale;
				}
				else
				{
					float4x4 tmp_mat = MathLib::scaling(MathLib::abs(key_dq.second), MathLib::abs(key_dq.second), key_dq.second)
						* MathLib::to_matrix(key_dq.first.first)
						* MathLib::translation(MathLib::udq_to_trans(key_dq.first.first, key_dq.first.second))
						* MathLib::scaling(MathLib::abs(parent.bind_scale), MathLib::abs(parent.bind_scale), parent.bind_scale)
						* MathLib::to_matrix(parent.bind_real)
						* MathLib::translation(MathLib::udq_to_trans(parent.bind_real, parent.bind_dual));

					float flip = 1;
					if (MathLib::dot(MathLib::cross(float3(tmp_mat(0, 0), tmp_mat(0, 1), tmp_mat(0, 2)),
						float3(tmp_mat(1, 0), tmp_mat(1, 1), tmp_mat(1, 2))),
						float3(tmp_mat(2, 0), tmp_mat(2, 1), tmp_mat(2, 2))) < 0)
					{
						tmp_mat(2, 0) = -tmp_mat(2, 0);
						tmp_mat(2, 1) = -tmp_mat(2, 1);
						tmp_mat(2, 2) = -tmp_mat(2, 2);

						flip = -1;
					}

					float3 scale;
					Quaternion rot;
					float3 trans;
					MathLib::decompose(scale, rot, trans, tmp_mat);

					joint.bind_real = rot;
					joint.bind_dual = MathLib::quat_trans_to_udq(rot, trans);
					joint.bind_scale = flip * scale.x();
				}
			}
			else
			{
				joint.bind_real = key_dq.first.first;
				joint.bind_dual = key_dq.first.second;
				joint.bind_scale = key_dq.second;
			}
		}

		bind_reals.resize(bind_joints.size());
		bind_duals.resize(bind_joints.size());
		for (size_t i = 0; i < bind_joints.size(); ++ i)
		{
			Joint const & joint = bind_joints[i];
			Joint const & inverse_joint = bind_inverse_joints[i];

			Quaternion bind_real, bind_dual;
			float bind_scale;
			if ((inverse_joint.bind_scale > 0) && (joint.bind_scale > 0))
			{
				bind_real = MathLib::mul_real(inverse_joint.bind_real, joint.bind_real);
				bind_dual = MathLib::mul_dual(inverse_joint.bind_real, inverse_joint.bind_dual,
					joint.bind_real, joint.bind_dual);
				bind_scale = inverse_joint.bind_scale * joint.bind_scale;

				if (bind_real.w() < 0)
				{
					bind_real = -bind_real;
					bind_dual = -bind_dual;
				}
			}
			else
			{
				float4x4 tmp_mat = MathLib::scaling(MathLib::abs(inverse_joint.bind_scale), MathLib::abs(inverse_joint.bind_scale), inverse_joint.bind_scale)
					* MathLib::to_matrix(inverse_joint.bind_real)
					* MathLib::translation(MathLib::udq_to_trans(inverse_joint.bind_real, inverse_joint.bind_dual))
					* MathLib::scaling(MathLib::abs(joint.bind_scale), MathLib::abs(joint.bind_scale), joint.bind_scale)
					* MathLib::to_matrix(joint.bind_real)
					* MathLib::translation(MathLib::udq_to_trans(joint.bind_real, joint.bind_dual));

				float flip = 1;
				if (MathLib::dot(MathLib::cross(float3(tmp_mat(0, 0), tmp_mat(0, 1), tmp_mat(0, 2)),
					float3(tmp_mat(1, 0), tmp_mat(1, 1), tmp_mat(1, 2))),
					float3(tmp_mat(2, 0), tmp_mat(2, 1), tmp_mat(2, 2))) < 0)
				{
					tmp_mat(2, 0) = -tmp_mat(2, 0);
					tmp_mat(2, 1) = -tmp_mat(2, 1);
					tmp_mat(2, 2) = -tmp_mat(2, 2);

					flip = -1;
				}

				float3 scale;
				Quaternion rot;
				float3 trans;
				MathLib::decompose(scale, rot, trans, tmp_mat);

				bind_real = rot;
				bind_dual = MathLib::quat_trans_to_udq(rot, trans);
				bind_scale = scale.x();

				if (flip * bind_real.w() < 0)
				{
					bind_real = -bind_real;
					bind_dual = -bind_dual;
				}
			}

			bind_reals[i] = bind_real * bind_scale;
			bind_duals[i] = bind_dual;
		}
	}
}  // namespace KlayGE
