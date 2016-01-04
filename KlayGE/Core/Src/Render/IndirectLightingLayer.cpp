/**
 * @file IndirectLightingLayer.cpp
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
#include <KlayGE/MultiResLayer.hpp>
#include <KlayGE/SSGIPostProcess.hpp>

#include <KlayGE/IndirectLightingLayer.hpp>

namespace KlayGE
{
	int const MIN_RSM_MIPMAP_SIZE = 8; // minimum mipmap size is 8x8
	int const MAX_RSM_MIPMAP_LEVELS = 7; // (log(512)-log(4))/log(2) + 1
	int const BEGIN_RSM_SAMPLING_LIGHT_LEVEL = 5;
	int const SAMPLE_LEVEL_CNT = MAX_RSM_MIPMAP_LEVELS - BEGIN_RSM_SAMPLING_LIGHT_LEVEL;
	int const VPL_COUNT = 64 * ((1UL << (SAMPLE_LEVEL_CNT * 2)) - 1) / (4 - 1);

	MultiResSILLayer::MultiResSILLayer()
	{
		multi_res_layer_ = MakeSharedPtr<MultiResLayer>();

		RenderFactory& rf = Context::Instance().RenderFactoryInstance();
		RenderEngine& re = rf.RenderEngineInstance();
		RenderDeviceCaps const & caps = re.DeviceCaps();

		vpl_tex_ = rf.MakeTexture2D(VPL_COUNT, 4, 1, 1, EF_ABGR16F, 1, 0, EAH_GPU_Read | EAH_GPU_Write, nullptr);	

		RenderEffectPtr vpls_lighting_effect = SyncLoadRenderEffect("VPLsLighting.fxml");
		vpls_lighting_instance_id_tech_ = vpls_lighting_effect->TechniqueByName("VPLsLightingInstanceID");
		vpls_lighting_no_instance_id_tech_ = vpls_lighting_effect->TechniqueByName("VPLsLightingNoInstanceID");

		vpl_view_param_ = vpls_lighting_effect->ParameterByName("view");
		vpl_proj_param_ = vpls_lighting_effect->ParameterByName("proj");
		vpl_depth_near_far_invfar_param_ = vpls_lighting_effect->ParameterByName("depth_near_far_invfar");
		vpl_light_pos_es_param_ = vpls_lighting_effect->ParameterByName("light_pos_es");
		vpl_light_color_param_ = vpls_lighting_effect->ParameterByName("light_color");
		vpl_light_falloff_param_ = vpls_lighting_effect->ParameterByName("light_falloff");
		vpl_x_coord_param_ = vpls_lighting_effect->ParameterByName("x_coord");
		vpl_gbuffer_tex_param_ = vpls_lighting_effect->ParameterByName("gbuffer_tex");
		vpl_depth_tex_param_ = vpls_lighting_effect->ParameterByName("depth_tex");
		*(vpls_lighting_effect->ParameterByName("vpls_tex")) = vpl_tex_;
		*(vpls_lighting_effect->ParameterByName("vpl_params")) = float2(1.0f / VPL_COUNT, 0.5f / VPL_COUNT);

		vpl_renderable_ = SyncLoadModel("indirect_light_proxy.meshml", EAH_GPU_Read | EAH_Immutable,
			CreateModelFactory<RenderModel>(), CreateMeshFactory<StaticMesh>())->Subrenderable(0);
		if (caps.instance_id_support)
		{
			vpl_renderable_->GetRenderLayout().NumInstances(VPL_COUNT);
		}
	}

	void MultiResSILLayer::GBuffer(TexturePtr const & rt0_tex, TexturePtr const & rt1_tex, TexturePtr const & depth_tex)
	{
		RenderFactory& rf = Context::Instance().RenderFactoryInstance();
		RenderEngine& re = rf.RenderEngineInstance();
		RenderDeviceCaps const & caps = re.DeviceCaps();

		g_buffer_rt0_tex_ = rt0_tex;
		g_buffer_depth_tex_ = depth_tex;

		uint32_t const width = rt0_tex->Width(0);
		uint32_t const height = rt0_tex->Height(0);

		ElementFormat fmt;
		if (caps.rendertarget_format_support(EF_B10G11R11F, 1, 0))
		{
			fmt = EF_B10G11R11F;
		}
		else
		{
			BOOST_ASSERT(caps.rendertarget_format_support(EF_ABGR16F, 1, 0));

			fmt = EF_ABGR16F;
		}

		indirect_lighting_tex_ = rf.MakeTexture2D(width / 2, height / 2, std::max(1U, g_buffer_rt0_tex_->NumMipMaps() - 1),
			1, fmt, 1, 0,  EAH_GPU_Read | EAH_GPU_Write, nullptr);

		multi_res_layer_->BindBuffers(rt0_tex, rt1_tex, depth_tex, indirect_lighting_tex_);
	}

	void MultiResSILLayer::RSM(TexturePtr const & rt0_tex, TexturePtr const & rt1_tex, TexturePtr const & depth_tex)
	{
		std::string RSM2VPLsSpotName = "RSM2VPLsSpot";

		rsm_texs_[0] = rt0_tex;
		rsm_texs_[1] = rt1_tex;
		rsm_depth_tex_ = depth_tex;

		rsm_to_vpls_pps_[LightSource::LT_Spot] = SyncLoadPostProcess("RSM2VPLs.ppml", RSM2VPLsSpotName);
		rsm_to_vpls_pps_[LightSource::LT_Spot]->InputPin(0, rsm_texs_[0]);
		rsm_to_vpls_pps_[LightSource::LT_Spot]->InputPin(1, rsm_texs_[1]);
		rsm_to_vpls_pps_[LightSource::LT_Spot]->InputPin(2, rsm_depth_tex_);
		rsm_to_vpls_pps_[LightSource::LT_Spot]->InputPin(3, rsm_depth_derivative_tex_);
		rsm_to_vpls_pps_[LightSource::LT_Spot]->OutputPin(0, vpl_tex_);

		RenderFactory& rf = Context::Instance().RenderFactoryInstance();
		RenderDeviceCaps const & caps = rf.RenderEngineInstance().DeviceCaps();

		ElementFormat fmt;
		if (caps.pack_to_rgba_required)
		{
			if (caps.rendertarget_format_support(EF_ABGR8, 1, 0))
			{
				fmt = EF_ABGR8;
			}
			else
			{
				BOOST_ASSERT(caps.rendertarget_format_support(EF_ARGB8, 1, 0));
				fmt = EF_ARGB8;
			}
		}
		else
		{
			if (caps.rendertarget_format_support(EF_R32F, 1, 0))
			{
				fmt = EF_R32F;
			}
			else
			{
				BOOST_ASSERT(caps.rendertarget_format_support(EF_R16F, 1, 0));
				fmt = EF_R16F;
			}
		}

		rsm_depth_derivative_tex_ = rf.MakeTexture2D(MIN_RSM_MIPMAP_SIZE, MIN_RSM_MIPMAP_SIZE, 1, 1, fmt, 1, 0, EAH_GPU_Read | EAH_GPU_Write, nullptr);

		rsm_to_depth_derivate_pp_ = SyncLoadPostProcess("MultiRes.ppml", "GBuffer2DepthDerivate");
		rsm_to_depth_derivate_pp_->InputPin(1, rsm_depth_tex_);
		rsm_to_depth_derivate_pp_->OutputPin(0, rsm_depth_derivative_tex_);
		float delta_x = 1.0f / rsm_depth_derivative_tex_->Width(0);
		float delta_y = 1.0f / rsm_depth_derivative_tex_->Height(0);
		float4 rsm_delta_offset(delta_x, delta_y, delta_x / 2, delta_y / 2);
		rsm_to_depth_derivate_pp_->SetParam(0, rsm_delta_offset);
	}

	void MultiResSILLayer::UpdateGBuffer(Camera const & vp_camera)
	{
		g_buffer_camera_ = &vp_camera;
		multi_res_layer_->UpdateGBuffer(vp_camera);
	}

	void MultiResSILLayer::UpdateRSM(Camera const & rsm_camera, LightSource const & light)
	{
		rsm_to_depth_derivate_pp_->SetParam(1, float2(rsm_camera.FarPlane(), 1 / rsm_camera.FarPlane()));
		this->ExtractVPLs(rsm_camera, light);
		this->VPLsLighting(light);
	}

	void MultiResSILLayer::CalcIndirectLighting(TexturePtr const & prev_shading_tex, float4x4 const & proj_to_prev)
	{
		KFL_UNUSED(prev_shading_tex);
		KFL_UNUSED(proj_to_prev);

		if (g_buffer_rt0_tex_->NumMipMaps() > 1)
		{
			multi_res_layer_->UpsampleMultiRes();
		}
	}

	void MultiResSILLayer::ExtractVPLs(Camera const & rsm_camera, LightSource const & light)
	{
		rsm_texs_[0]->BuildMipSubLevels();
		rsm_texs_[1]->BuildMipSubLevels();
		
		rsm_to_depth_derivate_pp_->Apply();
		
		float4x4 ls_to_es = rsm_camera.InverseViewMatrix() * g_buffer_camera_->ViewMatrix();
		float4x4 const & inv_proj = rsm_camera.InverseProjMatrix();
		LightSource::LightType type = light.Type();

		float4 vpl_params(static_cast<float>(VPL_COUNT), 2.0f, 
			              static_cast<float>(MIN_RSM_MIPMAP_SIZE), static_cast<float>(MIN_RSM_MIPMAP_SIZE * MIN_RSM_MIPMAP_SIZE));

		rsm_to_vpls_pps_[type]->SetParam(0, ls_to_es);
		rsm_to_vpls_pps_[type]->SetParam(1, vpl_params);
		rsm_to_vpls_pps_[type]->SetParam(2, light.Color());
		rsm_to_vpls_pps_[type]->SetParam(3, light.CosOuterInner());
		rsm_to_vpls_pps_[type]->SetParam(4, light.Falloff());
		rsm_to_vpls_pps_[type]->SetParam(5, g_buffer_camera_->InverseViewMatrix());
		float3 upper_left = MathLib::transform_coord(float3(-1, +1, 1), inv_proj);
		float3 upper_right = MathLib::transform_coord(float3(+1, +1, 1), inv_proj);
		float3 lower_left = MathLib::transform_coord(float3(-1, -1, 1), inv_proj);
		rsm_to_vpls_pps_[type]->SetParam(6, upper_left);
		rsm_to_vpls_pps_[type]->SetParam(7, upper_right - upper_left);
		rsm_to_vpls_pps_[type]->SetParam(8, lower_left - upper_left);
		rsm_to_vpls_pps_[type]->SetParam(9, int2(1, 0));
		rsm_to_vpls_pps_[type]->SetParam(10, 0.12f * rsm_camera.FarPlane());
		rsm_to_vpls_pps_[type]->SetParam(11, static_cast<float>(rsm_texs_[0]->NumMipMaps() - 1));
		rsm_to_vpls_pps_[type]->SetParam(12, float2(rsm_camera.FarPlane(), 1 / rsm_camera.FarPlane()));

		rsm_to_vpls_pps_[type]->Apply();
	}

	void MultiResSILLayer::VPLsLighting(LightSource const & light)
	{
		RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
		RenderDeviceCaps const & caps = re.DeviceCaps();

		*vpl_view_param_ = g_buffer_camera_->ViewMatrix();
		*vpl_proj_param_ = g_buffer_camera_->ProjMatrix();
		*vpl_depth_near_far_invfar_param_ = float3(g_buffer_camera_->NearPlane(),
			g_buffer_camera_->FarPlane(), 1 / g_buffer_camera_->FarPlane());

		float3 p = MathLib::transform_coord(light.Position(), g_buffer_camera_->ViewMatrix());
		*vpl_light_pos_es_param_ = float4(p.x(), p.y(), p.z(), 1);
		*vpl_light_color_param_ = light.Color();
		*vpl_light_falloff_param_ = light.Falloff();

		*vpl_gbuffer_tex_param_ = g_buffer_rt0_tex_;
		*vpl_depth_tex_param_ = g_buffer_depth_tex_;
		
		RenderLayout const & rl_vpl = vpl_renderable_->GetRenderLayout();
		for (uint32_t i = 0; i < indirect_lighting_tex_->NumMipMaps(); ++ i)
		{
			re.BindFrameBuffer(multi_res_layer_->MultiResFB(i));

			if (caps.instance_id_support)
			{
				re.Render(*vpls_lighting_instance_id_tech_, rl_vpl);
			}
			else
			{
				for (int j = 0; j < VPL_COUNT; ++ j)
				{
					*vpl_x_coord_param_ = (j + 0.5f) / VPL_COUNT;
					re.Render(*vpls_lighting_no_instance_id_tech_, rl_vpl);
				}
			}
		}
	}


	SSGILayer::SSGILayer()
	{
		multi_res_layer_ = MakeSharedPtr<MultiResLayer>();

		ssgi_pp_ = MakeSharedPtr<SSGIPostProcess>();
		ssgi_blur_pp_ = MakeSharedPtr<BlurPostProcess<SeparableBilateralFilterPostProcess>>(4, 1.0f,
			SyncLoadRenderEffect("SSGI.fxml")->TechniqueByName("SSGIBlurX"),
			SyncLoadRenderEffect("SSGI.fxml")->TechniqueByName("SSGIBlurY"));
	}

	void SSGILayer::GBuffer(TexturePtr const & rt0_tex, TexturePtr const & rt1_tex, TexturePtr const & depth_tex)
	{
		KFL_UNUSED(rt1_tex);

		BOOST_ASSERT(rt0_tex->NumMipMaps() >= 3);

		RenderFactory& rf = Context::Instance().RenderFactoryInstance();
		RenderEngine& re = rf.RenderEngineInstance();
		RenderDeviceCaps const & caps = re.DeviceCaps();

		g_buffer_rt0_tex_ = rt0_tex;
		g_buffer_depth_tex_ = depth_tex;

		uint32_t const width = rt0_tex->Width(0);
		uint32_t const height = rt0_tex->Height(0);

		ElementFormat fmt;
		if (caps.rendertarget_format_support(EF_B10G11R11F, 1, 0))
		{
			fmt = EF_B10G11R11F;
		}
		else
		{
			BOOST_ASSERT(caps.rendertarget_format_support(EF_ABGR16F, 1, 0));

			fmt = EF_ABGR16F;
		}
		small_ssgi_tex_ = rf.MakeTexture2D(width / 4, height / 4, g_buffer_rt0_tex_->NumMipMaps() - 2, 1, fmt, 1, 0, EAH_GPU_Read | EAH_GPU_Write, nullptr);
		indirect_lighting_tex_ = rf.MakeTexture2D(width / 2, height / 2, 1, 1, fmt, 1, 0,  EAH_GPU_Read | EAH_GPU_Write, nullptr);

		multi_res_layer_->BindBuffers(rt0_tex, rt1_tex, depth_tex, small_ssgi_tex_);
	}

	void SSGILayer::RSM(TexturePtr const & rt0_tex, TexturePtr const & rt1_tex, TexturePtr const & depth_tex)
	{
		KFL_UNUSED(rt0_tex);
		KFL_UNUSED(rt1_tex);
		KFL_UNUSED(depth_tex);
	}

	void SSGILayer::UpdateGBuffer(Camera const & vp_camera)
	{
		multi_res_layer_->UpdateGBuffer(vp_camera);
	}

	void SSGILayer::UpdateRSM(Camera const & rsm_camera, LightSource const & light)
	{
		KFL_UNUSED(rsm_camera);
		KFL_UNUSED(light);
	}

	void SSGILayer::CalcIndirectLighting(TexturePtr const & prev_shading_tex, float4x4 const & proj_to_prev)
	{
		KFL_UNUSED(proj_to_prev);

		RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();

		ssgi_pp_->InputPin(0, g_buffer_rt0_tex_);
		ssgi_pp_->InputPin(1, g_buffer_depth_tex_);
		ssgi_pp_->InputPin(2, prev_shading_tex);
		ssgi_pp_->OutputPin(0, TexturePtr());
		for (uint32_t i = 0; i < small_ssgi_tex_->NumMipMaps(); ++ i)
		{
			re.BindFrameBuffer(multi_res_layer_->MultiResFB(i));
			ssgi_pp_->Render();
		}

		multi_res_layer_->UpsampleMultiRes();

		ssgi_blur_pp_->InputPin(0, small_ssgi_tex_);
		ssgi_blur_pp_->InputPin(1, g_buffer_depth_tex_);
		ssgi_blur_pp_->OutputPin(0, indirect_lighting_tex_);
		ssgi_blur_pp_->Apply();
	}
}
