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
#include <KlayGE/RenderEffect.hpp>

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


	uint32_t CascadedShadowLayer::NumCascades() const
	{
		return static_cast<uint32_t>(intervals_.size());
	}

	void CascadedShadowLayer::NumCascades(uint32_t num_cascades)
	{
		intervals_.resize(num_cascades);
		scales_.resize(num_cascades);
		biases_.resize(num_cascades);
		crop_mats_.resize(num_cascades);
		inv_crop_mats_.resize(num_cascades);
	}

	std::vector<float2> const & CascadedShadowLayer::CascadeIntervals() const
	{
		return intervals_;
	}

	std::vector<float3> const & CascadedShadowLayer::CascadeScales() const
	{
		return scales_;
	}

	std::vector<float3> const & CascadedShadowLayer::CascadeBiases() const
	{
		return biases_;
	}

	float4x4 const & CascadedShadowLayer::CascadeCropMatrix(uint32_t index) const
	{
		BOOST_ASSERT(index < crop_mats_.size());
		return crop_mats_[index];
	}

	float4x4 const & CascadedShadowLayer::CascadeInverseCropMatrix(uint32_t index) const
	{
		BOOST_ASSERT(index < inv_crop_mats_.size());
		return inv_crop_mats_[index];
	}


	PSSMCascadedShadowLayer::PSSMCascadedShadowLayer()
		: lambda_(0.8f)
	{
	}

	void PSSMCascadedShadowLayer::Lambda(float lambda)
	{
		lambda_ = lambda;
	}

	void PSSMCascadedShadowLayer::UpdateCascades(Camera const & camera, float4x4 const & light_view_proj,
			float3 const & light_space_border)
	{
		float const range = camera.FarPlane() - camera.NearPlane();
		float const ratio = camera.FarPlane() / camera.NearPlane();

		std::vector<float> distances(intervals_.size() + 1);
		for (size_t i = 0; i < intervals_.size(); ++ i)
		{
			float p = i / static_cast<float>(intervals_.size());
			float log = camera.NearPlane() * std::pow(ratio, p);
			float uniform = camera.NearPlane() + range * p;
			distances[i] = lambda_ * (log - uniform) + uniform;
		}
		distances[intervals_.size()] = camera.FarPlane();

		for (size_t i = 0; i < intervals_.size(); ++ i)
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

			intervals_[i] = float2(distances[i], distances[i + 1]);
			scales_[i] = scale;
			biases_[i] = bias;
			crop_mats_[i] = MathLib::scaling(scale)
				* MathLib::translation(+(2.0f * bias.x() + scale.x() - 1.0f),
					-(2.0f * bias.y() + scale.y() - 1.0f), bias.z());
			inv_crop_mats_[i] = MathLib::inverse(crop_mats_[i]);
		}
	}


	SDSMCascadedShadowLayer::SDSMCascadedShadowLayer()
	{
		RenderFactory& rf = Context::Instance().RenderFactoryInstance();
		interval_buff_ = rf.MakeVertexBuffer(BU_Dynamic, EAH_GPU_Read | EAH_GPU_Write | EAH_GPU_Unordered | EAH_GPU_Structured, nullptr, EF_GR32F);
		scale_buff_ = rf.MakeVertexBuffer(BU_Dynamic, EAH_GPU_Write | EAH_GPU_Unordered | EAH_GPU_Structured, nullptr, EF_BGR32F);
		bias_buff_ = rf.MakeVertexBuffer(BU_Dynamic, EAH_GPU_Write | EAH_GPU_Unordered | EAH_GPU_Structured, nullptr, EF_BGR32F);
		cascade_min_buff_ = rf.MakeVertexBuffer(BU_Dynamic, EAH_GPU_Read | EAH_GPU_Write | EAH_GPU_Unordered | EAH_GPU_Structured, nullptr, EF_BGR32F);
		cascade_max_buff_ = rf.MakeVertexBuffer(BU_Dynamic, EAH_GPU_Read | EAH_GPU_Write | EAH_GPU_Unordered | EAH_GPU_Structured, nullptr, EF_BGR32F);

		interval_cpu_buff_ = rf.MakeVertexBuffer(BU_Dynamic, EAH_CPU_Read, nullptr);
		scale_cpu_buff_ = rf.MakeVertexBuffer(BU_Dynamic, EAH_CPU_Read, nullptr);
		bias_cpu_buff_ = rf.MakeVertexBuffer(BU_Dynamic, EAH_CPU_Read, nullptr);

		int const MAX_NUM_CASCADES = 4;

		interval_buff_->Resize(MAX_NUM_CASCADES * sizeof(float2));
		scale_buff_->Resize(MAX_NUM_CASCADES * sizeof(float3));
		bias_buff_->Resize(MAX_NUM_CASCADES * sizeof(float3));
		cascade_min_buff_->Resize(MAX_NUM_CASCADES * sizeof(float3));
		cascade_max_buff_->Resize(MAX_NUM_CASCADES * sizeof(float3));

		interval_cpu_buff_->Resize(interval_buff_->Size());
		scale_cpu_buff_->Resize(scale_buff_->Size());
		bias_cpu_buff_->Resize(bias_buff_->Size());

		RenderEffectPtr effect = SyncLoadRenderEffect("CascadedShadow.fxml");

		clear_z_bounds_tech_ = effect->TechniqueByName("ClearZBounds");
		reduce_z_bounds_from_depth_tech_ = effect->TechniqueByName("ReduceZBoundsFromDepth");
		compute_log_cascades_from_z_bounds_tech_ = effect->TechniqueByName("ComputeLogCascadesFromZBounds");
		clear_cascade_bounds_tech_ = effect->TechniqueByName("ClearCascadeBounds");
		reduce_bounds_from_depth_tech_ = effect->TechniqueByName("ReduceBoundsFromDepth");
		compute_custom_cascades_tech_ = effect->TechniqueByName("ComputeCustomCascades");

		interval_buff_param_ = effect->ParameterByName("interval_buff");
		interval_buff_uint_param_ = effect->ParameterByName("interval_buff_uint");
		interval_buff_read_param_ = effect->ParameterByName("interval_buff_read");
		scale_buff_param_ = effect->ParameterByName("scale_buff");
		bias_buff_param_ = effect->ParameterByName("bias_buff");
		cascade_min_buff_uint_param_ = effect->ParameterByName("cascade_min_buff_uint");
		cascade_max_buff_uint_param_ = effect->ParameterByName("cascade_max_buff_uint");
		cascade_min_buff_read_param_ = effect->ParameterByName("cascade_min_buff_read");
		cascade_max_buff_read_param_ = effect->ParameterByName("cascade_max_buff_read");
		depth_tex_param_ = effect->ParameterByName("depth_tex");
		num_cascades_param_ = effect->ParameterByName("num_cascades");
		depth_width_height_param_ = effect->ParameterByName("depth_width_height");
		near_far_param_ = effect->ParameterByName("near_far");
		upper_left_param_ = effect->ParameterByName("upper_left");
		xy_dir_param_ = effect->ParameterByName("xy_dir");
		view_to_light_view_proj_param_ = effect->ParameterByName("view_to_light_view_proj");
		light_space_border_param_ = effect->ParameterByName("light_space_border");
		max_cascade_scale_param_ = effect->ParameterByName("max_cascade_scale");
	}

	void SDSMCascadedShadowLayer::DepthTexture(TexturePtr const & depth_tex)
	{
		depth_tex_ = depth_tex;
	}

	void SDSMCascadedShadowLayer::UpdateCascades(Camera const & camera, float4x4 const & light_view_proj,
			float3 const & light_space_border)
	{
		RenderFactory& rf = Context::Instance().RenderFactoryInstance();
		RenderEngine& re = rf.RenderEngineInstance();

		re.BindFrameBuffer(FrameBufferPtr());

		float max_blur_light_space = 8.0f / 1024;
		float3 max_cascade_scale(max_blur_light_space / light_space_border.x(),
			max_blur_light_space / light_space_border.y(),
			std::numeric_limits<float>::max());

		int const TILE_DIM = 128;

		int dispatch_x = (depth_tex_->Width(0) + TILE_DIM - 1) / TILE_DIM;
		int dispatch_y = (depth_tex_->Height(0) + TILE_DIM - 1) / TILE_DIM;

		*interval_buff_param_ = interval_buff_;
		*interval_buff_uint_param_ = interval_buff_;
		*interval_buff_read_param_ = interval_buff_;
		*cascade_min_buff_uint_param_ = cascade_min_buff_;
		*cascade_max_buff_uint_param_ = cascade_max_buff_;
		*cascade_min_buff_read_param_ = cascade_min_buff_;
		*cascade_max_buff_read_param_ = cascade_max_buff_;
		*scale_buff_param_ = scale_buff_;
		*bias_buff_param_ = bias_buff_;
		*depth_tex_param_ = depth_tex_;
		*num_cascades_param_ = static_cast<uint32_t>(intervals_.size());
		*depth_width_height_param_ = int2(depth_tex_->Width(0), depth_tex_->Height(0));
		*near_far_param_ = float2(camera.NearPlane(), camera.FarPlane());
		float4x4 const & inv_proj = camera.InverseProjMatrix();
		float3 upper_left = MathLib::transform_coord(float3(-1, +1, 1), inv_proj);
		float3 upper_right = MathLib::transform_coord(float3(+1, +1, 1), inv_proj);
		float3 lower_left = MathLib::transform_coord(float3(-1, -1, 1), inv_proj);
		*upper_left_param_ = upper_left;
		*xy_dir_param_ = float2(upper_right.x() - upper_left.x(), lower_left.y() - upper_left.y());
		*view_to_light_view_proj_param_ = camera.InverseViewMatrix() * light_view_proj;
		*light_space_border_param_ = light_space_border;
		*max_cascade_scale_param_ = max_cascade_scale;

		re.Dispatch(*clear_z_bounds_tech_, 1, 1, 1);
		re.Dispatch(*reduce_z_bounds_from_depth_tech_, dispatch_x, dispatch_y, 1);
		re.Dispatch(*compute_log_cascades_from_z_bounds_tech_, 1, 1, 1);
		re.Dispatch(*clear_cascade_bounds_tech_, 1, 1, 1);
		re.Dispatch(*reduce_bounds_from_depth_tech_, dispatch_x, dispatch_y, 1);
		re.Dispatch(*compute_custom_cascades_tech_, 1, 1, 1);

		interval_buff_->CopyToBuffer(*interval_cpu_buff_);
		scale_buff_->CopyToBuffer(*scale_cpu_buff_);
		bias_buff_->CopyToBuffer(*bias_cpu_buff_);

		GraphicsBuffer::Mapper interval_mapper(*interval_cpu_buff_, BA_Read_Only);
		GraphicsBuffer::Mapper scale_mapper(*scale_cpu_buff_, BA_Read_Only);
		GraphicsBuffer::Mapper bias_mapper(*bias_cpu_buff_, BA_Read_Only);
		float2* interval_ptr = interval_mapper.Pointer<float2>();
		float3* scale_ptr = scale_mapper.Pointer<float3>();
		float3* bias_ptr = bias_mapper.Pointer<float3>();

		for (size_t i = 0; i < intervals_.size(); ++ i)
		{
			float3 const & scale = scale_ptr[i];
			float3 const & bias = bias_ptr[i];

			intervals_[i] = interval_ptr[i];
			scales_[i] = scale;
			biases_[i] = bias;

			crop_mats_[i] = MathLib::scaling(scale)
				* MathLib::translation(+(2.0f * bias.x() + scale.x() - 1.0f),
					-(2.0f * bias.y() + scale.y() - 1.0f), bias.z());
			inv_crop_mats_[i] = MathLib::inverse(crop_mats_[i]);
		}
	}
}
