#include <KlayGE/KlayGE.hpp>
#include <KlayGE/Math.hpp>
#include <KlayGE/Engine.hpp>

#include <KlayGE/MathTypes.hpp>

namespace KlayGE
{
	const Quaternion& Quaternion::Identity()
	{
		static Quaternion out(0, 0, 0, 1);
		return out;
	}
	
	Quaternion& Quaternion::operator*=(const Quaternion& rhs)
	{
		return Engine::MathInstance().Multiply(*this, *this, rhs);
	}
}