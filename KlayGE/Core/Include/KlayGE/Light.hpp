#ifndef _LIGHT_HPP
#define _LIGHT_HPP

#include <KlayGE/Vector.hpp>

#pragma comment(lib, "KlayGE_Core.lib")

namespace KlayGE
{
	struct Light
	{
		// Defines the type of light
		enum LightTypes
		{
			LT_Point,
			LT_Directional,
			LT_Spot
		};

		Light();
		Light(LightTypes type, Vector3 const & pos);

		LightTypes	lightType;

		Vector3		position;
		Vector3		direction;

		Color		diffuse;
		Color		specular;
		Color		ambient;

		float		range;
		float		attenuationConst;
		float		attenuationLinear;
		float		attenuationQuad;

		float		spotOuter;
		float		spotInner;
		float		spotFalloff;
	};
}

#endif			// _LIGHT_HPP
