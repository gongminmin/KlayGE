#include <KlayGE/KlayGE.hpp>
#include <KlayGE/Math.hpp>
#include <KlayGE/Engine.hpp>

#include <algorithm>
#include <functional>

#include <KlayGE/MathTypes.hpp>

namespace KlayGE
{
	const Matrix4& Matrix4::Identity()
	{
		static Matrix4 out(
			1, 0, 0, 0,
			0, 1, 0, 0,
			0, 0, 1, 0,
			0, 0, 0, 1);
		return out;
	}

	Matrix4& Matrix4::operator*=(const Matrix4& rhs)
	{
		return Engine::MathInstance().Multiply(*this, *this, rhs);
	}

	bool operator==(const Matrix4& lhs, const Matrix4& rhs)
	{
		return 0 == memcmp(rhs.Begin(), lhs.Begin(), Matrix4::elem_num * sizeof(Matrix4::value_type));
	}
}