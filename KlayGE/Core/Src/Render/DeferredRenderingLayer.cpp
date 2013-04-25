// DeferredRenderingLayer.cpp
// KlayGE Deferred Rendering Layer implement file
// Ver 4.0.0
// Copyright(C) Minmin Gong, 2011
// Homepage: http://www.klayge.org
//
// 4.0.0
// First release (2011.8.28)
//
// CHANGE LIST
//////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KFL/Util.hpp>
#include <KFL/Math.hpp>
#include <KlayGE/ResLoader.hpp>
#include <KlayGE/Renderable.hpp>
#include <KlayGE/RenderableHelper.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/RenderEffect.hpp>
#include <KlayGE/FrameBuffer.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/SceneObjectHelper.hpp>
#include <KlayGE/SceneManager.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/App3D.hpp>
#include <KlayGE/PostProcess.hpp>
#include <KlayGE/Query.hpp>
#include <KlayGE/Camera.hpp>
#include <KlayGE/Mesh.hpp>
#include <KlayGE/SSVOPostProcess.hpp>
#include <KlayGE/SSRPostProcess.hpp>

#include <KlayGE/DeferredRenderingLayer.hpp>

#define USE_NEW_LIGHT_SAMPLING

namespace KlayGE
{
	int const SM_SIZE = 512;
	
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

	template <typename T>
	void CreateConeMesh(std::vector<T>& vb, std::vector<uint16_t>& ib, uint16_t vertex_base, float radius, float height, uint16_t n)
	{
		for (int i = 0; i < n; ++ i)
		{
			vb.push_back(T());
			vb.back().x() = vb.back().y() = vb.back().z() = 0;
		}

		float outer_radius = radius / cos(PI / n);
		for (int i = 0; i < n; ++ i)
		{
			vb.push_back(T());
			float angle = i * 2 * PI / n;
			vb.back().x() = outer_radius * cos(angle);
			vb.back().y() = outer_radius * sin(angle);
			vb.back().z() = height;
		}

		vb.push_back(T());
		vb.back().x() = vb.back().y() = 0;
		vb.back().z() = height;

		for (int i = 0; i < n; ++ i)
		{
			vb.push_back(T());
			vb.back() = vb[vertex_base + n + i];
		}

		for (uint16_t i = 0; i < n - 1; ++ i)
		{
			ib.push_back(vertex_base + i);
			ib.push_back(vertex_base + n + i + 1);
			ib.push_back(vertex_base + n + i);
		}
		ib.push_back(vertex_base + n - 1);
		ib.push_back(vertex_base + n + 0);
		ib.push_back(vertex_base + n + n - 1);

		for (uint16_t i = 0; i < n - 1; ++ i)
		{
			ib.push_back(vertex_base + 2 * n);
			ib.push_back(vertex_base + 2 * n + 1 + i);
			ib.push_back(vertex_base + 2 * n + 1 + i + 1);
		}
		ib.push_back(vertex_base + 2 * n);
		ib.push_back(vertex_base + 2 * n + 1 + n - 1);
		ib.push_back(vertex_base + 2 * n + 1);
	}

	template <typename T>
	void CreatePyramidMesh(std::vector<T>& vb, std::vector<uint16_t>& ib, uint16_t vertex_base, float radius, float height)
	{
		for (int i = 0; i < 4; ++ i)
		{
			vb.push_back(T());
			vb.back().x() = vb.back().y() = vb.back().z() = 0;
		}

		float outer_radius = radius * sqrt(2.0f);
		vb.push_back(T());
		vb.back().x() = -outer_radius;
		vb.back().y() = -outer_radius;
		vb.back().z() = height;
		vb.push_back(T());
		vb.back().x() = +outer_radius;
		vb.back().y() = -outer_radius;
		vb.back().z() = height;
		vb.push_back(T());
		vb.back().x() = +outer_radius;
		vb.back().y() = +outer_radius;
		vb.back().z() = height;
		vb.push_back(T());
		vb.back().x() = -outer_radius;
		vb.back().y() = +outer_radius;
		vb.back().z() = height;

		vb.push_back(T());
		vb.back().x() = vb.back().y() = 0;
		vb.back().z() = height;

		for (int i = 0; i < 4; ++ i)
		{
			vb.push_back(T());
			vb.back() = vb[vertex_base + 4 + i];
		}

		ib.push_back(vertex_base + 0);
		ib.push_back(vertex_base + 5);
		ib.push_back(vertex_base + 4);
		ib.push_back(vertex_base + 0);
		ib.push_back(vertex_base + 6);
		ib.push_back(vertex_base + 5);
		ib.push_back(vertex_base + 0);
		ib.push_back(vertex_base + 7);
		ib.push_back(vertex_base + 6);
		ib.push_back(vertex_base + 0);
		ib.push_back(vertex_base + 4);
		ib.push_back(vertex_base + 7);

		ib.push_back(vertex_base + 8);
		ib.push_back(vertex_base + 9);
		ib.push_back(vertex_base + 10);
		ib.push_back(vertex_base + 8);
		ib.push_back(vertex_base + 10);
		ib.push_back(vertex_base + 11);
		ib.push_back(vertex_base + 8);
		ib.push_back(vertex_base + 11);
		ib.push_back(vertex_base + 12);
		ib.push_back(vertex_base + 8);
		ib.push_back(vertex_base + 12);
		ib.push_back(vertex_base + 9);
	}

	template <typename T>
	void CreateBoxMesh(std::vector<T>& vb, std::vector<uint16_t>& ib, uint16_t vertex_base, float half_length)
	{
		vb.push_back(T());
		vb.back().x() = -half_length;
		vb.back().y() = +half_length;
		vb.back().z() = -half_length;
		vb.push_back(T());
		vb.back().x() = +half_length;
		vb.back().y() = +half_length;
		vb.back().z() = -half_length;
		vb.push_back(T());
		vb.back().x() = +half_length;
		vb.back().y() = -half_length;
		vb.back().z() = -half_length;
		vb.push_back(T());
		vb.back().x() = -half_length;
		vb.back().y() = -half_length;
		vb.back().z() = -half_length;

		vb.push_back(T());
		vb.back().x() = -half_length;
		vb.back().y() = +half_length;
		vb.back().z() = +half_length;
		vb.push_back(T());
		vb.back().x() = +half_length;
		vb.back().y() = +half_length;
		vb.back().z() = +half_length;
		vb.push_back(T());
		vb.back().x() = +half_length;
		vb.back().y() = -half_length;
		vb.back().z() = +half_length;
		vb.push_back(T());
		vb.back().x() = -half_length;
		vb.back().y() = -half_length;
		vb.back().z() = +half_length;

		ib.push_back(vertex_base + 0);
		ib.push_back(vertex_base + 1);
		ib.push_back(vertex_base + 2);
		ib.push_back(vertex_base + 2);
		ib.push_back(vertex_base + 3);
		ib.push_back(vertex_base + 0);

		ib.push_back(vertex_base + 5);
		ib.push_back(vertex_base + 4);
		ib.push_back(vertex_base + 7);
		ib.push_back(vertex_base + 7);
		ib.push_back(vertex_base + 6);
		ib.push_back(vertex_base + 5);

		ib.push_back(vertex_base + 4);
		ib.push_back(vertex_base + 5);
		ib.push_back(vertex_base + 1);
		ib.push_back(vertex_base + 1);
		ib.push_back(vertex_base + 0);
		ib.push_back(vertex_base + 4);

		ib.push_back(vertex_base + 1);
		ib.push_back(vertex_base + 5);
		ib.push_back(vertex_base + 6);
		ib.push_back(vertex_base + 6);
		ib.push_back(vertex_base + 2);
		ib.push_back(vertex_base + 1);

		ib.push_back(vertex_base + 3);
		ib.push_back(vertex_base + 2);
		ib.push_back(vertex_base + 6);
		ib.push_back(vertex_base + 6);
		ib.push_back(vertex_base + 7);
		ib.push_back(vertex_base + 3);

		ib.push_back(vertex_base + 4);
		ib.push_back(vertex_base + 0);
		ib.push_back(vertex_base + 3);
		ib.push_back(vertex_base + 3);
		ib.push_back(vertex_base + 7);
		ib.push_back(vertex_base + 4);
	}


	DeferredRenderingLayer::DeferredRenderingLayer()
		: active_viewport_(0), ssr_enabled_(true), taa_enabled_(true),
			light_scale_(1), illum_(0), indirect_scale_(1.0f)
	{
		RenderFactory& rf = Context::Instance().RenderFactoryInstance();
		RenderEngine& re = rf.RenderEngineInstance();
		RenderDeviceCaps const & caps = re.DeviceCaps();

		ElementFormat ds_fmt;
		if (caps.texture_format_support(EF_D24S8))
		{
			ds_fmt = EF_D24S8;
			depth_texture_support_ = true;
		}
		else
		{
			if (caps.texture_format_support(EF_D16))
			{				
				ds_fmt = EF_D16;
				depth_texture_support_ = true;
			}
			else
			{
				ds_fmt = EF_Unknown;
				depth_texture_support_ = false;
			}
		}

		mrt_g_buffer_support_ = (caps.max_simultaneous_rts > 1);

		for (size_t vpi = 0; vpi < viewports_.size(); ++ vpi)
		{
			PerViewport& pvp = viewports_[vpi];
			for (size_t i = 0; i < pvp.g_buffers.size(); ++ i)
			{
				if (!depth_texture_support_)
				{
					pvp.pre_depth_buffers[i] = rf.MakeFrameBuffer();
				}
				pvp.g_buffers[i] = rf.MakeFrameBuffer();
				if (!mrt_g_buffer_support_)
				{
					pvp.g_buffers_rt1[i] = rf.MakeFrameBuffer();
				}
				pvp.lighting_buffers[i] = rf.MakeFrameBuffer();
				pvp.shading_buffers[i] = rf.MakeFrameBuffer();
				pvp.curr_merged_shading_buffers[i] = rf.MakeFrameBuffer();
				pvp.curr_merged_depth_buffers[i] = rf.MakeFrameBuffer();
				pvp.prev_merged_shading_buffers[i] = rf.MakeFrameBuffer();
				pvp.prev_merged_depth_buffers[i] = rf.MakeFrameBuffer();
			}
			pvp.shadowing_buffer = rf.MakeFrameBuffer();
		}

		{
			rl_cone_ = rf.MakeRenderLayout();
			rl_cone_->TopologyType(RenderLayout::TT_TriangleList);

			std::vector<float3> pos;
			std::vector<uint16_t> index;
			CreateConeMesh(pos, index, 0, 100.0f, 100.0f, 12);
			cone_obb_ = MathLib::compute_obbox(pos.begin(), pos.end());

			ElementInitData init_data;
			init_data.row_pitch = static_cast<uint32_t>(pos.size() * sizeof(pos[0]));
			init_data.slice_pitch = 0;
			init_data.data = &pos[0];
			rl_cone_->BindVertexStream(rf.MakeVertexBuffer(BU_Static, EAH_GPU_Read | EAH_Immutable, &init_data),
				make_tuple(vertex_element(VEU_Position, 0, EF_BGR32F)));

			init_data.row_pitch = static_cast<uint32_t>(index.size() * sizeof(index[0]));
			init_data.data = &index[0];
			rl_cone_->BindIndexStream(rf.MakeIndexBuffer(BU_Static, EAH_GPU_Read | EAH_Immutable, &init_data), EF_R16UI);
		}
		{
			rl_pyramid_ = rf.MakeRenderLayout();
			rl_pyramid_->TopologyType(RenderLayout::TT_TriangleList);

			std::vector<float3> pos;
			std::vector<uint16_t> index;
			CreatePyramidMesh(pos, index, 0, 100.0f, 100.0f);
			pyramid_obb_ = MathLib::compute_obbox(pos.begin(), pos.end());

			ElementInitData init_data;
			init_data.row_pitch = static_cast<uint32_t>(pos.size() * sizeof(pos[0]));
			init_data.slice_pitch = 0;
			init_data.data = &pos[0];
			rl_pyramid_->BindVertexStream(rf.MakeVertexBuffer(BU_Static, EAH_GPU_Read | EAH_Immutable, &init_data),
				make_tuple(vertex_element(VEU_Position, 0, EF_BGR32F)));

			init_data.row_pitch = static_cast<uint32_t>(index.size() * sizeof(index[0]));
			init_data.data = &index[0];
			rl_pyramid_->BindIndexStream(rf.MakeIndexBuffer(BU_Static, EAH_GPU_Read | EAH_Immutable, &init_data), EF_R16UI);
		}
		{
			rl_box_ = rf.MakeRenderLayout();
			rl_box_->TopologyType(RenderLayout::TT_TriangleList);

			std::vector<float3> pos;
			std::vector<uint16_t> index;
			CreateBoxMesh(pos, index, 0, 100.0f);
			box_obb_ = MathLib::compute_obbox(pos.begin(), pos.end());

			ElementInitData init_data;
			init_data.row_pitch = static_cast<uint32_t>(pos.size() * sizeof(pos[0]));
			init_data.slice_pitch = 0;
			init_data.data = &pos[0];
			rl_box_->BindVertexStream(rf.MakeVertexBuffer(BU_Static, EAH_GPU_Read | EAH_Immutable, &init_data),
				make_tuple(vertex_element(VEU_Position, 0, EF_BGR32F)));

			init_data.row_pitch = static_cast<uint32_t>(index.size() * sizeof(index[0]));
			init_data.data = &index[0];
			rl_box_->BindIndexStream(rf.MakeIndexBuffer(BU_Static, EAH_GPU_Read | EAH_Immutable, &init_data), EF_R16UI);
		}
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

		light_volume_rl_[LT_Ambient] = rl_quad_;
		light_volume_rl_[LT_Directional] = rl_quad_;
		light_volume_rl_[LT_Point] = rl_box_;
		light_volume_rl_[LT_Spot] = rl_cone_;

		g_buffer_effect_ = SyncLoadRenderEffect("GBuffer.fxml");
		dr_effect_ = SyncLoadRenderEffect("DeferredRendering.fxml");

		technique_shadows_[LT_Ambient] = dr_effect_->TechniqueByName("DeferredShadowingAmbient");
		technique_shadows_[LT_Directional] = dr_effect_->TechniqueByName("DeferredShadowingDirectional");
		technique_shadows_[LT_Point] = dr_effect_->TechniqueByName("DeferredShadowingPoint");
		technique_shadows_[LT_Spot] = dr_effect_->TechniqueByName("DeferredShadowingSpot");
		technique_lights_[LT_Ambient] = dr_effect_->TechniqueByName("DeferredRenderingAmbient");
		technique_lights_[LT_Directional] = dr_effect_->TechniqueByName("DeferredRenderingDirectional");
		technique_lights_[LT_Point] = dr_effect_->TechniqueByName("DeferredRenderingPoint");
		technique_lights_[LT_Spot] = dr_effect_->TechniqueByName("DeferredRenderingSpot");
		technique_light_depth_only_ = dr_effect_->TechniqueByName("DeferredRenderingLightDepthOnly");
		technique_light_stencil_ = dr_effect_->TechniqueByName("DeferredRenderingLightStencil");
		technique_clear_stencil_ = dr_effect_->TechniqueByName("ClearStencil");
		technique_no_lighting_ = dr_effect_->TechniqueByName("NoLightingTech");
		technique_shading_ = dr_effect_->TechniqueByName("ShadingTech");
		technique_merge_shadings_[0] = dr_effect_->TechniqueByName("MergeShadingTech");
		technique_merge_shadings_[1] = dr_effect_->TechniqueByName("MergeShadingAlphaBlendTech");
		technique_merge_depths_[0] = dr_effect_->TechniqueByName("MergeDepthTech");
		technique_merge_depths_[1] = dr_effect_->TechniqueByName("MergeDepthAlphaBlendTech");
		technique_copy_shading_depth_ = dr_effect_->TechniqueByName("CopyShadingDepthTech");
		technique_copy_depth_ = dr_effect_->TechniqueByName("CopyDepthTech");

		sm_buffer_ = rf.MakeFrameBuffer();
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
		sm_tex_ = rf.MakeTexture2D(SM_SIZE, SM_SIZE, 1, 1, fmt, 1, 0, EAH_GPU_Read | EAH_GPU_Write, nullptr);
		sm_buffer_->Attach(FrameBuffer::ATT_Color0, rf.Make2DRenderView(*sm_tex_, 0, 1, 0));
		RenderViewPtr sm_depth_view;
		if (depth_texture_support_)
		{
			sm_depth_tex_ = rf.MakeTexture2D(SM_SIZE, SM_SIZE, 1, 1, ds_fmt, 1, 0, EAH_GPU_Read | EAH_GPU_Write, nullptr);
			sm_depth_view = rf.Make2DDepthStencilRenderView(*sm_depth_tex_, 0, 1, 0);
		}
		else
		{
			sm_depth_view = rf.Make2DDepthStencilRenderView(SM_SIZE, SM_SIZE, EF_D16, 1, 0);
		}
		sm_buffer_->Attach(FrameBuffer::ATT_DepthStencil, sm_depth_view);

		for (uint32_t i = 0; i < NUM_SHADOWED_SPOT_LIGHTS; ++ i)
		{
			blur_sm_2d_texs_[i] = rf.MakeTexture2D(SM_SIZE, SM_SIZE, 1, 1, sm_tex_->Format(), 1, 0, EAH_GPU_Read | EAH_GPU_Write, nullptr);
		}
		for (uint32_t i = 0; i < NUM_SHADOWED_POINT_LIGHTS; ++ i)
		{
			blur_sm_cube_texs_[i] = rf.MakeTextureCube(SM_SIZE, 1, 1, sm_tex_->Format(), 1, 0, EAH_GPU_Read | EAH_GPU_Write, nullptr);
		}

		ssvo_pp_ = MakeSharedPtr<SSVOPostProcess>();
		ssvo_blur_pp_ = MakeSharedPtr<BlurPostProcess<SeparableBilateralFilterPostProcess> >(8, 1.0f,
			SyncLoadRenderEffect("SSVO.fxml")->TechniqueByName("SSVOBlurX"),
			SyncLoadRenderEffect("SSVO.fxml")->TechniqueByName("SSVOBlurY"));

		ssr_pp_ = MakeSharedPtr<SSRPostProcess>();

		taa_pp_ = SyncLoadPostProcess("TAA.ppml", "taa");

		if (mrt_g_buffer_support_)
		{
			rsm_buffer_ = rf.MakeFrameBuffer();

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

			rsm_texs_[0] = rf.MakeTexture2D(SM_SIZE, SM_SIZE, MAX_RSM_MIPMAP_LEVELS, 1, fmt8, 1, 0, EAH_GPU_Read | EAH_GPU_Write | EAH_Generate_Mips, nullptr);
			rsm_texs_[1] = rf.MakeTexture2D(SM_SIZE, SM_SIZE, MAX_RSM_MIPMAP_LEVELS, 1, fmt8, 1, 0, EAH_GPU_Read | EAH_GPU_Write | EAH_Generate_Mips, nullptr);
			rsm_buffer_->Attach(FrameBuffer::ATT_Color0, rf.Make2DRenderView(*rsm_texs_[0], 0, 1, 0)); // albedo
			rsm_buffer_->Attach(FrameBuffer::ATT_Color1, rf.Make2DRenderView(*rsm_texs_[1], 0, 1, 0)); // normal (light space)
			rsm_buffer_->Attach(FrameBuffer::ATT_DepthStencil, sm_depth_view);
			
			copy_to_light_buffer_pp_ = SyncLoadPostProcess("Copy2LightBuffer.ppml", "CopyToLightBuffer");
			copy_to_light_buffer_i_pp_ = SyncLoadPostProcess("Copy2LightBuffer.ppml", "CopyToLightBufferI");
		}


		sm_filter_pp_ = MakeSharedPtr<BlurPostProcess<SeparableGaussianFilterPostProcess> >(8, 1.0f);
		if (depth_texture_support_)
		{
			depth_to_vsm_pp_ = SyncLoadPostProcess("DepthToSM.ppml", "DepthToVSM");
			depth_to_vsm_pp_->InputPin(0, sm_depth_tex_);
			depth_to_vsm_pp_->OutputPin(0, sm_tex_);

			depth_to_linear_pp_ = SyncLoadPostProcess("DepthToSM.ppml", "DepthToSM");
		}

		g_buffer_tex_param_ = dr_effect_->ParameterByName("g_buffer_tex");
		g_buffer_1_tex_param_ = dr_effect_->ParameterByName("g_buffer_1_tex");
		depth_tex_param_ = dr_effect_->ParameterByName("depth_tex");
		lighting_tex_param_ = dr_effect_->ParameterByName("lighting_tex");
		shading_tex_param_ = dr_effect_->ParameterByName("shading_tex");
		depth_near_far_invfar_param_ = dr_effect_->ParameterByName("depth_near_far_invfar");
		light_attrib_param_ = dr_effect_->ParameterByName("light_attrib");
		light_color_param_ = dr_effect_->ParameterByName("light_color");
		light_falloff_param_ = dr_effect_->ParameterByName("light_falloff");
		light_view_proj_param_ = dr_effect_->ParameterByName("light_view_proj");
		light_volume_mv_param_ = dr_effect_->ParameterByName("light_volume_mv");
		light_volume_mvp_param_ = dr_effect_->ParameterByName("light_volume_mvp");
		view_to_light_model_param_ = dr_effect_->ParameterByName("view_to_light_model");
		light_pos_es_param_ = dr_effect_->ParameterByName("light_pos_es");
		light_dir_es_param_ = dr_effect_->ParameterByName("light_dir_es");
		projective_map_2d_tex_param_ = dr_effect_->ParameterByName("projective_map_2d_tex");
		projective_map_cube_tex_param_ = dr_effect_->ParameterByName("projective_map_cube_tex");
		shadow_map_2d_tex_param_ = dr_effect_->ParameterByName("shadow_map_2d_tex");
		shadow_map_cube_tex_param_ = dr_effect_->ParameterByName("shadow_map_cube_tex");
		inv_width_height_param_ = dr_effect_->ParameterByName("inv_width_height");
		shadowing_tex_param_ = dr_effect_->ParameterByName("shadowing_tex");
		near_q_param_ = dr_effect_->ParameterByName("near_q");
	}

	void DeferredRenderingLayer::SSGIEnabled(uint32_t vp, bool ssgi)
	{
		viewports_[vp].ssgi_enable = ssgi;
		this->SetupViewportGI(vp);
	}

	void DeferredRenderingLayer::SSVOEnabled(uint32_t vp, bool ssvo)
	{
		viewports_[vp].ssvo_enabled = ssvo;
	}

	void DeferredRenderingLayer::SSREnabled(bool ssr)
	{
		ssr_enabled_ = ssr;
	}

	void DeferredRenderingLayer::TemporalAAEnabled(bool taa)
	{
		taa_enabled_ = taa;
	}

	void DeferredRenderingLayer::AddDecal(RenderDecalPtr const & decal)
	{
		decals_.push_back(decal);
	}

	void DeferredRenderingLayer::SetupViewport(uint32_t index, FrameBufferPtr const & fb, uint32_t attrib)
	{
		PerViewport& pvp = viewports_[index];
		pvp.attrib = attrib;
		pvp.frame_buffer = fb;
		pvp.frame_buffer->GetViewport()->camera->JitterMode(true);

		if (fb)
		{
			pvp.attrib |= VPAM_Enabled;
		}

		uint32_t const width = pvp.frame_buffer->GetViewport()->width;
		uint32_t const height = pvp.frame_buffer->GetViewport()->height;

		RenderFactory& rf = Context::Instance().RenderFactoryInstance();
		RenderDeviceCaps const & caps = rf.RenderEngineInstance().DeviceCaps();

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

		std::vector<RenderViewPtr> ds_views(pvp.g_buffers.size());
		if (depth_texture_support_)
		{
			ElementFormat ds_fmt;
			if (caps.texture_format_support(EF_D24S8))
			{
				ds_fmt = EF_D24S8;
			}
			else
			{
				BOOST_ASSERT(caps.texture_format_support(EF_D16));
				
				ds_fmt = EF_D16;
			}

			for (size_t i = 0; i < pvp.g_buffers.size(); ++ i)
			{
				pvp.g_buffer_ds_texs[i] = rf.MakeTexture2D(width, height, 1, 1, ds_fmt, 1, 0,  EAH_GPU_Read | EAH_GPU_Write, nullptr);
				ds_views[i] = rf.Make2DDepthStencilRenderView(*pvp.g_buffer_ds_texs[i], 0, 0, 0);
			}
		}
		else
		{
			for (size_t i = 0; i < pvp.g_buffers.size(); ++ i)
			{
				ds_views[i] = rf.Make2DDepthStencilRenderView(width, height, EF_D16, 1, 0);
			}
		}

		for (size_t i = 0; i < pvp.g_buffers.size(); ++ i)
		{
			pvp.g_buffer_depth_texs[i] = rf.MakeTexture2D(width, height, MAX_IL_MIPMAP_LEVELS + 1, 1, depth_fmt, 1, 0,  EAH_GPU_Read | EAH_GPU_Write | EAH_Generate_Mips, nullptr);

			pvp.g_buffer_rt0_texs[i] = rf.MakeTexture2D(width, height, MAX_IL_MIPMAP_LEVELS + 1, 1, fmt8, 1, 0, EAH_GPU_Read | EAH_GPU_Write | EAH_Generate_Mips, nullptr);
			pvp.g_buffer_rt1_texs[i] = rf.MakeTexture2D(width, height, 1, 1, fmt8, 1, 0, EAH_GPU_Read | EAH_GPU_Write, nullptr);
			pvp.g_buffers[i]->Attach(FrameBuffer::ATT_Color0, rf.Make2DRenderView(*pvp.g_buffer_rt0_texs[i], 0, 1, 0));
			pvp.g_buffers[i]->Attach(FrameBuffer::ATT_DepthStencil, ds_views[i]);
			if (mrt_g_buffer_support_)
			{
				pvp.g_buffers[i]->Attach(FrameBuffer::ATT_Color1, rf.Make2DRenderView(*pvp.g_buffer_rt1_texs[i], 0, 1, 0));
			}
			else
			{
				pvp.g_buffers_rt1[i]->Attach(FrameBuffer::ATT_Color0, rf.Make2DRenderView(*pvp.g_buffer_rt1_texs[i], 0, 1, 0));
				pvp.g_buffers_rt1[i]->Attach(FrameBuffer::ATT_DepthStencil, ds_views[i]);
			}

			if (!depth_texture_support_)
			{
				pvp.pre_depth_buffers[i]->Attach(FrameBuffer::ATT_Color0, rf.Make2DRenderView(*pvp.g_buffer_depth_texs[i], 0, 1, 0));
				pvp.pre_depth_buffers[i]->Attach(FrameBuffer::ATT_DepthStencil, ds_views[i]);
			}
		}

		this->SetupViewportGI(index);

		ElementFormat fmt;
		if (caps.rendertarget_format_support(EF_B10G11R11F, 1, 0))
		{
			fmt = EF_B10G11R11F;
		}
		else
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
		pvp.shadowing_tex = rf.MakeTexture2D(width / 2, height / 2, 1, 1, fmt, 1, 0, EAH_GPU_Read | EAH_GPU_Write, nullptr);
		pvp.shadowing_buffer->Attach(FrameBuffer::ATT_Color0, rf.Make2DRenderView(*pvp.shadowing_tex, 0, 1, 0));

		for (size_t i = 0; i < pvp.g_buffers.size(); ++ i)
		{
			pvp.lighting_texs[i] = rf.MakeTexture2D(width, height, 1, 1, EF_ABGR16F, 1, 0, EAH_GPU_Read | EAH_GPU_Write, nullptr);
			pvp.lighting_buffers[i]->Attach(FrameBuffer::ATT_Color0, rf.Make2DRenderView(*pvp.lighting_texs[i], 0, 1, 0));
			pvp.lighting_buffers[i]->Attach(FrameBuffer::ATT_DepthStencil, ds_views[i]);
		}

		if (caps.rendertarget_format_support(EF_B10G11R11F, 1, 0))
		{
			fmt = EF_B10G11R11F;
		}
		else
		{
			BOOST_ASSERT(caps.rendertarget_format_support(EF_ABGR16F, 1, 0));

			fmt = EF_ABGR16F;
		}
		pvp.curr_merged_shading_tex = rf.MakeTexture2D(width, height, 1, 1, fmt, 1, 0, EAH_GPU_Read | EAH_GPU_Write, nullptr);
		pvp.curr_merged_depth_tex = rf.MakeTexture2D(width, height, 1, 1, depth_fmt, 1, 0, EAH_GPU_Read | EAH_GPU_Write, nullptr);
		pvp.prev_merged_shading_tex = rf.MakeTexture2D(width, height, 1, 1, fmt, 1, 0, EAH_GPU_Read | EAH_GPU_Write, nullptr);
		pvp.prev_merged_depth_tex = rf.MakeTexture2D(width, height, 1, 1, depth_fmt, 1, 0, EAH_GPU_Read | EAH_GPU_Write, nullptr);
		for (size_t i = 0; i < pvp.shading_buffers.size(); ++ i)
		{
			pvp.shading_texs[i] = rf.MakeTexture2D(width, height, 1, 1, (0 == i) ? fmt : EF_ABGR16F, 1, 0, EAH_GPU_Read | EAH_GPU_Write, nullptr);
			pvp.shading_buffers[i]->Attach(FrameBuffer::ATT_Color0, rf.Make2DRenderView(*pvp.shading_texs[i], 0, 1, 0));
			pvp.shading_buffers[i]->Attach(FrameBuffer::ATT_DepthStencil, ds_views[i]);

			pvp.curr_merged_shading_buffers[i]->Attach(FrameBuffer::ATT_Color0, rf.Make2DRenderView(*pvp.curr_merged_shading_tex, 0, 1, 0));
			pvp.curr_merged_shading_buffers[i]->Attach(FrameBuffer::ATT_DepthStencil, ds_views[i]);
			pvp.curr_merged_depth_buffers[i]->Attach(FrameBuffer::ATT_Color0, rf.Make2DRenderView(*pvp.curr_merged_depth_tex, 0, 1, 0));
			pvp.curr_merged_depth_buffers[i]->Attach(FrameBuffer::ATT_DepthStencil, ds_views[i]);

			pvp.prev_merged_shading_buffers[i]->Attach(FrameBuffer::ATT_Color0, rf.Make2DRenderView(*pvp.prev_merged_shading_tex, 0, 1, 0));
			pvp.prev_merged_shading_buffers[i]->Attach(FrameBuffer::ATT_DepthStencil, ds_views[i]);
			pvp.prev_merged_depth_buffers[i]->Attach(FrameBuffer::ATT_Color0, rf.Make2DRenderView(*pvp.prev_merged_depth_tex, 0, 1, 0));
			pvp.prev_merged_depth_buffers[i]->Attach(FrameBuffer::ATT_DepthStencil, ds_views[i]);
		}

		if (caps.rendertarget_format_support(EF_R8, 1, 0))
		{
			fmt = EF_R8;
		}
		else if (caps.rendertarget_format_support(EF_R16F, 1, 0))
		{
			fmt = EF_R16F;
		}
		else
		{
			BOOST_ASSERT(caps.rendertarget_format_support(EF_ABGR16F, 1, 0));

			fmt = EF_ABGR16F;
		}
		pvp.small_ssvo_tex = rf.MakeTexture2D(width / 2, height / 2, 1, 1, fmt, 1, 0, EAH_GPU_Read | EAH_GPU_Write, nullptr);
	}

	void DeferredRenderingLayer::EnableViewport(uint32_t index, bool enable)
	{
		PerViewport& pvp = viewports_[index];
		if (enable)
		{
			pvp.attrib |= VPAM_Enabled;
		}
		else
		{
			pvp.attrib &= ~VPAM_Enabled;
		}
	}

	uint32_t DeferredRenderingLayer::Update(uint32_t pass)
	{
		RenderFactory& rf = Context::Instance().RenderFactoryInstance();
		RenderEngine& re = rf.RenderEngineInstance();
		
		if (0 == pass)
		{
			this->BuildLightList();

			bool has_opaque_objs = false;
			bool has_transparency_back_objs = false;
			bool has_transparency_front_objs = false;
			this->BuildVisibleSceneObjList(has_opaque_objs, has_transparency_back_objs, has_transparency_front_objs);

			this->BuildPassScanList(has_opaque_objs, has_transparency_back_objs, has_transparency_front_objs);
		}

		uint32_t vp_index;
		PassType pass_type;
		int32_t org_no, index_in_pass;
		this->DecomposePassScanCode(vp_index, pass_type, org_no, index_in_pass, pass_scaned_[pass]);

		PassRT const pass_rt = GetPassRT(pass_type);
		PassTargetBuffer const pass_tb = GetPassTargetBuffer(pass_type);
		PassCategory const pass_cat = GetPassCategory(pass_type);

		active_viewport_ = vp_index;

		PerViewport& pvp = viewports_[vp_index];

		if ((pass_cat != PC_Lighting) && (pass_cat != PC_IndirectLighting) && (pass_cat != PC_Shading))
		{
			typedef KLAYGE_DECLTYPE(visible_scene_objs_) VisibleSceneObjsType;
			KLAYGE_FOREACH(VisibleSceneObjsType::reference deo, visible_scene_objs_)
			{
				deo->Pass(pass_type);
			}
		}

		uint32_t urv;
		switch (pass_cat)
		{
		case PC_Depth:
			// Pre depth for no depth texture support platforms
			this->PreparePVP(pvp);
			this->GenerateDepthBuffer(pvp, pass_tb);
			urv = App3DFramework::URV_Need_Flush | (App3DFramework::URV_Opaque_Only << pass_tb);
			break;

		case PC_GBuffer:
			if (0 == index_in_pass)
			{
				if ((PRT_RT0 == pass_rt) || (PRT_MRT == pass_rt))
				{
					CameraPtr const & camera = pvp.frame_buffer->GetViewport()->camera;
					pvp.shadowing_buffer->GetViewport()->camera = camera;

					if (depth_texture_support_)
					{
						this->PreparePVP(pvp);

						float q = camera->FarPlane() / (camera->FarPlane() - camera->NearPlane());
						float2 near_q(camera->NearPlane() * q, q);
						depth_to_linear_pp_->SetParam(0, near_q);
					}

					*depth_near_far_invfar_param_ = pvp.depth_near_far_invfar;

					this->GenerateGBuffer(pvp, pass_tb);
				}
				else
				{
					BOOST_ASSERT(PRT_RT1 == pass_rt);

					re.BindFrameBuffer(pvp.g_buffers_rt1[pass_tb]);
					re.CurFrameBuffer()->Attached(FrameBuffer::ATT_Color0)->ClearColor(Color(0, 0, 0, 0));
				}
				urv = App3DFramework::URV_Need_Flush | (App3DFramework::URV_Opaque_Only << pass_tb);
			}
			else
			{
				if ((PRT_RT0 == pass_rt) || (PRT_MRT == pass_rt))
				{
					this->PostGenerateGBuffer(pvp, pass_tb);
					if (PTB_Opaque == pass_tb)
					{
						if (indirect_lighting_enabled_ && !(pvp.attrib & VPAM_NoGI))
						{
							pvp.il_layer->UpdateGBuffer(pvp.frame_buffer->GetViewport()->camera);
						}
					}
				}

				if (PTB_Opaque == pass_tb)
				{
					if (!decals_.empty())
					{
						this->RenderDecals(pvp, PT_OpaqueGBufferRT1);
					}
				}
				urv = App3DFramework::URV_Flushed;
			}
			break;

		case PC_ShadowMap:
			{
				LightSourcePtr const & light = lights_[org_no];
				this->PrepareLightCamera(pvp, light, index_in_pass, pass_type);

				if (index_in_pass > 0)
				{
					this->PostGenerateShadowMap(org_no, index_in_pass);
				}

				if (((LT_Point == light->Type()) && (6 == index_in_pass))
					|| ((LT_Spot == light->Type()) && (1 == index_in_pass)))
				{
					urv = App3DFramework::URV_Flushed;
				}
				else
				{
					urv = App3DFramework::URV_Need_Flush;
					switch (pass_rt)
					{
					case PRT_ShadowMap:
					case PRT_ShadowMapWODepth:
						re.BindFrameBuffer(sm_buffer_);
						re.CurFrameBuffer()->Attached(FrameBuffer::ATT_DepthStencil)->ClearDepth(1.0f);
						break;

					default:
						BOOST_ASSERT(PRT_ReflectiveShadowMap == pass_rt);

						rsm_buffer_->GetViewport()->camera = sm_buffer_->GetViewport()->camera;
						re.BindFrameBuffer(rsm_buffer_);
						re.CurFrameBuffer()->Clear(FrameBuffer::CBM_Color | FrameBuffer::CBM_Depth | FrameBuffer::CBM_Stencil, Color(0, 0, 0, 0), 1.0f, 0);
						urv |= App3DFramework::URV_Opaque_Only;
						break;
					}
				}
			}
			break;

		case PC_IndirectLighting:
			if (depth_texture_support_)
			{
				depth_to_vsm_pp_->Apply();
			}
			pvp.il_layer->UpdateRSM(rsm_buffer_->GetViewport()->camera, lights_[org_no]);
			urv = App3DFramework::URV_Flushed;
			break;

		case PC_Lighting:
			{
				LightSourcePtr const & light = lights_[org_no];
				LightType type = light->Type();
				int32_t attr = light->Attrib();

				this->PrepareLightCamera(pvp, light, index_in_pass, pass_type);

				if (LT_Point == type)
				{
					*projective_map_cube_tex_param_ = light->ProjectiveTexture();
				}
				else
				{
					*projective_map_2d_tex_param_ = light->ProjectiveTexture();
				}

				*light_attrib_param_ = float4(attr & LSA_NoDiffuse ? 0.0f : 1.0f, attr & LSA_NoSpecular ? 0.0f : 1.0f,
					attr & LSA_NoShadow ? -1.0f : 1.0f, light->ProjectiveTexture() ? 1.0f : -1.0f);
				*light_color_param_ = light->Color();
				*light_falloff_param_ = light->Falloff();

				for (uint32_t i = 0; i < pvp.g_buffers.size(); ++ i)
				{
					if (pvp.g_buffer_enables[i])
					{
						this->UpdateShadowing(pvp, i, org_no);
						this->UpdateLighting(pvp, i, type);							
					}
				}

				urv = App3DFramework::URV_Flushed;
			}
			break;

		case PC_Shading:
		case PC_SpecialShading:
		default:
			if (0 == index_in_pass)
			{
				if (PC_Shading == pass_cat)
				{
					if (PTB_Opaque == pass_tb)
					{
						this->UpdateIndirectAndSSVO(pvp);
					}
					this->UpdateShading(pvp, pass_tb);
					urv = App3DFramework::URV_Flushed;
				}
				else
				{
					BOOST_ASSERT(PC_SpecialShading == pass_cat);

					re.BindFrameBuffer(pvp.shading_buffers[pass_tb]);
					urv = App3DFramework::URV_Need_Flush | App3DFramework::URV_Special_Shading_Only
						| (App3DFramework::URV_Opaque_Only << pass_tb);
				}
			}
			else
			{
				if (has_reflective_objs_ && ssr_enabled_)
				{
					this->AddSSR(pvp);
				}

				if (atmospheric_pp_)
				{
					this->AddAtmospheric(pvp);
				}

				this->MergeShadingAndDepth(pvp);

				std::swap(pvp.curr_merged_shading_buffers, pvp.prev_merged_shading_buffers);
				std::swap(pvp.curr_merged_shading_tex, pvp.prev_merged_shading_tex);
				std::swap(pvp.curr_merged_depth_buffers, pvp.prev_merged_depth_buffers);
				std::swap(pvp.curr_merged_depth_tex, pvp.prev_merged_depth_tex);

				if (has_simple_forward_objs_ && !(pvp.attrib & VPAM_NoSimpleForward))
				{
					typedef KLAYGE_DECLTYPE(visible_scene_objs_) VisibleSceneObjsType;
					KLAYGE_FOREACH(VisibleSceneObjsType::reference deo, visible_scene_objs_)
					{
						deo->Pass(PT_SimpleForward);
					}

					urv = App3DFramework::URV_Need_Flush | App3DFramework::URV_Simple_Forward_Only;
				}
				else
				{
					urv = App3DFramework::URV_Flushed;
				}

				if (pass_scaned_.size() - 1 == pass)
				{
					urv |= App3DFramework::URV_Finished;
				}
			}
			break;
		}

		return urv;
	}

	void DeferredRenderingLayer::BuildLightList()
	{
		SceneManager& scene_mgr = Context::Instance().SceneManagerInstance();

		lights_.clear();
		sm_light_indices_.clear();	

		uint32_t const num_lights = scene_mgr.NumLights();
		bool with_ambient = false;
		for (uint32_t i = 0; i < num_lights; ++ i)
		{
			if (LT_Ambient == scene_mgr.GetLight(i)->Type())
			{
				with_ambient = true;
				break;
			}
		}
		if (!with_ambient)
		{
			LightSourcePtr ambient_light = MakeSharedPtr<AmbientLightSource>();
			ambient_light->Color(float3(0.1f, 0.1f, 0.1f));
			lights_.push_back(ambient_light);
			sm_light_indices_.push_back(-1);
		}

		uint32_t num_sm_2d_lights = 0;
		uint32_t num_sm_cube_lights = 0;
		for (uint32_t i = 0; i < num_lights; ++ i)
		{
			LightSourcePtr const & light = scene_mgr.GetLight(i);
			if (light->Enabled())
			{
				lights_.push_back(light);

				if (0 == (light->Attrib() & LSA_NoShadow))
				{
					switch (light->Type())
					{
					case LT_Spot:
						if (num_sm_2d_lights < NUM_SHADOWED_SPOT_LIGHTS)
						{
							sm_light_indices_.push_back(num_sm_2d_lights);
							++ num_sm_2d_lights;
						}
						break;

					case LT_Point:
						if (num_sm_cube_lights < NUM_SHADOWED_POINT_LIGHTS)
						{
							sm_light_indices_.push_back(num_sm_cube_lights);
							++ num_sm_cube_lights;
						}
						break;

					default:
						sm_light_indices_.push_back(-1);
						break;
					}
				}
				else
				{
					sm_light_indices_.push_back(-1);
				}
			}
		}

		indirect_lighting_enabled_ = false;
		if (rsm_buffer_ && (illum_ != 1))
		{
			for (size_t i = 0; i < lights_.size(); ++ i)
			{
				if (lights_[i]->Attrib() & LSA_IndirectLighting)
				{
					indirect_lighting_enabled_ = true;
					break;
				}
			}
		}
	}

	void DeferredRenderingLayer::BuildVisibleSceneObjList(bool& has_opaque_objs, bool& has_transparency_back_objs, bool& has_transparency_front_objs)
	{
		SceneManager& scene_mgr = Context::Instance().SceneManagerInstance();

		has_opaque_objs = false;
		has_reflective_objs_ = false;
		has_simple_forward_objs_ = false;
		visible_scene_objs_.clear();
		uint32_t const num_scene_objs = scene_mgr.NumSceneObjects();
		for (uint32_t i = 0; i < num_scene_objs; ++ i)
		{
			SceneObjectPtr const & so = scene_mgr.GetSceneObject(i);
			if ((0 == (so->Attrib() & SceneObject::SOA_Overlay)) && so->Visible())
			{
				visible_scene_objs_.push_back(so.get());

				has_opaque_objs = true;

				if (so->TransparencyBackFace())
				{
					has_transparency_back_objs = true;
				}
				if (so->TransparencyFrontFace())
				{
					has_transparency_front_objs = true;
				}
				if (so->Reflection())
				{
					has_reflective_objs_ = true;
				}
				if (so->SimpleForward())
				{
					has_simple_forward_objs_ = true;
				}
			}
		}
	}

	void DeferredRenderingLayer::BuildPassScanList(bool has_opaque_objs, bool has_transparency_back_objs, bool has_transparency_front_objs)
	{
		pass_scaned_.clear();
		for (uint32_t vpi = 0; vpi < viewports_.size(); ++ vpi)
		{
			PerViewport& pvp = viewports_[vpi];
			if (pvp.attrib & VPAM_Enabled)
			{
				pvp.g_buffer_enables[Opaque_GBuffer] = (pvp.attrib & VPAM_NoOpaque) ? false : has_opaque_objs;
				pvp.g_buffer_enables[TransparencyBack_GBuffer] = (pvp.attrib & VPAM_NoTransparencyBack) ? false : has_transparency_back_objs;
				pvp.g_buffer_enables[TransparencyFront_GBuffer] = (pvp.attrib & VPAM_NoTransparencyFront) ? false : has_transparency_front_objs;

				pvp.light_visibles.assign(lights_.size(), true);

				for (uint32_t i = 0; i < pvp.g_buffers.size(); ++ i)
				{
					if (pvp.g_buffer_enables[i])
					{
						this->AppendGBufferPassScanCode(vpi, i);
					}
				}
			}
		}
		for (uint32_t i = 0; i < lights_.size(); ++ i)
		{
			LightSourcePtr const & light = lights_[i];
			if (light->Enabled())
			{
				this->AppendShadowPassScanCode(i);
			}
		}
		for (uint32_t vpi = 0; vpi < viewports_.size(); ++ vpi)
		{
			PerViewport& pvp = viewports_[vpi];
			if (pvp.attrib & VPAM_Enabled)
			{
				for (uint32_t i = 0; i < lights_.size(); ++ i)
				{
					LightSourcePtr const & light = lights_[i];
					if (light->Enabled())
					{
						this->AppendLightingPassScanCode(vpi, i);
					}
				}
				this->AppendShadingPassScanCode(vpi);
			}
		}
	}

	void DeferredRenderingLayer::AppendGBufferPassScanCode(uint32_t vp_index, uint32_t g_buffer_index)
	{
		PassTargetBuffer ptb = static_cast<PassTargetBuffer>(g_buffer_index);

		if (!depth_texture_support_)
		{
			pass_scaned_.push_back(this->ComposePassScanCode(vp_index,
				ComposePassType(PRT_None, ptb, PC_Depth), 0, 0));
		}

		if (mrt_g_buffer_support_)
		{
			pass_scaned_.push_back(this->ComposePassScanCode(vp_index,
				ComposePassType(PRT_MRT, ptb, PC_GBuffer), 0, 0));
			pass_scaned_.push_back(this->ComposePassScanCode(vp_index,
				ComposePassType(PRT_MRT, ptb, PC_GBuffer), 0, 1));
		}
		else
		{
			pass_scaned_.push_back(this->ComposePassScanCode(vp_index,
				ComposePassType(PRT_RT0, ptb, PC_GBuffer), 0, 0));
			pass_scaned_.push_back(this->ComposePassScanCode(vp_index,
				ComposePassType(PRT_RT0, ptb, PC_GBuffer), 0, 1));
			pass_scaned_.push_back(this->ComposePassScanCode(vp_index,
				ComposePassType(PRT_RT1, ptb, PC_GBuffer), 0, 0));
			pass_scaned_.push_back(this->ComposePassScanCode(vp_index,
				ComposePassType(PRT_RT1, ptb, PC_GBuffer), 0, 1));
		}
	}

	void DeferredRenderingLayer::AppendShadowPassScanCode(uint32_t light_index)
	{
		PassType shadow_pt;
		if (depth_texture_support_)
		{
			shadow_pt = PT_GenShadowMap;
		}
		else
		{
			shadow_pt = PT_GenShadowMapWODepthTexture;
		}

		LightSourcePtr const & light = lights_[light_index];
		LightType type = light->Type();
		int32_t attr = light->Attrib();
		switch (type)
		{
		case LT_Spot:
			{
				int sm_seq;
				if (attr & LSA_IndirectLighting)
				{
					if (rsm_buffer_ && (illum_ != 1))
					{
						sm_seq = 2;
						shadow_pt = PT_GenReflectiveShadowMap;
					}
					else
					{
						sm_seq = 1;
					}
				}
				else
				{
					if (0 == (attr & LSA_NoShadow))
					{
						sm_seq = 1;
					}
					else
					{
						sm_seq = 0;
					}
				}

				if (sm_seq != 0)
				{
					pass_scaned_.push_back(this->ComposePassScanCode(0, shadow_pt, light_index, 0));
					pass_scaned_.push_back(this->ComposePassScanCode(0, shadow_pt, light_index, 1));
				}
			}
			break;

		case LT_Point:
			if (0 == (attr & LSA_NoShadow))
			{
				for (int j = 0; j < 7; ++ j)
				{
					pass_scaned_.push_back(this->ComposePassScanCode(0, shadow_pt, light_index, j));
				}
			}
			break;

		default:
			if (0 == (attr & LSA_NoShadow))
			{							
				pass_scaned_.push_back(this->ComposePassScanCode(0, shadow_pt, light_index, 0));
			}
			break;
		}
	}

	void DeferredRenderingLayer::AppendLightingPassScanCode(uint32_t vp_index, uint32_t light_index)
	{
		SceneManager& scene_mgr = Context::Instance().SceneManagerInstance();

		PerViewport& pvp = viewports_[vp_index];
		LightSourcePtr const & light = lights_[light_index];

		int type = light->Type();
		int32_t attr = light->Attrib();
		switch (type)
		{
		case LT_Spot:
			{
				float4x4 const & inv_light_view = light->SMCamera(0)->InverseViewMatrix();

				float const scale = light->CosOuterInner().w();
				float4x4 mat = MathLib::scaling(scale * light_scale_, scale * light_scale_, light_scale_);
				float4x4 light_model = mat * inv_light_view;

				if (scene_mgr.OBBVisible(MathLib::transform_obb(cone_obb_, light_model)))
				{
					if (attr & LSA_IndirectLighting)
					{
						if (rsm_buffer_ && (illum_ != 1))
						{
							pass_scaned_.push_back(this->ComposePassScanCode(vp_index, PT_IndirectLighting, light_index, 0));
						}
					}
					pass_scaned_.push_back(this->ComposePassScanCode(vp_index, PT_Lighting, light_index, 0));
				}
				else
				{
					pvp.light_visibles[light_index] = false;
				}
			}
			break;

		case LT_Point:
			{
				float3 const & p = light->Position();
									
				float4x4 light_model = MathLib::scaling(light_scale_, light_scale_, light_scale_)
					* MathLib::translation(p);

				if (scene_mgr.OBBVisible(MathLib::transform_obb(box_obb_, light_model)))
				{
					pass_scaned_.push_back(this->ComposePassScanCode(vp_index, PT_Lighting, light_index, 0));
				}
				else
				{
					pvp.light_visibles[light_index] = false;
				}
			}
			break;

		default:
			pass_scaned_.push_back(this->ComposePassScanCode(vp_index, PT_Lighting, light_index, 0));
			break;
		}
	}

	void DeferredRenderingLayer::AppendShadingPassScanCode(uint32_t vp_index)
	{
		PerViewport& pvp = viewports_[vp_index];

		pass_scaned_.push_back(this->ComposePassScanCode(vp_index, PT_OpaqueShading, 0, 0));
		pass_scaned_.push_back(this->ComposePassScanCode(vp_index, PT_OpaqueSpecialShading, 0, 0));
		if (pvp.g_buffer_enables[TransparencyBack_GBuffer])
		{
			pass_scaned_.push_back(this->ComposePassScanCode(vp_index, PT_TransparencyBackShading, 0, 0));
			pass_scaned_.push_back(this->ComposePassScanCode(vp_index, PT_TransparencyBackSpecialShading, 0, 0));
		}
		if (pvp.g_buffer_enables[TransparencyFront_GBuffer])
		{
			pass_scaned_.push_back(this->ComposePassScanCode(vp_index, PT_TransparencyFrontShading, 0, 0));
			pass_scaned_.push_back(this->ComposePassScanCode(vp_index, PT_TransparencyFrontSpecialShading, 0, 0));
		}
		pass_scaned_.push_back(this->ComposePassScanCode(vp_index, PT_OpaqueSpecialShading, 0, 1));
	}

	void DeferredRenderingLayer::PreparePVP(PerViewport& pvp)
	{
		CameraPtr const & camera = pvp.frame_buffer->GetViewport()->camera;
		pvp.inv_view = camera->InverseViewMatrix();
		pvp.inv_proj = camera->InverseProjMatrix();
		pvp.proj_to_prev = pvp.inv_proj * pvp.inv_view * pvp.view * pvp.proj;
		pvp.view = camera->ViewMatrix();
		pvp.proj = camera->ProjMatrix();
		pvp.depth_near_far_invfar = float3(camera->NearPlane(), camera->FarPlane(), 1 / camera->FarPlane());
	}

	void DeferredRenderingLayer::GenerateDepthBuffer(PerViewport const & pvp, uint32_t g_buffer_index)
	{
		RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();

		CameraPtr const & camera = pvp.frame_buffer->GetViewport()->camera;
		pvp.pre_depth_buffers[g_buffer_index]->GetViewport()->camera = camera;

		re.BindFrameBuffer(pvp.pre_depth_buffers[g_buffer_index]);

		float depth = (TransparencyBack_GBuffer == g_buffer_index) ? 0.0f : 1.0f;
		int32_t stencil = (Opaque_GBuffer == g_buffer_index) ? 0 : 128;
		re.CurFrameBuffer()->Clear(FrameBuffer::CBM_Color | FrameBuffer::CBM_Depth | FrameBuffer::CBM_Stencil, Color(0, 0, 0, 0), depth, stencil);
	}

	void DeferredRenderingLayer::GenerateGBuffer(PerViewport const & pvp, uint32_t g_buffer_index)
	{
		RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();

		CameraPtr const & camera = pvp.frame_buffer->GetViewport()->camera;
		pvp.g_buffers[g_buffer_index]->GetViewport()->camera = camera;
		if (!mrt_g_buffer_support_)
		{
			pvp.g_buffers_rt1[g_buffer_index]->GetViewport()->camera = camera;
		}
		pvp.lighting_buffers[g_buffer_index]->GetViewport()->camera = camera;
		pvp.shading_buffers[g_buffer_index]->GetViewport()->camera = camera;
		pvp.curr_merged_shading_buffers[g_buffer_index]->GetViewport()->camera = camera;
		pvp.curr_merged_depth_buffers[g_buffer_index]->GetViewport()->camera = camera;
		pvp.prev_merged_shading_buffers[g_buffer_index]->GetViewport()->camera = camera;
		pvp.prev_merged_depth_buffers[g_buffer_index]->GetViewport()->camera = camera;

		re.BindFrameBuffer(pvp.g_buffers[g_buffer_index]);

		float depth = (TransparencyBack_GBuffer == g_buffer_index) ? 0.0f : 1.0f;
		int32_t stencil = (Opaque_GBuffer == g_buffer_index) ? 0 : 128;
		re.CurFrameBuffer()->Clear(FrameBuffer::CBM_Color | FrameBuffer::CBM_Depth | FrameBuffer::CBM_Stencil, Color(0, 0, 0, 0), depth, stencil);
	}

	void DeferredRenderingLayer::PostGenerateGBuffer(PerViewport const & pvp, uint32_t g_buffer_index)
	{
		pvp.g_buffer_rt0_texs[g_buffer_index]->BuildMipSubLevels();

		if (depth_texture_support_)
		{
			depth_to_linear_pp_->InputPin(0, pvp.g_buffer_ds_texs[g_buffer_index]);
			depth_to_linear_pp_->OutputPin(0, pvp.g_buffer_depth_texs[g_buffer_index]);
			depth_to_linear_pp_->Apply();
		}

		pvp.g_buffer_depth_texs[g_buffer_index]->BuildMipSubLevels();
	}

	void DeferredRenderingLayer::RenderDecals(PerViewport const & pvp, PassType pass_type)
	{
		RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();

		re.BindFrameBuffer((PT_OpaqueGBufferRT1 == pass_type) ? pvp.g_buffers_rt1[Opaque_GBuffer]
			: pvp.g_buffers[Opaque_GBuffer]);
		typedef KLAYGE_DECLTYPE(decals_) DecalsType;
		KLAYGE_FOREACH(DecalsType::reference de, decals_)
		{
			de->Pass(pass_type);
			de->Render();
		}
	}

	void DeferredRenderingLayer::PrepareLightCamera(PerViewport const & pvp,
		LightSourcePtr const & light, int32_t index_in_pass, PassType pass_type)
	{
		LightType const type = light->Type();

		switch (type)
		{
		case LT_Point:
		case LT_Spot:
			{
				CameraPtr sm_camera;
				float3 dir_es(0, 0, 0);
				if (LT_Spot == type)
				{
					dir_es = MathLib::transform_normal(light->Direction(), pvp.view);
					sm_camera = light->SMCamera(0);
				}
				else
				{
					int32_t face = std::min(index_in_pass, 5);
					std::pair<float3, float3> ad = CubeMapViewVector<float>(static_cast<Texture::CubeFaces>(face));
					dir_es = MathLib::transform_normal(MathLib::transform_quat(ad.first, light->Rotation()), pvp.view);
					sm_camera = light->SMCamera(face);
				}
				float4 light_dir_es_actived = float4(dir_es.x(), dir_es.y(), dir_es.z(), 0);

				sm_buffer_->GetViewport()->camera = sm_camera;

				*light_view_proj_param_ = pvp.inv_view * sm_camera->ViewProjMatrix();

				float4x4 light_to_view = sm_camera->InverseViewMatrix() * pvp.view;
				float4x4 light_to_proj = light_to_view * pvp.proj;

				if (depth_texture_support_ && ((PT_GenShadowMap == pass_type) || (PT_GenReflectiveShadowMap == pass_type)))
				{
					float q = sm_camera->FarPlane() / (sm_camera->FarPlane() - sm_camera->NearPlane());
					float2 near_q(sm_camera->NearPlane() * q, q);
					depth_to_vsm_pp_->SetParam(0, near_q);

					float4x4 inv_sm_proj = sm_camera->InverseProjMatrix();
					depth_to_vsm_pp_->SetParam(1, inv_sm_proj);
				}

				float3 const & p = light->Position();
				float3 loc_es = MathLib::transform_coord(p, pvp.view);
				float4 light_pos_es_actived = float4(loc_es.x(), loc_es.y(), loc_es.z(), 1);

				switch (type)
				{
				case LT_Spot:
					{
						light_pos_es_actived.w() = light->CosOuterInner().x();
						light_dir_es_actived.w() = light->CosOuterInner().y();

						float const scale = light->CosOuterInner().w();
						float4x4 light_model = MathLib::scaling(scale * light_scale_, scale * light_scale_, light_scale_);
						*light_volume_mv_param_ = light_model * light_to_view;
						*light_volume_mvp_param_ = light_model * light_to_proj;
					}
					break;

				case LT_Point:
					if (PT_Lighting == pass_type)
					{
						float4x4 light_model = MathLib::scaling(light_scale_, light_scale_, light_scale_)
							* MathLib::to_matrix(light->Rotation()) * MathLib::translation(p);
						*light_volume_mv_param_ = light_model * pvp.view;
						*light_volume_mvp_param_ = light_model * pvp.view * pvp.proj;
						*view_to_light_model_param_ = pvp.inv_view * MathLib::inverse(light_model);
					}
					else
					{
						*light_volume_mv_param_ = light_to_view;
						*light_volume_mvp_param_ = light_to_proj;
					}
					break;

				default:
					BOOST_ASSERT(false);
					break;
				}
					
				*light_pos_es_param_ = light_pos_es_actived;
				*light_dir_es_param_ = light_dir_es_actived;
			}
			break;

		case LT_Directional:
			{
				float3 dir_es = MathLib::transform_normal(light->Direction(), pvp.view);
				*light_dir_es_param_ = float4(dir_es.x(), dir_es.y(), dir_es.z(), 0);
			}
			*light_volume_mv_param_ = pvp.inv_proj;
			*light_volume_mvp_param_ = float4x4::Identity();
			break;

		case LT_Ambient:
			{
				float3 dir_es = MathLib::transform_normal(float3(0, 1, 0), pvp.view);
				*light_dir_es_param_ = float4(dir_es.x(), dir_es.y(), dir_es.z(), 0);
			}
			*light_volume_mv_param_ = pvp.inv_proj;
			*light_volume_mvp_param_ = float4x4::Identity();
			break;

		default:
			BOOST_ASSERT(false);
			break;
		}
	}

	void DeferredRenderingLayer::PostGenerateShadowMap(int32_t org_no, int32_t index_in_pass)
	{
		if (depth_texture_support_)
		{
			depth_to_vsm_pp_->Apply();
		}

		sm_filter_pp_->InputPin(0, sm_tex_);
		if (LT_Point == lights_[org_no]->Type())
		{
			sm_filter_pp_->OutputPin(0, blur_sm_cube_texs_[sm_light_indices_[org_no]], 0, 0, index_in_pass - 1);
		}
		else
		{
			sm_filter_pp_->OutputPin(0, blur_sm_2d_texs_[sm_light_indices_[org_no]]);
		}
		sm_filter_pp_->Apply();
	}

	void DeferredRenderingLayer::UpdateShadowing(PerViewport const & pvp, uint32_t g_buffer_index,
		int32_t org_no)
	{
		RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
		
		LightSourcePtr const & light = lights_[org_no];
		LightType type = light->Type();

		int32_t light_index = sm_light_indices_[org_no];
		if ((light_index >= 0) && (0 == (light->Attrib() & LSA_NoShadow)))
		{
			switch (type)
			{
			case LT_Spot:
				*shadow_map_2d_tex_param_ = blur_sm_2d_texs_[light_index];
				break;

			case LT_Point:
				*shadow_map_cube_tex_param_ = blur_sm_cube_texs_[light_index];
				break;

			default:
				break;
			}
		}

		re.BindFrameBuffer(pvp.shadowing_buffer);
		re.CurFrameBuffer()->Attached(FrameBuffer::ATT_Color0)->ClearColor(Color(1, 1, 1, 1));

		*g_buffer_tex_param_ = pvp.g_buffer_rt0_texs[g_buffer_index];
		*depth_tex_param_ = pvp.g_buffer_depth_texs[g_buffer_index];
		*inv_width_height_param_ = float2(1.0f / pvp.frame_buffer->GetViewport()->width,
			1.0f / pvp.frame_buffer->GetViewport()->height);
		*shadowing_tex_param_ = pvp.shadowing_tex;

		if (0 == (light->Attrib() & LSA_NoShadow))
		{
			re.Render(*technique_shadows_[type], *light_volume_rl_[type]);
		}
	}

	void DeferredRenderingLayer::UpdateLighting(PerViewport const & pvp, uint32_t g_buffer_index,
		LightType type)
	{
		RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();

		re.BindFrameBuffer(pvp.lighting_buffers[g_buffer_index]);
		// Clear stencil to 0 with write mask
		re.Render(*technique_clear_stencil_, *rl_quad_);

		RenderLayoutPtr const & rl = light_volume_rl_[type];

		if ((LT_Point == type) || (LT_Spot == type))
		{
			re.Render(*technique_light_stencil_, *rl);
		}

		re.Render(*technique_lights_[type], *rl);
	}

	void DeferredRenderingLayer::UpdateIndirectAndSSVO(PerViewport const & pvp)
	{
		if ((indirect_lighting_enabled_ && !(pvp.attrib & VPAM_NoGI)) && (illum_ != 1))
		{
			pvp.il_layer->CalcIndirectLighting(pvp.prev_merged_shading_tex, pvp.proj_to_prev);
			this->AccumulateToLightingTex(pvp);
		}

		if (pvp.ssvo_enabled && !(pvp.attrib & VPAM_NoSSVO))
		{
			ssvo_pp_->InputPin(0, pvp.g_buffer_rt0_texs[Opaque_GBuffer]);
			ssvo_pp_->InputPin(1, pvp.g_buffer_depth_texs[Opaque_GBuffer]);
			ssvo_pp_->OutputPin(0, pvp.small_ssvo_tex);
			ssvo_pp_->Apply();

			ssvo_blur_pp_->InputPin(0, pvp.small_ssvo_tex);
			ssvo_blur_pp_->InputPin(1, pvp.g_buffer_depth_texs[Opaque_GBuffer]);
			ssvo_blur_pp_->OutputPin(0, pvp.lighting_texs[Opaque_GBuffer]);
			ssvo_blur_pp_->Apply();
		}
	}

	void DeferredRenderingLayer::UpdateShading(PerViewport const & pvp, uint32_t g_buffer_index)
	{
		RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
		
		re.BindFrameBuffer(pvp.shading_buffers[g_buffer_index]);
		*g_buffer_tex_param_ = pvp.g_buffer_rt0_texs[g_buffer_index];
		*g_buffer_1_tex_param_ = pvp.g_buffer_rt1_texs[g_buffer_index];
		*depth_tex_param_ = pvp.g_buffer_depth_texs[g_buffer_index];
		*lighting_tex_param_ = pvp.lighting_texs[g_buffer_index];
		*light_volume_mv_param_ = pvp.inv_proj;

		if (Opaque_GBuffer == g_buffer_index)
		{
			re.Render(*technique_no_lighting_, *rl_quad_);
		}
		else
		{
			re.CurFrameBuffer()->Attached(FrameBuffer::ATT_Color0)->ClearColor(Color(0, 0, 0, 0));
		}
		re.Render(*technique_shading_, *rl_quad_);
	}

	void DeferredRenderingLayer::AddSSR(PerViewport const & pvp)
	{
		RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();

		for (size_t i = 0; i < pvp.g_buffers.size(); ++ i)
		{
			if (pvp.g_buffer_enables[i])
			{
				re.BindFrameBuffer(pvp.shading_buffers[i]);
				ssr_pp_->InputPin(0, pvp.g_buffer_rt0_texs[Opaque_GBuffer]);
				ssr_pp_->InputPin(1, pvp.g_buffer_rt1_texs[Opaque_GBuffer]);
				ssr_pp_->InputPin(2, pvp.g_buffer_depth_texs[Opaque_GBuffer]);
				ssr_pp_->InputPin(3, pvp.prev_merged_shading_tex);
				ssr_pp_->InputPin(4, pvp.g_buffer_depth_texs[i]);
				ssr_pp_->Apply();
			}
		}
	}

	void DeferredRenderingLayer::AddAtmospheric(PerViewport const & pvp)
	{
		RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();

		for (size_t i = 0; i < pvp.g_buffers.size(); ++ i)
		{
			if (pvp.g_buffer_enables[i])
			{
				re.BindFrameBuffer(pvp.shading_buffers[i]);
				atmospheric_pp_->SetParam(0, pvp.inv_proj);
				atmospheric_pp_->InputPin(0, pvp.g_buffer_depth_texs[i]);
				atmospheric_pp_->Render();
			}
		}
	}

	void DeferredRenderingLayer::MergeShadingAndDepth(PerViewport const & pvp)
	{
		RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();

		for (size_t i = 0; i < pvp.g_buffers.size(); ++ i)
		{
			if (pvp.g_buffer_enables[i])
			{
				re.BindFrameBuffer(pvp.curr_merged_shading_buffers[i]);
				*shading_tex_param_ = pvp.shading_texs[i];
				re.Render(*technique_merge_shadings_[i != 0], *rl_quad_);
			}
		}

		for (size_t i = 0; i < pvp.g_buffers.size(); ++ i)
		{
			if (pvp.g_buffer_enables[i])
			{
				re.BindFrameBuffer(pvp.curr_merged_depth_buffers[i]);
				*depth_tex_param_ = pvp.g_buffer_depth_texs[i];
				re.Render(*technique_merge_depths_[i != 0], *rl_quad_);
			}
		}

		re.BindFrameBuffer(pvp.frame_buffer);
		{
			*depth_tex_param_ = pvp.curr_merged_depth_tex;

			CameraPtr const & camera = pvp.frame_buffer->GetViewport()->camera;
			float q = camera->FarPlane() / (camera->FarPlane() - camera->NearPlane());
			float2 near_q(camera->NearPlane() * q, q);
			*near_q_param_ = near_q;
		}
		App3DFramework& app = Context::Instance().AppInstance();
		if ((app.FrameTime() < 1.0f / 30) && taa_enabled_)
		{
			taa_pp_->InputPin(0, pvp.curr_merged_shading_tex);
			taa_pp_->InputPin(1, pvp.prev_merged_shading_tex);
			taa_pp_->Render();
			re.Render(*technique_copy_depth_, *rl_quad_);
		}
		else
		{
			*shading_tex_param_ = pvp.curr_merged_shading_tex;
			re.Render(*technique_copy_shading_depth_, *rl_quad_);
		}
	}

	void DeferredRenderingLayer::SetupViewportGI(uint32_t vp)
	{
		PerViewport& pvp = viewports_[vp];
		if (pvp.ssgi_enable)
		{
			pvp.il_layer = MakeSharedPtr<SSGILayer>();
		}
		else
		{
			if (rsm_buffer_)
			{
				pvp.il_layer = MakeSharedPtr<MultiResSILLayer>();
			}
			else
			{
				pvp.il_layer.reset();
			}
		}

		if (pvp.il_layer && pvp.g_buffer_rt0_texs[Opaque_GBuffer] && rsm_texs_[0])
		{
			pvp.il_layer->GBuffer(pvp.g_buffer_rt0_texs[Opaque_GBuffer], pvp.g_buffer_rt0_texs[Opaque_GBuffer],
				pvp.g_buffer_depth_texs[Opaque_GBuffer]);
			pvp.il_layer->RSM(rsm_texs_[0], rsm_texs_[1], sm_tex_);
		}
	}

	void DeferredRenderingLayer::AccumulateToLightingTex(PerViewport const & pvp)
	{
		PostProcessPtr const & copy_to_light_buffer_pp = (0 == illum_) ? copy_to_light_buffer_pp_ : copy_to_light_buffer_i_pp_;
		copy_to_light_buffer_pp->SetParam(0, indirect_scale_ * 256 / VPL_COUNT);
		copy_to_light_buffer_pp->SetParam(1, float2(1.0f / pvp.g_buffer_rt0_texs[Opaque_GBuffer]->Width(0), 1.0f / pvp.g_buffer_rt0_texs[Opaque_GBuffer]->Height(0)));
		copy_to_light_buffer_pp->SetParam(2, pvp.depth_near_far_invfar);
		copy_to_light_buffer_pp->SetParam(3, pvp.inv_proj);
		copy_to_light_buffer_pp->InputPin(0, pvp.il_layer->IndirectLightingTex());
		copy_to_light_buffer_pp->InputPin(1, pvp.g_buffer_rt0_texs[Opaque_GBuffer]);
		copy_to_light_buffer_pp->InputPin(2, pvp.g_buffer_depth_texs[Opaque_GBuffer]);
		copy_to_light_buffer_pp->OutputPin(0, pvp.lighting_texs[Opaque_GBuffer]);
		copy_to_light_buffer_pp->Apply();
	}

	void DeferredRenderingLayer::DisplayIllum(int illum)
	{
		illum_ = illum;
	}

	void DeferredRenderingLayer::IndirectScale(float scale)
	{
		indirect_scale_ = scale;
	}

	uint32_t DeferredRenderingLayer::ComposePassScanCode(uint32_t vp_index, PassType pass_type, int32_t org_no, int32_t index_in_pass)
	{
		return (vp_index << 28) | (pass_type << 18) | (org_no << 6) | index_in_pass;
	}

	void DeferredRenderingLayer::DecomposePassScanCode(uint32_t& vp_index, PassType& pass_type, int32_t& org_no, int32_t& index_in_pass, uint32_t code)
	{
		vp_index = code >> 28;				//  4 bits, [31 - 28]
		pass_type = static_cast<PassType>((code >> 18) & 0x03FF);	//  10 bits, [27 - 18]
		org_no = (code >> 6) & 0x0FFF;		// 12 bits, [17 - 6]
		index_in_pass = (code >> 0) & 0x3F;		//  6 bits, [5 -  0]
	}
}
