/**
 * @file CascadedShadowLayer.hpp
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

#ifndef _CASCADEDSHADOWLAYER_HPP
#define _CASCADEDSHADOWLAYER_HPP

#pragma once

#ifndef KLAYGE_CORE_SOURCE
#define KLAYGE_LIB_NAME KlayGE_Core
#include <KlayGE/config/auto_link.hpp>
#endif

#include <KlayGE/PreDeclare.hpp>

namespace KlayGE
{
	struct CascadeInfo
	{
		float2 interval;
		float3 scale;
		float3 bias;

		float4x4 crop_mat;
		float4x4 inv_crop_mat;
	};

	KLAYGE_CORE_API AABBox CalcFrustumExtents(Camera const & camera, float near_z, float far_z,
		float4x4 const & light_view_proj);

	class KLAYGE_CORE_API CascadedShadowLayer
	{
	public:
		virtual ~CascadedShadowLayer()
		{
		}

		virtual uint32_t NumCascades() const = 0;
		virtual void NumCascades(uint32_t num_cascades) = 0;

		virtual void UpdateCascades(Camera const & camera, float4x4 const & light_view_proj,
			float lambda, float3 const & light_space_border) = 0;
		virtual CascadeInfo const & GetCascadeInfo(uint32_t index) const = 0;
	};

	class KLAYGE_CORE_API PSSMCascadedShadowLayer : public CascadedShadowLayer
	{
	public:
		PSSMCascadedShadowLayer();
		explicit PSSMCascadedShadowLayer(uint32_t num_cascades);

		virtual uint32_t NumCascades() const KLAYGE_OVERRIDE;
		virtual void NumCascades(uint32_t num_cascades) KLAYGE_OVERRIDE;

		virtual void UpdateCascades(Camera const & camera, float4x4 const & light_view_proj,
			float lambda, float3 const & light_space_border) KLAYGE_OVERRIDE;
		virtual CascadeInfo const & GetCascadeInfo(uint32_t index) const KLAYGE_OVERRIDE;

	private:
		std::vector<CascadeInfo> cascades_;
	};
}

#endif		// _CASCADEDSHADOWLAYER_HPP
