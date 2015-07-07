/**
 * @file PreDeclare.hpp
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

#ifndef _KFL_PREDECLARE_HPP
#define _KFL_PREDECLARE_HPP

#pragma once

#include <memory>

namespace KlayGE
{
	class ResIdentifier;
	typedef std::shared_ptr<ResIdentifier> ResIdentifierPtr;
	class DllLoader;

	class XMLDocument;
	typedef std::shared_ptr<XMLDocument> XMLDocumentPtr;
	class XMLNode;
	typedef std::shared_ptr<XMLNode> XMLNodePtr;
	class XMLAttribute;
	typedef std::shared_ptr<XMLAttribute> XMLAttributePtr;

	class bad_join;
	template <typename ResultType>
	class joiner;
	class threader;
	class thread_pool;

	class half;
	template <typename T, int N>
	class Vector_T;
	typedef Vector_T<int32_t, 1> int1;
	typedef Vector_T<int32_t, 2> int2;
	typedef Vector_T<int32_t, 3> int3;
	typedef Vector_T<int32_t, 4> int4;
	typedef Vector_T<uint32_t, 1> uint1;
	typedef Vector_T<uint32_t, 2> uint2;
	typedef Vector_T<uint32_t, 3> uint3;
	typedef Vector_T<uint32_t, 4> uint4;
	typedef Vector_T<float, 1> float1;
	typedef Vector_T<float, 2> float2;
	typedef Vector_T<float, 3> float3;
	typedef Vector_T<float, 4> float4;
	template <typename T>
	class Matrix4_T;
	typedef Matrix4_T<float> float4x4;
	template <typename T>
	class Quaternion_T;
	typedef Quaternion_T<float> Quaternion;
	template <typename T>
	class Plane_T;
	typedef Plane_T<float> Plane;
	template <typename T>
	class Color_T;
	typedef Color_T<float> Color;
	template <typename T>
	class Size_T;
	typedef Size_T<float> Size;
	typedef Size_T<int32_t> ISize;
	typedef Size_T<uint32_t> UISize;
	typedef std::shared_ptr<Size> SizePtr;
	typedef std::shared_ptr<ISize> ISizePtr;
	typedef std::shared_ptr<UISize> UISizePtr;
	template <typename T>
	class Rect_T;
	typedef Rect_T<float> Rect;
	typedef Rect_T<int32_t> IRect;
	typedef Rect_T<uint32_t> UIRect;
	typedef std::shared_ptr<Rect> RectPtr;
	typedef std::shared_ptr<IRect> IRectPtr;
	typedef std::shared_ptr<UIRect> UIRectPtr;
	template <typename T>
	class Bound_T;
	typedef Bound_T<float> Bound;
	typedef std::shared_ptr<Bound> BoundPtr;
	template <typename T>
	class Sphere_T;
	typedef Sphere_T<float> Sphere;
	typedef std::shared_ptr<Sphere> SpherePtr;
	template <typename T>
	class AABBox_T;
	typedef AABBox_T<float> AABBox;
	typedef std::shared_ptr<AABBox> AABBoxPtr;
	template <typename T>
	class Frustum_T;
	typedef Frustum_T<float> Frustum;
	typedef std::shared_ptr<Frustum> FrustumPtr;
	template <typename T>
	class OBBox_T;
	typedef OBBox_T<float> OBBox;
	typedef std::shared_ptr<OBBox> OBBoxPtr;
}

#endif			// _KFL_PREDECLARE_HPP
