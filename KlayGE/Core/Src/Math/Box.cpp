#include <KlayGE/KlayGE.hpp>
#include <KlayGE/Math.hpp>
#include <KlayGE/Engine.hpp>

#include <algorithm>
#include <functional>

#include <KlayGE/MathTypes.hpp>

namespace KlayGE
{
	Box::Box(const Vector3& vMin, const Vector3& vMax)
	{
		Engine::MathInstance().Minimize(min_, vMin, vMax);
		Engine::MathInstance().Maximize(max_, vMin, vMax);
	}

	// ¸³Öµ²Ù×÷·û
	Box& Box::operator&=(const Box& rhs)
	{
		Engine::MathInstance().Maximize(this->Min(), this->Min(), rhs.Min());
		Engine::MathInstance().Minimize(this->Max(), this->Max(), rhs.Max());
		return *this;
	}

	Box& Box::operator|=(const Box& rhs)
	{
		Engine::MathInstance().Minimize(this->Min(), this->Min(), rhs.Min());
		Engine::MathInstance().Maximize(this->Max(), this->Max(), rhs.Max());
		return *this;
	}

	bool Box::VecInBound(const Vector3& v) const
	{
		return Engine::MathInstance().VecInBox(*this, v);
	}

	bool Box::IsHit(const Box& box) const
	{
		return (Engine::MathInstance().VecInBox(box, this->Min())) || (Engine::MathInstance().VecInBox(box, this->Max()));
	}

	float Box::MaxRadiusSq() const
	{
		return std::max(Engine::MathInstance().LengthSq(this->Max()), Engine::MathInstance().LengthSq(this->Min()));
	}
}