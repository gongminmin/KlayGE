#ifndef _MATERIAL_HPP
#define _MATERIAL_HPP

#include <KlayGE/Math.hpp>

#pragma comment(lib, "KlayGE_Core.lib")

namespace KlayGE
{
	struct Material
	{
		Material()
			: diffuse(0, 0, 0, 0),
				ambient(0, 0, 0, 0),
				specular(0, 0, 0, 0),
				emissive(0, 0, 0, 0),
				shininess(0)
			{ }

		Material(const Color& diffuse, const Color& ambient,
			const Color& specular, const Color& emissive, float shininess)
			: diffuse(diffuse),
				ambient(ambient),
				specular(specular),
				emissive(emissive),
				shininess(shininess)
			{ }

		explicit Material(const Color& col)
			: diffuse(col),
				ambient(col),
				specular(0, 0, 0, 0),
				emissive(0, 0, 0, 0),
				shininess(0)
			{ }

		Color diffuse;
        Color ambient;
		Color specular;
		Color emissive;
		float shininess;
	};
}

#endif			// _MATERIAL_HPP