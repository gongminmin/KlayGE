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

#include <KFL/Operators.hpp>
#include <KFL/Bound.hpp>

namespace KlayGE
{
	template <typename T>
	class Sphere_T final : public Bound_T<T>
	{
	public:
		constexpr Sphere_T() noexcept : radius_()
		{
		}
		constexpr Sphere_T(Vector_T<T, 3> center, T radius) noexcept : center_(std::move(center)), radius_(std::move(radius))
		{
		}
		constexpr Sphere_T(Sphere_T const& rhs) noexcept : center_(rhs.center_), radius_(rhs.radius_)
		{
		}
		constexpr Sphere_T(Sphere_T&& rhs) noexcept : center_(std::move(rhs.center_)), radius_(std::move(rhs.radius_))
		{
		}

		// ��ֵ������
		Sphere_T& operator+=(Vector_T<T, 3> const & rhs) noexcept;
		Sphere_T& operator-=(Vector_T<T, 3> const & rhs) noexcept;
		Sphere_T& operator*=(T rhs) noexcept;
		Sphere_T& operator/=(T rhs) noexcept;

		Sphere_T& operator=(Sphere_T const & rhs) noexcept;
		Sphere_T& operator=(Sphere_T&& rhs) noexcept;

		// ����
		constexpr Vector_T<T, 3>& Center() noexcept
		{
			return center_;
		}
		constexpr Vector_T<T, 3> const & Center() const noexcept
		{
			return center_;
		}
		constexpr T& Radius() noexcept
		{
			return radius_;
		}
		constexpr T Radius() const noexcept
		{
			return radius_;
		}

		virtual bool IsEmpty() const noexcept override;
		virtual bool VecInBound(Vector_T<T, 3> const & v) const noexcept override;
		virtual T MaxRadiusSq() const noexcept override;

		bool Intersect(AABBox_T<T> const & aabb) const noexcept;
		bool Intersect(OBBox_T<T> const & obb) const noexcept;
		bool Intersect(Sphere_T<T> const & sphere) const noexcept;
		bool Intersect(Frustum_T<T> const & frustum) const noexcept;

		bool operator==(Sphere_T<T> const & rhs) const noexcept;

		KLAYGE_DEFAULT_ADD_OPERATOR2(Sphere_T<T>, KLAYGE_ESC(Vector_T<T, 3>));
		KLAYGE_DEFAULT_SUB_OPERATOR2(Sphere_T<T>, KLAYGE_ESC(Vector_T<T, 3>));
		KLAYGE_DEFAULT_MUL_OPERATOR2(Sphere_T<T>, T);
		KLAYGE_DEFAULT_MUL_OPERATOR3(T, Sphere_T<T>);
		KLAYGE_DEFAULT_DIV_OPERATOR2(Sphere_T<T>, T);

		KLAYGE_DEFAULT_EQUALITY_COMPARE_OPERATOR(Sphere_T<T>);

	private:
		Vector_T<T, 3> center_;
		T radius_;
	};

	using Sphere = Sphere_T<float>;
}

#endif			// _KFL_SPHERE_HPP
