#include <KlayGE/KlayGE.hpp>
#include <KlayGE/Math.hpp>

#include <algorithm>
#include <functional>

#include <KlayGE/MathTypes.hpp>

namespace KlayGE
{
	Box::Box(const Vector3& vMin, const Vector3& vMax)
	{
		MathLib::Minimize(min_, vMin, vMax);
		MathLib::Maximize(max_, vMin, vMax);
	}

	// ¸³Öµ²Ù×÷·û
	Box& Box::operator&=(const Box& rhs)
	{
		MathLib::Maximize(this->Min(), this->Min(), rhs.Min());
		MathLib::Minimize(this->Max(), this->Max(), rhs.Max());
		return *this;
	}

	Box& Box::operator|=(const Box& rhs)
	{
		MathLib::Minimize(this->Min(), this->Min(), rhs.Min());
		MathLib::Maximize(this->Max(), this->Max(), rhs.Max());
		return *this;
	}

	bool Box::VecInBound(const Vector3& v) const
	{
		return MathLib::VecInBox(*this, v);
	}

	bool Box::IsHit(const Box& box) const
	{
		return (MathLib::VecInBox(box, this->Min())) || (MathLib::VecInBox(box, this->Max()));
	}

	float Box::MaxRadiusSq() const
	{
		return std::max(MathLib::LengthSq(this->Max()), MathLib::LengthSq(this->Min()));
	}
}