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
#include <string>
#include <array>

namespace KlayGE
{
	struct KLAYGE_CORE_API RenderMaterial
	{
		enum TextureSlot
		{
			TS_Albedo,
			TS_Metalness,
			TS_Glossiness,
			TS_Emissive,
			TS_Normal,
			TS_Height,
			TS_Occlusion,

			TS_NumTextureSlots
		};

		enum SurfaceDetailMode
		{
			SDM_Parallax = 0,
			SDM_FlatTessellation,
			SDM_SmoothTessellation
		};

		std::string name;

		float4 albedo = float4(0, 0, 0, 0);
		float metalness = 0;
		float glossiness = 0;
		float3 emissive = float3(0, 0, 0);

		bool transparent = false;
		float alpha_test = 0;
		bool sss = false;
		bool two_sided = false;

		float normal_scale = 1;
		float occlusion_strength = 1;

		std::array<std::string, TS_NumTextureSlots> tex_names;

		SurfaceDetailMode detail_mode = SDM_Parallax;
		float2 height_offset_scale = float2(0, 1);
		float4 tess_factors = float4(1, 1, 1, 1);
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

	KLAYGE_CORE_API RenderMaterialPtr SyncLoadRenderMaterial(std::string_view mtlml_name);
	KLAYGE_CORE_API RenderMaterialPtr ASyncLoadRenderMaterial(std::string_view mtlml_name);
	KLAYGE_CORE_API void SaveRenderMaterial(RenderMaterialPtr const & mtl, std::string const & mtlml_name);
}

#endif		//_RENDERMATERIAL_HPP
