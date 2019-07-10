/**
 * @file MultiResLayer.cpp
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
#include <KlayGE/ResLoader.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/RenderEffect.hpp>
#include <KlayGE/FrameBuffer.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/PostProcess.hpp>
#include <KlayGE/Mesh.hpp>
#include <KlayGE/Camera.hpp>
#include <KlayGE/Light.hpp>
#include <KlayGE/SSGIPostProcess.hpp>

#include <KlayGE/MultiResLayer.hpp>

namespace KlayGE
{
	MultiResLayer::MultiResLayer()
	{
		RenderFactory& rf = Context::Instance().RenderFactoryInstance();

		{
			rl_quad_ = rf.MakeRenderLayout();
			rl_quad_->TopologyType(RenderLayout::TT_TriangleStrip);

			float3 pos[] = 
			{
				float3(+1, +1, 1),
				float3(-1, +1, 1),
				float3(+1, -1, 1),
				float3(-1, -1, 1)
			};

			rl_quad_->BindVertexStream(rf.MakeVertexBuffer(BU_Static, EAH_GPU_Read | EAH_Immutable,
				static_cast<uint32_t>(sizeof(pos)), &pos[0]), VertexElement(VEU_Position, 0, EF_BGR32F));
		}

		gbuffer_to_depth_derivate_pp_ = SyncLoadPostProcess("MultiRes.ppml", "GBuffer2DepthDerivate");
		depth_derivate_mipmap_pp_ =  SyncLoadPostProcess("MultiRes.ppml", "DepthDerivateMipMap");
		gbuffer_to_normal_cone_pp_ =  SyncLoadPostProcess("MultiRes.ppml", "GBuffer2NormalCone");
		normal_cone_mipmap_pp_ =  SyncLoadPostProcess("MultiRes.ppml", "NormalConeMipMap");

		subsplat_stencil_effect_ = SyncLoadRenderEffect("MultiRes.fxml");
		subsplat_stencil_tech_ = subsplat_stencil_effect_->TechniqueByName("SetSubsplatStencil");

		subsplat_cur_lower_level_param_ = subsplat_stencil_effect_->ParameterByName("cur_lower_level");
		subsplat_is_not_first_last_level_param_ = subsplat_stencil_effect_->ParameterByName("is_not_first_last_level");
		subsplat_depth_deriv_tex_param_ = subsplat_stencil_effect_->ParameterByName("depth_deriv_tex");
		subsplat_normal_cone_tex_param_ = subsplat_stencil_effect_->ParameterByName("normal_cone_tex");
		subsplat_depth_normal_threshold_param_ = subsplat_stencil_effect_->ParameterByName("depth_normal_threshold");

		upsampling_pp_ = SyncLoadPostProcess("MultiRes.ppml", "Upsampling");
	}

	void MultiResLayer::BindBuffers(TexturePtr const & rt0_tex, TexturePtr const & rt1_tex, TexturePtr const & depth_tex,
			TexturePtr const & multi_res_tex)
	{
		KFL_UNUSED(rt1_tex);

		RenderFactory& rf = Context::Instance().RenderFactoryInstance();
		RenderEngine& re = rf.RenderEngineInstance();
		RenderDeviceCaps const & caps = re.DeviceCaps();

		g_buffer_rt0_tex_ = rt0_tex;
		g_buffer_rt0_srv_ = rf.MakeTextureSrv(rt0_tex);
		g_buffer_depth_srv_ = rf.MakeTextureSrv(depth_tex);

		multi_res_tex_ = multi_res_tex;
		multi_res_srv_ = rf.MakeTextureSrv(multi_res_tex);
		if (multi_res_tex->NumMipMaps() > 1)
		{
			auto const fmt8 = caps.BestMatchTextureRenderTargetFormat(MakeSpan({EF_ABGR8, EF_ARGB8}), 1, 0);
			BOOST_ASSERT(fmt8 != EF_Unknown);

			auto const depth_fmt = caps.BestMatchTextureRenderTargetFormat(
				caps.pack_to_rgba_required ? MakeSpan({EF_ABGR8, EF_ARGB8}) : MakeSpan({EF_R16F, EF_R32F}), 1, 0);
			BOOST_ASSERT(depth_fmt != EF_Unknown);

			depth_derivative_tex_ = rf.MakeTexture2D(multi_res_tex->Width(0), multi_res_tex->Height(0),
				multi_res_tex->NumMipMaps(), 1, depth_fmt, 1, 0, EAH_GPU_Read | EAH_GPU_Write);
			normal_cone_tex_ = rf.MakeTexture2D(multi_res_tex->Width(0), multi_res_tex->Height(0),
				multi_res_tex->NumMipMaps(), 1, fmt8, 1, 0, EAH_GPU_Read | EAH_GPU_Write);

			if (caps.flexible_srvs_support)
			{
				depth_derivative_mip_srvs_.resize(depth_derivative_tex_->NumMipMaps());
				depth_derivative_mip_rtvs_.resize(depth_derivative_tex_->NumMipMaps());
				for (uint32_t i = 0; i < depth_derivative_tex_->NumMipMaps(); ++i)
				{
					depth_derivative_mip_srvs_[i] = rf.MakeTextureSrv(depth_derivative_tex_, 0, 1, i, 1);
					depth_derivative_mip_rtvs_[i] = rf.Make2DRtv(depth_derivative_tex_, 0, 1, i);
				}

				normal_cone_mip_srvs_.resize(normal_cone_tex_->NumMipMaps());
				normal_cone_mip_rtvs_.resize(normal_cone_tex_->NumMipMaps());
				for (uint32_t i = 0; i < normal_cone_tex_->NumMipMaps(); ++i)
				{
					normal_cone_mip_srvs_[i] = rf.MakeTextureSrv(normal_cone_tex_, 0, 1, i, 1);
					normal_cone_mip_rtvs_[i] = rf.Make2DRtv(normal_cone_tex_, 0, 1, i);
				}
			}
			else
			{
				depth_derivative_small_tex_ = rf.MakeTexture2D(multi_res_tex->Width(1), multi_res_tex->Height(1),
					multi_res_tex->NumMipMaps() - 1, 1, EF_R16F, 1, 0, EAH_GPU_Write);
				depth_derivative_small_mip_rtvs_.resize(depth_derivative_small_tex_->NumMipMaps());
				for (uint32_t i = 0; i < depth_derivative_small_tex_->NumMipMaps(); ++i)
				{
					depth_derivative_small_mip_rtvs_[i] = rf.Make2DRtv(depth_derivative_small_tex_, 0, 1, i);
				}

				normal_cone_small_tex_ = rf.MakeTexture2D(multi_res_tex->Width(1), multi_res_tex->Height(1),
					multi_res_tex->NumMipMaps() - 1, 1, fmt8, 1, 0, EAH_GPU_Write);
				normal_cone_small_mip_rtvs_.resize(normal_cone_small_tex_->NumMipMaps());
				for (uint32_t i = 0; i < normal_cone_small_tex_->NumMipMaps(); ++i)
				{
					normal_cone_small_mip_rtvs_[i] = rf.Make2DRtv(normal_cone_small_tex_, 0, 1, i);
				}

				depth_derivative_srv_ = rf.MakeTextureSrv(depth_derivative_tex_);
				normal_cone_srv_ = rf.MakeTextureSrv(normal_cone_tex_);
			}

			multi_res_pingpong_tex_ = rf.MakeTexture2D(multi_res_tex->Width(0), multi_res_tex->Height(0),
				multi_res_tex->NumMipMaps() - 1, 1, multi_res_tex_->Format(), 1, 0, EAH_GPU_Write);
			multi_res_pingpong_mip_rtvs_.resize(multi_res_pingpong_tex_->NumMipMaps());
			for (uint32_t i = 0; i < multi_res_pingpong_tex_->NumMipMaps(); ++i)
			{
				multi_res_pingpong_mip_rtvs_[i] = rf.Make2DRtv(multi_res_pingpong_tex_, 0, 1, i);
			}
		}
		multi_res_fbs_.resize(multi_res_tex->NumMipMaps());
		for (uint32_t i = 0; i < multi_res_tex->NumMipMaps(); ++ i)
		{
			float4 constexpr subsplat_clear_value(0, 128, 0, 0);
			auto subsplat_ds_tex = rf.MakeTexture2D(multi_res_tex->Width(i), multi_res_tex->Height(i), 1, 1, EF_D24S8,
				1, 0, EAH_GPU_Write, {}, &subsplat_clear_value);
			auto subsplat_ds_view = rf.Make2DDsv(subsplat_ds_tex, 0, 1, 0);

			FrameBufferPtr fb = rf.MakeFrameBuffer();
			fb->Attach(FrameBuffer::Attachment::Color0, rf.Make2DRtv(multi_res_tex, 0, 1, i));
			fb->Attach(subsplat_ds_view);
			multi_res_fbs_[i] = fb;
		}
	}

	void MultiResLayer::UpdateGBuffer(Camera const & vp_camera)
	{
		if (multi_res_tex_->NumMipMaps() > 1)
		{
			this->CreateDepthDerivativeMipMap();
			this->CreateNormalConeMipMap();
			this->SetSubsplatStencil(vp_camera);
		}
		else
		{
			RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
			re.BindFrameBuffer(multi_res_fbs_[0]);
			multi_res_fbs_[0]->Clear(FrameBuffer::CBM_Color | FrameBuffer::CBM_Depth | FrameBuffer::CBM_Stencil, Color(0, 0, 0, 0), 0.0f, 0);
		}
	}

	void MultiResLayer::CreateDepthDerivativeMipMap()
	{
		auto& rf = Context::Instance().RenderFactoryInstance();
		auto const & caps = rf.RenderEngineInstance().DeviceCaps();

		gbuffer_to_depth_derivate_pp_->InputPin(0, g_buffer_rt0_srv_);
		gbuffer_to_depth_derivate_pp_->InputPin(1, g_buffer_depth_srv_);
		gbuffer_to_depth_derivate_pp_->OutputPin(0, depth_derivative_mip_rtvs_[0]);
		float delta_x = 1.0f / g_buffer_rt0_tex_->Width(0);
		float delta_y = 1.0f / g_buffer_rt0_tex_->Height(0);
		float4 delta_offset(delta_x, delta_y, delta_x / 2, delta_y / 2);
		gbuffer_to_depth_derivate_pp_->SetParam(0, delta_offset);
		gbuffer_to_depth_derivate_pp_->Apply();

		if (!caps.flexible_srvs_support)
		{
			depth_derivate_mipmap_pp_->InputPin(0, depth_derivative_srv_);
		}
		for (uint32_t i = 1; i < depth_derivative_tex_->NumMipMaps(); ++ i)
		{
			uint32_t const width = depth_derivative_tex_->Width(i - 1);
			uint32_t const height = depth_derivative_tex_->Height(i - 1);
			uint32_t const lower_width = depth_derivative_tex_->Width(i);
			uint32_t const lower_height = depth_derivative_tex_->Height(i);

			delta_x = 1.0f / width;
			delta_y = 1.0f / height;
			delta_offset = float4(delta_x, delta_y, delta_x / 2, delta_y / 2);
			depth_derivate_mipmap_pp_->SetParam(0, delta_offset);

			if (caps.flexible_srvs_support)
			{
				depth_derivate_mipmap_pp_->InputPin(0, depth_derivative_mip_srvs_[i - 1]);
				depth_derivate_mipmap_pp_->OutputPin(0, depth_derivative_mip_rtvs_[i]);
				depth_derivate_mipmap_pp_->Apply();
			}
			else
			{
				depth_derivate_mipmap_pp_->OutputPin(0, depth_derivative_small_mip_rtvs_[i - 1]);
				depth_derivate_mipmap_pp_->Apply();

				depth_derivative_small_tex_->CopyToSubTexture2D(*depth_derivative_tex_, 0, i, 0, 0, lower_width, lower_height,
					0, i - 1, 0, 0, lower_width, lower_height);
			}
		}
	}

	void MultiResLayer::CreateNormalConeMipMap()
	{
		auto& rf = Context::Instance().RenderFactoryInstance();
		auto const & caps = rf.RenderEngineInstance().DeviceCaps();

		gbuffer_to_normal_cone_pp_->InputPin(0, g_buffer_rt0_srv_);
		gbuffer_to_normal_cone_pp_->OutputPin(0, normal_cone_mip_rtvs_[0]);
		float delta_x = 1.0f / g_buffer_rt0_tex_->Width(0);
		float delta_y = 1.0f / g_buffer_rt0_tex_->Height(0);
		float4 delta_offset(delta_x, delta_y, delta_x / 2, delta_y / 2);
		gbuffer_to_normal_cone_pp_->SetParam(0, delta_offset);
		gbuffer_to_normal_cone_pp_->Apply();

		if (!caps.flexible_srvs_support)
		{
			normal_cone_mipmap_pp_->InputPin(0, normal_cone_srv_);
		}
		for (uint32_t i = 1; i < normal_cone_tex_->NumMipMaps(); ++ i)
		{
			uint32_t const width = normal_cone_tex_->Width(i - 1);
			uint32_t const height = normal_cone_tex_->Height(i - 1);
			uint32_t const lower_width = normal_cone_tex_->Width(i);
			uint32_t const lower_height = normal_cone_tex_->Height(i);

			delta_x = 1.0f / width;
			delta_y = 1.0f / height;
			delta_offset = float4(delta_x, delta_y, delta_x / 2, delta_y / 2);

			normal_cone_mipmap_pp_->SetParam(0, delta_offset);

			if (caps.flexible_srvs_support)
			{
				normal_cone_mipmap_pp_->InputPin(0, normal_cone_mip_srvs_[i - 1]);
				normal_cone_mipmap_pp_->OutputPin(0, normal_cone_mip_rtvs_[i]);
				normal_cone_mipmap_pp_->Apply();
			}
			else
			{
				normal_cone_mipmap_pp_->OutputPin(0, normal_cone_small_mip_rtvs_[i - 1]);
				normal_cone_mipmap_pp_->Apply();

				normal_cone_small_tex_->CopyToSubTexture2D(*normal_cone_tex_, 0, i, 0, 0, lower_width, lower_height,
					0, i - 1, 0, 0, lower_width, lower_height);
			}
		}
	}

	void MultiResLayer::SetSubsplatStencil(Camera const & vp_camera)
	{
		*subsplat_depth_deriv_tex_param_ = depth_derivative_tex_;
		*subsplat_normal_cone_tex_param_ = normal_cone_tex_;
		*subsplat_depth_normal_threshold_param_ = float2(0.001f * vp_camera.FarPlane(), 0.77f);

		RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
		for (size_t i = 0; i < multi_res_fbs_.size(); ++ i)
		{
			re.BindFrameBuffer(multi_res_fbs_[i]);
			multi_res_fbs_[i]->Clear(FrameBuffer::CBM_Color | FrameBuffer::CBM_Depth | FrameBuffer::CBM_Stencil, Color(0, 0, 0, 0), 0.0f, 128);

			*subsplat_cur_lower_level_param_ = int2(static_cast<int>(i), static_cast<int>(i + 1));
			*subsplat_is_not_first_last_level_param_ = int2(i > 0, i < multi_res_fbs_.size() - 1);

			re.Render(*subsplat_stencil_effect_, *subsplat_stencil_tech_, *rl_quad_);
		}
	}

	void MultiResLayer::UpsampleMultiRes()
	{
		upsampling_pp_->InputPin(0, multi_res_srv_);
		for (int i = multi_res_tex_->NumMipMaps() - 2; i >= 0; -- i)
		{
			uint32_t const width = multi_res_tex_->Width(i);
			uint32_t const height = multi_res_tex_->Height(i);
			uint32_t const lower_width = multi_res_tex_->Width(i + 1);
			uint32_t const lower_height = multi_res_tex_->Height(i + 1);

			upsampling_pp_->SetParam(0, float4(static_cast<float>(lower_width), static_cast<float>(lower_height),
				1.0f / lower_width, 1.0f / lower_height));
			upsampling_pp_->SetParam(1, int2(i + 1, i));
			
			upsampling_pp_->OutputPin(0, multi_res_pingpong_mip_rtvs_[i]);
			upsampling_pp_->Apply();

			multi_res_pingpong_tex_->CopyToSubTexture2D(*multi_res_tex_, 0, i, 0, 0, width, height,
				0, i, 0, 0, width, height);
		}
	}
}
