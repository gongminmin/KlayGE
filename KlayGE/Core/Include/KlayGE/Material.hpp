#ifndef _MATERIAL_HPP
#define _MATERIAL_HPP

#include <KlayGE/Math.hpp>

#if defined(DEBUG) | defined(_DEBUG)
	#pragma comment(lib, "KlayGE_Core_d.lib")
#else
	#pragma comment(lib, "KlayGE_Core.lib")
#endif

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

		Material(Color const & diffuse, Color const & ambient,
			Color const & specular, Color const & emissive, float shininess)
			: diffuse(diffuse),
				ambient(ambient),
				specular(specular),
				emissive(emissive),
				shininess(shininess)
			{ }

		explicit Material(Color const & col)
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