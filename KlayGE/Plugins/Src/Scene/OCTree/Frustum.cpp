// Frustum.cpp
// KlayGE 视锥类 实现文件
// Ver 2.1.2
// 版权所有(C) 龚敏敏, 2004
// Homepage: http://klayge.sourceforge.net
//
// 2.1.2
// 初次建立 (2004.6.15)
//
// 修改记录
//////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KlayGE/Math.hpp>

#include <KlayGE/OCTree/Frustum.hpp>

namespace KlayGE
{
	void Frustum::CalculateFrustum(const Matrix4& view, const Matrix4& proj)
	{
		// Create the clip.
		Matrix4 clip(view * proj);

		// Calculate the left side of the frustum.
		planes_[0] = Plane(clip(0, 3) + clip(0, 0), clip(1, 3) + clip(1, 0), clip(2, 3) + clip(2, 0),
			clip(3, 3) + clip(3, 0));

		// Calculate the right side of the frustum.
		planes_[1] = Plane(clip(0, 3) - clip(0, 0), clip(1, 3) - clip(1, 0), clip(2, 3) - clip(2, 0),
			clip(3, 3) - clip(3, 0));

		// Calculate the bottom side of the frustum.
		planes_[2] = Plane(clip(0, 3) + clip(0, 1), clip(1, 3) + clip(1, 1), clip(2, 3) + clip(2, 1),
			clip(3, 3) + clip(3, 1));

		// Calculate the top side of the frustum.
		planes_[3] = Plane(clip(0, 3) - clip(0, 1), clip(1, 3) - clip(1, 1), clip(2, 3) - clip(2, 1),
			clip(3, 3) - clip(3, 1));

		// Calculate the far side of the frustum.
		planes_[4] = Plane(clip(0, 3) - clip(0, 2), clip(1, 3) - clip(1, 2), clip(2, 3) - clip(2, 2),
			clip(3, 3) - clip(3, 2));

		// Calculate the near side of the frustum.
		planes_[5] = Plane(clip(0, 2), clip(1, 2), clip(2, 2), clip(3, 2));

		// Loop through each side of the frustum and normalize it.
		for (PlanesType::iterator iter = planes_.begin(); iter != planes_.end(); ++ iter)
		{
			MathLib::Normalize(*iter, *iter);
		}
	}

	bool Frustum::Visiable(const Vector3& v) const
	{
		for (PlanesType::const_iterator iter = planes_.begin(); iter != planes_.end(); ++ iter)
		{
			if (MathLib::Dot(*iter, Vector4(v.x(), v.y(), v.z(), 1)) < 0)
			{
				return false;
			}
		}

		return true;
	}
}
