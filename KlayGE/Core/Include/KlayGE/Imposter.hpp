/**
 * @file Imposter.hpp
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

#ifndef _IMPOSTER_HPP
#define _IMPOSTER_HPP

#pragma once

#include <KlayGE/PreDeclare.hpp>

namespace KlayGE
{
	class KLAYGE_CORE_API Imposter final : boost::noncopyable
	{
	public:
		Imposter(uint32_t num_azimuth, uint32_t num_elevation, uint32_t size, TexturePtr const & rt0_tex, TexturePtr const & rt1_tex);

		float2 StartTexCoord(float azimuth, float elevation) const;
		float2 StartTexCoord(float3 const & dir) const;

		uint32_t NumAzimuth() const
		{
			return num_azimuth_;
		}
		uint32_t NumElevation() const
		{
			return num_elevation_;
		}
		float AzimuthAngleStep() const
		{
			return azimuth_angle_step_;
		}
		float ElevationAngleStep() const
		{
			return elevation_angle_step_;
		}

		float2 ImposterSize() const;

		TexturePtr const & RT0Texture() const
		{
			return rt0_tex_;
		}
		TexturePtr const & RT1Texture() const
		{
			return rt1_tex_;
		}

	private:
		uint32_t num_azimuth_;
		uint32_t num_elevation_;
		float azimuth_angle_step_;
		float elevation_angle_step_;
		float2 size_;
		TexturePtr rt0_tex_;
		TexturePtr rt1_tex_;
	};

	KLAYGE_CORE_API ImposterPtr SyncLoadImposter(std::string_view impml_name);
	KLAYGE_CORE_API ImposterPtr ASyncLoadImposter(std::string_view impml_name);
}

#endif		// _IMPOSTER_HPP
