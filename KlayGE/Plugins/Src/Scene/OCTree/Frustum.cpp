#include <KlayGE/KlayGE.hpp>
#include <KlayGE/Math.hpp>

#include <KlayGE/OCTree/Frustum.hpp>

namespace KlayGE
{
	void Frustum::CalculateFrustum(const Matrix4& view, const Matrix4& proj)
	{
		// Create the clip.
		Matrix4 clip(view * proj);

		// Calculate the right side of the frustum.
		frustum_[0] = Plane(clip[3]  - clip[0], clip[7]  - clip[4], clip[11] - clip[8], clip[15] - clip[12]);

		// Calculate the left side of the frustum.
		frustum_[1] = Plane(clip[3]  + clip[0], clip[7]  + clip[4], clip[11] + clip[8], clip[15] + clip[12]);

		// Calculate the bottom side of the frustum.
		frustum_[2] = Plane(clip[3]  + clip[1], clip[7]  + clip[5], clip[11] + clip[9], clip[15] + clip[13]);

		// Calculate the top side of the frustum.
		frustum_[3] = Plane(clip[3]  - clip[1], clip[7]  - clip[5], clip[11] - clip[9], clip[15] - clip[13]);

		// Calculate the far side of the frustum.
		frustum_[4] = Plane(clip[3]  - clip[2], clip[7]  - clip[6], clip[11] - clip[10], clip[15] - clip[14]);

		// Calculate the near side of the frustum.
		frustum_[5] = Plane(clip[3]  + clip[2], clip[7]  + clip[6], clip[11] + clip[10], clip[15] + clip[14]);

		// Loop through each side of the frustum and normalize it.
		for (int i = 0; i < 6; ++ i)
		{
			MathLib::Normalize(frustum_[i], frustum_[i]);
		}
	}

	bool Frustum::Visiable(const Vector3& v) const
	{
		for (int i = 0; i < 6; ++ i)
		{
			if (MathLib::Dot(frustum_[i], Vector4(v.x(), v.y(), v.z(), 1)) < 0)
			{
				return false;
			}
		}

		return true;
	}
}
