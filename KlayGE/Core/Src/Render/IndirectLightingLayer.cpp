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
#include <KlayGE/SSGIPostProcess.hpp>

#include <KlayGE/IndirectLightingLayer.hpp>

#define USE_NEW_LIGHT_SAMPLING

namespace KlayGE
{
	int const VPL_COUNT_SQRT = 16;
	
	float const VPL_DELTA = 1.0f / VPL_COUNT_SQRT;
	float const VPL_OFFSET = 0.5f * VPL_DELTA;

	int const MAX_IL_MIPMAP_LEVELS = 3;

#ifdef USE_NEW_LIGHT_SAMPLING
	int const MIN_RSM_MIPMAP_SIZE = 8; // minimum mipmap size is 8x8
	int const MAX_RSM_MIPMAP_LEVELS = 7; // (log(512)-log(4))/log(2) + 1
	int const BEGIN_RSM_SAMPLING_LIGHT_LEVEL = 5;
	int const SAMPLE_LEVEL_CNT = MAX_RSM_MIPMAP_LEVELS - BEGIN_RSM_SAMPLING_LIGHT_LEVEL;
	int const VPL_COUNT = 64 * ((1UL << (SAMPLE_LEVEL_CNT * 2)) - 1) / (4 - 1);
#else
	int const MAX_RSM_MIPMAP_LEVELS = 6;
	int const VPL_COUNT = VPL_COUNT_SQRT * VPL_COUNT_SQRT;
#endif

	MultiResSILLayer::MultiResSILLayer()
	{
		RenderFactory& rf = Context::Instance().RenderFactoryInstance();
		RenderEngine& re = rf.RenderEngineInstance();
		RenderDeviceCaps const & caps = re.DeviceCaps();

		{
			rl_quad_ = rf.MakeRenderLayout();
			rl_quad_->TopologyType(RenderLayout::TT_TriangleStrip);

			std::vector<float3> pos;
			std::vector<uint16_t> index;

			pos.push_back(float3(+1, +1, 1));
			pos.push_back(float3(-1, +1, 1));
			pos.push_back(float3(+1, -1, 1));
			pos.push_back(float3(-1, -1, 1));

			ElementInitData init_data;
			init_data.row_pitch = static_cast<uint32_t>(pos.size() * sizeof(pos[0]));
			init_data.slice_pitch = 0;
			init_data.data = &pos[0];
			rl_quad_->BindVertexStream(rf.MakeVertexBuffer(BU_Static, EAH_GPU_Read | EAH_Immutable, &init_data),
				make_tuple(vertex_element(VEU_Position, 0, EF_BGR32F)));
		}

		vpl_tex_ = rf.MakeTexture2D(VPL_COUNT, 4, 1, 1, EF_ABGR16F, 1, 0, EAH_GPU_Read | EAH_GPU_Write, nullptr);	

		gbuffer_to_depth_derivate_pp_ = LoadPostProcess(ResLoader::Instance().Open("CustomMipMap.ppml"), "GBuffer2DepthDerivate");
		depth_derivate_mipmap_pp_ =  LoadPostProcess(ResLoader::Instance().Open("CustomMipMap.ppml"), "DepthDerivateMipMap");
		gbuffer_to_normal_cone_pp_ =  LoadPostProcess(ResLoader::Instance().Open("CustomMipMap.ppml"), "GBuffer2NormalCone");
		normal_cone_mipmap_pp_ =  LoadPostProcess(ResLoader::Instance().Open("CustomMipMap.ppml"), "NormalConeMipMap");

		RenderEffectPtr subsplat_stencil_effect = rf.LoadEffect("SetSubsplatStencil.fxml");
		subsplat_stencil_tech_ = subsplat_stencil_effect->TechniqueByName("SetSubsplatStencil");

		subsplat_cur_lower_level_param_ = subsplat_stencil_effect->ParameterByName("cur_lower_level");
		subsplat_is_not_first_last_level_param_ = subsplat_stencil_effect->ParameterByName("is_not_first_last_level");
		subsplat_depth_deriv_tex_param_ = subsplat_stencil_effect->ParameterByName("depth_deriv_tex");
		subsplat_normal_cone_tex_param_ = subsplat_stencil_effect->ParameterByName("normal_cone_tex");
		subsplat_depth_normal_threshold_param_ = subsplat_stencil_effect->ParameterByName("depth_normal_threshold");

		RenderEffectPtr vpls_lighting_effect = rf.LoadEffect("VPLsLighting.fxml");
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

		upsampling_pp_ = LoadPostProcess(ResLoader::Instance().Open("Upsampling.ppml"), "Upsampling");

		rl_vpl_ = SyncLoadModel("indirect_light_proxy.meshml", EAH_GPU_Read | EAH_Immutable, CreateModelFactory<RenderModel>(), CreateMeshFactory<StaticMesh>())->Mesh(0)->GetRenderLayout();
		if (caps.instance_id_support)
		{
			rl_vpl_->NumInstances(VPL_COUNT);
		}
	}

	void MultiResSILLayer::GBuffer(TexturePtr const & rt0_tex, TexturePtr const & rt1_tex, TexturePtr const & depth_tex)
	{
		RenderFactory& rf = Context::Instance().RenderFactoryInstance();
		RenderEngine& re = rf.RenderEngineInstance();
		RenderDeviceCaps const & caps = re.DeviceCaps();

		g_buffer_texs_[0] = rt0_tex;
		g_buffer_texs_[1] = rt1_tex;
		g_buffer_depth_tex_ = depth_tex;

		ElementFormat fmt8;
		if (caps.rendertarget_format_support(EF_ABGR8, 1, 0))
		{
			fmt8 = EF_ABGR8;
		}
		else
		{
			BOOST_ASSERT(caps.rendertarget_format_support(EF_ARGB8, 1, 0));

			fmt8 = EF_ARGB8;
		}

		ElementFormat depth_fmt;
		if (caps.rendertarget_format_support(EF_R16F, 1, 0))
		{
			depth_fmt = EF_R16F;
		}
		else
		{
			if (caps.rendertarget_format_support(EF_R32F, 1, 0))
			{
				depth_fmt = EF_R32F;
			}
			else
			{
				BOOST_ASSERT(caps.rendertarget_format_support(EF_ABGR16F, 1, 0));

				depth_fmt = EF_ABGR16F;
			}
		}

		uint32_t const width = rt0_tex->Width(0);
		uint32_t const height = rt0_tex->Height(0);

		depth_deriative_tex_ = rf.MakeTexture2D(width / 2, height / 2, MAX_IL_MIPMAP_LEVELS, 1, depth_fmt, 1, 0, EAH_GPU_Read | EAH_GPU_Write, nullptr);
		normal_cone_tex_ = rf.MakeTexture2D(width / 2, height / 2, MAX_IL_MIPMAP_LEVELS, 1, fmt8, 1, 0, EAH_GPU_Read | EAH_GPU_Write, nullptr);
		if (depth_deriative_tex_->NumMipMaps() > 1)
		{
			depth_deriative_small_tex_ = rf.MakeTexture2D(width / 4, height / 4, MAX_IL_MIPMAP_LEVELS - 1, 1, EF_R16F, 1, 0, EAH_GPU_Write, nullptr);
			normal_cone_small_tex_ = rf.MakeTexture2D(width / 4, height / 4, MAX_IL_MIPMAP_LEVELS - 1, 1, fmt8, 1, 0, EAH_GPU_Write, nullptr);
		}
		indirect_lighting_tex_ = rf.MakeTexture2D(width / 2, height / 2, MAX_IL_MIPMAP_LEVELS, 1, EF_ABGR16F, 1, 0,  EAH_GPU_Read | EAH_GPU_Write, nullptr);
		indirect_lighting_pingpong_tex_ = rf.MakeTexture2D(width / 2, height / 2, MAX_IL_MIPMAP_LEVELS - 1, 1, EF_ABGR16F, 1, 0, EAH_GPU_Write, nullptr);
		vpls_lighting_fbs_.resize(MAX_IL_MIPMAP_LEVELS);
		for (uint32_t i = 0; i < indirect_lighting_tex_->NumMipMaps(); ++ i)
		{
			RenderViewPtr subsplat_ds_view = rf.Make2DDepthStencilRenderView(indirect_lighting_tex_->Width(i), indirect_lighting_tex_->Height(i),
				EF_D24S8, 1, 0);

			FrameBufferPtr fb = rf.MakeFrameBuffer();
			fb->Attach(FrameBuffer::ATT_Color0, rf.Make2DRenderView(*indirect_lighting_tex_, 0, 1, i));
			fb->Attach(FrameBuffer::ATT_DepthStencil, subsplat_ds_view);
			vpls_lighting_fbs_[i] = fb;
		}
	}

	void MultiResSILLayer::RSM(TexturePtr const & rt0_tex, TexturePtr const & rt1_tex, TexturePtr const & depth_tex)
	{
		RenderFactory& rf = Context::Instance().RenderFactoryInstance();
		RenderEngine& re = rf.RenderEngineInstance();
		RenderDeviceCaps const & caps = re.DeviceCaps();

#ifdef USE_NEW_LIGHT_SAMPLING
		std::string RSM2VPLsSpotName = "RSM2VPLsSpotNew";
#else
		std::string RSM2VPLsSpotName = "RSM2VPLsSpot";
#endif

		rsm_texs_[0] = rt0_tex;
		rsm_texs_[1] = rt1_tex;
		rsm_depth_tex_ = depth_tex;

		rsm_to_vpls_pps_[LT_Spot] = LoadPostProcess(ResLoader::Instance().Open("RSM2VPLs.ppml"), RSM2VPLsSpotName);
		rsm_to_vpls_pps_[LT_Spot]->InputPin(0, rsm_texs_[0]);
		rsm_to_vpls_pps_[LT_Spot]->InputPin(1, rsm_texs_[1]);
		rsm_to_vpls_pps_[LT_Spot]->InputPin(2, rsm_depth_tex_);
		rsm_to_vpls_pps_[LT_Spot]->OutputPin(0, vpl_tex_);

#ifdef USE_NEW_LIGHT_SAMPLING
		ElementFormat fmt;
		if (caps.rendertarget_format_support(EF_GR32F, 1, 0))
		{
			fmt = EF_GR32F;
		}
		else if (caps.rendertarget_format_support(EF_ABGR32F, 1, 0))
		{
			fmt = EF_ABGR32F;
		}
		else if (caps.rendertarget_format_support(EF_GR16F, 1, 0))
		{
			fmt = EF_GR16F;
		}
		else
		{
			BOOST_ASSERT(caps.rendertarget_format_support(EF_ABGR16F, 1, 0));

			fmt = EF_ABGR16F;
		}

		rsm_depth_derivative_tex_ = rf.MakeTexture2D(MIN_RSM_MIPMAP_SIZE, MIN_RSM_MIPMAP_SIZE, 1, 1, fmt, 1, 0, EAH_GPU_Read | EAH_GPU_Write, nullptr);

		rsm_to_depth_derivate_pp_ =  LoadPostProcess(ResLoader::Instance().Open("CustomMipMap.ppml"), "GBuffer2DepthDerivate");
		rsm_to_depth_derivate_pp_->InputPin(1, rsm_depth_tex_);
		rsm_to_depth_derivate_pp_->OutputPin(0, rsm_depth_derivative_tex_);
		float delta_x = 1.0f / rsm_depth_derivative_tex_->Width(0);
		float delta_y = 1.0f / rsm_depth_derivative_tex_->Height(0);
		float4 rsm_delta_offset(delta_x, delta_y, delta_x / 2, delta_y / 2);
		rsm_to_depth_derivate_pp_->SetParam(0, rsm_delta_offset);
#endif
	}

	void MultiResSILLayer::UpdateGBuffer(CameraPtr const & vp_camera)
	{
		g_buffer_camera_ = vp_camera;

		this->CreateDepthDerivativeMipMap();
		this->CreateNormalConeMipMap();
		this->SetSubsplatStencil();
	}

	void MultiResSILLayer::UpdateRSM(CameraPtr const & rsm_camera, LightSourcePtr const & light)
	{
#ifdef USE_NEW_LIGHT_SAMPLING
		this->ExtractVPLsNew(rsm_camera, light);
#else
		this->ExtractVPLs(rsm_camera, light);
#endif
		this->VPLsLighting(light);
	}

	void MultiResSILLayer::CalcIndirectLighting(TexturePtr const & /*prev_shading_tex*/, CameraPtr const & /*prev_camera*/)
	{
		this->UpsampleMultiresLighting();
	}

	void MultiResSILLayer::CreateDepthDerivativeMipMap()
	{
		gbuffer_to_depth_derivate_pp_->InputPin(0, g_buffer_texs_[0]);
		gbuffer_to_depth_derivate_pp_->InputPin(1, g_buffer_depth_tex_);
		gbuffer_to_depth_derivate_pp_->OutputPin(0, depth_deriative_tex_);
		float delta_x = 1.0f / g_buffer_texs_[0]->Width(0);
		float delta_y = 1.0f / g_buffer_texs_[0]->Height(0);
		float4 delta_offset(delta_x, delta_y, delta_x / 2, delta_y / 2);
		gbuffer_to_depth_derivate_pp_->SetParam(0, delta_offset);
		gbuffer_to_depth_derivate_pp_->Apply();

		depth_derivate_mipmap_pp_->InputPin(0, depth_deriative_tex_);
		for (uint32_t i = 1; i < depth_deriative_tex_->NumMipMaps(); ++ i)
		{
			int width = depth_deriative_tex_->Width(i - 1);
			int height = depth_deriative_tex_->Height(i - 1);

			delta_x = 1.0f / width;
			delta_y = 1.0f / height;
			float4 delta_offset(delta_x, delta_y, delta_x / 2, delta_y / 2);			
			depth_derivate_mipmap_pp_->SetParam(0, delta_offset);
			depth_derivate_mipmap_pp_->SetParam(1, i - 1.0f);
			
			depth_derivate_mipmap_pp_->OutputPin(0, depth_deriative_small_tex_, i - 1);
			depth_derivate_mipmap_pp_->Apply();

			depth_deriative_small_tex_->CopyToSubTexture2D(*depth_deriative_tex_, 0, i, 0, 0, width / 2, height / 2,
				0, i - 1, 0, 0, width / 2, height / 2);
		}
	}

	void MultiResSILLayer::CreateNormalConeMipMap()
	{
		gbuffer_to_normal_cone_pp_->InputPin(0, g_buffer_texs_[0]);
		gbuffer_to_normal_cone_pp_->OutputPin(0, normal_cone_tex_);
		float delta_x = 1.0f / g_buffer_texs_[0]->Width(0);
		float delta_y = 1.0f / g_buffer_texs_[0]->Height(0);
		float4 delta_offset(delta_x, delta_y, delta_x / 2, delta_y / 2);
		gbuffer_to_normal_cone_pp_->SetParam(0, delta_offset);
		gbuffer_to_normal_cone_pp_->Apply();

		normal_cone_mipmap_pp_->InputPin(0, normal_cone_tex_);
		for (uint32_t i = 1; i < normal_cone_tex_->NumMipMaps(); ++ i)
		{
			int width = normal_cone_tex_->Width(i - 1);
			int height = normal_cone_tex_->Height(i - 1);
			float delta_x = 1.0f / width;
			float delta_y = 1.0f / height;
			float4 delta_offset(delta_x, delta_y, delta_x / 2, delta_y / 2);

			normal_cone_mipmap_pp_->SetParam(0, delta_offset);
			normal_cone_mipmap_pp_->SetParam(1, i - 1.0f);

			normal_cone_mipmap_pp_->OutputPin(0, normal_cone_small_tex_, i - 1);
			normal_cone_mipmap_pp_->Apply();

			normal_cone_small_tex_->CopyToSubTexture2D(*normal_cone_tex_, 0, i, 0, 0, width / 2, height / 2,
				0, i - 1, 0, 0, width / 2, height / 2);
		}
	}

	void MultiResSILLayer::SetSubsplatStencil()
	{
		*subsplat_depth_deriv_tex_param_ = depth_deriative_tex_;
		*subsplat_normal_cone_tex_param_ = normal_cone_tex_;
		*subsplat_depth_normal_threshold_param_ = float2(0.001f * g_buffer_camera_->FarPlane(), 0.77f);

		RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
		for (size_t i = 0; i < vpls_lighting_fbs_.size(); ++ i)
		{
			re.BindFrameBuffer(vpls_lighting_fbs_[i]);
			vpls_lighting_fbs_[i]->Clear(FrameBuffer::CBM_Color | FrameBuffer::CBM_Depth | FrameBuffer::CBM_Stencil, Color(0, 0, 0, 0), 0.0f, 128);

			*subsplat_cur_lower_level_param_ = float2(static_cast<float>(i), static_cast<float>(i + 1));
			*subsplat_is_not_first_last_level_param_ = int2(i > 0, i < vpls_lighting_fbs_.size() - 1);

			re.Render(*subsplat_stencil_tech_, *rl_quad_);
		}
	}

	void MultiResSILLayer::ExtractVPLs(CameraPtr const & rsm_camera, LightSourcePtr const & light)
	{
		rsm_texs_[0]->BuildMipSubLevels();
		rsm_texs_[1]->BuildMipSubLevels();
		float4x4 ls_to_es = rsm_camera->InverseViewMatrix() * g_buffer_camera_->ViewMatrix();
		float mip_level = (MathLib::log(static_cast<float>(rsm_texs_[0]->Width(0))) - MathLib::log(static_cast<float>(VPL_COUNT_SQRT))) / MathLib::log(2);
		float4 vpl_params = float4(static_cast<float>(VPL_COUNT), static_cast<float>(VPL_COUNT_SQRT), VPL_DELTA, VPL_OFFSET);

		float4x4 const & inv_proj = rsm_camera->InverseProjMatrix();

		LightType type = light->Type();
		rsm_to_vpls_pps_[type]->SetParam(0, ls_to_es);
		rsm_to_vpls_pps_[type]->SetParam(1, mip_level);
		rsm_to_vpls_pps_[type]->SetParam(2, vpl_params);
		rsm_to_vpls_pps_[type]->SetParam(3, light->Color());
		rsm_to_vpls_pps_[type]->SetParam(4, light->CosOuterInner());
		rsm_to_vpls_pps_[type]->SetParam(5, light->Falloff());
		rsm_to_vpls_pps_[type]->SetParam(6, g_buffer_camera_->InverseViewMatrix());
		float3 upper_left = MathLib::transform_coord(float3(-1, +1, 1), inv_proj);
		float3 upper_right = MathLib::transform_coord(float3(+1, +1, 1), inv_proj);
		float3 lower_left = MathLib::transform_coord(float3(-1, -1, 1), inv_proj);
		rsm_to_vpls_pps_[type]->SetParam(7, upper_left);
		rsm_to_vpls_pps_[type]->SetParam(8, upper_right - upper_left);
		rsm_to_vpls_pps_[type]->SetParam(9, lower_left - upper_left);
		rsm_to_vpls_pps_[type]->Apply();
	}

	void MultiResSILLayer::VPLsLighting(LightSourcePtr const & light)
	{
		RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
		RenderDeviceCaps const & caps = re.DeviceCaps();

		*vpl_view_param_ = g_buffer_camera_->ViewMatrix();
		*vpl_proj_param_ = g_buffer_camera_->ProjMatrix();;
		*vpl_depth_near_far_invfar_param_ = float3(g_buffer_camera_->NearPlane(),
			g_buffer_camera_->FarPlane(), 1 / g_buffer_camera_->FarPlane());

		float3 p = MathLib::transform_coord(light->Position(), g_buffer_camera_->ViewMatrix());
		*vpl_light_pos_es_param_ = float4(p.x(), p.y(), p.z(), 1);
		*vpl_light_color_param_ = light->Color();
		*vpl_light_falloff_param_ = light->Falloff();

		*vpl_gbuffer_tex_param_ = g_buffer_texs_[0];
		*vpl_depth_tex_param_ = g_buffer_depth_tex_;
		
		for (size_t i = 0; i < vpls_lighting_fbs_.size(); ++ i)
		{
			re.BindFrameBuffer(vpls_lighting_fbs_[i]);

			if (caps.instance_id_support)
			{
				re.Render(*vpls_lighting_instance_id_tech_, *rl_vpl_);
			}
			else
			{
				for (int j = 0; j < VPL_COUNT; ++ j)
				{
					*vpl_x_coord_param_ = (j + 0.5f) / VPL_COUNT;
					re.Render(*vpls_lighting_no_instance_id_tech_, *rl_vpl_);
				}
			}
		}
	}

	void MultiResSILLayer::UpsampleMultiresLighting()
	{
		for (int i = indirect_lighting_tex_->NumMipMaps() - 2; i >= 0; -- i)
		{
			uint32_t const width = indirect_lighting_tex_->Width(i);
			uint32_t const height = indirect_lighting_tex_->Height(i);

			upsampling_pp_->SetParam(0, float4(1.0f / indirect_lighting_tex_->Width(i + 1) , 1.0f / indirect_lighting_tex_->Height(i + 1),
				1.0f / width, 1.0f / height));
			upsampling_pp_->SetParam(1, int2(i + 1, i));
			
			upsampling_pp_->InputPin(0, indirect_lighting_tex_);
			upsampling_pp_->OutputPin(0, indirect_lighting_pingpong_tex_, i);
			upsampling_pp_->Apply();

			indirect_lighting_pingpong_tex_->CopyToSubTexture2D(*indirect_lighting_tex_, 0, i, 0, 0, width, height,
				0, i, 0, 0, width, height);
		}
	}

#ifdef USE_NEW_LIGHT_SAMPLING
	void MultiResSILLayer::ExtractVPLsNew(CameraPtr const & rsm_camera, LightSourcePtr const & light)
	{
		rsm_texs_[0]->BuildMipSubLevels();
		rsm_texs_[1]->BuildMipSubLevels();
		
		rsm_to_depth_derivate_pp_->Apply();
		
		float4x4 ls_to_es = rsm_camera->InverseViewMatrix() * g_buffer_camera_->ViewMatrix();
		float4x4 const & inv_proj = rsm_camera->InverseProjMatrix();
		LightType type = light->Type();

		float4 vpl_params(static_cast<float>(VPL_COUNT), 2.0f, 
			              static_cast<float>(MIN_RSM_MIPMAP_SIZE), static_cast<float>(MIN_RSM_MIPMAP_SIZE * MIN_RSM_MIPMAP_SIZE));

		rsm_to_vpls_pps_[type]->SetParam(0, ls_to_es);
		//rsm_to_vpls_pps_[type]->SetParam(1, static_cast<float>(1));
		rsm_to_vpls_pps_[type]->SetParam(2, vpl_params);
		rsm_to_vpls_pps_[type]->SetParam(3, light->Color());
		rsm_to_vpls_pps_[type]->SetParam(4, light->CosOuterInner());
		rsm_to_vpls_pps_[type]->SetParam(5, light->Falloff());
		rsm_to_vpls_pps_[type]->SetParam(6, g_buffer_camera_->InverseViewMatrix());
		float3 upper_left = MathLib::transform_coord(float3(-1, +1, 1), inv_proj);
		float3 upper_right = MathLib::transform_coord(float3(+1, +1, 1), inv_proj);
		float3 lower_left = MathLib::transform_coord(float3(-1, -1, 1), inv_proj);
		rsm_to_vpls_pps_[type]->SetParam(7, upper_left);
		rsm_to_vpls_pps_[type]->SetParam(8, upper_right - upper_left);
		rsm_to_vpls_pps_[type]->SetParam(9, lower_left - upper_left);
		rsm_to_vpls_pps_[type]->SetParam(10, int2(1, 0));
		rsm_to_vpls_pps_[type]->SetParam(11, 0.12f * rsm_camera->FarPlane());
		rsm_to_vpls_pps_[type]->SetParam(12, static_cast<float>(rsm_texs_[0]->NumMipMaps() - 1));

		rsm_to_vpls_pps_[type]->InputPin(3, rsm_depth_derivative_tex_);

		rsm_to_vpls_pps_[type]->Apply();
	}
#endif


	SSGILayer::SSGILayer()
	{
		RenderFactory& rf = Context::Instance().RenderFactoryInstance();

		ssgi_pp_ = MakeSharedPtr<SSGIPostProcess>();
		ssgi_blur_pp_ = MakeSharedPtr<BlurPostProcess<SeparableBilateralFilterPostProcess> >(4, 1.0f,
			rf.LoadEffect("SSGI.fxml")->TechniqueByName("SSGIBlurX"), rf.LoadEffect("SSGI.fxml")->TechniqueByName("SSGIBlurY"));
	}

	void SSGILayer::GBuffer(TexturePtr const & rt0_tex, TexturePtr const & rt1_tex, TexturePtr const & depth_tex)
	{
		RenderFactory& rf = Context::Instance().RenderFactoryInstance();
		RenderEngine& re = rf.RenderEngineInstance();
		RenderDeviceCaps const & caps = re.DeviceCaps();

		g_buffer_texs_[0] = rt0_tex;
		g_buffer_texs_[1] = rt1_tex;
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
		small_ssgi_tex_ = rf.MakeTexture2D(width / 4, height / 4, 1, 1, fmt, 1, 0, EAH_GPU_Read | EAH_GPU_Write, nullptr);

		indirect_lighting_tex_ = rf.MakeTexture2D(width / 2, height / 2, 1, 1, EF_ABGR16F, 1, 0,  EAH_GPU_Read | EAH_GPU_Write, nullptr);
	}

	void SSGILayer::RSM(TexturePtr const & /*rt0_tex*/, TexturePtr const & /*rt1_tex*/, TexturePtr const & /*depth_tex*/)
	{
	}

	void SSGILayer::UpdateGBuffer(CameraPtr const & /*vp_camera*/)
	{
	}

	void SSGILayer::UpdateRSM(CameraPtr const & /*rsm_camera*/, LightSourcePtr const & /*light*/)
	{
	}

	void SSGILayer::CalcIndirectLighting(TexturePtr const & prev_shading_tex, CameraPtr const & /*prev_camera*/)
	{
		ssgi_pp_->InputPin(0, g_buffer_texs_[0]);
		ssgi_pp_->InputPin(1, g_buffer_depth_tex_);
		ssgi_pp_->InputPin(2, prev_shading_tex);
		ssgi_pp_->OutputPin(0, small_ssgi_tex_);
		ssgi_pp_->Apply();

		ssgi_blur_pp_->InputPin(0, small_ssgi_tex_);
		ssgi_blur_pp_->InputPin(1, g_buffer_depth_tex_);
		ssgi_blur_pp_->OutputPin(0, indirect_lighting_tex_);
		ssgi_blur_pp_->Apply();
	}
}
