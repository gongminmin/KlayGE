/**
 * @file IndirectLightingLayer.hpp
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

#ifndef _INDIRECTLIGHTINGLAYER_HPP
#define _INDIRECTLIGHTINGLAYER_HPP

#pragma once

#include <KlayGE/PreDeclare.hpp>
#include <KlayGE/MultiResLayer.hpp>

#include <array>

namespace KlayGE
{
	class KLAYGE_CORE_API IndirectLightingLayer : boost::noncopyable
	{
	public:
		virtual ~IndirectLightingLayer() noexcept;

		virtual void GBuffer(TexturePtr const & rt0_tex, TexturePtr const & rt1_tex, TexturePtr const & depth_tex) = 0;
		virtual void RSM(TexturePtr const & rt0_tex, TexturePtr const & rt1_tex, TexturePtr const & depth_tex) = 0;

		TexturePtr const& IndirectLightingTex() const
		{
			return indirect_lighting_tex_;
		}
		ShaderResourceViewPtr const& IndirectLightingSrv() const
		{
			return indirect_lighting_srv_;
		}

		virtual void UpdateGBuffer(Camera const & vp_camera) = 0;
		virtual void UpdateRSM(Camera const & rsm_camera, LightSource const & light) = 0;
		virtual void CalcIndirectLighting(TexturePtr const & prev_shading_tex, float4x4 const & proj_to_prev) = 0;

	protected:
		TexturePtr indirect_lighting_tex_;
		ShaderResourceViewPtr indirect_lighting_srv_;
	};

	class KLAYGE_CORE_API MultiResSILLayer final : public IndirectLightingLayer
	{
	public:
		MultiResSILLayer();

		virtual void GBuffer(TexturePtr const & rt0_tex, TexturePtr const & rt1_tex, TexturePtr const & depth_tex) override;
		virtual void RSM(TexturePtr const & rt0_tex, TexturePtr const & rt1_tex, TexturePtr const & depth_tex) override;

		virtual void UpdateGBuffer(Camera const & vp_camera) override;
		virtual void UpdateRSM(Camera const & rsm_camera, LightSource const & light) override;
		virtual void CalcIndirectLighting(TexturePtr const & prev_shading_tex, float4x4 const & proj_to_prev) override;

	private:
		void ExtractVPLs(Camera const & rsm_camera, LightSource const & light);
		void VPLsLighting(LightSource const & light);

	private:
		std::unique_ptr<MultiResLayer> multi_res_layer_;

		TexturePtr g_buffer_rt0_tex_;
		TexturePtr g_buffer_depth_tex_;
		Camera const * g_buffer_camera_;

		std::array<TexturePtr, 2> rsm_texs_;
		TexturePtr rsm_depth_tex_;

		std::array<PostProcessPtr, LightSource::LT_NumLightTypes> rsm_to_vpls_pps_;
		TexturePtr vpl_tex_;
		RenderTargetViewPtr vpl_rtv_;

		RenderEffectPtr vpls_lighting_effect_;
		RenderTechnique* vpls_lighting_instance_id_tech_;

		RenderEffectParameter* vpl_view_param_;
		RenderEffectParameter* vpl_proj_param_;
		RenderEffectParameter* vpl_light_pos_es_param_;
		RenderEffectParameter* vpl_light_color_param_;
		RenderEffectParameter* vpl_light_falloff_param_;
		RenderEffectParameter* vpl_x_coord_param_;
		RenderEffectParameter* vpl_gbuffer_tex_param_;
		RenderEffectParameter* vpl_depth_tex_param_;

		RenderablePtr vpl_renderable_;

		TexturePtr rsm_depth_derivative_tex_;
		PostProcessPtr rsm_to_depth_derivate_pp_;
	};

	class KLAYGE_CORE_API SSGILayer final : public IndirectLightingLayer
	{
	public:
		SSGILayer();

		virtual void GBuffer(TexturePtr const & rt0_tex, TexturePtr const & rt1_tex, TexturePtr const & depth_tex) override;
		virtual void RSM(TexturePtr const & rt0_tex, TexturePtr const & rt1_tex, TexturePtr const & depth_tex) override;

		virtual void UpdateGBuffer(Camera const & vp_camera) override;
		virtual void UpdateRSM(Camera const & rsm_camera, LightSource const & light) override;
		virtual void CalcIndirectLighting(TexturePtr const & prev_shading_tex, float4x4 const & proj_to_prev) override;

	private:
		std::unique_ptr<MultiResLayer> multi_res_layer_;

		TexturePtr g_buffer_rt0_tex_;
		ShaderResourceViewPtr g_buffer_rt0_srv_;
		TexturePtr g_buffer_depth_tex_;
		ShaderResourceViewPtr g_buffer_depth_srv_;

		PostProcessPtr ssgi_pp_;
		PostProcessPtr ssgi_blur_pp_;

		TexturePtr small_ssgi_tex_;
		ShaderResourceViewPtr small_ssgi_srv_;

		RenderTargetViewPtr indirect_lighting_rtv_;
	};
}

#endif		// _INDIRECTLIGHTINGLAYER_HPP
