// Matrix4.hpp
// KlayGE 4D矩阵 实现文件
// Ver 2.0.3
// 版权所有(C) 龚敏敏, 2003--2004
// Homepage: http://klayge.sourceforge.net
//
//
// 修改记录
///////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KlayGE/Math.hpp>
#include <KlayGE/Memory.hpp>

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
		return MathLib::Multiply(*this, *this, rhs);
	}

	bool operator==(const Matrix4& lhs, const Matrix4& rhs)
	{
		return 0 == MemoryLib::Compare(rhs.begin(), lhs.begin(), Matrix4::elem_num * sizeof(Matrix4::value_type));
	}
}
