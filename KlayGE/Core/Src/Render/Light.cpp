#include <KlayGE/KlayGE.hpp>
#include <KlayGE/Math.hpp>

#include <cassert>

#include <KlayGE/Light.hpp>

namespace KlayGE
{
	// ¹¹Ôìº¯Êý
	/////////////////////////////////////////////////////////////////////////////////
	Light::Light()
		: lightType(LT_Point),
			position(0, 0, 0), direction(0, 0, 1),
			diffuse(1, 1, 1, 1), specular(0, 0, 0, 0), ambient(0, 0, 0, 0),
			range(1000),
			attenuationConst(1), attenuationLinear(0), attenuationQuad(0),
			spotOuter(0), spotInner(0), spotFalloff(0)
	{
	}

	Light::Light(LightTypes type, Vector3 const & pos)
		: lightType(type),
			position(pos),
			diffuse(1, 1, 1, 1), specular(0, 0, 0, 0), ambient(0, 0, 0, 0),
			range(1000),
			attenuationConst(1), attenuationLinear(0), attenuationQuad(0),
			spotOuter(0), spotInner(0), spotFalloff(0)
	{
		MathLib::Normalize(direction, pos);
	}
}
