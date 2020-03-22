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
#include <KFL/Half.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/Camera.hpp>
#include <KlayGE/RenderEffect.hpp>
#include <KlayGE/PostProcess.hpp>

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


	CascadedShadowLayer::~CascadedShadowLayer() noexcept = default;

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

	void CascadedShadowLayer::UpdateCropMats()
	{
		for (size_t i = 0; i < intervals_.size(); ++ i)
		{
			float3 const scale = scales_[i];
			float3 const bias = biases_[i];

			crop_mats_[i] = MathLib::scaling(scale)
				* MathLib::translation(+(2.0f * bias.x() + scale.x() - 1.0f),
					-(2.0f * bias.y() + scale.y() - 1.0f), bias.z());
		}
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
		}

		this->UpdateCropMats();
	}


	SDSMCascadedShadowLayer::SDSMCascadedShadowLayer()
	{
		RenderFactory& rf = Context::Instance().RenderFactoryInstance();
		RenderDeviceCaps const & caps = rf.RenderEngineInstance().DeviceCaps();
		cs_support_ = caps.cs_support && (caps.max_shader_model >= ShaderModel(5, 0));

		if (cs_support_)
		{
			interval_buff_ = rf.MakeVertexBuffer(BU_Dynamic,
				EAH_GPU_Read | EAH_GPU_Write | EAH_GPU_Unordered | EAH_GPU_Structured,
				MAX_NUM_CASCADES * sizeof(float2), nullptr, sizeof(float2));
			interval_buff_float_uav_ = rf.MakeBufferUav(interval_buff_, EF_GR32F);
			interval_buff_uint_uav_ = rf.MakeBufferUav(interval_buff_, EF_GR32UI);
			interval_buff_srv_ = rf.MakeBufferSrv(interval_buff_, EF_GR32F);

			scale_buff_ = rf.MakeVertexBuffer(BU_Dynamic, EAH_GPU_Write | EAH_GPU_Unordered | EAH_GPU_Structured,
				MAX_NUM_CASCADES * sizeof(float3), nullptr, sizeof(float3));
			scale_buff_uav_ = rf.MakeBufferUav(scale_buff_, EF_BGR32F);

			bias_buff_ = rf.MakeVertexBuffer(BU_Dynamic, EAH_GPU_Write | EAH_GPU_Unordered | EAH_GPU_Structured,
				MAX_NUM_CASCADES * sizeof(float3), nullptr, sizeof(float3));
			bias_buff_uav_ = rf.MakeBufferUav(bias_buff_, EF_BGR32F);

			cascade_min_buff_ = rf.MakeVertexBuffer(BU_Dynamic,
				EAH_GPU_Read | EAH_GPU_Write | EAH_GPU_Unordered | EAH_GPU_Structured,
				MAX_NUM_CASCADES * sizeof(float3), nullptr, sizeof(float3));
			cascade_min_buff_uint_uav_ = rf.MakeBufferUav(cascade_min_buff_, EF_BGR32UI);
			cascade_min_buff_srv_ = rf.MakeBufferSrv(cascade_min_buff_, EF_BGR32F);

			cascade_max_buff_ = rf.MakeVertexBuffer(BU_Dynamic,
				EAH_GPU_Read | EAH_GPU_Write | EAH_GPU_Unordered | EAH_GPU_Structured,
				MAX_NUM_CASCADES * sizeof(float3), nullptr, sizeof(float3));
			cascade_max_buff_uint_uav_ = rf.MakeBufferUav(cascade_max_buff_, EF_BGR32UI);
			cascade_max_buff_srv_ = rf.MakeBufferSrv(cascade_max_buff_, EF_BGR32F);

			interval_cpu_buff_ = rf.MakeVertexBuffer(BU_Dynamic, EAH_CPU_Read, interval_buff_->Size(), nullptr);
			scale_cpu_buff_ = rf.MakeVertexBuffer(BU_Dynamic, EAH_CPU_Read, scale_buff_->Size(), nullptr);
			bias_cpu_buff_ = rf.MakeVertexBuffer(BU_Dynamic, EAH_CPU_Read, bias_buff_->Size(), nullptr);

			effect_ = SyncLoadRenderEffect("CascadedShadow.fxml");

			clear_z_bounds_tech_ = effect_->TechniqueByName("ClearZBounds");
			reduce_z_bounds_from_depth_tech_ = effect_->TechniqueByName("ReduceZBoundsFromDepth");
			compute_log_cascades_from_z_bounds_tech_ = effect_->TechniqueByName("ComputeLogCascadesFromZBounds");
			clear_cascade_bounds_tech_ = effect_->TechniqueByName("ClearCascadeBounds");
			reduce_bounds_from_depth_tech_ = effect_->TechniqueByName("ReduceBoundsFromDepth");
			compute_custom_cascades_tech_ = effect_->TechniqueByName("ComputeCustomCascades");

			interval_buff_param_ = effect_->ParameterByName("interval_buff");
			interval_buff_uint_param_ = effect_->ParameterByName("interval_buff_uint");
			interval_buff_read_param_ = effect_->ParameterByName("interval_buff_read");
			scale_buff_param_ = effect_->ParameterByName("scale_buff");
			bias_buff_param_ = effect_->ParameterByName("bias_buff");
			cascade_min_buff_uint_param_ = effect_->ParameterByName("cascade_min_buff_uint");
			cascade_max_buff_uint_param_ = effect_->ParameterByName("cascade_max_buff_uint");
			cascade_min_buff_read_param_ = effect_->ParameterByName("cascade_min_buff_read");
			cascade_max_buff_read_param_ = effect_->ParameterByName("cascade_max_buff_read");
			depth_tex_param_ = effect_->ParameterByName("depth_tex");
			num_cascades_param_ = effect_->ParameterByName("num_cascades");
			inv_depth_width_height_param_ = effect_->ParameterByName("inv_depth_width_height");
			near_far_param_ = effect_->ParameterByName("near_far");
			upper_left_param_ = effect_->ParameterByName("upper_left");
			xy_dir_param_ = effect_->ParameterByName("xy_dir");
			view_to_light_view_proj_param_ = effect_->ParameterByName("view_to_light_view_proj");
			light_space_border_param_ = effect_->ParameterByName("light_space_border");
			max_cascade_scale_param_ = effect_->ParameterByName("max_cascade_scale");
		}
		else
		{
			reduce_z_bounds_from_depth_pp_ = SyncLoadPostProcess("CascadedShadow.ppml", "reduce_z_bounds_from_depth");
			reduce_z_bounds_from_depth_mip_map_pp_ = SyncLoadPostProcess("CascadedShadow.ppml", "reduce_z_bounds_from_depth_mip_map");
			compute_log_cascades_from_z_bounds_pp_ = SyncLoadPostProcess("CascadedShadow.ppml", "compute_log_cascades_from_z_bounds");

			interval_tex_ = rf.MakeTexture2D(MAX_NUM_CASCADES, 1, 1, 1, EF_GR16F, 1, 0, EAH_GPU_Read | EAH_GPU_Write);
			interval_rtv_ = rf.Make2DRtv(interval_tex_, 0, 1, 0);
			interval_cpu_tex_ = rf.MakeTexture2D(MAX_NUM_CASCADES, 1, 1, 1, EF_GR16F, 1, 0, EAH_CPU_Read);
		}
	}

	void SDSMCascadedShadowLayer::DepthTexture(TexturePtr const & depth_tex)
	{
		auto& rf = Context::Instance().RenderFactoryInstance();

		depth_tex_ = depth_tex;
		depth_srv_ = rf.MakeTextureSrv(depth_tex_, 0, 1, 0, 1);

		if (!cs_support_)
		{
			uint32_t const width = depth_tex->Width(0);
			uint32_t const height = depth_tex->Height(0);

			depth_derivative_tex_ = rf.MakeTexture2D(width / 2, height / 2, 0, 1, EF_GR16F, 1, 0, EAH_GPU_Read | EAH_GPU_Write);
			auto depth_derivative_srv = rf.MakeTextureSrv(depth_derivative_tex_);
			auto depth_derivative_rtv = rf.Make2DRtv(depth_derivative_tex_, 0, 1, 0);
			if (rf.RenderEngineInstance().DeviceCaps().flexible_srvs_support)
			{
				depth_derivative_mip_srvs_.resize(depth_derivative_tex_->NumMipMaps());
				depth_derivative_mip_rtvs_.resize(depth_derivative_tex_->NumMipMaps());
				for (uint32_t i = 0; i < depth_derivative_tex_->NumMipMaps(); ++ i)
				{
					depth_derivative_mip_srvs_[i] = rf.MakeTextureSrv(depth_derivative_tex_, 0, 1, i, 1);
					depth_derivative_mip_rtvs_[i] = rf.Make2DRtv(depth_derivative_tex_, 0, 1, i);
				}
			}
			else
			{
				depth_derivative_small_tex_ = rf.MakeTexture2D(width / 4, height / 4, 0, 1, EF_GR16F, 1, 0, EAH_GPU_Write);
				depth_derivative_small_mip_rtvs_.resize(depth_derivative_small_tex_->NumMipMaps());
				for (uint32_t i = 0; i < depth_derivative_small_tex_->NumMipMaps(); ++i)
				{
					depth_derivative_small_mip_rtvs_[i] = rf.Make2DRtv(depth_derivative_small_tex_, 0, 1, i);
				}
				reduce_z_bounds_from_depth_mip_map_pp_->InputPin(0, depth_derivative_srv);
			}

			float delta_x = 1.0f / depth_tex_->Width(0);
			float delta_y = 1.0f / depth_tex_->Height(0);
			reduce_z_bounds_from_depth_pp_->SetParam(0, float4(delta_x, delta_y, -delta_x / 2, -delta_y / 2));
			reduce_z_bounds_from_depth_pp_->InputPin(0, depth_srv_);
			reduce_z_bounds_from_depth_pp_->OutputPin(0, depth_derivative_rtv);
			compute_log_cascades_from_z_bounds_pp_->SetParam(0, static_cast<float>(depth_derivative_tex_->NumMipMaps() - 1));
			compute_log_cascades_from_z_bounds_pp_->InputPin(0, depth_derivative_srv);
			compute_log_cascades_from_z_bounds_pp_->OutputPin(0, interval_rtv_);
		}
	}

	void SDSMCascadedShadowLayer::UpdateCascades(Camera const & camera, float4x4 const & light_view_proj,
			float3 const & light_space_border)
	{
		RenderFactory& rf = Context::Instance().RenderFactoryInstance();
		RenderEngine& re = rf.RenderEngineInstance();

		uint32_t const num_cascades = static_cast<uint32_t>(intervals_.size());

		if (cs_support_)
		{
			re.BindFrameBuffer(FrameBufferPtr());

			float max_blur_light_space = 8.0f / 1024;
			float3 max_cascade_scale(max_blur_light_space / light_space_border.x(),
				max_blur_light_space / light_space_border.y(),
				std::numeric_limits<float>::max());

			int const TILE_DIM = 128;

			int dispatch_x = (depth_tex_->Width(0) + TILE_DIM - 1) / TILE_DIM;
			int dispatch_y = (depth_tex_->Height(0) + TILE_DIM - 1) / TILE_DIM;

			*interval_buff_param_ = interval_buff_float_uav_;
			*interval_buff_uint_param_ = interval_buff_uint_uav_;
			*interval_buff_read_param_ = interval_buff_srv_;
			*cascade_min_buff_uint_param_ = cascade_min_buff_uint_uav_;
			*cascade_max_buff_uint_param_ = cascade_max_buff_uint_uav_;
			*cascade_min_buff_read_param_ = cascade_min_buff_srv_;
			*cascade_max_buff_read_param_ = cascade_max_buff_srv_;
			*scale_buff_param_ = scale_buff_uav_;
			*bias_buff_param_ = bias_buff_uav_;
			*depth_tex_param_ = depth_srv_;
			*num_cascades_param_ = static_cast<int32_t>(num_cascades);
			*inv_depth_width_height_param_ = float2(1.0f / depth_tex_->Width(0), 1.0f / depth_tex_->Height(0));
			*near_far_param_ = float2(camera.NearPlane(), camera.FarPlane());
			float4x4 const & inv_proj = camera.InverseProjMatrix();
			float const flipping = re.RequiresFlipping() ? -1.0f : +1.0f;
			float3 const upper_left = MathLib::transform_coord(float3(-1, -flipping, 1), inv_proj);
			float3 const upper_right = MathLib::transform_coord(float3(+1, -flipping, 1), inv_proj);
			float3 const lower_left = MathLib::transform_coord(float3(-1, flipping, 1), inv_proj);
			*upper_left_param_ = upper_left;
			*xy_dir_param_ = float2(upper_right.x() - upper_left.x(), lower_left.y() - upper_left.y());
			*view_to_light_view_proj_param_ = camera.InverseViewMatrix() * light_view_proj;
			*light_space_border_param_ = light_space_border;
			*max_cascade_scale_param_ = max_cascade_scale;

			re.Dispatch(*effect_, *clear_z_bounds_tech_, 1, 1, 1);
			re.Dispatch(*effect_, *reduce_z_bounds_from_depth_tech_, dispatch_x, dispatch_y, 1);
			re.Dispatch(*effect_, *compute_log_cascades_from_z_bounds_tech_, 1, 1, 1);
			re.Dispatch(*effect_, *clear_cascade_bounds_tech_, 1, 1, 1);
			re.Dispatch(*effect_, *reduce_bounds_from_depth_tech_, dispatch_x, dispatch_y, 1);
			re.Dispatch(*effect_, *compute_custom_cascades_tech_, 1, 1, 1);

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
				intervals_[i] = interval_ptr[i];
				scales_[i] = scale_ptr[i];
				biases_[i] = bias_ptr[i];
			}
		}
		else
		{
			float2 const near_far(camera.NearPlane(), camera.FarPlane());

			reduce_z_bounds_from_depth_pp_->SetParam(1, near_far);
			reduce_z_bounds_from_depth_pp_->Apply();

			for (uint32_t i = 1; i < depth_derivative_tex_->NumMipMaps(); ++ i)
			{
				uint32_t const width = depth_derivative_tex_->Width(i - 1);
				uint32_t const height = depth_derivative_tex_->Height(i - 1);
				uint32_t const lower_width = depth_derivative_tex_->Width(i);
				uint32_t const lower_height = depth_derivative_tex_->Height(i);

				float const delta_x = 1.0f / width;
				float const delta_y = 1.0f / height;
				float4 const delta_offset(delta_x, delta_y, -delta_x / 2, -delta_y / 2);
				reduce_z_bounds_from_depth_mip_map_pp_->SetParam(0, delta_offset);

				if (re.DeviceCaps().flexible_srvs_support)
				{
					reduce_z_bounds_from_depth_mip_map_pp_->InputPin(0, depth_derivative_mip_srvs_[i - 1]);
					reduce_z_bounds_from_depth_mip_map_pp_->OutputPin(0, depth_derivative_mip_rtvs_[i]);
					reduce_z_bounds_from_depth_mip_map_pp_->Apply();
				}
				else
				{
					reduce_z_bounds_from_depth_mip_map_pp_->OutputPin(0, depth_derivative_small_mip_rtvs_[i - 1]);
					reduce_z_bounds_from_depth_mip_map_pp_->Apply();

					depth_derivative_small_tex_->CopyToSubTexture2D(*depth_derivative_tex_, 0, i, 0, 0, lower_width, lower_height, 0, i - 1,
						0, 0, lower_width, lower_height, TextureFilter::Point);
				}
			}

			compute_log_cascades_from_z_bounds_pp_->SetParam(1, static_cast<int32_t>(num_cascades));
			compute_log_cascades_from_z_bounds_pp_->SetParam(2, near_far);
			compute_log_cascades_from_z_bounds_pp_->Apply();

			interval_tex_->CopyToSubTexture2D(
				*interval_cpu_tex_, 0, 0, 0, 0, num_cascades, 1, 0, 0, 0, 0, num_cascades, 1, TextureFilter::Point);

			Texture::Mapper interval_mapper(*interval_cpu_tex_, 0, 0,
				TMA_Read_Only, 0, 0, num_cascades, 1);
			Vector_T<half, 2>* interval_ptr = interval_mapper.Pointer<Vector_T<half, 2>>();

			for (size_t i = 0; i < intervals_.size(); ++ i)
			{
				float2 const interval(static_cast<float>(interval_ptr[i].x()),
					static_cast<float>(interval_ptr[i].y()));

				AABBox aabb = CalcFrustumExtents(camera, interval.x(), interval.y(), light_view_proj);

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

				intervals_[i] = interval;
				scales_[i] = scale;
				biases_[i] = bias;
			}
		}

		this->UpdateCropMats();
	}
}
