#include <KlayGE/KlayGE.hpp>
#include <KlayGE/SharePtr.hpp>
#include <KlayGE/Math.hpp>
#include <KlayGE/Engine.hpp>

#include <cassert>

#include <KlayGE/Light.hpp>

namespace KlayGE
{
	// ¹¹Ôìº¯Êý
	/////////////////////////////////////////////////////////////////////////////////
	Light::Light()
		: lightType(LT_Point),
			position(MakeVector(0.0f, 0.0f, 0.0f)), direction(MakeVector(0.0f, 0.0f, 1.0f)),
			diffuse(1, 1, 1, 1), specular(0, 0, 0, 0), ambient(0, 0, 0, 0),
			range(1000),
			attenuationConst(1), attenuationLinear(0), attenuationQuad(0),
			spotOuter(0), spotInner(0), spotFalloff(0)
	{
	}

	Light::Light(LightTypes type, const Vector3& pos)
		: lightType(type),
			position(pos),
			diffuse(1, 1, 1, 1), specular(0, 0, 0, 0), ambient(0, 0, 0, 0),
			range(1000),
			attenuationConst(1), attenuationLinear(0), attenuationQuad(0),
			spotOuter(0), spotInner(0), spotFalloff(0)
	{
		Engine::MathInstance().Normalize(direction, pos);
	}
}
