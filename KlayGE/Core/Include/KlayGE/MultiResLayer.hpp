/**
 * @file MultiResLayer.hpp
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

#ifndef _KLAYGE_MULTIRESLAYER_HPP
#define _KLAYGE_MULTIRESLAYER_HPP

#pragma once

#include <KlayGE/PreDeclare.hpp>

namespace KlayGE
{
	class KLAYGE_CORE_API MultiResLayer final : boost::noncopyable
	{
	public:
		MultiResLayer();

		void BindBuffers(TexturePtr const & rt0_tex, TexturePtr const & rt1_tex, TexturePtr const & depth_tex,
			TexturePtr const & multi_res_tex);

		void UpdateGBuffer(Camera const & vp_camera);
		void UpsampleMultiRes();

		FrameBufferPtr const & MultiResFB(uint32_t index) const
		{
			return multi_res_fbs_[index];
		}

	private:
		void CreateDepthDerivativeMipMap();
		void CreateNormalConeMipMap();
		void SetSubsplatStencil(Camera const & vp_camera);

	private:
		RenderLayoutPtr rl_quad_;

		TexturePtr g_buffer_rt0_tex_;
		ShaderResourceViewPtr g_buffer_rt0_srv_;
		ShaderResourceViewPtr g_buffer_depth_srv_;

		TexturePtr depth_derivative_tex_;
		ShaderResourceViewPtr depth_derivative_srv_;
		std::vector<ShaderResourceViewPtr> depth_derivative_mip_srvs_;
		std::vector<RenderTargetViewPtr> depth_derivative_mip_rtvs_;
		TexturePtr depth_derivative_small_tex_;
		std::vector<RenderTargetViewPtr> depth_derivative_small_mip_rtvs_;
		TexturePtr normal_cone_tex_;
		ShaderResourceViewPtr normal_cone_srv_;
		std::vector<ShaderResourceViewPtr> normal_cone_mip_srvs_;
		std::vector<RenderTargetViewPtr> normal_cone_mip_rtvs_;
		TexturePtr normal_cone_small_tex_;
		std::vector<RenderTargetViewPtr> normal_cone_small_mip_rtvs_;

		TexturePtr multi_res_tex_;
		ShaderResourceViewPtr multi_res_srv_;
		TexturePtr multi_res_pingpong_tex_;
		std::vector<RenderTargetViewPtr> multi_res_pingpong_mip_rtvs_;
		std::vector<FrameBufferPtr> multi_res_fbs_;

		RenderEffectPtr subsplat_stencil_effect_;
		RenderTechnique* subsplat_stencil_tech_;

		PostProcessPtr gbuffer_to_depth_derivate_pp_;
		PostProcessPtr depth_derivate_mipmap_pp_;
		PostProcessPtr gbuffer_to_normal_cone_pp_;
		PostProcessPtr normal_cone_mipmap_pp_;

		PostProcessPtr upsampling_pp_;

		RenderEffectParameter* subsplat_cur_lower_level_param_;
		RenderEffectParameter* subsplat_is_not_first_last_level_param_;
		RenderEffectParameter* subsplat_depth_deriv_tex_param_;
		RenderEffectParameter* subsplat_normal_cone_tex_param_;
		RenderEffectParameter* subsplat_depth_normal_threshold_param_;
	};
}

#endif		// _KLAYGE_MULTIRESLAYER_HPP
