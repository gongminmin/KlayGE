/**
 * @file RenderMaterial.hpp
 * @author Minmin Gong
 *
 * @section DESCRIPTION
 *
 * This source file is part of KlayGE
 * For the latest info, see http://www.klayge.org
 *
 * @section LICENSE
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published
 * by the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 * You may alternatively use this source under the terms of
 * the KlayGE Proprietary License (KPL). You can obtained such a license
 * from http://www.klayge.org/licensing/.
 */

#ifndef _RENDERMATERIAL_HPP
#define _RENDERMATERIAL_HPP

#pragma once

#include <KlayGE/PreDeclare.hpp>
#include <vector>

namespace KlayGE
{
	struct KLAYGE_CORE_API RenderMaterial
	{
		enum SurfaceDetailMode
		{
			SDM_Parallax = 0,
			SDM_FlatTessellation,
			SDM_SmoothTessellation
		};

		float4 albedo;
		float metalness;
		float glossiness;
		float3 emissive;

		bool transparent;
		float alpha_test;
		bool sss;

		std::string albedo_tex_name;
		std::string metalness_tex_name;
		std::string glossiness_tex_name;
		std::string emissive_tex_name;
		std::string normal_tex_name;
		std::string height_tex_name;

		SurfaceDetailMode detail_mode;
		float2 height_offset_scale;
		float4 tess_factors;
	};

	float const MAX_SHININESS = 8192;
	float const INV_LOG_MAX_SHININESS = 1 / log(MAX_SHININESS);

	inline float Shininess2Glossiness(float shininess)
	{
		return log(shininess) * INV_LOG_MAX_SHININESS;
	}

	inline float Glossiness2Shininess(float glossiness)
	{
		return pow(MAX_SHININESS, glossiness);
	}
}

#endif		//_RENDERMATERIAL_HPP
