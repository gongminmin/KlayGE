// Math.hpp
// KlayGE 数学函数库 实现文件
// Ver 2.1.0
// 版权所有(C) 龚敏敏, 2003-2004
// Homepage: http://klayge.sourceforge.net
//
// 2.1.0
// 去掉了汇编代码 (2004.4.18)
//
// 修改记录
///////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>

#include <cstdlib>

#include <KlayGE/Math.hpp>

namespace KlayGE
{
	namespace MathLib
	{
		// 颜色
		///////////////////////////////////////////////////////////////////////////////
		Color& Negative(Color& out, const Color& rhs)
		{
			out = Color(1 - rhs.r(), 1 - rhs.g(), 1 - rhs.b(), rhs.a());
			return out;
		}

		Color& Modulate(Color& out, const Color& lhs, const Color& rhs)
		{
			out = Color(lhs.r() * rhs.r(), lhs.g() * rhs.g(), lhs.b() * rhs.b(), lhs.a() * rhs.a());
			return out;
		}


		// 范围
		///////////////////////////////////////////////////////////////////////////////
		bool VecInSphere(const Sphere& sphere, const Vector3& v)
		{
			if (Length(v - sphere.Center()) < sphere.Radius())
			{
				return true;
			}
			return false;
		}

		bool BoundProbe(const Sphere& sphere, const Vector3& orig, const Vector3& dir)
		{
			const float a = LengthSq(dir);
			const float b = 2 * Dot(dir, orig - sphere.Center());
			const float c = LengthSq(orig - sphere.Center()) - sphere.Radius() * sphere.Radius();

			if (b * b - 4 * a * c < 0)
			{
				return false;
			}
			return true;
		}

		bool VecInBox(const Box& box, const Vector3& v)
		{
			return (InBound(v.x(), box.Min().x(), box.Max().x()))
				&& (InBound(v.y(), box.Min().y(), box.Max().y()))
				&& (InBound(v.z(), box.Min().z(), box.Max().z()));
		}

		bool BoundProbe(const Box& box, const Vector3& orig, const Vector3& dir)
		{
			const Vector3 leftBottomNear(box.LeftBottomNear());
			const Vector3 leftTopNear(box.LeftTopNear());
			const Vector3 rightTopNear(box.RightTopNear());
			const Vector3 leftTopFar(box.LeftTopFar());

			Plane pNear;
			FromPoints(pNear, leftBottomNear, leftTopNear, rightTopNear);
			Plane pTop;
			FromPoints(pTop, leftTopNear, leftTopFar, rightTopNear);
			Plane pLeft;
			FromPoints(pLeft, leftTopFar, leftTopNear, leftBottomNear);

			Vector3 vec;
			if (IntersectLine(vec, pNear, orig, dir))
			{
				if ((!InBound(vec.x(), leftBottomNear.x(), rightTopNear.x()))
					|| (!InBound(vec.y(), leftBottomNear.y(), leftTopNear.y())))
				{
					return false;
				}
			}

			if (IntersectLine(vec, pTop, orig, dir))
			{
				if ((!InBound(vec.x(), leftTopNear.x(), rightTopNear.x()))
					|| (!InBound(vec.z(), leftTopNear.z(), leftTopFar.z())))
				{
					return false;
				}
			}

			if (IntersectLine(vec, pLeft, orig, dir))
			{
				if ((!InBound(vec.y(), leftBottomNear.y(), leftTopNear.y()))
					|| (!InBound(vec.z(), leftBottomNear.z(), leftTopFar.z())))
				{
					return false;
				}
			}

			return true;
		}
	}
}
