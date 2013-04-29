/**
 * @file CascadedShadowLayer.cpp
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

#include <KlayGE/KlayGE.hpp>
#include <KFL/Math.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/Camera.hpp>

#include <algorithm>

#include <KlayGE/CascadedShadowLayer.hpp>

namespace KlayGE
{
	AABBox CalcFrustumExtents(Camera const & camera, float near_z, float far_z, float4x4 const & light_view_proj)
	{
		float const inv_scale_x = 1.0f / camera.ProjMatrix()(0, 0);
		float const inv_scale_y = 1.0f / camera.ProjMatrix()(1, 1);

		float4x4 const view_to_light_proj = camera.InverseViewMatrix() * light_view_proj;

		float3 corners[8];

		float near_x = inv_scale_x * near_z;
		float near_y = inv_scale_y * near_z;
		corners[0] = float3(-near_x, +near_y, near_z);
		corners[1] = float3(+near_x, +near_y, near_z);
		corners[2] = float3(-near_x, -near_y, near_z);
		corners[3] = float3(+near_x, -near_y, near_z);

		float far_x = inv_scale_x * far_z;
		float far_y = inv_scale_y * far_z;
		corners[4] = float3(-far_x, +far_y, far_z);
		corners[5] = float3(+far_x, +far_y, far_z);
		corners[6] = float3(-far_x, -far_y, far_z);
		corners[7] = float3(+far_x, -far_y, far_z);

		for (uint32_t i = 0; i < 8; ++ i)
		{
			corners[i] = MathLib::transform_coord(corners[i], view_to_light_proj);
		}

		return MathLib::compute_aabbox(corners, corners + 8);
	}


	PSSMCascadedShadowLayer::PSSMCascadedShadowLayer()
	{
	}

	PSSMCascadedShadowLayer::PSSMCascadedShadowLayer(uint32_t num_cascades)
	{
		this->NumCascades(num_cascades);
	}

	uint32_t PSSMCascadedShadowLayer::NumCascades() const
	{
		return static_cast<uint32_t>(cascades_.size());
	}

	void PSSMCascadedShadowLayer::NumCascades(uint32_t num_cascades)
	{
		cascades_.resize(num_cascades);
	}

	void PSSMCascadedShadowLayer::UpdateCascades(Camera const & camera, float4x4 const & light_view_proj,
			float lambda, float3 const & light_space_border)
	{
		float const range = camera.FarPlane() - camera.NearPlane();
		float const ratio = camera.FarPlane() / camera.NearPlane();

		std::vector<float> distances(cascades_.size() + 1);
		for (size_t i = 0; i < cascades_.size(); ++ i)
		{
			float p = i / static_cast<float>(cascades_.size());
			float log = camera.NearPlane() * std::pow(ratio, p);
			float uniform = camera.NearPlane() + range * p;
			distances[i] = lambda * (log - uniform) + uniform;
		}
		distances[cascades_.size()] = camera.FarPlane();

		for (size_t i = 0; i < cascades_.size(); ++ i)
		{
			AABBox aabb = CalcFrustumExtents(camera, distances[i], distances[i + 1],
								  light_view_proj);

			aabb &= AABBox(float3(-1, -1, -1), float3(+1, +1, +1));

			aabb.Min() -= light_space_border;
			aabb.Max() += light_space_border;

			aabb.Min().x() = +aabb.Min().x() * 0.5f + 0.5f;
			aabb.Min().y() = -aabb.Min().y() * 0.5f + 0.5f;
			aabb.Max().x() = +aabb.Max().x() * 0.5f + 0.5f;
			aabb.Max().y() = -aabb.Max().y() * 0.5f + 0.5f;

			std::swap(aabb.Min().y(), aabb.Max().y());

			float3 const scale = float3(1.0f, 1.0f, 1.0f) / (aabb.Max() - aabb.Min());
			float3 const bias = -aabb.Min() * scale;

			cascades_[i].interval = float2(distances[i], distances[i + 1]);
			cascades_[i].scale = scale;
			cascades_[i].bias = bias;
			cascades_[i].crop_mat = MathLib::scaling(scale)
				* MathLib::translation(+(2.0f * bias.x() + scale.x() - 1.0f),
					-(2.0f * bias.y() + scale.y() - 1.0f), bias.z());
			cascades_[i].inv_crop_mat = MathLib::inverse(cascades_[i].crop_mat);
		}
	}

	CascadeInfo const & PSSMCascadedShadowLayer::GetCascadeInfo(uint32_t index) const
	{
		BOOST_ASSERT(index < cascades_.size());
		return cascades_[index];
	}
}
