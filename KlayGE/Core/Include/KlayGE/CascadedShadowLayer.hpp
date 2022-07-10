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

#include <KlayGE/PreDeclare.hpp>

namespace KlayGE
{
	enum CascadedShadowLayerType
	{
		CSLT_Auto,
		CSLT_PSSM,
		CSLT_SDSM
	};

	KLAYGE_CORE_API AABBox CalcFrustumExtents(Camera const & camera, float near_z, float far_z,
		float4x4 const & light_view_proj);

	class KLAYGE_CORE_API CascadedShadowLayer : boost::noncopyable
	{
	public:
		static uint32_t const MAX_NUM_CASCADES = 4UL;

	public:
		virtual ~CascadedShadowLayer() noexcept;

		virtual CascadedShadowLayerType Type() const = 0;

		virtual uint32_t NumCascades() const;
		virtual void NumCascades(uint32_t num_cascades);

		virtual void UpdateCascades(Camera const & camera, float4x4 const & light_view_proj,
			float3 const & light_space_border) = 0;

		std::vector<float2> const & CascadeIntervals() const;
		std::vector<float3> const & CascadeScales() const;
		std::vector<float3> const & CascadeBiases() const;
		float4x4 const & CascadeCropMatrix(uint32_t index) const;

	protected:
		void UpdateCropMats();

	protected:
		std::vector<float2> intervals_;
		std::vector<float3> scales_;
		std::vector<float3> biases_;
		std::vector<float4x4> crop_mats_;
	};

	class KLAYGE_CORE_API PSSMCascadedShadowLayer final : public CascadedShadowLayer
	{
	public:
		PSSMCascadedShadowLayer();

		CascadedShadowLayerType Type() const override
		{
			return CSLT_PSSM;
		}

		void Lambda(float lambda);

		virtual void UpdateCascades(Camera const & camera, float4x4 const & light_view_proj,
			float3 const & light_space_border) override;

	private:
		float lambda_;
	};

	class KLAYGE_CORE_API SDSMCascadedShadowLayer final : public CascadedShadowLayer
	{
	public:
		SDSMCascadedShadowLayer();

		CascadedShadowLayerType Type() const override
		{
			return CSLT_SDSM;
		}

		void DepthTexture(TexturePtr const & depth_tex);

		virtual void UpdateCascades(Camera const & camera, float4x4 const & light_view_proj,
			float3 const & light_space_border) override;

	private:
		TexturePtr depth_tex_;
		ShaderResourceViewPtr depth_srv_;
		bool cs_support_;

		// For CS implement
		RenderEffectPtr effect_;
		RenderTechnique* clear_z_bounds_tech_;
		RenderTechnique* reduce_z_bounds_from_depth_tech_;
		RenderTechnique* compute_log_cascades_from_z_bounds_tech_;
		RenderTechnique* clear_cascade_bounds_tech_;
		RenderTechnique* reduce_bounds_from_depth_tech_;
		RenderTechnique* compute_custom_cascades_tech_;
		RenderEffectParameter* interval_buff_param_;
		RenderEffectParameter* interval_buff_uint_param_;
		RenderEffectParameter* interval_buff_read_param_;
		RenderEffectParameter* scale_buff_param_;
		RenderEffectParameter* bias_buff_param_;
		RenderEffectParameter* cascade_min_buff_uint_param_;
		RenderEffectParameter* cascade_max_buff_uint_param_;
		RenderEffectParameter* cascade_min_buff_read_param_;
		RenderEffectParameter* cascade_max_buff_read_param_;
		RenderEffectParameter* depth_tex_param_;
		RenderEffectParameter* num_cascades_param_;
		RenderEffectParameter* inv_depth_width_height_param_;
		RenderEffectParameter* near_far_param_;
		RenderEffectParameter* upper_left_param_;
		RenderEffectParameter* xy_dir_param_;
		RenderEffectParameter* view_to_light_view_proj_param_;
		RenderEffectParameter* light_space_border_param_;
		RenderEffectParameter* max_cascade_scale_param_;

		GraphicsBufferPtr interval_buff_;
		UnorderedAccessViewPtr interval_buff_float_uav_;
		UnorderedAccessViewPtr interval_buff_uint_uav_;
		ShaderResourceViewPtr interval_buff_srv_;
		GraphicsBufferPtr scale_buff_;
		UnorderedAccessViewPtr scale_buff_uav_;
		GraphicsBufferPtr bias_buff_;
		UnorderedAccessViewPtr bias_buff_uav_;
		GraphicsBufferPtr cascade_min_buff_;
		UnorderedAccessViewPtr cascade_min_buff_uint_uav_;
		ShaderResourceViewPtr cascade_min_buff_srv_;
		GraphicsBufferPtr cascade_max_buff_;
		UnorderedAccessViewPtr cascade_max_buff_uint_uav_;
		ShaderResourceViewPtr cascade_max_buff_srv_;

		GraphicsBufferPtr interval_cpu_buff_;
		GraphicsBufferPtr scale_cpu_buff_;
		GraphicsBufferPtr bias_cpu_buff_;

		// For PS implement
		TexturePtr depth_derivative_tex_;
		std::vector<ShaderResourceViewPtr> depth_derivative_mip_srvs_;
		std::vector<RenderTargetViewPtr> depth_derivative_mip_rtvs_;
		TexturePtr depth_derivative_small_tex_;
		std::vector<RenderTargetViewPtr> depth_derivative_small_mip_rtvs_;
		PostProcessPtr reduce_z_bounds_from_depth_pp_;
		PostProcessPtr reduce_z_bounds_from_depth_mip_map_pp_;
		PostProcessPtr compute_log_cascades_from_z_bounds_pp_;

		TexturePtr interval_tex_;
		RenderTargetViewPtr interval_rtv_;
		TexturePtr interval_cpu_tex_;
	};
}

#endif		// _CASCADEDSHADOWLAYER_HPP
