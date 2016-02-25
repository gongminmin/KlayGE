/**
 * @file Sphere.hpp
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

#ifndef _KFL_SPHERE_HPP
#define _KFL_SPHERE_HPP

#pragma once

#include <KFL/PreDeclare.hpp>

#include <boost/operators.hpp>

#include <KFL/Bound.hpp>

namespace KlayGE
{
	template <typename T>
	class Sphere_T : boost::addable2<Sphere_T<T>, Vector_T<T, 3>,
						boost::subtractable2<Sphere_T<T>, Vector_T<T, 3>,
						boost::multipliable2<Sphere_T<T>, T,
						boost::dividable2<Sphere_T<T>, T,
						boost::equality_comparable<Sphere_T<T>>>>>>,
				public Bound_T<T>
	{
	public:
		Sphere_T() KLAYGE_NOEXCEPT
		{
		}
		Sphere_T(Vector_T<T, 3> const & center, T radius) KLAYGE_NOEXCEPT;
		Sphere_T(Sphere_T const & rhs) KLAYGE_NOEXCEPT;
		Sphere_T(Sphere_T&& rhs) KLAYGE_NOEXCEPT;

		// ��ֵ������
		Sphere_T& operator+=(Vector_T<T, 3> const & rhs) KLAYGE_NOEXCEPT;
		Sphere_T& operator-=(Vector_T<T, 3> const & rhs) KLAYGE_NOEXCEPT;
		Sphere_T& operator*=(T rhs) KLAYGE_NOEXCEPT;
		Sphere_T& operator/=(T rhs) KLAYGE_NOEXCEPT;

		Sphere_T& operator=(Sphere_T const & rhs) KLAYGE_NOEXCEPT;
		Sphere_T& operator=(Sphere_T&& rhs) KLAYGE_NOEXCEPT;

		// һԪ������
		Sphere_T const & operator+() const KLAYGE_NOEXCEPT;
		Sphere_T const & operator-() const KLAYGE_NOEXCEPT;

		// ����
		Vector_T<T, 3>& Center() KLAYGE_NOEXCEPT
		{
			return center_;
		}
		Vector_T<T, 3> const & Center() const KLAYGE_NOEXCEPT
		{
			return center_;
		}
		T& Radius() KLAYGE_NOEXCEPT
		{
			return radius_;
		}
		T Radius() const KLAYGE_NOEXCEPT
		{
			return radius_;
		}

		virtual bool IsEmpty() const KLAYGE_NOEXCEPT override;
		virtual bool VecInBound(Vector_T<T, 3> const & v) const KLAYGE_NOEXCEPT override;
		virtual T MaxRadiusSq() const KLAYGE_NOEXCEPT override;

		bool Intersect(AABBox_T<T> const & aabb) const KLAYGE_NOEXCEPT;
		bool Intersect(OBBox_T<T> const & obb) const KLAYGE_NOEXCEPT;
		bool Intersect(Sphere_T<T> const & sphere) const KLAYGE_NOEXCEPT;
		bool Intersect(Frustum_T<T> const & frustum) const KLAYGE_NOEXCEPT;

		bool operator==(Sphere_T<T> const & rhs) const KLAYGE_NOEXCEPT;

	private:
		Vector_T<T, 3> center_;
		T radius_;
	};
}

#endif			// _KFL_SPHERE_HPP
