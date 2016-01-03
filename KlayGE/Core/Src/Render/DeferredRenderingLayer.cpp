/**
 * @file DeferredRenderingLayer.cpp
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
#include <KlayGE/Camera.hpp>
#include <KlayGE/Mesh.hpp>
#include <KlayGE/SSVOPostProcess.hpp>
#include <KlayGE/SSRPostProcess.hpp>
#include <KlayGE/SSSBlur.hpp>
#include <KlayGE/PerfProfiler.hpp>

#include <boost/lexical_cast.hpp>

#include <KlayGE/DeferredRenderingLayer.hpp>

namespace KlayGE
{
	int const SM_SIZE = 512;
	
	int const MAX_IL_MIPMAP_LEVELS = 3;

	int const MAX_RSM_MIPMAP_LEVELS = 7; // (log(512)-log(4))/log(2) + 1
	int const BEGIN_RSM_SAMPLING_LIGHT_LEVEL = 5;
	int const SAMPLE_LEVEL_CNT = MAX_RSM_MIPMAP_LEVELS - BEGIN_RSM_SAMPLING_LIGHT_LEVEL;
	int const VPL_COUNT = 64 * ((1UL << (SAMPLE_LEVEL_CNT * 2)) - 1) / (4 - 1);

	float const ESM_SCALE_FACTOR = 300.0f;

#if DEFAULT_DEFERRED == LIGHT_INDEXED_DEFERRED
	uint32_t const TILE_SIZE = 32;
#endif

	template <typename T>
	void CreateConeMesh(std::vector<T>& vb, std::vector<uint16_t>& ib, uint16_t vertex_base, float radius, float height, uint16_t n)
	{
		for (int i = 0; i < n; ++ i)
		{
			T v;
			v.x() = v.y() = v.z() = 0;
			vb.push_back(v);
		}

		float outer_radius = radius / cos(PI / n);
		for (int i = 0; i < n; ++ i)
		{
			T v;
			float angle = i * 2 * PI / n;
			v.x() = outer_radius * cos(angle);
			v.y() = outer_radius * sin(angle);
			v.z() = height;
			vb.push_back(v);
		}

		{
			T v;
			v.x() = v.y() = 0;
			v.z() = height;
			vb.push_back(v);
		}

		for (int i = 0; i < n; ++ i)
		{
			vb.push_back(vb[vertex_base + n + i]);
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
			T v;
			v.x() = v.y() = v.z() = 0;
			vb.push_back(v);
		}

		float outer_radius = radius * sqrt(2.0f);
		{
			T v;
			v.x() = -outer_radius;
			v.y() = -outer_radius;
			v.z() = height;
			vb.push_back(v);
		}
		{
			T v;
			v.x() = +outer_radius;
			v.y() = -outer_radius;
			v.z() = height;
			vb.push_back(v);
		}
		{
			T v;
			v.x() = +outer_radius;
			v.y() = +outer_radius;
			v.z() = height;
			vb.push_back(v);
		}
		{
			T v;
			v.x() = -outer_radius;
			v.y() = +outer_radius;
			v.z() = height;
			vb.push_back(v);
		}

		{
			T v;
			v.x() = v.y() = 0;
			v.z() = height;
			vb.push_back(v);
		}

		for (int i = 0; i < 4; ++ i)
		{
			vb.push_back(vb[vertex_base + 4 + i]);
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
		{
			T v;
			v.x() = -half_length;
			v.y() = +half_length;
			v.z() = -half_length;
			vb.push_back(v);
		}
		{
			T v;
			v.x() = +half_length;
			v.y() = +half_length;
			v.z() = -half_length;
			vb.push_back(v);
		}
		{
			T v;
			v.x() = +half_length;
			v.y() = -half_length;
			v.z() = -half_length;
			vb.push_back(v);
		}
		{
			T v;
			v.x() = -half_length;
			v.y() = -half_length;
			v.z() = -half_length;
			vb.push_back(v);
		}

		{
			T v;
			v.x() = -half_length;
			v.y() = +half_length;
			v.z() = +half_length;
			vb.push_back(v);
		}
		{
			T v;
			v.x() = +half_length;
			v.y() = +half_length;
			v.z() = +half_length;
			vb.push_back(v);
		}
		{
			T v;
			v.x() = +half_length;
			v.y() = -half_length;
			v.z() = +half_length;
			vb.push_back(v);
		}
		{
			T v;
			v.x() = -half_length;
			v.y() = -half_length;
			v.z() = +half_length;
			vb.push_back(v);
		}

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


	class DeferredRenderingDebugPostProcess : public PostProcess
	{
	public:
		DeferredRenderingDebugPostProcess()
			: PostProcess(L"DeferredRenderingDebug")
		{
			input_pins_.emplace_back("g_buffer_tex", TexturePtr());
			input_pins_.emplace_back("g_buffer_1_tex", TexturePtr());
			input_pins_.emplace_back("depth_tex", TexturePtr());
			input_pins_.emplace_back("lighting_tex", TexturePtr());
			input_pins_.emplace_back("ssvo_tex", TexturePtr());

			output_pins_.emplace_back("out_tex", TexturePtr());

			this->Technique(SyncLoadRenderEffect("DeferredRenderingDebug.fxml")->TechniqueByName("ShowPosition"));
		}

		void Display(DeferredRenderingLayer::DisplayType display_type)
		{
			switch (display_type)
			{
			case DeferredRenderingLayer::DT_Final:
				break;

			case DeferredRenderingLayer::DT_Position:
				technique_ = technique_->Effect().TechniqueByName("ShowPosition");
				break;

			case DeferredRenderingLayer::DT_Normal:
				technique_ = technique_->Effect().TechniqueByName("ShowNormal");
				break;

			case DeferredRenderingLayer::DT_Depth:
				technique_ = technique_->Effect().TechniqueByName("ShowDepth");
				break;

			case DeferredRenderingLayer::DT_Diffuse:
				technique_ = technique_->Effect().TechniqueByName("ShowDiffuse");
				break;

			case DeferredRenderingLayer::DT_Specular:
				technique_ = technique_->Effect().TechniqueByName("ShowSpecular");
				break;

			case DeferredRenderingLayer::DT_Shininess:
				technique_ = technique_->Effect().TechniqueByName("ShowShininess");
				break;

			case DeferredRenderingLayer::DT_Edge:
				break;

			case DeferredRenderingLayer::DT_SSVO:
				technique_ = technique_->Effect().TechniqueByName("ShowSSVO");
				break;

#if DEFAULT_DEFERRED == TRIDITIONAL_DEFERRED
			case DeferredRenderingLayer::DT_DiffuseLighting:
				technique_ = technique_->Effect().TechniqueByName("ShowDiffuseLighting");
				break;

			case DeferredRenderingLayer::DT_SpecularLighting:
				technique_ = technique_->Effect().TechniqueByName("ShowSpecularLighting");
				break;
#endif

			default:
				break;
			}
		}

		void OnRenderBegin() override
		{
			PostProcess::OnRenderBegin();

			Camera const & camera = Context::Instance().AppInstance().ActiveCamera();
			*(technique_->Effect().ParameterByName("inv_proj")) = camera.InverseProjMatrix();
			*(technique_->Effect().ParameterByName("depth_near_far_invfar")) = float3(camera.NearPlane(), camera.FarPlane(), 1 / camera.FarPlane());
		}
	};


	DeferredRenderingLayer::DeferredRenderingLayer()
		: active_viewport_(0),
			sss_enabled_(true), translucency_enabled_(true),
			ssr_enabled_(true), taa_enabled_(true),
			light_scale_(1), illum_(0), indirect_scale_(1.0f),
			curr_cascade_index_(-1), force_line_mode_(false),
			dr_debug_pp_(MakeSharedPtr<DeferredRenderingDebugPostProcess>()),
			display_type_(DT_Final)
	{
		RenderFactory& rf = Context::Instance().RenderFactoryInstance();
		RenderEngine& re = rf.RenderEngineInstance();
		RenderDeviceCaps const & caps = re.DeviceCaps();

		ElementFormat ds_fmt;
		if (caps.rendertarget_format_support(EF_D24S8, 1, 0))
		{
			ds_fmt = EF_D24S8;
		}
		else
		{
			BOOST_ASSERT(caps.rendertarget_format_support(EF_D16, 1, 0));
			ds_fmt = EF_D16;
		}

		mrt_g_buffer_support_ = (caps.max_simultaneous_rts > 1);
		depth_texture_support_ = caps.depth_texture_support;
		tex_array_support_ = (caps.max_texture_array_length >= 4);

#if DEFAULT_DEFERRED == LIGHT_INDEXED_DEFERRED
		if ((caps.max_shader_model >= ShaderModel(5, 0)) && (caps.cs_support))
		{
			static_assert(32 == TILE_SIZE, "TILE_SIZE must be 32.");

			cs_tbdr_ = true;
		}
		else
		{
			cs_tbdr_ = false;
		}
#endif

		for (size_t vpi = 0; vpi < viewports_.size(); ++ vpi)
		{
			PerViewport& pvp = viewports_[vpi];
			if (!depth_texture_support_)
			{
				pvp.pre_depth_fb = rf.MakeFrameBuffer();
			}
			pvp.g_buffer = rf.MakeFrameBuffer();
			if (!mrt_g_buffer_support_)
			{
				pvp.g_buffer_rt1 = rf.MakeFrameBuffer();
			}
#if DEFAULT_DEFERRED == TRIDITIONAL_DEFERRED
			pvp.lighting_fb = rf.MakeFrameBuffer();
#endif
			pvp.shading_fb = rf.MakeFrameBuffer();
			pvp.shadowing_fb = rf.MakeFrameBuffer();
			pvp.projective_shadowing_fb = rf.MakeFrameBuffer();
			pvp.reflection_fb = rf.MakeFrameBuffer();
			pvp.curr_merged_shading_fb = rf.MakeFrameBuffer();
			pvp.curr_merged_depth_fb = rf.MakeFrameBuffer();
			pvp.prev_merged_shading_fb = rf.MakeFrameBuffer();
			pvp.prev_merged_depth_fb = rf.MakeFrameBuffer();
#if DEFAULT_DEFERRED == LIGHT_INDEXED_DEFERRED
			if (cs_tbdr_)
			{
				pvp.lighting_mask_fb = rf.MakeFrameBuffer();
			}
			else
			{
				pvp.light_index_fb = rf.MakeFrameBuffer();
			}
#endif
		}

		{
			rl_cone_ = rf.MakeRenderLayout();
			rl_cone_->TopologyType(RenderLayout::TT_TriangleList);

			std::vector<float3> pos;
			std::vector<uint16_t> index;
			CreateConeMesh(pos, index, 0, 100.0f, 100.0f, 12);
			cone_aabb_ = MathLib::compute_aabbox(pos.begin(), pos.end());

			rl_cone_->BindVertexStream(rf.MakeVertexBuffer(BU_Static, EAH_GPU_Read | EAH_Immutable,
				static_cast<uint32_t>(pos.size() * sizeof(pos[0])), &pos[0]), 
				std::make_tuple(vertex_element(VEU_Position, 0, EF_BGR32F)));

			rl_cone_->BindIndexStream(rf.MakeIndexBuffer(BU_Static, EAH_GPU_Read | EAH_Immutable,
				static_cast<uint32_t>(index.size() * sizeof(index[0])), &index[0]),
				EF_R16UI);
		}
		{
			rl_pyramid_ = rf.MakeRenderLayout();
			rl_pyramid_->TopologyType(RenderLayout::TT_TriangleList);

			std::vector<float3> pos;
			std::vector<uint16_t> index;
			CreatePyramidMesh(pos, index, 0, 100.0f, 100.0f);
			pyramid_aabb_ = MathLib::compute_aabbox(pos.begin(), pos.end());

			rl_pyramid_->BindVertexStream(rf.MakeVertexBuffer(BU_Static, EAH_GPU_Read | EAH_Immutable,
				static_cast<uint32_t>(pos.size() * sizeof(pos[0])), &pos[0]),
				std::make_tuple(vertex_element(VEU_Position, 0, EF_BGR32F)));

			rl_pyramid_->BindIndexStream(rf.MakeIndexBuffer(BU_Static, EAH_GPU_Read | EAH_Immutable,
				static_cast<uint32_t>(index.size() * sizeof(index[0])), &index[0]), EF_R16UI);
		}
		{
			rl_box_ = rf.MakeRenderLayout();
			rl_box_->TopologyType(RenderLayout::TT_TriangleList);

			std::vector<float3> pos;
			std::vector<uint16_t> index;
			CreateBoxMesh(pos, index, 0, 100.0f);
			box_aabb_ = MathLib::compute_aabbox(pos.begin(), pos.end());

			rl_box_->BindVertexStream(rf.MakeVertexBuffer(BU_Static, EAH_GPU_Read | EAH_Immutable,
				static_cast<uint32_t>(pos.size() * sizeof(pos[0])), &pos[0]),
				std::make_tuple(vertex_element(VEU_Position, 0, EF_BGR32F)));

			rl_box_->BindIndexStream(rf.MakeIndexBuffer(BU_Static, EAH_GPU_Read | EAH_Immutable,
				static_cast<uint32_t>(index.size() * sizeof(index[0])), &index[0]), EF_R16UI);
		}
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
				static_cast<uint32_t>(sizeof(pos)), pos),
				std::make_tuple(vertex_element(VEU_Position, 0, EF_BGR32F)));
		}

		light_volume_rl_[LightSource::LT_Ambient] = rl_quad_;
		light_volume_rl_[LightSource::LT_Directional] = rl_quad_;
		light_volume_rl_[LightSource::LT_Point] = rl_box_;
		light_volume_rl_[LightSource::LT_Spot] = rl_cone_;
		light_volume_rl_[LightSource::LT_Sun] = rl_quad_;
		light_volume_rl_[LightSource::LT_SphereArea] = rl_box_;
		light_volume_rl_[LightSource::LT_TubeArea] = rl_box_;

		default_ambient_light_ = MakeSharedPtr<AmbientLightSource>();

		g_buffer_effect_ = SyncLoadRenderEffect("GBufferNoSkinning.fxml");
		if (caps.max_shader_model >= ShaderModel(3, 0))
		{
			g_buffer_skinning_effect_ = SyncLoadRenderEffect("GBufferSkinning128.fxml");
		}
		else
		{
			g_buffer_skinning_effect_ = SyncLoadRenderEffect("GBufferSkinning64.fxml");
		}
#if DEFAULT_DEFERRED == TRIDITIONAL_DEFERRED
		dr_effect_ = SyncLoadRenderEffect("DeferredRendering.fxml");
#elif DEFAULT_DEFERRED == LIGHT_INDEXED_DEFERRED
		if (cs_tbdr_)
		{
			light_batch_ = 1024;
			dr_effect_ = SyncLoadRenderEffect("TileBasedDeferredRendering.fxml");
		}
		else
		{
			if (caps.max_shader_model >= ShaderModel(4, 0))
			{
				light_batch_ = 32;
				dr_effect_ = SyncLoadRenderEffect("LightIndexedDeferredRendering32.fxml");
			}
			else
			{
				light_batch_ = 4;
				dr_effect_ = SyncLoadRenderEffect("LightIndexedDeferredRendering4.fxml");
			}
		}
#endif

		technique_shadows_[LightSource::LT_Point][0] = dr_effect_->TechniqueByName("DeferredShadowingPointR");
		technique_shadows_[LightSource::LT_Point][1] = dr_effect_->TechniqueByName("DeferredShadowingPointG");
		technique_shadows_[LightSource::LT_Point][2] = dr_effect_->TechniqueByName("DeferredShadowingPointB");
		technique_shadows_[LightSource::LT_Point][3] = dr_effect_->TechniqueByName("DeferredShadowingPointA");
		technique_shadows_[LightSource::LT_Point][4] = dr_effect_->TechniqueByName("DeferredShadowingPoint");
		technique_shadows_[LightSource::LT_Spot][0] = dr_effect_->TechniqueByName("DeferredShadowingSpotR");
		technique_shadows_[LightSource::LT_Spot][1] = dr_effect_->TechniqueByName("DeferredShadowingSpotG");
		technique_shadows_[LightSource::LT_Spot][2] = dr_effect_->TechniqueByName("DeferredShadowingSpotB");
		technique_shadows_[LightSource::LT_Spot][3] = dr_effect_->TechniqueByName("DeferredShadowingSpotA");
		technique_shadows_[LightSource::LT_Spot][4] = dr_effect_->TechniqueByName("DeferredShadowingSpot");
		technique_shadows_[LightSource::LT_Sun][0] = dr_effect_->TechniqueByName("DeferredShadowingSunR");
		technique_shadows_[LightSource::LT_Sun][1] = dr_effect_->TechniqueByName("DeferredShadowingSunG");
		technique_shadows_[LightSource::LT_Sun][2] = dr_effect_->TechniqueByName("DeferredShadowingSunB");
		technique_shadows_[LightSource::LT_Sun][3] = dr_effect_->TechniqueByName("DeferredShadowingSunA");
		technique_shadows_[LightSource::LT_Sun][4] = dr_effect_->TechniqueByName("DeferredShadowingSun");
		technique_shadows_[LightSource::LT_SphereArea][0] = dr_effect_->TechniqueByName("DeferredShadowingPointR");
		technique_shadows_[LightSource::LT_SphereArea][1] = dr_effect_->TechniqueByName("DeferredShadowingPointG");
		technique_shadows_[LightSource::LT_SphereArea][2] = dr_effect_->TechniqueByName("DeferredShadowingPointB");
		technique_shadows_[LightSource::LT_SphereArea][3] = dr_effect_->TechniqueByName("DeferredShadowingPointA");
		technique_shadows_[LightSource::LT_SphereArea][4] = dr_effect_->TechniqueByName("DeferredShadowingPoint");
		technique_shadows_[LightSource::LT_TubeArea][0] = dr_effect_->TechniqueByName("DeferredShadowingPointR");
		technique_shadows_[LightSource::LT_TubeArea][1] = dr_effect_->TechniqueByName("DeferredShadowingPointG");
		technique_shadows_[LightSource::LT_TubeArea][2] = dr_effect_->TechniqueByName("DeferredShadowingPointB");
		technique_shadows_[LightSource::LT_TubeArea][3] = dr_effect_->TechniqueByName("DeferredShadowingPointA");
		technique_shadows_[LightSource::LT_TubeArea][4] = dr_effect_->TechniqueByName("DeferredShadowingPoint");
#if DEFAULT_DEFERRED == TRIDITIONAL_DEFERRED
		technique_lights_[LightSource::LT_Ambient] = dr_effect_->TechniqueByName("DeferredRenderingAmbient");
		technique_lights_[LightSource::LT_Directional] = dr_effect_->TechniqueByName("DeferredRenderingDirectional");
		technique_lights_[LightSource::LT_Point] = dr_effect_->TechniqueByName("DeferredRenderingPoint");
		technique_lights_[LightSource::LT_Spot] = dr_effect_->TechniqueByName("DeferredRenderingSpot");
		technique_lights_[LightSource::LT_Sun] = dr_effect_->TechniqueByName("DeferredRenderingSun");
		technique_lights_[LightSource::LT_SphereArea] = dr_effect_->TechniqueByName("DeferredRenderingSphereArea");
		technique_lights_[LightSource::LT_TubeArea] = dr_effect_->TechniqueByName("DeferredRenderingTubeArea");
		technique_light_depth_only_ = dr_effect_->TechniqueByName("DeferredRenderingLightDepthOnly");
		technique_light_stencil_ = dr_effect_->TechniqueByName("DeferredRenderingLightStencil");
#endif
		technique_no_lighting_ = dr_effect_->TechniqueByName("NoLightingTech");
		technique_shading_ = dr_effect_->TechniqueByName("ShadingTech");
		technique_merge_shadings_[0] = dr_effect_->TechniqueByName("MergeShadingTech");
		technique_merge_shadings_[1] = dr_effect_->TechniqueByName("MergeShadingAlphaBlendTech");
		technique_merge_depths_[0] = dr_effect_->TechniqueByName("MergeDepthTech");
		technique_merge_depths_[1] = dr_effect_->TechniqueByName("MergeDepthAlphaBlendTech");
		technique_copy_shading_depth_ = dr_effect_->TechniqueByName("CopyShadingDepthTech");
		technique_copy_depth_ = dr_effect_->TechniqueByName("CopyDepthTech");
#if DEFAULT_DEFERRED == LIGHT_INDEXED_DEFERRED
		if (cs_tbdr_)
		{
			technique_tbdr_unified_ = dr_effect_->TechniqueByName("TBDRUnified");
		}
		else
		{
			technique_draw_light_index_point_ = dr_effect_->TechniqueByName("DrawLightIndexPoint");
			technique_draw_light_index_spot_ = dr_effect_->TechniqueByName("DrawLightIndexSpot");
			technique_lidr_ambient_ = dr_effect_->TechniqueByName("LIDRAmbient");
			technique_lidr_sun_ = dr_effect_->TechniqueByName("LIDRSun");
			technique_lidr_directional_ = dr_effect_->TechniqueByName("LIDRDirectional");
			technique_lidr_point_shadow_ = dr_effect_->TechniqueByName("LIDRPointShadow");
			technique_lidr_point_no_shadow_ = dr_effect_->TechniqueByName("LIDRPointNoShadow");
			technique_lidr_spot_shadow_ = dr_effect_->TechniqueByName("LIDRSpotShadow");
			technique_lidr_spot_no_shadow_ = dr_effect_->TechniqueByName("LIDRSpotNoShadow");
			technique_lidr_sphere_area_shadow_ = dr_effect_->TechniqueByName("LIDRSphereAreaShadow");
			technique_lidr_sphere_area_no_shadow_ = dr_effect_->TechniqueByName("LIDRSphereAreaNoShadow");
			technique_lidr_tube_area_shadow_ = dr_effect_->TechniqueByName("LIDRTubeAreaShadow");
			technique_lidr_tube_area_no_shadow_ = dr_effect_->TechniqueByName("LIDRTubeAreaNoShadow");
		}
#endif

		sm_fb_ = rf.MakeFrameBuffer();
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
		sm_tex_ = rf.MakeTexture2D(SM_SIZE, SM_SIZE, 1, 1, fmt, 1, 0, EAH_GPU_Read | EAH_GPU_Write, nullptr);
		sm_fb_->Attach(FrameBuffer::ATT_Color0, rf.Make2DRenderView(*sm_tex_, 0, 1, 0));
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
		sm_fb_->Attach(FrameBuffer::ATT_DepthStencil, sm_depth_view);

		csm_fb_ = rf.MakeFrameBuffer();
		csm_tex_ = rf.MakeTexture2D(SM_SIZE * 2, SM_SIZE * 2, 1, 1, fmt, 1, 0, EAH_GPU_Read | EAH_GPU_Write, nullptr);
		csm_fb_->Attach(FrameBuffer::ATT_Color0, rf.Make2DRenderView(*csm_tex_, 0, 1, 0));
		csm_fb_->Attach(FrameBuffer::ATT_DepthStencil,
			rf.Make2DDepthStencilRenderView(SM_SIZE * 2, SM_SIZE * 2, ds_fmt, 1, 0));

		for (uint32_t i = 0; i < filtered_sm_2d_texs_.size(); ++ i)
		{
			unfiltered_sm_2d_texs_[i] = rf.MakeTexture2D(SM_SIZE, SM_SIZE, 1, 1, sm_tex_->Format(), 1, 0, EAH_GPU_Read | EAH_GPU_Write, nullptr);
			filtered_sm_2d_texs_[i] = rf.MakeTexture2D(SM_SIZE, SM_SIZE, 1, 1, sm_tex_->Format(), 1, 0, EAH_GPU_Read | EAH_GPU_Write, nullptr);
		}
		for (uint32_t i = 0; i < filtered_sm_cube_texs_.size(); ++ i)
		{
			filtered_sm_cube_texs_[i] = rf.MakeTextureCube(SM_SIZE, 1, 1, sm_tex_->Format(), 1, 0, EAH_GPU_Read | EAH_GPU_Write, nullptr);
		}

		ssvo_pp_ = MakeSharedPtr<SSVOPostProcess>();
		ssvo_blur_pp_ = MakeSharedPtr<BlurPostProcess<SeparableBilateralFilterPostProcess>>(8, 1.0f,
			SyncLoadRenderEffect("SSVO.fxml")->TechniqueByName("SSVOBlurX"),
			SyncLoadRenderEffect("SSVO.fxml")->TechniqueByName("SSVOBlurY"));
		ssr_pp_ = MakeSharedPtr<SSRPostProcess>();
		taa_pp_ = SyncLoadPostProcess("TAA.ppml", "taa");

		sss_blur_pp_ = MakeSharedPtr<SSSBlurPP>();
		sss_blur_pp_->SetParam(0, 1.0f);
		sss_blur_pp_->SetParam(1, 1.0f);

		translucency_pp_ = SyncLoadPostProcess("Translucency.ppml", "Translucency");
		translucency_pp_->SetParam(6, 100.0f);

		if (depth_texture_support_ && mrt_g_buffer_support_ && caps.fp_color_support)
		{
			rsm_fb_ = rf.MakeFrameBuffer();

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
			rsm_fb_->Attach(FrameBuffer::ATT_Color0, rf.Make2DRenderView(*rsm_texs_[0], 0, 1, 0)); // normal (light space)
			rsm_fb_->Attach(FrameBuffer::ATT_Color1, rf.Make2DRenderView(*rsm_texs_[1], 0, 1, 0)); // albedo
			rsm_fb_->Attach(FrameBuffer::ATT_DepthStencil, sm_depth_view);
			
			copy_to_light_buffer_pp_ = SyncLoadPostProcess("Copy2LightBuffer.ppml", "CopyToLightBuffer");
			copy_to_light_buffer_i_pp_ = SyncLoadPostProcess("Copy2LightBuffer.ppml", "CopyToLightBufferI");
		}


		sm_filter_pp_ = MakeSharedPtr<LogGaussianBlurPostProcess>(4, true);
		sm_filter_pp_->InputPin(0, sm_tex_);
		csm_filter_pp_ = MakeSharedPtr<LogGaussianBlurPostProcess>(4, true);
		csm_filter_pp_->InputPin(0, csm_tex_);
		if (depth_texture_support_)
		{
			depth_to_esm_pp_ = SyncLoadPostProcess("Depth.ppml", "DepthToESM");
			depth_to_esm_pp_->InputPin(0, sm_depth_tex_);
			depth_to_esm_pp_->OutputPin(0, sm_tex_);

			depth_to_linear_pp_ = SyncLoadPostProcess("Depth.ppml", "DepthToLinear");
		}
		depth_mipmap_pp_ = SyncLoadPostProcess("Depth.ppml", "DepthMipmapBilinear");

		g_buffer_tex_param_ = dr_effect_->ParameterByName("g_buffer_tex");
		g_buffer_1_tex_param_ = dr_effect_->ParameterByName("g_buffer_1_tex");
		depth_tex_param_ = dr_effect_->ParameterByName("depth_tex");
#if DEFAULT_DEFERRED == TRIDITIONAL_DEFERRED
		lighting_tex_param_ = dr_effect_->ParameterByName("lighting_tex");
#endif
		shading_tex_param_ = dr_effect_->ParameterByName("shading_tex");
		depth_near_far_invfar_param_ = dr_effect_->ParameterByName("depth_near_far_invfar");
		light_attrib_param_ = dr_effect_->ParameterByName("light_attrib");
		light_radius_extend_param_ = dr_effect_->ParameterByName("light_radius_extend");
		light_color_param_ = dr_effect_->ParameterByName("light_color");
		light_falloff_range_param_ = dr_effect_->ParameterByName("light_falloff_range");
		light_view_proj_param_ = dr_effect_->ParameterByName("light_view_proj");
		light_volume_mv_param_ = dr_effect_->ParameterByName("light_volume_mv");
		light_volume_mvp_param_ = dr_effect_->ParameterByName("light_volume_mvp");
		view_to_light_model_param_ = dr_effect_->ParameterByName("view_to_light_model");
		light_pos_es_param_ = dr_effect_->ParameterByName("light_pos_es");
		light_dir_es_param_ = dr_effect_->ParameterByName("light_dir_es");
		projective_map_2d_tex_param_ = dr_effect_->ParameterByName("projective_map_2d_tex");
		projective_map_cube_tex_param_ = dr_effect_->ParameterByName("projective_map_cube_tex");
		filtered_sm_2d_tex_param_ = dr_effect_->ParameterByName("filtered_sm_2d_tex");
		filtered_sm_cube_tex_param_ = dr_effect_->ParameterByName("filtered_sm_cube_tex");
		inv_width_height_param_ = dr_effect_->ParameterByName("inv_width_height");
		shadowing_tex_param_ = dr_effect_->ParameterByName("shadowing_tex");
		projective_shadowing_tex_param_ = dr_effect_->ParameterByName("projective_shadowing_tex");
		shadowing_channel_param_ = dr_effect_->ParameterByName("shadowing_channel");
		esm_scale_factor_param_ = dr_effect_->ParameterByName("esm_scale_factor");
		sm_far_plane_param_ = dr_effect_->ParameterByName("sm_far_plane");
		near_q_param_ = dr_effect_->ParameterByName("near_q");
		cascade_intervals_param_ = dr_effect_->ParameterByName("cascade_intervals");
		cascade_scale_bias_param_ = dr_effect_->ParameterByName("cascade_scale_bias");
		num_cascades_param_ = dr_effect_->ParameterByName("num_cascades");
		view_z_to_light_view_param_ = dr_effect_->ParameterByName("view_z_to_light_view");
		if (tex_array_support_)
		{
			filtered_csm_texs_param_[0] = dr_effect_->ParameterByName("filtered_csm_tex_array");
		}
		else
		{
			filtered_csm_texs_param_[0] = dr_effect_->ParameterByName("filtered_csm_0_tex");
			filtered_csm_texs_param_[1] = dr_effect_->ParameterByName("filtered_csm_1_tex");
			filtered_csm_texs_param_[2] = dr_effect_->ParameterByName("filtered_csm_2_tex");
			filtered_csm_texs_param_[3] = dr_effect_->ParameterByName("filtered_csm_3_tex");
		}
		skylight_diff_spec_mip_param_ = dr_effect_->ParameterByName("skylight_diff_spec_mip");
		skylight_mip_bias_param_ = dr_effect_->ParameterByName("skylight_mip_bias");
		inv_view_param_ = dr_effect_->ParameterByName("inv_view");
		skylight_y_cube_tex_param_ = dr_effect_->ParameterByName("skylight_y_cube_tex");
		skylight_c_cube_tex_param_ = dr_effect_->ParameterByName("skylight_c_cube_tex");
#if DEFAULT_DEFERRED == LIGHT_INDEXED_DEFERRED
		min_max_depth_tex_param_ = dr_effect_->ParameterByName("min_max_depth_tex");
		lights_color_param_ = dr_effect_->ParameterByName("lights_color");
		lights_pos_es_param_ = dr_effect_->ParameterByName("lights_pos_es");
		lights_dir_es_param_ = dr_effect_->ParameterByName("lights_dir_es");
		lights_falloff_range_param_ = dr_effect_->ParameterByName("lights_falloff_range");
		lights_attrib_param_ = dr_effect_->ParameterByName("lights_attrib");
		lights_radius_extend_param_ = dr_effect_->ParameterByName("lights_radius_extend");
		lights_aabb_min_param_ = dr_effect_->ParameterByName("lights_aabb_min");
		lights_aabb_max_param_ = dr_effect_->ParameterByName("lights_aabb_max");
		tile_scale_param_ = dr_effect_->ParameterByName("tile_scale");
		camera_proj_01_param_ = dr_effect_->ParameterByName("camera_proj_01");

		if (cs_tbdr_)
		{
			technique_depth_to_tiled_min_max_ = dr_effect_->TechniqueByName("DepthToTiledMinMax");
			technique_tbdr_lighting_mask_ = dr_effect_->TechniqueByName("TBDRLightingMask");

			width_height_param_ = dr_effect_->ParameterByName("width_height");
			depth_to_tiled_depth_in_tex_param_ = dr_effect_->ParameterByName("depth_in_tex");
			depth_to_tiled_min_max_depth_rw_tex_param_ = dr_effect_->ParameterByName("min_max_depth_rw_tex");
			upper_left_param_ = dr_effect_->ParameterByName("upper_left");
			x_dir_param_ = dr_effect_->ParameterByName("x_dir");
			y_dir_param_ = dr_effect_->ParameterByName("y_dir");
			lighting_mask_tex_param_ = dr_effect_->ParameterByName("lighting_mask_tex");
			shading_in_tex_param_ = dr_effect_->ParameterByName("shading_in_tex");
			shading_rw_tex_param_ = dr_effect_->ParameterByName("shading_rw_tex");
			lights_type_param_ = dr_effect_->ParameterByName("lights_type");

			copy_pp_ = SyncLoadPostProcess("Copy.ppml", "copy");
		}
		else
		{
			light_index_tex_param_ = dr_effect_->ParameterByName("light_index_tex");

			depth_to_min_max_pp_ = SyncLoadPostProcess("Depth.ppml", "DepthToMinMax");
			reduce_min_max_pp_ = SyncLoadPostProcess("Depth.ppml", "ReduceMinMax");
		}
#endif

		this->SetCascadedShadowType(CSLT_Auto);

#ifndef KLAYGE_SHIP
		PerfProfiler& profiler = PerfProfiler::Instance();
		shadow_map_perf_ = profiler.CreatePerfRange(0, "Gen shadow map");
		std::string buffer_name[] = { "Opaque", "Transparency back", "TransparencyFront" };
		for (uint32_t i = PTB_Opaque; i < PTB_None; ++ i)
		{
			if (!depth_texture_support_)
			{
				depth_perfs_[i] = profiler.CreatePerfRange(0, "Depth (" + buffer_name[i] + ")");
			}
			gbuffer_perfs_[i] = profiler.CreatePerfRange(0, "GBuffer (" + buffer_name[i] + ")");
			shadowing_perfs_[i] = profiler.CreatePerfRange(0, "Shadowing (" + buffer_name[i] + ")");
			indirect_lighting_perfs_[i] = profiler.CreatePerfRange(0, "Indirect lighting (" + buffer_name[i] + ")");
			shading_perfs_[i] = profiler.CreatePerfRange(0, "Shading (" + buffer_name[i] + ")");
			reflection_perfs_[i] = profiler.CreatePerfRange(0, "Reflection (" + buffer_name[i] + ")");
			special_shading_perfs_[i] = profiler.CreatePerfRange(0, "Special shading (" + buffer_name[i] + ")");
		}
		sss_blur_pp_perf_ = profiler.CreatePerfRange(0, "SSS Blur PP");
		ssr_pp_perf_ = profiler.CreatePerfRange(0, "SSR PP");
		atmospheric_pp_perf_ = profiler.CreatePerfRange(0, "Atmospheric PP");
		taa_pp_perf_ = profiler.CreatePerfRange(0, "TAA PP");
#endif
	}

	void DeferredRenderingLayer::Suspend()
	{
		// TODO
	}

	void DeferredRenderingLayer::Resume()
	{
		// TODO
	}

	void DeferredRenderingLayer::SSGIEnabled(uint32_t vp, bool ssgi)
	{
		this->SetupViewportGI(vp, ssgi);
	}

	void DeferredRenderingLayer::SSVOEnabled(uint32_t vp, bool ssvo)
	{
		viewports_[vp].ssvo_enabled = ssvo;
	}

	void DeferredRenderingLayer::SSSEnabled(bool ssr)
	{
		sss_enabled_ = ssr;
	}

	void DeferredRenderingLayer::SSSStrength(float strength)
	{
		sss_blur_pp_->SetParam(0, strength);
	}

	void DeferredRenderingLayer::SSSCorrection(float correction)
	{
		sss_blur_pp_->SetParam(1, correction);
	}

	void DeferredRenderingLayer::TranslucencyEnabled(bool trans)
	{
		translucency_enabled_ = trans;
	}

	void DeferredRenderingLayer::TranslucencyStrength(float strength)
	{
		translucency_pp_->SetParam(6, strength);
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
		if (caps.pack_to_rgba_required)
		{
			if (caps.rendertarget_format_support(EF_ABGR8, 1, 0))
			{
				depth_fmt = EF_ABGR8;
			}
			else
			{
				BOOST_ASSERT(caps.rendertarget_format_support(EF_ARGB8, 1, 0));
				depth_fmt = EF_ARGB8;
			}
		}
		else
		{
			if (caps.rendertarget_format_support(EF_R16F, 1, 0))
			{
				depth_fmt = EF_R16F;
			}
			else
			{
				BOOST_ASSERT(caps.rendertarget_format_support(EF_R32F, 1, 0));
				depth_fmt = EF_R32F;
			}
		}

		RenderViewPtr ds_view;
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

			pvp.g_buffer_ds_tex = rf.MakeTexture2D(width, height, 1, 1, ds_fmt, 1, 0, EAH_GPU_Read | EAH_GPU_Write, nullptr);
			ds_view = rf.Make2DDepthStencilRenderView(*pvp.g_buffer_ds_tex, 0, 1, 0);
		}
		else
		{
			ds_view = rf.Make2DDepthStencilRenderView(width, height, EF_D24S8, 1, 0);
		}

		pvp.g_buffer_rt0_tex = rf.MakeTexture2D(width, height, MAX_IL_MIPMAP_LEVELS + 1, 1, fmt8, 1, 0,
			EAH_GPU_Read | EAH_GPU_Write | EAH_Generate_Mips, nullptr);
		pvp.g_buffer_rt1_tex = rf.MakeTexture2D(width, height, 1, 1, fmt8, 1, 0,
			EAH_GPU_Read | EAH_GPU_Write, nullptr);
		pvp.g_buffer_depth_tex = rf.MakeTexture2D(width, height, MAX_IL_MIPMAP_LEVELS + 1, 1, depth_fmt, 1, 0,
			EAH_GPU_Read | EAH_GPU_Write | EAH_Generate_Mips, nullptr);
		if (caps.pack_to_rgba_required)
		{
			// Workaround for some HWs that don't support render-to-mipmap-level
			pvp.g_buffer_depth_pingpong_texs.resize(MAX_IL_MIPMAP_LEVELS);
			for (int i = 0; i < MAX_IL_MIPMAP_LEVELS; ++ i)
			{
				pvp.g_buffer_depth_pingpong_texs[i] = rf.MakeTexture2D(
					pvp.g_buffer_depth_tex->Width(i + 1), pvp.g_buffer_depth_tex->Height(i + 1), 1, 1,
					depth_fmt, 1, 0, EAH_GPU_Read | EAH_GPU_Write, nullptr);
			}
		}
		pvp.g_buffer_rt0_backup_tex = rf.MakeTexture2D(width, height, 1, 1, fmt8, 1, 0,
			EAH_GPU_Read, nullptr);
#if DEFAULT_DEFERRED == LIGHT_INDEXED_DEFERRED
		{
			ElementFormat min_max_depth_fmt;
			if (EF_R16F == depth_fmt)
			{
				min_max_depth_fmt = EF_GR16F;
			}
			else if (EF_R32F == depth_fmt)
			{
				min_max_depth_fmt = EF_GR32F;
			}
			else
			{
				min_max_depth_fmt = depth_fmt;
			}

			pvp.g_buffer_min_max_depth_texs.clear();
			if (cs_tbdr_)
			{
				uint32_t w = std::max(1U, (width + TILE_SIZE - 1) / TILE_SIZE);
				uint32_t h = std::max(1U, (height + TILE_SIZE - 1) / TILE_SIZE);
				pvp.g_buffer_min_max_depth_texs.push_back(rf.MakeTexture2D(w, h, 1, 1, min_max_depth_fmt, 1, 0,
					EAH_GPU_Read | EAH_GPU_Write | EAH_GPU_Unordered, nullptr));
			}
			else
			{
				uint32_t w = std::max(1U, (width + 1) / 2);
				uint32_t h = std::max(1U, (height + 1) / 2);
				for (uint32_t ts = TILE_SIZE; ts > 1; ts /= 2)
				{
					pvp.g_buffer_min_max_depth_texs.push_back(rf.MakeTexture2D(w, h, 1, 1, min_max_depth_fmt, 1, 0,
						EAH_GPU_Read | EAH_GPU_Write, nullptr));
					w = std::max(1U, (w + 1) / 2);
					h = std::max(1U, (h + 1) / 2);
				}
			}
		}
#endif

		RenderViewPtr g_buffer_rt0_view = rf.Make2DRenderView(*pvp.g_buffer_rt0_tex, 0, 1, 0);
		RenderViewPtr g_buffer_rt1_view = rf.Make2DRenderView(*pvp.g_buffer_rt1_tex, 0, 1, 0);
		RenderViewPtr g_buffer_depth_view = rf.Make2DRenderView(*pvp.g_buffer_depth_tex, 0, 1, 0);

		pvp.g_buffer->Attach(FrameBuffer::ATT_Color0, g_buffer_rt0_view);
		pvp.g_buffer->Attach(FrameBuffer::ATT_DepthStencil, ds_view);
		if (mrt_g_buffer_support_)
		{
			pvp.g_buffer->Attach(FrameBuffer::ATT_Color1, g_buffer_rt1_view);
		}
		else
		{
			pvp.g_buffer_rt1->Attach(FrameBuffer::ATT_Color0, g_buffer_rt1_view);
			pvp.g_buffer_rt1->Attach(FrameBuffer::ATT_DepthStencil, ds_view);
		}

		if (!depth_texture_support_)
		{
			pvp.pre_depth_fb->Attach(FrameBuffer::ATT_Color0, g_buffer_depth_view);
			pvp.pre_depth_fb->Attach(FrameBuffer::ATT_DepthStencil, ds_view);
		}

		this->SetupViewportGI(index, false);

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
		if (tex_array_support_)
		{
			if (caps.pack_to_rgba_required)
			{
				pvp.filtered_csm_texs[0] = rf.MakeTexture2D(SM_SIZE * 2, SM_SIZE * 2, 1,
					CascadedShadowLayer::MAX_NUM_CASCADES, fmt, 1, 0, EAH_GPU_Read | EAH_GPU_Write, nullptr);
			}
			else
			{
				pvp.filtered_csm_texs[0] = rf.MakeTexture2D(SM_SIZE * 2, SM_SIZE * 2, 3,
					CascadedShadowLayer::MAX_NUM_CASCADES, fmt, 1, 0, EAH_GPU_Read | EAH_GPU_Write | EAH_Generate_Mips, nullptr);
			}
		}
		else
		{
			if (caps.pack_to_rgba_required)
			{
				for (size_t i = 0; i < pvp.filtered_csm_texs.size(); ++ i)
				{
					pvp.filtered_csm_texs[i] = rf.MakeTexture2D(SM_SIZE * 2, SM_SIZE * 2, 1, 1, fmt, 1, 0,
						EAH_GPU_Read | EAH_GPU_Write, nullptr);
				}
			}
			else
			{
				for (size_t i = 0; i < pvp.filtered_csm_texs.size(); ++ i)
				{
					pvp.filtered_csm_texs[i] = rf.MakeTexture2D(SM_SIZE * 2, SM_SIZE * 2, 3, 1, fmt, 1, 0,
						EAH_GPU_Read | EAH_GPU_Write | EAH_Generate_Mips, nullptr);
				}
			}
		}

		if (caps.rendertarget_format_support(EF_ABGR8, 1, 0))
		{
			fmt = EF_ABGR8;
		}
		else
		{
			BOOST_ASSERT(caps.rendertarget_format_support(EF_ARGB8, 1, 0));
			fmt = EF_ARGB8;
		}
		pvp.shadowing_tex = rf.MakeTexture2D(width / 2, height / 2, 1, 1, fmt, 1, 0, EAH_GPU_Read | EAH_GPU_Write, nullptr);
		pvp.shadowing_fb->Attach(FrameBuffer::ATT_Color0, rf.Make2DRenderView(*pvp.shadowing_tex, 0, 1, 0));

		if (caps.fp_color_support && caps.rendertarget_format_support(EF_B10G11R11F, 1, 0))
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

			ElementFormat fmt_srgb = MakeSRGB(fmt);
			if (caps.texture_format_support(fmt_srgb) && caps.rendertarget_format_support(fmt_srgb, 1, 0))
			{
				fmt = fmt_srgb;
			}
		}
		pvp.projective_shadowing_tex = rf.MakeTexture2D(width / 2, height / 2, 1, 1, fmt, 1, 0, EAH_GPU_Read | EAH_GPU_Write, nullptr);
		pvp.projective_shadowing_fb->Attach(FrameBuffer::ATT_Color0, rf.Make2DRenderView(*pvp.projective_shadowing_tex, 0, 1, 0));

		pvp.reflection_tex = rf.MakeTexture2D(width / 2, height / 2, 1, 1, fmt, 1, 0, EAH_GPU_Read | EAH_GPU_Write, nullptr);
		pvp.reflection_fb->Attach(FrameBuffer::ATT_Color0, rf.Make2DRenderView(*pvp.reflection_tex, 0, 1, 0));
		pvp.reflection_fb->Attach(FrameBuffer::ATT_DepthStencil, rf.Make2DDepthStencilRenderView(
			pvp.reflection_tex->Width(0), pvp.reflection_tex->Height(0), ds_view->Format(), 1, 0));

		ElementFormat shading_fmt;
		if (caps.fp_color_support)
		{
			BOOST_ASSERT(caps.rendertarget_format_support(EF_ABGR16F, 1, 0));
			shading_fmt = EF_ABGR16F;
		}
		else
		{
			if (caps.rendertarget_format_support(EF_ABGR8, 1, 0))
			{
				shading_fmt = EF_ABGR8;
			}
			else
			{
				BOOST_ASSERT(caps.rendertarget_format_support(EF_ARGB8, 1, 0));
				shading_fmt = EF_ARGB8;
			}

			ElementFormat fmt_srgb = MakeSRGB(fmt);
			if (caps.texture_format_support(fmt_srgb) && caps.rendertarget_format_support(fmt_srgb, 1, 0))
			{
				fmt = fmt_srgb;
			}
		}

#if DEFAULT_DEFERRED == TRIDITIONAL_DEFERRED
		pvp.lighting_tex = rf.MakeTexture2D(width, height, 1, 1, shading_fmt, 1, 0, EAH_GPU_Read | EAH_GPU_Write, nullptr);
		pvp.lighting_fb->Attach(FrameBuffer::ATT_Color0, rf.Make2DRenderView(*pvp.lighting_tex, 0, 1, 0));
		pvp.lighting_fb->Attach(FrameBuffer::ATT_DepthStencil, ds_view);
#endif

		if (caps.fp_color_support)
		{
			if (caps.rendertarget_format_support(EF_B10G11R11F, 1, 0))
			{
				fmt = EF_B10G11R11F;
			}
			else
			{
				BOOST_ASSERT(caps.rendertarget_format_support(EF_ABGR16F, 1, 0));
				fmt = EF_ABGR16F;
			}
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
		uint32_t hint = EAH_GPU_Read | EAH_GPU_Write;
#if DEFAULT_DEFERRED == LIGHT_INDEXED_DEFERRED
		if (cs_tbdr_)
		{
			hint |= EAH_GPU_Unordered;
		}
#endif
		pvp.shading_tex = rf.MakeTexture2D(width, height, 1, 1, shading_fmt, 1, 0, hint, nullptr);
		pvp.curr_merged_shading_tex = rf.MakeTexture2D(width, height, 1, 1, fmt, 1, 0, hint, nullptr);
		pvp.curr_merged_depth_tex = rf.MakeTexture2D(width, height, 1, 1, depth_fmt, 1, 0, EAH_GPU_Read | EAH_GPU_Write, nullptr);
		pvp.prev_merged_shading_tex = rf.MakeTexture2D(width, height, 1, 1, fmt, 1, 0, hint, nullptr);
		pvp.prev_merged_depth_tex = rf.MakeTexture2D(width, height, 1, 1, depth_fmt, 1, 0, EAH_GPU_Read | EAH_GPU_Write, nullptr);
#if DEFAULT_DEFERRED == LIGHT_INDEXED_DEFERRED
		if (cs_tbdr_)
		{
			pvp.temp_shading_tex = rf.MakeTexture2D(width, height, 1, 1, shading_fmt, 1, 0,
				EAH_GPU_Read | EAH_GPU_Unordered, nullptr);

			ElementFormat lighting_mask_fmt;
			if (caps.rendertarget_format_support(EF_R8, 1, 0))
			{
				lighting_mask_fmt = EF_R8;
			}
			else if (caps.rendertarget_format_support(EF_ABGR8, 1, 0))
			{
				lighting_mask_fmt = EF_ABGR8;
			}
			else
			{
				BOOST_ASSERT(caps.rendertarget_format_support(EF_ARGB8, 1, 0));
				lighting_mask_fmt = EF_ARGB8;
			}
			pvp.lighting_mask_tex = rf.MakeTexture2D(width, height, 1, 1, lighting_mask_fmt, 1, 0, EAH_GPU_Read | EAH_GPU_Write, nullptr);
			pvp.lighting_mask_fb->Attach(FrameBuffer::ATT_Color0, rf.Make2DRenderView(*pvp.lighting_mask_tex, 0, 1, 0));
			pvp.lighting_mask_fb->Attach(FrameBuffer::ATT_DepthStencil, ds_view);
		}
#endif

		pvp.shading_fb->Attach(FrameBuffer::ATT_Color0, rf.Make2DRenderView(*pvp.shading_tex, 0, 1, 0));
		pvp.shading_fb->Attach(FrameBuffer::ATT_DepthStencil, ds_view);

		pvp.curr_merged_shading_fb->Attach(FrameBuffer::ATT_Color0, rf.Make2DRenderView(*pvp.curr_merged_shading_tex, 0, 1, 0));
		pvp.curr_merged_shading_fb->Attach(FrameBuffer::ATT_DepthStencil, ds_view);
		pvp.curr_merged_depth_fb->Attach(FrameBuffer::ATT_Color0, rf.Make2DRenderView(*pvp.curr_merged_depth_tex, 0, 1, 0));
		pvp.curr_merged_depth_fb->Attach(FrameBuffer::ATT_DepthStencil, ds_view);

		pvp.prev_merged_shading_fb->Attach(FrameBuffer::ATT_Color0, rf.Make2DRenderView(*pvp.prev_merged_shading_tex, 0, 1, 0));
		pvp.prev_merged_shading_fb->Attach(FrameBuffer::ATT_DepthStencil, ds_view);
		pvp.prev_merged_depth_fb->Attach(FrameBuffer::ATT_Color0, rf.Make2DRenderView(*pvp.prev_merged_depth_tex, 0, 1, 0));
		pvp.prev_merged_depth_fb->Attach(FrameBuffer::ATT_DepthStencil, ds_view);

		if (caps.rendertarget_format_support(EF_R8, 1, 0))
		{
			fmt = EF_R8;
		}
		else if (caps.rendertarget_format_support(EF_R16F, 1, 0))
		{
			fmt = EF_R16F;
		}
		else if (caps.rendertarget_format_support(EF_ABGR8, 1, 0))
		{
			fmt = EF_ABGR8;
		}
		else
		{
			BOOST_ASSERT(caps.rendertarget_format_support(EF_ARGB8, 1, 0));
			fmt = EF_ARGB8;
		}
		pvp.small_ssvo_tex = rf.MakeTexture2D(width / 2, height / 2, 1, 1, fmt, 1, 0, EAH_GPU_Read | EAH_GPU_Write, nullptr);

#if DEFAULT_DEFERRED == LIGHT_INDEXED_DEFERRED
		if (!cs_tbdr_)
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
			pvp.light_index_tex = rf.MakeTexture2D((width + (TILE_SIZE - 1)) / TILE_SIZE,
				(height + (TILE_SIZE - 1)) / TILE_SIZE, 1, 1, fmt, 1, 0, EAH_GPU_Read | EAH_GPU_Write, nullptr);
			pvp.light_index_fb->Attach(FrameBuffer::ATT_Color0, rf.Make2DRenderView(*pvp.light_index_tex, 0, 1, 0));
		}
#endif

		if (0 == index)
		{
			dr_debug_pp_->InputPin(0, this->GBufferRT0Tex(index));
			dr_debug_pp_->InputPin(1, this->GBufferRT1Tex(index));
			dr_debug_pp_->InputPin(2, this->DepthTex(index));
#if DEFAULT_DEFERRED == TRIDITIONAL_DEFERRED
			dr_debug_pp_->InputPin(3, this->LightingTex(index));
#endif
			dr_debug_pp_->InputPin(4, this->SmallSSVOTex(index));
		}
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
		SceneManager& scene_mgr = Context::Instance().SceneManagerInstance();
		
		if (0 == pass)
		{
			curr_cascade_index_ = -1;

			this->BuildLightList();

			bool has_opaque_objs = false;
			bool has_transparency_back_objs = false;
			bool has_transparency_front_objs = false;
			this->BuildVisibleSceneObjList(has_opaque_objs, has_transparency_back_objs, has_transparency_front_objs);

			this->BuildPassScanList(has_opaque_objs, has_transparency_back_objs, has_transparency_front_objs);

			num_objects_rendered_ = 0;
			num_renderables_rendered_ = 0;
			num_primitives_rendered_ = 0;
			num_vertices_rendered_ = 0;
		}

		uint32_t vp_index;
		PassType pass_type;
		int32_t org_no, index_in_pass;
		bool is_profile;
		this->DecomposePassScanCode(vp_index, pass_type, org_no, index_in_pass, is_profile, pass_scaned_[pass]);

		PassRT const pass_rt = GetPassRT(pass_type);
		PassTargetBuffer const pass_tb = GetPassTargetBuffer(pass_type);
		PassCategory const pass_cat = GetPassCategory(pass_type);

#ifndef KLAYGE_SHIP
		if (is_profile)
		{
			PerfRangePtr perf;
			switch (pass_cat)
			{
			case PC_Depth:
				perf = depth_perfs_[pass_tb];
				break;

			case PC_GBuffer:
				perf = gbuffer_perfs_[pass_tb];
				break;

			case PC_ShadowMap:
				perf = shadow_map_perf_;
				break;

			case PC_IndirectLighting:
				perf = indirect_lighting_perfs_[pass_tb];
				break;

			case PC_Shadowing:
				perf = shadowing_perfs_[pass_tb];
				break;

			case PC_Shading:
				perf = shading_perfs_[pass_tb];
				break;

			case PC_Reflection:
				perf = reflection_perfs_[pass_tb];
				break;

			case PC_SpecialShading:
				perf = special_shading_perfs_[pass_tb];
				break;

			default:
				BOOST_ASSERT(false);
				break;
			}

			if (0 == index_in_pass)
			{
				perf->Begin();
			}
			else
			{
				perf->End();
			}

			return 0;
		}
#endif

		active_viewport_ = vp_index;

		PerViewport& pvp = viewports_[vp_index];

		if ((pass_cat != PC_Shadowing) && (pass_cat != PC_IndirectLighting) && (pass_cat != PC_Shading))
		{
			for (auto const & deo : visible_scene_objs_)
			{
				deo->Pass(pass_type);
			}
		}

		uint32_t urv;
		switch (pass_cat)
		{
		case PC_Depth:
			re.ForceLineMode(force_line_mode_);

			// Pre depth for no depth texture support platforms
			this->PreparePVP(pvp);
			this->GenerateDepthBuffer(pvp, pass_tb);
			urv = App3DFramework::URV_NeedFlush | (App3DFramework::URV_OpaqueOnly << pass_tb);
			break;

		case PC_GBuffer:
			if (0 == index_in_pass)
			{
				re.ForceLineMode(force_line_mode_);

				if ((PRT_RT0 == pass_rt) || (PRT_MRT == pass_rt))
				{
					CameraPtr const & camera = pvp.frame_buffer->GetViewport()->camera;
					pvp.shadowing_fb->GetViewport()->camera = camera;
					pvp.projective_shadowing_fb->GetViewport()->camera = camera;
					pvp.reflection_fb->GetViewport()->camera = camera;

					if (depth_texture_support_)
					{
						this->PreparePVP(pvp);

						float q = camera->FarPlane() / (camera->FarPlane() - camera->NearPlane());
						float4 near_q_far(camera->NearPlane() * q, q, camera->FarPlane(), 1 / camera->FarPlane());
						depth_to_linear_pp_->SetParam(0, near_q_far);
					}

					*depth_near_far_invfar_param_ = pvp.depth_near_far_invfar;

					*g_buffer_tex_param_ = pvp.g_buffer_rt0_tex;
					*depth_tex_param_ = pvp.g_buffer_depth_tex;
					*inv_width_height_param_ = float2(1.0f / pvp.frame_buffer->GetViewport()->width,
						1.0f / pvp.frame_buffer->GetViewport()->height);
					*shadowing_tex_param_ = pvp.shadowing_tex;
					*projective_shadowing_tex_param_ = pvp.projective_shadowing_tex;

					this->GenerateGBuffer(pvp, pass_tb);
				}
				else
				{
					BOOST_ASSERT(PRT_RT1 == pass_rt);

					re.BindFrameBuffer(pvp.g_buffer_rt1);
					pvp.g_buffer_rt1->Attached(FrameBuffer::ATT_Color0)->Discard();
				}
				urv = App3DFramework::URV_NeedFlush | (App3DFramework::URV_OpaqueOnly << pass_tb);
			}
			else
			{
				if (1 == index_in_pass)
				{
					num_objects_rendered_ += scene_mgr.NumObjectsRendered();
					num_renderables_rendered_ += scene_mgr.NumRenderablesRendered();
					num_primitives_rendered_ += scene_mgr.NumPrimitivesRendered();
					num_vertices_rendered_ += scene_mgr.NumVerticesRendered();
				}

				if ((PRT_RT0 == pass_rt) || (PRT_MRT == pass_rt))
				{
					re.ForceLineMode(false);

					this->PostGenerateGBuffer(pvp);
					if (PTB_Opaque == pass_tb)
					{
						if (indirect_lighting_enabled_ && !(pvp.attrib & VPAM_NoGI))
						{
							pvp.il_layer->UpdateGBuffer(*pvp.frame_buffer->GetViewport()->camera);
						}

						if (cascaded_shadow_index_ >= 0)
						{
							Camera const & scene_camera = *pvp.frame_buffer->GetViewport()->camera;
							Camera const & light_camera = *lights_[cascaded_shadow_index_]->SMCamera(0);

							checked_cast<SunLightSource*>(lights_[cascaded_shadow_index_])->UpdateSMCamera(scene_camera);

							float const BLUR_FACTOR = 0.2f;
							blur_size_light_space_.x() = BLUR_FACTOR * 0.5f * light_camera.ProjMatrix()(0, 0);
							blur_size_light_space_.y() = BLUR_FACTOR * 0.5f * light_camera.ProjMatrix()(1, 1);
							
							float3 cascade_border(blur_size_light_space_.x(), blur_size_light_space_.y(),
								light_camera.ProjMatrix()(2, 2));
							cascaded_shadow_layer_->NumCascades(pvp.num_cascades);
							if (CSLT_SDSM == cascaded_shadow_layer_->Type())
							{
								checked_pointer_cast<SDSMCascadedShadowLayer>(cascaded_shadow_layer_)->DepthTexture(
									pvp.g_buffer_depth_tex);
							}
							cascaded_shadow_layer_->UpdateCascades(scene_camera, light_camera.ViewProjMatrix(),
								cascade_border);
						}
					}
				}

				if (PTB_Opaque == pass_tb)
				{
					if (!decals_.empty())
					{
						this->RenderDecals(pvp, (PRT_MRT == pass_rt) ? PT_OpaqueGBufferMRT : PT_OpaqueGBufferRT1);
					}

					if ((DT_Position == display_type_) || (DT_Normal == display_type_)
						|| (DT_Depth == display_type_) || (DT_Diffuse == display_type_)
						|| (DT_Specular == display_type_) || (DT_Shininess == display_type_))
					{
						re.BindFrameBuffer(FrameBufferPtr());
						re.CurFrameBuffer()->Attached(FrameBuffer::ATT_DepthStencil)->ClearDepth(1.0f);
						dr_debug_pp_->Apply();
						urv = App3DFramework::URV_SkipPostProcess | App3DFramework::URV_Finished;
					}
					else
					{
						urv = 0;
					}
				}
				else
				{
					urv = 0;
				}
			}
			break;

		case PC_ShadowMap:
			{
				auto const & light = *lights_[org_no];
				this->PrepareLightCamera(pvp, light, index_in_pass, pass_type);

				if (index_in_pass > 0)
				{
					this->PostGenerateShadowMap(pvp, org_no, index_in_pass);
				}

				if ((((LightSource::LT_Point == light.Type()) || (LightSource::LT_SphereArea == light.Type())
					|| (LightSource::LT_TubeArea == light.Type())) && (6 == index_in_pass))
					|| ((LightSource::LT_Spot == light.Type()) && (1 == index_in_pass))
					|| ((LightSource::LT_Sun == light.Type()) && (static_cast<int32_t>(pvp.num_cascades) == index_in_pass)))
				{
					urv = 0;
				}
				else
				{
					urv = App3DFramework::URV_NeedFlush;
					switch (pass_rt)
					{
					case PRT_ShadowMap:
						re.BindFrameBuffer(sm_fb_);
						sm_fb_->Attached(FrameBuffer::ATT_Color0)->Discard();
						sm_fb_->Attached(FrameBuffer::ATT_DepthStencil)->ClearDepth(1.0f);
						break;

					case PRT_ShadowMapWODepth:
					case PRT_CascadedShadowMap:
						{
							CameraPtr const & light_camera = sm_fb_->GetViewport()->camera;
							if (PRT_ShadowMapWODepth == pass_rt)
							{
								re.BindFrameBuffer(sm_fb_);
							}
							else
							{
								csm_fb_->GetViewport()->camera = light_camera;
								re.BindFrameBuffer(csm_fb_);
							}
							float const far_plane = light_camera->FarPlane();
							re.CurFrameBuffer()->Clear(FrameBuffer::CBM_Color | FrameBuffer::CBM_Depth,
								Color(far_plane, 0, 0, 0), 1.0f, 0);
						}
						break;

					default:
						BOOST_ASSERT(PRT_ReflectiveShadowMap == pass_rt);

						rsm_fb_->GetViewport()->camera = sm_fb_->GetViewport()->camera;
						re.BindFrameBuffer(rsm_fb_);
						rsm_fb_->Attached(FrameBuffer::ATT_Color0)->Discard();
						rsm_fb_->Attached(FrameBuffer::ATT_Color1)->Discard();
						rsm_fb_->Attached(FrameBuffer::ATT_DepthStencil)->ClearDepthStencil(1.0f, 0);
						urv |= App3DFramework::URV_OpaqueOnly;
						break;
					}
				}
			}
			break;

		case PC_IndirectLighting:
			if (depth_texture_support_)
			{
				depth_to_esm_pp_->Apply();
			}
			pvp.il_layer->UpdateRSM(*rsm_fb_->GetViewport()->camera, *lights_[org_no]);
			urv = 0;
			break;

		case PC_Shadowing:
			{
				auto const & light = *lights_[org_no];
				LightSource::LightType type = light.Type();
				int32_t attr = light.Attrib();

				this->PrepareLightCamera(pvp, light, index_in_pass, pass_type);

				if ((LightSource::LT_Point == type) || (LightSource::LT_SphereArea == type) 
					|| (LightSource::LT_TubeArea == type))
				{
					*projective_map_cube_tex_param_ = light.ProjectiveTexture();
				}
				else
				{
					*projective_map_2d_tex_param_ = light.ProjectiveTexture();
				}

				*light_attrib_param_ = float4((attr & LightSource::LSA_NoDiffuse) ? 0.0f : 1.0f,
					(attr & LightSource::LSA_NoSpecular) ? 0.0f : 1.0f,
					(attr & LightSource::LSA_NoShadow) ? -1.0f : 1.0f,
					light.ProjectiveTexture() ? 1.0f : -1.0f);

				this->UpdateShadowing(pvp, org_no);

				urv = 0;
			}
			break;

		case PC_Shading:
			{
#if DEFAULT_DEFERRED == LIGHT_INDEXED_DEFERRED
				if (PTB_Opaque == pass_tb)
				{
					*g_buffer_1_tex_param_ = pvp.g_buffer_rt1_tex;
					*light_volume_mv_param_ = pvp.inv_proj;

					re.BindFrameBuffer(pvp.curr_merged_shading_fb);
					re.Render(*technique_no_lighting_, *rl_quad_);
				}

				if (cs_tbdr_)
				{
					this->UpdateTileBasedLighting(pvp, pass_tb);
				}
				else
				{
					this->UpdateLightIndexedLighting(pvp, pass_tb);
				}
#elif DEFAULT_DEFERRED == TRIDITIONAL_DEFERRED
				for (uint32_t li = 0; li < lights_.size(); ++ li)
				{
					auto const & light = *lights_[li];
					if (light.Enabled() && pvp.light_visibles[li])
					{
						LightSource::LightType type = light.Type();
						int32_t attr = light.Attrib();

						this->PrepareLightCamera(pvp, light, index_in_pass, pass_type);

						*light_attrib_param_ = float4(attr & LightSource::LSA_NoDiffuse ? 0.0f : 1.0f,
							attr & LightSource::LSA_NoSpecular ? 0.0f : 1.0f,
							attr & LightSource::LSA_NoShadow ? -1.0f : 1.0f, light.ProjectiveTexture() ? 1.0f : -1.0f);
						*light_color_param_ = light.Color();
						*light_falloff_range_param_ = float4(light.Falloff().x(), light.Falloff().y(),
							light.Falloff().z(), light.Range() * light_scale_);

						float3 extend_es = MathLib::transform_normal(light.Extend(), pvp.view);
						*light_radius_extend_param_ = float4(light.Radius(), extend_es.x(),
							extend_es.y(), extend_es.z());

						this->UpdateLighting(pvp, type, li);
					}
				}

				this->UpdateShading(pvp, pass_tb);
#endif

				if (has_sss_objs_ && translucency_enabled_)
				{
					for (uint32_t li = 0; li < lights_.size(); ++ li)
					{
						this->AddTranslucency(li, pvp, pass_tb);
					}
				}

				if (PTB_Opaque == pass_tb)
				{
					this->MergeIndirectLighting(pvp, pass_tb);
					this->MergeSSVO(pvp, pass_tb);
				}
				urv = 0;
			}
			break;

		case PC_Reflection:
			re.BindFrameBuffer(pvp.reflection_fb);
			re.CurFrameBuffer()->Attached(FrameBuffer::ATT_DepthStencil)->ClearDepth(1.0f);
			urv = App3DFramework::URV_NeedFlush | App3DFramework::URV_ReflectionOnly
				| (App3DFramework::URV_OpaqueOnly << pass_tb);
			break;

		case PC_SpecialShading:
		default:
			if (0 == index_in_pass)
			{
				if (PTB_Opaque == pass_tb)
				{
					re.BindFrameBuffer(pvp.curr_merged_shading_fb);
				}
				else
				{
					re.BindFrameBuffer(pvp.shading_fb);
				}
				urv = App3DFramework::URV_NeedFlush | App3DFramework::URV_SpecialShadingOnly
					| (App3DFramework::URV_OpaqueOnly << pass_tb);
			}
			else if (1 == index_in_pass)
			{
				this->MergeShadingAndDepth(pvp, pass_tb);
				urv = 0;
			}
			else
			{
				if (has_sss_objs_ && sss_enabled_)
				{
#ifndef KLAYGE_SHIP
					sss_blur_pp_perf_->Begin();
#endif
					this->AddSSS(pvp);
#ifndef KLAYGE_SHIP
					sss_blur_pp_perf_->End();
#endif
				}

				if (has_reflective_objs_ && ssr_enabled_)
				{
#ifndef KLAYGE_SHIP
					ssr_pp_perf_->Begin();
#endif
					this->AddSSR(pvp);
#ifndef KLAYGE_SHIP
					ssr_pp_perf_->End();
#endif
				}

				if (atmospheric_pp_)
				{
#ifndef KLAYGE_SHIP
					atmospheric_pp_perf_->Begin();
#endif
					this->AddAtmospheric(pvp);
#ifndef KLAYGE_SHIP
					atmospheric_pp_perf_->End();
#endif
				}

#ifndef KLAYGE_SHIP
				taa_pp_perf_->Begin();
#endif
				this->AddTAA(pvp);
#ifndef KLAYGE_SHIP
				taa_pp_perf_->End();
#endif

				std::swap(pvp.curr_merged_shading_fb, pvp.prev_merged_shading_fb);
				std::swap(pvp.curr_merged_shading_tex, pvp.prev_merged_shading_tex);
				std::swap(pvp.curr_merged_depth_fb, pvp.prev_merged_depth_fb);
				std::swap(pvp.curr_merged_depth_tex, pvp.prev_merged_depth_tex);

				if (has_simple_forward_objs_ && !(pvp.attrib & VPAM_NoSimpleForward))
				{
					for (auto const & deo : visible_scene_objs_)
					{
						if (deo->SimpleForward())
						{
							deo->Pass(PT_SimpleForward);
						}
					}

					urv = App3DFramework::URV_NeedFlush | App3DFramework::URV_SimpleForwardOnly;
				}
				else
				{
					urv = 0;
				}

				if (pass_scaned_.size() - 1 == pass)
				{
					urv |= App3DFramework::URV_Finished;

					if ((DT_SSVO == display_type_)
#if DEFAULT_DEFERRED == TRIDITIONAL_DEFERRED
						|| (DT_DiffuseLighting == display_type_)
						|| (DT_SpecularLighting == display_type_)
#endif
						)
					{
						re.BindFrameBuffer(FrameBufferPtr());
						re.CurFrameBuffer()->Attached(FrameBuffer::ATT_DepthStencil)->ClearDepth(1.0f);
						dr_debug_pp_->Apply();
						urv |= App3DFramework::URV_SkipPostProcess;
					}
				}
			}
			break;
		}

		scene_mgr.SmallObjectThreshold((PC_ShadowMap == pass_cat) ? 0.002f : 0.0f);
		return urv;
	}

	void DeferredRenderingLayer::BuildLightList()
	{
		SceneManager& scene_mgr = Context::Instance().SceneManagerInstance();

		lights_.clear();
		sm_light_indices_.clear();

		uint32_t const num_lights = scene_mgr.NumLights();
		
		for (uint32_t i = 0; i < num_lights; ++ i)
		{
			auto light = scene_mgr.GetLight(i).get();
			if (light->Enabled() && (LightSource::LT_Ambient == light->Type()))
			{
				lights_.push_back(light);
				break;
			}
		}
		if (lights_.empty())
		{
			lights_.push_back(default_ambient_light_.get());
		}
		sm_light_indices_.emplace_back(-1, 0);

		uint32_t num_ambient_lights = 0;
		float3 ambient_clr(0, 0, 0);

		projective_light_index_ = -1;
		cascaded_shadow_index_ = -1;
		uint32_t num_sm_lights = 0;
		uint32_t num_sm_2d_lights = 0;
		uint32_t num_sm_cube_lights = 0;
		for (uint32_t i = 0; i < num_lights; ++ i)
		{
			auto light = scene_mgr.GetLight(i).get();
			if (light->Enabled())
			{
				if (LightSource::LT_Ambient == light->Type())
				{
					float4 const & clr = light->Color();
					ambient_clr += float3(clr.x(), clr.y(), clr.z());
					++ num_ambient_lights;
				}
				else
				{
					lights_.push_back(light);

					if (0 == (light->Attrib() & LightSource::LSA_NoShadow))
					{
						switch (light->Type())
						{
						case LightSource::LT_Sun:
							sm_light_indices_.emplace_back(0, num_sm_lights);
							++ num_sm_lights;
							cascaded_shadow_index_ = static_cast<int32_t>(i + 1 - num_ambient_lights);
							break;

						case LightSource::LT_Spot:
							if ((projective_light_index_ < 0) && light->ProjectiveTexture())
							{
								projective_light_index_ = static_cast<int32_t>(i + 1 - num_ambient_lights);
								sm_light_indices_.emplace_back(0, 4);
							}
							else if ((num_sm_2d_lights < MAX_NUM_SHADOWED_SPOT_LIGHTS)
								&& (num_sm_lights < MAX_NUM_SHADOWED_LIGHTS))
							{
								sm_light_indices_.emplace_back(num_sm_2d_lights, num_sm_lights);
								++ num_sm_2d_lights;
								++ num_sm_lights;
							}
							else
							{
								sm_light_indices_.emplace_back(-1, 0);
							}
							break;

						case LightSource::LT_Point:
						case LightSource::LT_SphereArea:
						case LightSource::LT_TubeArea:
							if ((projective_light_index_ < 0) && light->ProjectiveTexture())
							{
								projective_light_index_ = static_cast<int32_t>(i + 1 - num_ambient_lights);
								sm_light_indices_.emplace_back(0, 4);
							}
							else if ((num_sm_cube_lights < MAX_NUM_SHADOWED_POINT_LIGHTS)
								&& (num_sm_lights < MAX_NUM_SHADOWED_LIGHTS))
							{
								sm_light_indices_.emplace_back(num_sm_cube_lights, num_sm_lights);
								++ num_sm_cube_lights;
								++ num_sm_lights;
							}
							else
							{
								sm_light_indices_.emplace_back(-1, 0);
							}
							break;

						default:
							sm_light_indices_.emplace_back(-1, 0);
							break;
						}
					}
					else
					{
						sm_light_indices_.emplace_back(-1, 0);
					}
				}
			}
		}
		if (0 == num_ambient_lights)
		{
			ambient_clr = float3(0.1f, 0.1f, 0.1f);
		}
		lights_[0]->Color(ambient_clr);

		indirect_lighting_enabled_ = false;
		if (rsm_fb_ && (illum_ != 1))
		{
			for (size_t i = 0; i < lights_.size(); ++ i)
			{
				if (lights_[i]->Attrib() & LightSource::LSA_IndirectLighting)
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
		has_sss_objs_ = false;
		has_reflective_objs_ = false;
		has_simple_forward_objs_ = false;
		visible_scene_objs_.clear();
		uint32_t const num_scene_objs = scene_mgr.NumSceneObjects();
		for (uint32_t i = 0; i < num_scene_objs; ++ i)
		{
			SceneObject* so = scene_mgr.GetSceneObject(i).get();
			if (so->Visible())
			{
				visible_scene_objs_.push_back(so);

				has_opaque_objs = true;

				if (so->TransparencyBackFace())
				{
					has_transparency_back_objs = true;
				}
				if (so->TransparencyFrontFace())
				{
					has_transparency_front_objs = true;
				}
				if (so->SSS())
				{
					has_sss_objs_ = true;
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

#ifndef KLAYGE_SHIP
		pass_scaned_.push_back(this->ComposePassScanCode(0, PT_GenShadowMap, 0, 0, true));
#endif
		for (uint32_t i = 0; i < lights_.size(); ++ i)
		{
			auto const & light = *lights_[i];
			if (light.Enabled())
			{
				this->AppendShadowPassScanCode(i);
			}
		}
#ifndef KLAYGE_SHIP
		pass_scaned_.push_back(this->ComposePassScanCode(0, PT_GenShadowMap, 0, 1, true));
#endif

#ifdef KLAYGE_DEBUG
		bool no_viewport = true;
#endif
		for (uint32_t vpi = 0; vpi < viewports_.size(); ++ vpi)
		{
			PerViewport& pvp = viewports_[vpi];
			if (pvp.attrib & VPAM_Enabled)
			{
#ifdef KLAYGE_DEBUG
				no_viewport = false;
#endif

				pvp.g_buffer_enables[Opaque_GBuffer] = (pvp.attrib & VPAM_NoOpaque) ? false : has_opaque_objs;
				pvp.g_buffer_enables[TransparencyBack_GBuffer] = (pvp.attrib & VPAM_NoTransparencyBack) ? false : has_transparency_back_objs;
				pvp.g_buffer_enables[TransparencyFront_GBuffer] = (pvp.attrib & VPAM_NoTransparencyFront) ? false : has_transparency_front_objs;

				pvp.light_visibles.resize(lights_.size());
				for (uint32_t li = 0; li < lights_.size(); ++ li)
				{
					auto const & light = *lights_[li];
					if (light.Enabled())
					{
						this->CheckLightVisible(vpi, li);
					}
					else
					{
						pvp.light_visibles[li] = false;
					}
				}

				for (uint32_t i = 0; i < Num_GBuffers; ++ i)
				{
					if (pvp.g_buffer_enables[i])
					{
						this->AppendGBufferPassScanCode(vpi, i);
					}
					if (0 == i)
					{
						if (cascaded_shadow_index_ >= 0)
						{
							this->AppendCascadedShadowPassScanCode(vpi, cascaded_shadow_index_);
						}
					}

					if (pvp.g_buffer_enables[i])
					{
#ifndef KLAYGE_SHIP
						pass_scaned_.push_back(this->ComposePassScanCode(vpi,
							ComposePassType(PRT_None, static_cast<PassTargetBuffer>(i), PC_Shadowing), 0, 0, true));
#endif
						for (uint32_t li = 0; li < lights_.size(); ++ li)
						{
							auto const & light = *lights_[li];
							if (light.Enabled() && (0 == (light.Attrib() & LightSource::LSA_NoShadow))
								&& pvp.light_visibles[li])
							{
								this->AppendShadowingPassScanCode(vpi, i, li);
							}
						}
#ifndef KLAYGE_SHIP
						pass_scaned_.push_back(this->ComposePassScanCode(vpi,
							ComposePassType(PRT_None, static_cast<PassTargetBuffer>(i), PC_Shadowing), 0, 1, true));
#endif
						if (!(pvp.attrib & VPAM_NoGI))
						{
#ifndef KLAYGE_SHIP
							pass_scaned_.push_back(this->ComposePassScanCode(vpi,
								ComposePassType(PRT_None, static_cast<PassTargetBuffer>(i), PC_IndirectLighting), 0, 0, true));
#endif
							for (uint32_t li = 0; li < lights_.size(); ++ li)
							{
								auto const & light = *lights_[li];
								if (light.Enabled())
								{
									PassTargetBuffer const pass_tb = static_cast<PassTargetBuffer>(i);
									if ((LightSource::LT_Spot == light.Type()) && (PTB_Opaque == pass_tb)
										&& (light.Attrib() & LightSource::LSA_IndirectLighting)
										&& rsm_fb_ && (illum_ != 1)
										&& pvp.light_visibles[li])
									{
										this->AppendIndirectLightingPassScanCode(vpi, li);
									}
								}
							}
#ifndef KLAYGE_SHIP
							pass_scaned_.push_back(this->ComposePassScanCode(vpi,
								ComposePassType(PRT_None, static_cast<PassTargetBuffer>(i), PC_IndirectLighting), 0, 1, true));
#endif
						}

						this->AppendShadingPassScanCode(vpi, i);
					}
				}

				pass_scaned_.push_back(this->ComposePassScanCode(vpi, PT_OpaqueSpecialShading, 0, 2, false));
			}
		}

#ifdef KLAYGE_DEBUG
		if (no_viewport)
		{
			LogError("No viewport available.");
		}
#endif
	}

	void DeferredRenderingLayer::CheckLightVisible(uint32_t vp_index, uint32_t light_index)
	{
		SceneManager& scene_mgr = Context::Instance().SceneManagerInstance();

		PerViewport& pvp = viewports_[vp_index];
		auto const & light = *lights_[light_index];

		float light_scale = std::min(light.Range() * 0.01f, 1.0f) * light_scale_;
		switch (light.Type())
		{
		case LightSource::LT_Spot:
			{
				float4x4 const & inv_light_view = light.SMCamera(0)->InverseViewMatrix();
				float const scale = light.CosOuterInner().w();
				float4x4 mat = MathLib::scaling(scale * light_scale, scale * light_scale, light_scale);
				float4x4 light_model = mat * inv_light_view;
				pvp.light_visibles[light_index] = (scene_mgr.AABBVisible(MathLib::transform_aabb(cone_aabb_, light_model)) != BO_No);
			}
			break;

		case LightSource::LT_Point:
		case LightSource::LT_SphereArea:
		case LightSource::LT_TubeArea:
			{
				float3 const & p = light.Position();
				float4x4 light_model = MathLib::scaling(light_scale, light_scale, light_scale)
					* MathLib::translation(p);
				pvp.light_visibles[light_index] = (scene_mgr.AABBVisible(MathLib::transform_aabb(box_aabb_, light_model)) != BO_No);
			}
			break;

		default:
			pvp.light_visibles[light_index] = true;
			break;
		}
	}

	void DeferredRenderingLayer::AppendGBufferPassScanCode(uint32_t vp_index, uint32_t g_buffer_index)
	{
		PassTargetBuffer ptb = static_cast<PassTargetBuffer>(g_buffer_index);

		if (!depth_texture_support_)
		{
#ifndef KLAYGE_SHIP
			pass_scaned_.push_back(this->ComposePassScanCode(vp_index,
				ComposePassType(PRT_None, ptb, PC_Depth), 0, 0, true));
#endif
			pass_scaned_.push_back(this->ComposePassScanCode(vp_index,
				ComposePassType(PRT_None, ptb, PC_Depth), 0, 0, false));
#ifndef KLAYGE_SHIP
			pass_scaned_.push_back(this->ComposePassScanCode(vp_index,
				ComposePassType(PRT_None, ptb, PC_Depth), 0, 1, true));
#endif
		}

		if (mrt_g_buffer_support_)
		{
#ifndef KLAYGE_SHIP
			pass_scaned_.push_back(this->ComposePassScanCode(vp_index,
				ComposePassType(PRT_MRT, ptb, PC_GBuffer), 0, 0, true));
#endif
			pass_scaned_.push_back(this->ComposePassScanCode(vp_index,
				ComposePassType(PRT_MRT, ptb, PC_GBuffer), 0, 0, false));
			pass_scaned_.push_back(this->ComposePassScanCode(vp_index,
				ComposePassType(PRT_MRT, ptb, PC_GBuffer), 0, 1, false));
#ifndef KLAYGE_SHIP
			pass_scaned_.push_back(this->ComposePassScanCode(vp_index,
				ComposePassType(PRT_MRT, ptb, PC_GBuffer), 0, 1, true));
#endif
		}
		else
		{
#ifndef KLAYGE_SHIP
			pass_scaned_.push_back(this->ComposePassScanCode(vp_index,
				ComposePassType(PRT_RT0, ptb, PC_GBuffer), 0, 0, true));
#endif
			pass_scaned_.push_back(this->ComposePassScanCode(vp_index,
				ComposePassType(PRT_RT0, ptb, PC_GBuffer), 0, 0, false));
			pass_scaned_.push_back(this->ComposePassScanCode(vp_index,
				ComposePassType(PRT_RT0, ptb, PC_GBuffer), 0, 1, false));
#ifndef KLAYGE_SHIP
			pass_scaned_.push_back(this->ComposePassScanCode(vp_index,
				ComposePassType(PRT_RT0, ptb, PC_GBuffer), 0, 1, true));
			pass_scaned_.push_back(this->ComposePassScanCode(vp_index,
				ComposePassType(PRT_RT1, ptb, PC_GBuffer), 0, 0, true));
#endif
			pass_scaned_.push_back(this->ComposePassScanCode(vp_index,
				ComposePassType(PRT_RT1, ptb, PC_GBuffer), 0, 0, false));
			pass_scaned_.push_back(this->ComposePassScanCode(vp_index,
				ComposePassType(PRT_RT1, ptb, PC_GBuffer), 0, 1, false));
#ifndef KLAYGE_SHIP
			pass_scaned_.push_back(this->ComposePassScanCode(vp_index,
				ComposePassType(PRT_RT1, ptb, PC_GBuffer), 0, 1, true));
#endif
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

		auto const & light = *lights_[light_index];
		LightSource::LightType type = light.Type();
		int32_t attr = light.Attrib();
		switch (type)
		{
		case LightSource::LT_Spot:
			{
				int sm_seq;
				if (attr & LightSource::LSA_IndirectLighting)
				{
					if (rsm_fb_ && (illum_ != 1))
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
					if (0 == (attr & LightSource::LSA_NoShadow))
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
					pass_scaned_.push_back(this->ComposePassScanCode(0, shadow_pt, light_index, 0, false));
					pass_scaned_.push_back(this->ComposePassScanCode(0, shadow_pt, light_index, 1, false));
				}
			}
			break;

		case LightSource::LT_Point:
		case LightSource::LT_SphereArea:
		case LightSource::LT_TubeArea:
			if (0 == (attr & LightSource::LSA_NoShadow))
			{
				for (int j = 0; j < 7; ++ j)
				{
					pass_scaned_.push_back(this->ComposePassScanCode(0, shadow_pt, light_index, j, false));
				}
			}
			break;

		case LightSource::LT_Sun:
			break;

		default:
			if (0 == (attr & LightSource::LSA_NoShadow))
			{							
				pass_scaned_.push_back(this->ComposePassScanCode(0, shadow_pt, light_index, 0, false));
			}
			break;
		}
	}

	void DeferredRenderingLayer::AppendCascadedShadowPassScanCode(uint32_t vp_index, uint32_t light_index)
	{
		BOOST_ASSERT(LightSource::LT_Sun == lights_[light_index]->Type());

#ifndef KLAYGE_SHIP
		pass_scaned_.push_back(this->ComposePassScanCode(vp_index, PT_GenCascadedShadowMap,
			light_index, 0, true));
#endif

		PerViewport& pvp = viewports_[vp_index];
		for (uint32_t i = 0; i < pvp.num_cascades + 1; ++ i)
		{
			pass_scaned_.push_back(this->ComposePassScanCode(vp_index, PT_GenCascadedShadowMap,
				light_index, i, false));
		}

#ifndef KLAYGE_SHIP
		pass_scaned_.push_back(this->ComposePassScanCode(vp_index, PT_GenCascadedShadowMap,
			light_index, 1, true));
#endif
	}

	void DeferredRenderingLayer::AppendShadowingPassScanCode(uint32_t vp_index, uint32_t g_buffer_index, uint32_t light_index)
	{
		PassTargetBuffer const pass_tb = static_cast<PassTargetBuffer>(g_buffer_index);
		pass_scaned_.push_back(this->ComposePassScanCode(vp_index,
			ComposePassType(PRT_None, pass_tb, PC_Shadowing), light_index, 0, false));
	}

	void DeferredRenderingLayer::AppendIndirectLightingPassScanCode(uint32_t vp_index, uint32_t light_index)
	{
		pass_scaned_.push_back(this->ComposePassScanCode(vp_index, PT_IndirectLighting, light_index, 0, false));
	}

	void DeferredRenderingLayer::AppendShadingPassScanCode(uint32_t vp_index, uint32_t g_buffer_index)
	{
		PassTargetBuffer const pass_tb = static_cast<PassTargetBuffer>(g_buffer_index);

#ifndef KLAYGE_SHIP
		pass_scaned_.push_back(this->ComposePassScanCode(vp_index,
			ComposePassType(PRT_None, pass_tb, PC_Shading), 0, 0, true));
#endif
		pass_scaned_.push_back(this->ComposePassScanCode(vp_index, 
			ComposePassType(PRT_None, pass_tb, PC_Shading), 0, 0, false));
#ifndef KLAYGE_SHIP
		pass_scaned_.push_back(this->ComposePassScanCode(vp_index,
			ComposePassType(PRT_None, pass_tb, PC_Shading), 0, 1, true));
#endif

		if (has_reflective_objs_)
		{
#ifndef KLAYGE_SHIP
			pass_scaned_.push_back(this->ComposePassScanCode(vp_index,
				ComposePassType(PRT_None, pass_tb, PC_Reflection), 0, 0, true));
#endif
			pass_scaned_.push_back(this->ComposePassScanCode(vp_index,
				ComposePassType(PRT_None, pass_tb, PC_Reflection), 0, 0, false));
#ifndef KLAYGE_SHIP
			pass_scaned_.push_back(this->ComposePassScanCode(vp_index,
				ComposePassType(PRT_None, pass_tb, PC_Reflection), 0, 1, true));
#endif
		}

#ifndef KLAYGE_SHIP
		pass_scaned_.push_back(this->ComposePassScanCode(vp_index,
			ComposePassType(PRT_None, pass_tb, PC_SpecialShading), 0, 0, true));
#endif
		pass_scaned_.push_back(this->ComposePassScanCode(vp_index,
			ComposePassType(PRT_None, pass_tb, PC_SpecialShading), 0, 0, false));
		pass_scaned_.push_back(this->ComposePassScanCode(vp_index,
			ComposePassType(PRT_None, pass_tb, PC_SpecialShading), 0, 1, false));
#ifndef KLAYGE_SHIP
		pass_scaned_.push_back(this->ComposePassScanCode(vp_index,
			ComposePassType(PRT_None, pass_tb, PC_SpecialShading), 0, 1, true));
#endif
	}

	void DeferredRenderingLayer::PreparePVP(PerViewport& pvp)
	{
		Camera const & camera = *pvp.frame_buffer->GetViewport()->camera;
		pvp.inv_view = camera.InverseViewMatrix();
		pvp.inv_proj = camera.InverseProjMatrix();
		pvp.proj_to_prev = pvp.inv_proj * pvp.inv_view * pvp.view * pvp.proj;
		pvp.view = camera.ViewMatrix();
		pvp.proj = camera.ProjMatrix();
		pvp.depth_near_far_invfar = float3(camera.NearPlane(), camera.FarPlane(), 1 / camera.FarPlane());
	}

	void DeferredRenderingLayer::GenerateDepthBuffer(PerViewport const & pvp, uint32_t g_buffer_index)
	{
		RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();

		CameraPtr const & camera = pvp.frame_buffer->GetViewport()->camera;
		pvp.pre_depth_fb->GetViewport()->camera = camera;

		re.BindFrameBuffer(pvp.pre_depth_fb);

		float depth = (TransparencyBack_GBuffer == g_buffer_index) ? 0.0f : 1.0f;
		int32_t stencil = (Opaque_GBuffer == g_buffer_index) ? 0 : 128;
		pvp.g_buffer->Clear(FrameBuffer::CBM_Color | FrameBuffer::CBM_Depth | FrameBuffer::CBM_Stencil,
			Color(0, 0, 0, 0), depth, stencil);
	}

	void DeferredRenderingLayer::GenerateGBuffer(PerViewport const & pvp, uint32_t g_buffer_index)
	{
		RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();

		if (Opaque_GBuffer == g_buffer_index)
		{
			CameraPtr const & camera = pvp.frame_buffer->GetViewport()->camera;
			pvp.g_buffer->GetViewport()->camera = camera;
			if (!mrt_g_buffer_support_)
			{
				pvp.g_buffer_rt1->GetViewport()->camera = camera;
			}
#if DEFAULT_DEFERRED == TRIDITIONAL_DEFERRED
			pvp.lighting_fb->GetViewport()->camera = camera;
#endif
			pvp.shading_fb->GetViewport()->camera = camera;
			pvp.curr_merged_shading_fb->GetViewport()->camera = camera;
			pvp.curr_merged_depth_fb->GetViewport()->camera = camera;
			pvp.prev_merged_shading_fb->GetViewport()->camera = camera;
			pvp.prev_merged_depth_fb->GetViewport()->camera = camera;
		}

		re.BindFrameBuffer(pvp.g_buffer);

		float depth = (TransparencyBack_GBuffer == g_buffer_index) ? 0.0f : 1.0f;
		int32_t stencil = (Opaque_GBuffer == g_buffer_index) ? 0 : 128;
		pvp.g_buffer->Clear(FrameBuffer::CBM_Color | FrameBuffer::CBM_Depth | FrameBuffer::CBM_Stencil,
			Color(0, 0, 0, 0), depth, stencil);
	}

	void DeferredRenderingLayer::PostGenerateGBuffer(PerViewport const & pvp)
	{
		RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
		RenderDeviceCaps const & caps = re.DeviceCaps();

		pvp.g_buffer_rt0_tex->BuildMipSubLevels();

		if (depth_texture_support_)
		{
			depth_to_linear_pp_->InputPin(0, pvp.g_buffer_ds_tex);
			depth_to_linear_pp_->OutputPin(0, pvp.g_buffer_depth_tex);
			depth_to_linear_pp_->Apply();
		}

		if (caps.pack_to_rgba_required)
		{
			depth_mipmap_pp_->InputPin(0, pvp.g_buffer_depth_tex);
			for (uint32_t i = 1; i < pvp.g_buffer_depth_tex->NumMipMaps(); ++ i)
			{
				uint32_t const width = pvp.g_buffer_depth_tex->Width(i - 1);
				uint32_t const height = pvp.g_buffer_depth_tex->Height(i - 1);
				uint32_t const lower_width = pvp.g_buffer_depth_tex->Width(i);
				uint32_t const lower_height = pvp.g_buffer_depth_tex->Height(i);

				depth_mipmap_pp_->SetParam(0, float2(0.5f / width, 0.5f / height));

				depth_mipmap_pp_->OutputPin(0, pvp.g_buffer_depth_pingpong_texs[i - 1], 0);
				depth_mipmap_pp_->Apply();

				pvp.g_buffer_depth_pingpong_texs[i - 1]->CopyToSubTexture2D(*pvp.g_buffer_depth_tex, 0, i, 0, 0,
					lower_width, lower_height, 0, 0, 0, 0, lower_width, lower_height);
			}
		}
		else
		{
			pvp.g_buffer_depth_tex->BuildMipSubLevels();
		}

#if DEFAULT_DEFERRED == LIGHT_INDEXED_DEFERRED
		if (cs_tbdr_)
		{
			this->CreateDepthMinMaxMapCS(pvp);
		}
		else
		{
			this->CreateDepthMinMaxMap(pvp);
		}
#endif
	}

	void DeferredRenderingLayer::RenderDecals(PerViewport const & pvp, PassType pass_type)
	{
		RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();

		uint32_t const width = pvp.g_buffer_rt0_tex->Width(0);
		uint32_t const height = pvp.g_buffer_rt0_tex->Height(0);
		pvp.g_buffer_rt0_tex->CopyToSubTexture2D(*pvp.g_buffer_rt0_backup_tex, 0, 0,
			0, 0, width, height, 0, 0, 0, 0, width, height);

		re.BindFrameBuffer((PT_OpaqueGBufferRT1 == pass_type) ? pvp.g_buffer_rt1
			: pvp.g_buffer);
		for (auto const & de : decals_)
		{
			de->Pass(pass_type);
			de->Render();
		}
	}

	void DeferredRenderingLayer::PrepareLightCamera(PerViewport const & pvp,
		LightSource const & light, int32_t index_in_pass, PassType pass_type)
	{
		LightSource::LightType const type = light.Type();
		PassCategory const pass_cat = GetPassCategory(pass_type);

		switch (type)
		{
		case LightSource::LT_Point:
		case LightSource::LT_Spot:
		case LightSource::LT_Sun:
		case LightSource::LT_SphereArea:
		case LightSource::LT_TubeArea:
			{
				CameraPtr sm_camera;
				float3 dir_es(0, 0, 0);
				if (LightSource::LT_Spot == type)
				{
					dir_es = MathLib::transform_normal(light.Direction(), pvp.view);
					sm_camera = light.SMCamera(0);
				}
				else if (LightSource::LT_Sun == type)
				{
					dir_es = MathLib::transform_normal(-light.Direction(), pvp.view);
					sm_camera = light.SMCamera(0);
				}
				else
				{
					int32_t face = std::min(index_in_pass, 5);
					std::pair<float3, float3> ad = CubeMapViewVector<float>(static_cast<Texture::CubeFaces>(face));
					dir_es = MathLib::transform_normal(MathLib::transform_quat(ad.first, light.Rotation()), pvp.view);
					sm_camera = light.SMCamera(face);
				}
				float4 light_dir_es_actived = float4(dir_es.x(), dir_es.y(), dir_es.z(), 0);

				sm_fb_->GetViewport()->camera = sm_camera;

				if ((LightSource::LT_Sun == type) && (pass_cat != PC_Shadowing) && (pass_cat != PC_Shading))
				{
					curr_cascade_index_ = index_in_pass;
				}
				else
				{
					curr_cascade_index_ = -1;
				}

				*light_view_proj_param_ = pvp.inv_view * sm_camera->ViewProjMatrix();

				float4x4 light_to_view = sm_camera->InverseViewMatrix() * pvp.view;
				float4x4 light_to_proj = light_to_view * pvp.proj;

				if (depth_texture_support_ && (index_in_pass > 0)
					&& ((PT_GenShadowMap == pass_type) || (PT_GenReflectiveShadowMap == pass_type)))
				{
					float q = sm_camera->FarPlane() / (sm_camera->FarPlane() - sm_camera->NearPlane());

					float4 near_q_far(sm_camera->NearPlane() * q, q, sm_camera->FarPlane(), 1 / sm_camera->FarPlane());
					depth_to_esm_pp_->SetParam(0, near_q_far);

					float4x4 inv_sm_proj = sm_camera->InverseProjMatrix();
					depth_to_esm_pp_->SetParam(1, inv_sm_proj);
				}

				float3 const & p = light.Position();
				float3 loc_es = MathLib::transform_coord(p, pvp.view);
				float4 light_pos_es_actived = float4(loc_es.x(), loc_es.y(), loc_es.z(), 1);

				float light_scale = std::min(light.Range() * 0.01f, 1.0f) * light_scale_;
				switch (type)
				{
				case LightSource::LT_Spot:
					{
						light_pos_es_actived.w() = light.CosOuterInner().x();
						light_dir_es_actived.w() = light.CosOuterInner().y();

						float const scale = light.CosOuterInner().w();
						float4x4 light_model = MathLib::scaling(scale * light_scale, scale * light_scale, light_scale);
						*light_volume_mv_param_ = light_model * light_to_view;
						*light_volume_mvp_param_ = light_model * light_to_proj;
					}
					break;

				case LightSource::LT_Point:
				case LightSource::LT_SphereArea:
				case LightSource::LT_TubeArea:
					if ((PC_Shadowing == pass_cat) || (PC_Shading == pass_cat))
					{
						float4x4 const light_model = MathLib::scaling(light_scale, light_scale, light_scale)
							* MathLib::to_matrix(light.Rotation()) * MathLib::translation(p);
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

				case LightSource::LT_Sun:
					*light_volume_mv_param_ = pvp.inv_proj;
					*light_volume_mvp_param_ = float4x4::Identity();
					break;

				default:
					BOOST_ASSERT(false);
					break;
				}
					
				*light_pos_es_param_ = light_pos_es_actived;
				*light_dir_es_param_ = light_dir_es_actived;
			}
			break;

		case LightSource::LT_Directional:
			{
				float3 const dir_es = MathLib::transform_normal(-light.Direction(), pvp.view);
				*light_dir_es_param_ = float4(dir_es.x(), dir_es.y(), dir_es.z(), 0);
			}
			*light_volume_mv_param_ = pvp.inv_proj;
			*light_volume_mvp_param_ = float4x4::Identity();
			break;

		case LightSource::LT_Ambient:
			{
				float3 const dir_es = MathLib::transform_normal(float3(0, 1, 0), pvp.view);
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

	void DeferredRenderingLayer::PostGenerateShadowMap(PerViewport const & pvp, int32_t org_no, int32_t index_in_pass)
	{
		RenderFactory& rf = Context::Instance().RenderFactoryInstance();
		RenderEngine& re = rf.RenderEngineInstance();
		RenderDeviceCaps const & caps = re.DeviceCaps();

		LightSource::LightType type = lights_[org_no]->Type();

		if (depth_texture_support_)
		{
			if (type != LightSource::LT_Sun)
			{
				sm_fb_->Attached(FrameBuffer::ATT_Color0)->Discard();
				depth_to_esm_pp_->Apply();
			}
		}

		PostProcessChainPtr pp_chain;
		if (LightSource::LT_Sun == type)
		{
			pp_chain = checked_pointer_cast<PostProcessChain>(csm_filter_pp_);
			if (tex_array_support_)
			{
				pp_chain->OutputPin(0, pvp.filtered_csm_texs[0], 0, index_in_pass - 1, 0);
			}
			else
			{
				pp_chain->OutputPin(0, pvp.filtered_csm_texs[index_in_pass - 1]);
			}
		}
		else
		{
			pp_chain = checked_pointer_cast<PostProcessChain>(sm_filter_pp_);
			if ((LightSource::LT_Point == type) || (LightSource::LT_SphereArea == type)
				|| (LightSource::LT_TubeArea == type))
			{
				pp_chain->OutputPin(0, filtered_sm_cube_texs_[sm_light_indices_[org_no].first], 0, 0, index_in_pass - 1);
			}
			else 
			{
				pp_chain->OutputPin(0, filtered_sm_2d_texs_[sm_light_indices_[org_no].first]);
				if (has_sss_objs_ && translucency_enabled_)
				{
					sm_tex_->CopyToTexture(*unfiltered_sm_2d_texs_[sm_light_indices_[org_no].first]);
				}
			}
		}

		int2 kernel_size;
		if (LightSource::LT_Sun == type)
		{
			float3 const & scale = cascaded_shadow_layer_->CascadeScales()[index_in_pass - 1];
			float2 blur_kernel_size = blur_size_light_space_ * float2(scale.x(), scale.y()) * static_cast<float>(csm_tex_->Width(0));
			kernel_size.x() = MathLib::clamp(static_cast<int32_t>(blur_kernel_size.x() + 0.5f), 1, 4);
			kernel_size.y() = MathLib::clamp(static_cast<int32_t>(blur_kernel_size.y() + 0.5f), 1, 4);
		}
		else
		{
			kernel_size.x() = 4;
			kernel_size.y() = 4;
		}
		checked_pointer_cast<SeparableLogGaussianFilterPostProcess>(pp_chain->GetPostProcess(0))->KernelRadius(kernel_size.x());
		checked_pointer_cast<SeparableLogGaussianFilterPostProcess>(pp_chain->GetPostProcess(1))->KernelRadius(kernel_size.y());

		checked_pointer_cast<LogGaussianBlurPostProcess>(pp_chain)->ESMScaleFactor(ESM_SCALE_FACTOR, *sm_fb_->GetViewport()->camera);
		pp_chain->Apply();

		if (!caps.pack_to_rgba_required)
		{
			if (LightSource::LT_Sun == type)
			{
				if (tex_array_support_)
				{
					if (static_cast<int32_t>(pvp.num_cascades) == index_in_pass)
					{
						pvp.filtered_csm_texs[0]->BuildMipSubLevels();
					}
				}
				else
				{
					pvp.filtered_csm_texs[index_in_pass - 1]->BuildMipSubLevels();
				}
			}
		}
	}

	void DeferredRenderingLayer::UpdateShadowing(PerViewport const & pvp, int32_t org_no)
	{
		RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
		
		auto const & light = *lights_[org_no];
		Camera* sm_camera = nullptr;
		LightSource::LightType type = light.Type();

		BOOST_ASSERT(0 == (light.Attrib() & LightSource::LSA_NoShadow));

		int32_t const light_index = sm_light_indices_[org_no].first;
		int32_t const shadowing_channel = sm_light_indices_[org_no].second;
		if (((light_index >= 0) && (0 == (light.Attrib() & LightSource::LSA_NoShadow)))
			|| (LightSource::LT_Sun == type))
		{
			switch (type)
			{
			case LightSource::LT_Spot:
				sm_camera = light.SMCamera(0).get();
				*filtered_sm_2d_tex_param_ = filtered_sm_2d_texs_[light_index];
				break;

			case LightSource::LT_Point:
			case LightSource::LT_SphereArea:
			case LightSource::LT_TubeArea:
				sm_camera = light.SMCamera(0).get();
				*filtered_sm_cube_tex_param_ = filtered_sm_cube_texs_[light_index];
				break;

			case LightSource::LT_Sun:
				{
					sm_camera = lights_[cascaded_shadow_index_]->SMCamera(0).get();

					*light_view_proj_param_ = pvp.inv_view * sm_camera->ViewProjMatrix();

					std::vector<float4> cascade_scale_bias(pvp.num_cascades);
					for (uint32_t i = 0; i < pvp.num_cascades; ++ i)
					{
						float3 const & scale = cascaded_shadow_layer_->CascadeScales()[i];
						float3 const & bias = cascaded_shadow_layer_->CascadeBiases()[i];
						cascade_scale_bias[i] = float4(scale.x(), scale.y(),
							bias.x(), bias.y());
					}
					*cascade_intervals_param_ = cascaded_shadow_layer_->CascadeIntervals();
					*cascade_scale_bias_param_ = cascade_scale_bias;
					*num_cascades_param_ = static_cast<int32_t>(pvp.num_cascades);

					float4x4 light_view = pvp.inv_view * sm_camera->ViewMatrix();
					*view_z_to_light_view_param_ = light_view.Col(2);
				}
				if (tex_array_support_)
				{
					*filtered_csm_texs_param_[0] = pvp.filtered_csm_texs[0];
				}
				else
				{
					for (uint32_t i = 0; i < pvp.num_cascades; ++ i)
					{
						*filtered_csm_texs_param_[i] = pvp.filtered_csm_texs[i];
					}
				}
				break;

			default:
				break;
			}
		}

		if (org_no == projective_light_index_)
		{
			re.BindFrameBuffer(pvp.projective_shadowing_fb);
			pvp.projective_shadowing_fb->Attached(FrameBuffer::ATT_Color0)->ClearColor(Color(1, 1, 1, 1));
		}
		else
		{
			re.BindFrameBuffer(pvp.shadowing_fb);
			if (shadowing_channel <= 0)
			{
				pvp.shadowing_fb->Attached(FrameBuffer::ATT_Color0)->ClearColor(Color(1, 1, 1, 1));
			}
		}

		if (sm_camera)
		{
			*esm_scale_factor_param_ = ESM_SCALE_FACTOR / (sm_camera->FarPlane() - sm_camera->NearPlane());
			*sm_far_plane_param_ = sm_camera->FarPlane();
		}

		re.Render(*technique_shadows_[type][shadowing_channel], *light_volume_rl_[type]);
	}

#if DEFAULT_DEFERRED == TRIDITIONAL_DEFERRED
	void DeferredRenderingLayer::UpdateLighting(PerViewport const & pvp, LightSource::LightType type, int32_t org_no)
	{
		RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();

		LightSource const & light = *lights_[org_no];
		int32_t shadowing_channel;
		if (0 == (light.Attrib() & LightSource::LSA_NoShadow))
		{
			shadowing_channel = sm_light_indices_[org_no].second;
		}
		else
		{
			shadowing_channel = -1;
		}
		*shadowing_channel_param_ = shadowing_channel;

		re.BindFrameBuffer(pvp.lighting_fb);

		RenderLayoutPtr const & rl = light_volume_rl_[type];

		if ((LightSource::LT_Point == type) || (LightSource::LT_Spot == type)
			|| (LightSource::LT_SphereArea == type) || (LightSource::LT_TubeArea == type))
		{
			re.Render(*technique_light_stencil_, *rl);
		}

		if (LightSource::LT_Ambient == type)
		{
			if (light.SkylightTexY())
			{
				*skylight_y_cube_tex_param_ = light.SkylightTexY();
				*skylight_c_cube_tex_param_ = light.SkylightTexC();

				uint32_t const mip = light.SkylightTexY()->NumMipMaps();
				*skylight_diff_spec_mip_param_ = int3(mip - 1, mip - 2, 1);
				*skylight_mip_bias_param_ = mip / -2.0f;

				*inv_view_param_ = pvp.inv_view;
			}
			else
			{
				*skylight_diff_spec_mip_param_ = int3(0, 0, 0);
				*skylight_mip_bias_param_ = 0.0f;
			}
		}

		re.Render(*technique_lights_[type], *rl);
	}

	void DeferredRenderingLayer::UpdateShading(PerViewport const & pvp, uint32_t g_buffer_index)
	{
		RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();

		*g_buffer_tex_param_ = pvp.g_buffer_rt0_tex;
		*g_buffer_1_tex_param_ = pvp.g_buffer_rt1_tex;
		*depth_tex_param_ = pvp.g_buffer_depth_tex;
		*lighting_tex_param_ = pvp.lighting_tex;
		*light_volume_mv_param_ = pvp.inv_proj;

		if (Opaque_GBuffer == g_buffer_index)
		{
			re.BindFrameBuffer(pvp.curr_merged_shading_fb);
			re.Render(*technique_no_lighting_, *rl_quad_);
		}
		else
		{
			re.BindFrameBuffer(pvp.shading_fb);
			re.CurFrameBuffer()->Attached(FrameBuffer::ATT_Color0)->Discard();
		}
		re.Render(*technique_shading_, *rl_quad_);
	}
#endif

	void DeferredRenderingLayer::MergeIndirectLighting(PerViewport const & pvp, uint32_t g_buffer_index)
	{
		if ((indirect_lighting_enabled_ && !(pvp.attrib & VPAM_NoGI)) && (illum_ != 1))
		{
			pvp.il_layer->CalcIndirectLighting(pvp.prev_merged_shading_tex, pvp.proj_to_prev);
			this->AccumulateToLightingTex(pvp, g_buffer_index);
		}
	}

	void DeferredRenderingLayer::MergeSSVO(PerViewport const & pvp, uint32_t g_buffer_index)
	{
		if (pvp.ssvo_enabled && !(pvp.attrib & VPAM_NoSSVO))
		{
			ssvo_pp_->InputPin(0, pvp.g_buffer_rt0_tex);
			ssvo_pp_->InputPin(1, pvp.g_buffer_depth_tex);
			ssvo_pp_->OutputPin(0, pvp.small_ssvo_tex);
			ssvo_pp_->Apply();

			ssvo_blur_pp_->InputPin(0, pvp.small_ssvo_tex);
			ssvo_blur_pp_->InputPin(1, pvp.g_buffer_depth_tex);
			ssvo_blur_pp_->OutputPin(0, (Opaque_GBuffer == g_buffer_index) ? pvp.curr_merged_shading_tex : pvp.shading_tex);
			ssvo_blur_pp_->Apply();
		}
	}

	void DeferredRenderingLayer::AddTranslucency(uint32_t org_no,
			PerViewport const & pvp, uint32_t g_buffer_index)
	{
		auto & light = lights_[org_no];
		LightSource::LightType const type = light->Type();
		int32_t const light_index = sm_light_indices_[org_no].first;
		if (light->Enabled() && pvp.light_visibles[org_no]
			&& (((light_index >= 0) && (0 == (light->Attrib() & LightSource::LSA_NoShadow)))
				|| (LightSource::LT_Sun == type)))
		{
			Camera* light_camera = nullptr;
			switch (type)
			{
			case LightSource::LT_Spot:
				light_camera = light->SMCamera(0).get();
				translucency_pp_->InputPin(3, unfiltered_sm_2d_texs_[light_index]);
				break;

			case LightSource::LT_Sun:
				// TODO
				break;

			default:
				break;
			}

			if (light_camera)
			{
				translucency_pp_->OutputFrameBuffer()->Attach(FrameBuffer::ATT_DepthStencil,
					pvp.g_buffer->Attached(FrameBuffer::ATT_DepthStencil));
				translucency_pp_->InputPin(0, pvp.g_buffer_rt0_tex);
				translucency_pp_->InputPin(1, pvp.g_buffer_rt1_tex);
				translucency_pp_->InputPin(2, pvp.g_buffer_depth_tex);
				translucency_pp_->OutputPin(0,
					(Opaque_GBuffer == g_buffer_index) ? pvp.curr_merged_shading_tex : pvp.shading_tex);

				Camera const & scene_camera = *pvp.frame_buffer->GetViewport()->camera;

				translucency_pp_->SetParam(0, pvp.inv_view * light_camera->ViewProjMatrix());
				translucency_pp_->SetParam(1, pvp.inv_view * light_camera->ViewMatrix());
				translucency_pp_->SetParam(2, pvp.inv_proj);
				translucency_pp_->SetParam(3, MathLib::transform_coord(light->Position(), pvp.view));
				translucency_pp_->SetParam(4, float3(light->Color()));
				translucency_pp_->SetParam(5, light->Falloff());
				translucency_pp_->SetParam(7, scene_camera.FarPlane());
				translucency_pp_->SetParam(8, light_camera->FarPlane());
				translucency_pp_->Apply();
			}
		}
	}

	void DeferredRenderingLayer::AddSSS(PerViewport const & pvp)
	{
		sss_blur_pp_->OutputFrameBuffer()->Attach(FrameBuffer::ATT_DepthStencil,
			pvp.g_buffer->Attached(FrameBuffer::ATT_DepthStencil));
		sss_blur_pp_->InputPin(0, pvp.curr_merged_shading_tex);
		sss_blur_pp_->InputPin(1, pvp.g_buffer_depth_tex);
		sss_blur_pp_->OutputPin(0, pvp.curr_merged_shading_tex);
		sss_blur_pp_->Apply();
	}

	void DeferredRenderingLayer::AddSSR(PerViewport const & pvp)
	{
		RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();

		re.BindFrameBuffer(pvp.curr_merged_shading_fb);
		ssr_pp_->InputPin(0, pvp.g_buffer_rt0_tex);
		ssr_pp_->InputPin(1, pvp.g_buffer_rt1_tex);
		ssr_pp_->InputPin(2, pvp.g_buffer_depth_tex);
		ssr_pp_->InputPin(3, pvp.prev_merged_shading_tex);
		ssr_pp_->InputPin(4, pvp.curr_merged_depth_tex);
		ssr_pp_->Apply();
	}

	void DeferredRenderingLayer::AddAtmospheric(PerViewport const & pvp)
	{
		RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();

		re.BindFrameBuffer(pvp.curr_merged_shading_fb);
		atmospheric_pp_->SetParam(0, pvp.inv_proj);
		atmospheric_pp_->InputPin(0, pvp.curr_merged_depth_tex);
		atmospheric_pp_->Render();
	}

	void DeferredRenderingLayer::AddTAA(PerViewport const & pvp)
	{
		RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();

		re.BindFrameBuffer(pvp.frame_buffer);
		pvp.frame_buffer->Discard(FrameBuffer::CBM_Color | FrameBuffer::CBM_Depth | FrameBuffer::CBM_Stencil);
		{
			*depth_tex_param_ = pvp.curr_merged_depth_tex;

			Camera const & camera = *pvp.frame_buffer->GetViewport()->camera;
			float q = camera.FarPlane() / (camera.FarPlane() - camera.NearPlane());
			float2 near_q(camera.NearPlane() * q, q);
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

	void DeferredRenderingLayer::MergeShadingAndDepth(PerViewport const & pvp, uint32_t g_buffer_index)
	{
		RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();

		if (g_buffer_index != 0)
		{
			re.BindFrameBuffer(pvp.curr_merged_shading_fb);
			*shading_tex_param_ = pvp.shading_tex;
			re.Render(*technique_merge_shadings_[g_buffer_index != 0], *rl_quad_);
		}

		re.BindFrameBuffer(pvp.curr_merged_depth_fb);
		*depth_tex_param_ = pvp.g_buffer_depth_tex;
		re.Render(*technique_merge_depths_[g_buffer_index != 0], *rl_quad_);
	}

	void DeferredRenderingLayer::SetupViewportGI(uint32_t vp, bool ssgi_enable)
	{
		PerViewport& pvp = viewports_[vp];
		if (ssgi_enable)
		{
			pvp.il_layer = MakeSharedPtr<SSGILayer>();
		}
		else
		{
			if (rsm_fb_)
			{
				pvp.il_layer = MakeSharedPtr<MultiResSILLayer>();
			}
			else
			{
				pvp.il_layer.reset();
			}
		}

		if (pvp.il_layer && pvp.g_buffer_rt0_tex && rsm_texs_[0])
		{
			pvp.il_layer->GBuffer(pvp.g_buffer_rt0_tex, pvp.g_buffer_rt1_tex,
				pvp.g_buffer_depth_tex);
			pvp.il_layer->RSM(rsm_texs_[0], rsm_texs_[1], sm_tex_);
		}
	}

	void DeferredRenderingLayer::SetCascadedShadowType(CascadedShadowLayerType type)
	{
		switch (type)
		{
		case CSLT_Auto:
			{
				RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
				RenderDeviceCaps const & caps = re.DeviceCaps();
				if (caps.cs_support && (caps.max_shader_model >= ShaderModel(5, 0)))
				{
					cascaded_shadow_layer_ = MakeSharedPtr<SDSMCascadedShadowLayer>();
				}
				else
				{
					cascaded_shadow_layer_ = MakeSharedPtr<PSSMCascadedShadowLayer>();
				}
			}
			break;

		case CSLT_PSSM:
			cascaded_shadow_layer_ = MakeSharedPtr<PSSMCascadedShadowLayer>();
			break;

		case CSLT_SDSM:
		default:
			cascaded_shadow_layer_ = MakeSharedPtr<SDSMCascadedShadowLayer>();
			break;
		}
	}

	void DeferredRenderingLayer::SetViewportCascades(uint32_t vp, uint32_t num_cascades, float pssm_lambda)
	{
		PerViewport& pvp = viewports_[vp];
		pvp.num_cascades = num_cascades;
		if (CSLT_PSSM == cascaded_shadow_layer_->Type())
		{
			checked_pointer_cast<PSSMCascadedShadowLayer>(cascaded_shadow_layer_)->Lambda(pssm_lambda);
		}
	}

	void DeferredRenderingLayer::AccumulateToLightingTex(PerViewport const & pvp, uint32_t g_buffer_index)
	{
		PostProcessPtr const & copy_to_light_buffer_pp = (0 == illum_) ? copy_to_light_buffer_pp_ : copy_to_light_buffer_i_pp_;
		copy_to_light_buffer_pp->SetParam(0, indirect_scale_ * 256 / VPL_COUNT);
		copy_to_light_buffer_pp->SetParam(1, float2(1.0f / pvp.g_buffer_rt0_tex->Width(0), 1.0f / pvp.g_buffer_rt0_tex->Height(0)));
		copy_to_light_buffer_pp->SetParam(2, pvp.depth_near_far_invfar);
		copy_to_light_buffer_pp->SetParam(3, pvp.inv_proj);
		copy_to_light_buffer_pp->InputPin(0, pvp.il_layer->IndirectLightingTex());
		copy_to_light_buffer_pp->InputPin(1, pvp.g_buffer_rt0_tex);
		copy_to_light_buffer_pp->InputPin(2, pvp.g_buffer_rt1_tex);
		copy_to_light_buffer_pp->InputPin(3, pvp.g_buffer_depth_tex);
		copy_to_light_buffer_pp->OutputPin(0,
			(Opaque_GBuffer == g_buffer_index) ? pvp.curr_merged_shading_tex : pvp.shading_tex);
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

	uint32_t DeferredRenderingLayer::ComposePassScanCode(uint32_t vp_index, PassType pass_type,
		int32_t org_no, int32_t index_in_pass, bool is_profile) const
	{
		return (vp_index << 28) | (pass_type << 18) | (org_no << 6)
			| (index_in_pass << 1) | (is_profile ? 1 : 0);
	}

	void DeferredRenderingLayer::DecomposePassScanCode(uint32_t& vp_index, PassType& pass_type,
		int32_t& org_no, int32_t& index_in_pass, bool& is_profile, uint32_t code) const
	{
		vp_index = code >> 28;				//  4 bits, [31 - 28]
		pass_type = static_cast<PassType>((code >> 18) & 0x03FF);	//  10 bits, [27 - 18]
		org_no = (code >> 6) & 0x0FFF;		// 12 bits, [17 - 6]
		index_in_pass = (code >> 1) & 0x1F;		//  5 bits, [5 -  1]
		is_profile = (code & 1) ? true : false;
	}

#if DEFAULT_DEFERRED == LIGHT_INDEXED_DEFERRED
	void DeferredRenderingLayer::UpdateLightIndexedLighting(PerViewport const & pvp, uint32_t g_buffer_index)
	{
		std::vector<uint32_t> directional_lights;
		std::vector<uint32_t> point_lights_shadow;
		std::vector<uint32_t> point_lights_no_shadow;
		std::vector<uint32_t> spot_lights_shadow;
		std::vector<uint32_t> spot_lights_no_shadow;
		std::vector<uint32_t> sphere_area_lights_shadow;
		std::vector<uint32_t> sphere_area_lights_no_shadow;
		std::vector<uint32_t> tube_area_lights_shadow;
		std::vector<uint32_t> tube_area_lights_no_shadow;
		for (uint32_t li = 0; li < lights_.size(); ++ li)
		{
			auto const & light = *lights_[li];
			if (light.Enabled() && pvp.light_visibles[li])
			{
				LightSource::LightType const type = light.Type();
				switch (type)
				{
				case LightSource::LT_Ambient:
				case LightSource::LT_Sun:
					this->UpdateLightIndexedLightingAmbientSun(pvp, type, li, g_buffer_index);
					break;

				case LightSource::LT_Directional:
					directional_lights.push_back(li);
					break;

				case LightSource::LT_Point:
					if (light.Attrib() & LightSource::LSA_NoShadow)
					{
						point_lights_no_shadow.push_back(li);
					}
					else
					{
						point_lights_shadow.push_back(li);
					}
					break;

				case LightSource::LT_Spot:
					if (light.Attrib() & LightSource::LSA_NoShadow)
					{
						spot_lights_no_shadow.push_back(li);
					}
					else
					{
						spot_lights_shadow.push_back(li);
					}
					break;

				case LightSource::LT_SphereArea:
					if (light.Attrib() & LightSource::LSA_NoShadow)
					{
						sphere_area_lights_no_shadow.push_back(li);
					}
					else
					{
						sphere_area_lights_shadow.push_back(li);
					}
					break;

				case LightSource::LT_TubeArea:
					if (light.Attrib() & LightSource::LSA_NoShadow)
					{
						tube_area_lights_no_shadow.push_back(li);
					}
					else
					{
						tube_area_lights_shadow.push_back(li);
					}
					break;

				default:
					BOOST_ASSERT(false);
					break;
				}
			}
		}

		{
			uint32_t li = 0;
			while (li < directional_lights.size())
			{
				uint32_t nl = std::min(light_batch_, static_cast<uint32_t>(directional_lights.size() - li));
				auto iter_beg = directional_lights.begin() + li;
				auto iter_end = iter_beg + nl;
				this->UpdateLightIndexedLightingDirectional(pvp, g_buffer_index, iter_beg, iter_end);
				li += nl;
			}
		}
		{
			uint32_t li = 0;
			while (li < point_lights_no_shadow.size())
			{
				uint32_t nl = std::min(light_batch_, static_cast<uint32_t>(point_lights_no_shadow.size() - li));
				auto iter_beg = point_lights_no_shadow.begin() + li;
				auto iter_end = iter_beg + nl;
				this->UpdateLightIndexedLightingPointSpotArea(pvp, g_buffer_index, iter_beg, iter_end);
				li += nl;
			}
		}
		{
			uint32_t li = 0;
			while (li < point_lights_shadow.size())
			{
				uint32_t nl = std::min(light_batch_, static_cast<uint32_t>(point_lights_shadow.size() - li));
				auto iter_beg = point_lights_shadow.begin() + li;
				auto iter_end = iter_beg + nl;
				this->UpdateLightIndexedLightingPointSpotArea(pvp, g_buffer_index, iter_beg, iter_end);
				li += nl;
			}
		}
		{
			uint32_t li = 0;
			while (li < spot_lights_no_shadow.size())
			{
				uint32_t nl = std::min(light_batch_, static_cast<uint32_t>(spot_lights_no_shadow.size() - li));
				auto iter_beg = spot_lights_no_shadow.begin() + li;
				auto iter_end = iter_beg + nl;
				this->UpdateLightIndexedLightingPointSpotArea(pvp, g_buffer_index, iter_beg, iter_end);
				li += nl;
			}
		}
		{
			uint32_t li = 0;
			while (li < spot_lights_shadow.size())
			{
				uint32_t nl = std::min(light_batch_, static_cast<uint32_t>(spot_lights_shadow.size() - li));
				auto iter_beg = spot_lights_shadow.begin() + li;
				auto iter_end = iter_beg + nl;
				this->UpdateLightIndexedLightingPointSpotArea(pvp, g_buffer_index, iter_beg, iter_end);
				li += nl;
			}
		}
		{
			uint32_t li = 0;
			while (li < sphere_area_lights_no_shadow.size())
			{
				uint32_t nl = std::min(light_batch_, static_cast<uint32_t>(sphere_area_lights_no_shadow.size() - li));
				auto iter_beg = sphere_area_lights_no_shadow.begin() + li;
				auto iter_end = iter_beg + nl;
				this->UpdateLightIndexedLightingPointSpotArea(pvp, g_buffer_index, iter_beg, iter_end);
				li += nl;
			}
		}
		{
			uint32_t li = 0;
			while (li < sphere_area_lights_shadow.size())
			{
				uint32_t nl = std::min(light_batch_, static_cast<uint32_t>(sphere_area_lights_shadow.size() - li));
				auto iter_beg = sphere_area_lights_shadow.begin() + li;
				auto iter_end = iter_beg + nl;
				this->UpdateLightIndexedLightingPointSpotArea(pvp, g_buffer_index, iter_beg, iter_end);
				li += nl;
			}
		}
		{
			uint32_t li = 0;
			while (li < tube_area_lights_no_shadow.size())
			{
				uint32_t nl = std::min(light_batch_, static_cast<uint32_t>(tube_area_lights_no_shadow.size() - li));
				auto iter_beg = tube_area_lights_no_shadow.begin() + li;
				auto iter_end = iter_beg + nl;
				this->UpdateLightIndexedLightingPointSpotArea(pvp, g_buffer_index, iter_beg, iter_end);
				li += nl;
			}
		}
		{
			uint32_t li = 0;
			while (li < tube_area_lights_shadow.size())
			{
				uint32_t nl = std::min(light_batch_, static_cast<uint32_t>(tube_area_lights_shadow.size() - li));
				auto iter_beg = tube_area_lights_shadow.begin() + li;
				auto iter_end = iter_beg + nl;
				this->UpdateLightIndexedLightingPointSpotArea(pvp, g_buffer_index, iter_beg, iter_end);
				li += nl;
			}
		}
	}

	void DeferredRenderingLayer::UpdateLightIndexedLightingAmbientSun(PerViewport const & pvp, LightSource::LightType type,
		int32_t org_no, uint32_t g_buffer_index)
	{
		RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();

		auto const & light = *lights_[org_no];
		int32_t const attr = light.Attrib();
		
		int32_t shadowing_channel;
		if (0 == (attr & LightSource::LSA_NoShadow))
		{
			shadowing_channel = sm_light_indices_[org_no].second;
		}
		else
		{
			shadowing_channel = -1;
		}
		*shadowing_channel_param_ = shadowing_channel;

		*light_attrib_param_ = float4(attr & LightSource::LSA_NoDiffuse ? 0.0f : 1.0f,
			attr & LightSource::LSA_NoSpecular ? 0.0f : 1.0f, 0, 0);
		*light_color_param_ = light.Color();
		*light_falloff_range_param_ = float4(light.Falloff().x(), light.Falloff().y(),
			light.Falloff().z(), 0);

		RenderTechniquePtr tech;
		if (LightSource::LT_Sun == type)
		{
			float3 dir_es = MathLib::transform_normal(-light.Direction(), pvp.view);
			CameraPtr sm_camera = light.SMCamera(0);
			*light_dir_es_param_ = float4(dir_es.x(), dir_es.y(), dir_es.z(), 0);

			sm_fb_->GetViewport()->camera = sm_camera;

			curr_cascade_index_ = -1;
			*light_view_proj_param_ = pvp.inv_view * sm_camera->ViewProjMatrix();

			float3 const & p = light.Position();
			float3 loc_es = MathLib::transform_coord(p, pvp.view);
			*light_pos_es_param_ = float4(loc_es.x(), loc_es.y(), loc_es.z(), 1);

			tech = technique_lidr_sun_;
		}
		else
		{
			BOOST_ASSERT(LightSource::LT_Ambient == type);

			float3 dir_es = MathLib::transform_normal(float3(0, 1, 0), pvp.view);
			*light_dir_es_param_ = float4(dir_es.x(), dir_es.y(), dir_es.z(), 0);

			if (light.SkylightTexY())
			{
				*skylight_y_cube_tex_param_ = light.SkylightTexY();
				*skylight_c_cube_tex_param_ = light.SkylightTexC();

				uint32_t const mip = light.SkylightTexY()->NumMipMaps();
				*skylight_diff_spec_mip_param_ = int3(mip - 1, mip - 2, 1);
				*skylight_mip_bias_param_ = mip / -2.0f;

				*inv_view_param_ = pvp.inv_view;
			}
			else
			{
				*skylight_diff_spec_mip_param_ = int3(0, 0, 0);
				*skylight_mip_bias_param_ = 0.0f;
			}

			tech = technique_lidr_ambient_;
		}

		*g_buffer_tex_param_ = pvp.g_buffer_rt0_tex;
		*g_buffer_1_tex_param_ = pvp.g_buffer_rt1_tex;
		*light_volume_mv_param_ = pvp.inv_proj;

		if (Opaque_GBuffer == g_buffer_index)
		{
			re.BindFrameBuffer(pvp.curr_merged_shading_fb);
		}
		else
		{
			re.BindFrameBuffer(pvp.shading_fb);
		}
		re.Render(*tech, *rl_quad_);
	}

	void DeferredRenderingLayer::UpdateLightIndexedLightingDirectional(PerViewport const & pvp,
		uint32_t g_buffer_index, std::vector<uint32_t>::const_iterator iter_beg, std::vector<uint32_t>::const_iterator iter_end)
	{
		std::vector<float4> lights_color;
		std::vector<float4> lights_dir_es;
		std::vector<float4> lights_attrib;
		for (auto iter = iter_beg; iter != iter_end; ++ iter)
		{
			auto const & light = *lights_[*iter];
			BOOST_ASSERT(LightSource::LT_Directional == light.Type());
			int32_t attr = light.Attrib();

			lights_color.push_back(light.Color());

			float3 dir_es = MathLib::transform_normal(-light.Direction(), pvp.view);
			lights_dir_es.push_back(float4(dir_es.x(), dir_es.y(), dir_es.z(), 0));

			lights_attrib.push_back(float4(attr & LightSource::LSA_NoDiffuse ? 0.0f : 1.0f,
				attr & LightSource::LSA_NoSpecular ? 0.0f : 1.0f, 0, 0));
		}

		lights_attrib[0].w() = iter_end - iter_beg + 0.5f;

		*lights_color_param_ = lights_color;
		*lights_dir_es_param_ = lights_dir_es;
		*lights_attrib_param_ = lights_attrib;

		*g_buffer_tex_param_ = pvp.g_buffer_rt0_tex;
		*g_buffer_1_tex_param_ = pvp.g_buffer_rt1_tex;
		*depth_tex_param_ = pvp.g_buffer_depth_tex;
		*light_volume_mv_param_ = pvp.inv_proj;

		RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
		re.BindFrameBuffer(
			(Opaque_GBuffer == g_buffer_index) ? pvp.curr_merged_shading_fb : pvp.shading_fb);
		re.Render(*technique_lidr_directional_, *rl_quad_);
	}

	void DeferredRenderingLayer::UpdateLightIndexedLightingPointSpotArea(PerViewport const & pvp, uint32_t g_buffer_index,
		std::vector<uint32_t>::const_iterator iter_beg, std::vector<uint32_t>::const_iterator iter_end)
	{
		RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();

		re.BindFrameBuffer(pvp.light_index_fb);
		pvp.light_index_fb->Attached(FrameBuffer::ATT_Color0)->ClearColor(Color(0, 0, 0, 0));

		*min_max_depth_tex_param_ = pvp.g_buffer_min_max_depth_texs.back();

		uint32_t w = (pvp.g_buffer_depth_tex->Width(0) + TILE_SIZE - 1) & ~(TILE_SIZE - 1);
		uint32_t h = (pvp.g_buffer_depth_tex->Height(0) + TILE_SIZE - 1) & ~(TILE_SIZE - 1);
		float2 tile_scale(w / (2.0f * TILE_SIZE), h / (2.0f * TILE_SIZE));
		*tile_scale_param_ = float4(tile_scale.x(), tile_scale.y(),
			static_cast<float>(pvp.light_index_tex->Width(0)),
			static_cast<float>(pvp.light_index_tex->Height(0)));

		*camera_proj_01_param_ = float2(pvp.proj(0, 0) * tile_scale.x(), pvp.proj(1, 1) * tile_scale.y());

		LightSource::LightType type = lights_[*iter_beg]->Type();
		bool with_shadow = !(lights_[*iter_beg]->Attrib() & LightSource::LSA_NoShadow);

		BOOST_ASSERT((LightSource::LT_Point == type) || (LightSource::LT_Spot == type)
			|| (LightSource::LT_SphereArea == type) || (LightSource::LT_TubeArea == type));

		std::vector<float4> lights_color;
		std::vector<float4> lights_pos_es;
		std::vector<float4> lights_dir_es;
		std::vector<float4> lights_falloff_range;
		std::vector<float4> lights_attrib;
		std::vector<float4> lights_radius_extend;
		std::vector<float3> lights_aabb_min;
		std::vector<float3> lights_aabb_max;
		for (auto iter = iter_beg; iter != iter_end; ++ iter)
		{
			auto const & light = *lights_[*iter];
			BOOST_ASSERT(type == light.Type());

			int32_t const attr = light.Attrib();

			lights_color.push_back(light.Color());

			float3 const & p = light.Position();
			float3 const loc_es = MathLib::transform_coord(p, pvp.view);
			lights_pos_es.push_back(float4(loc_es.x(), loc_es.y(), loc_es.z(), 1));

			float3 dir_es(0, 0, 0);
			if (LightSource::LT_Spot == type)
			{
				dir_es = MathLib::transform_normal(light.Direction(), pvp.view);
			}
			lights_dir_es.push_back(float4(dir_es.x(), dir_es.y(), dir_es.z(), 0));

			if (LightSource::LT_Spot == type)
			{
				lights_pos_es.back().w() = light.CosOuterInner().x();
				lights_dir_es.back().w() = light.CosOuterInner().y();
			}

			int channel = -1;
			if (with_shadow)
			{
				BOOST_ASSERT(0 == (light.Attrib() & LightSource::LSA_NoShadow));
				BOOST_ASSERT(iter - iter_beg < 4);
				channel = sm_light_indices_[*iter].second;
			}

			lights_attrib.push_back(float4(attr & LightSource::LSA_NoDiffuse ? 0.0f : 1.0f,
				attr & LightSource::LSA_NoSpecular ? 0.0f : 1.0f, channel + 0.5f, 0));

			float3 extend_es = MathLib::transform_normal(light.Extend(), pvp.view);
			lights_radius_extend.push_back(float4(light.Radius(), extend_es.x(),
				extend_es.y(), extend_es.z()));

			float const range = light.Range() * light_scale_;
			AABBox aabb(float3(0, 0, 0), float3(0, 0, 0));
			if (LightSource::LT_Spot == type)
			{
				float4x4 light_to_view = light.SMCamera(0)->InverseViewMatrix() * pvp.view;
				float const scale = light.CosOuterInner().w();
				float4x4 light_model = MathLib::scaling(range * 0.01f * float3(scale, scale, 1));
				float4x4 light_mv = light_model * light_to_view;
				aabb = MathLib::transform_aabb(cone_aabb_, light_mv);
			}
			lights_falloff_range.push_back(float4(light.Falloff().x(), light.Falloff().y(),
				light.Falloff().z(), range));
			lights_aabb_min.push_back(aabb.Min());
			lights_aabb_max.push_back(aabb.Max());
		}

		if (lights_attrib.size() < 3)
		{
			lights_attrib.resize(3);
		}

		lights_attrib[0].w() = iter_end - iter_beg + 0.5f;

		w = pvp.g_buffer_depth_tex->Width(0);
		h = pvp.g_buffer_depth_tex->Height(0);
		float2 tc_to_tile_scale(static_cast<float>(w) / ((w + TILE_SIZE - 1) & ~(TILE_SIZE - 1)),
			static_cast<float>(h) / ((h + TILE_SIZE - 1) & ~(TILE_SIZE - 1)));
		lights_attrib[1].w() = tc_to_tile_scale.x();
		lights_attrib[2].w() = tc_to_tile_scale.y();

		*lights_color_param_ = lights_color;
		*lights_pos_es_param_ = lights_pos_es;
		*lights_dir_es_param_ = lights_dir_es;
		*lights_falloff_range_param_ = lights_falloff_range;
		*lights_attrib_param_ = lights_attrib;
		*lights_radius_extend_param_ = lights_radius_extend;
		*lights_aabb_min_param_ = lights_aabb_min;
		*lights_aabb_max_param_ = lights_aabb_max;

		RenderTechniquePtr tech;
		if ((LightSource::LT_Point == type) || (LightSource::LT_SphereArea == type)
			|| (LightSource::LT_TubeArea == type))
		{
			tech = technique_draw_light_index_point_;
		}
		else
		{
			tech = technique_draw_light_index_spot_;
		}
		re.Render(*tech, *rl_quad_);

		*light_index_tex_param_ = pvp.light_index_tex;
		*g_buffer_tex_param_ = pvp.g_buffer_rt0_tex;
		*g_buffer_1_tex_param_ = pvp.g_buffer_rt1_tex;
		*depth_tex_param_ = pvp.g_buffer_depth_tex;
		*light_volume_mv_param_ = pvp.inv_proj;

		if (Opaque_GBuffer == g_buffer_index)
		{
			re.BindFrameBuffer(pvp.curr_merged_shading_fb);
		}
		else
		{
			re.BindFrameBuffer(pvp.shading_fb);
		}
		switch (type)
		{
		case LightSource::LT_Point:
			tech = with_shadow ? technique_lidr_point_shadow_ : technique_lidr_point_no_shadow_;
			break;
		
		case LightSource::LT_Spot:
			tech = with_shadow ? technique_lidr_spot_shadow_ : technique_lidr_spot_no_shadow_;
			break;

		case LightSource::LT_SphereArea:
			tech = with_shadow ? technique_lidr_sphere_area_shadow_ : technique_lidr_sphere_area_no_shadow_;
			break;

		case LightSource::LT_TubeArea:
			tech = with_shadow ? technique_lidr_tube_area_shadow_ : technique_lidr_tube_area_no_shadow_;
			break;

		default:
			BOOST_ASSERT(false);
			break;
		}
		re.Render(*tech, *rl_quad_);
	}

	void DeferredRenderingLayer::CreateDepthMinMaxMap(PerViewport const & pvp)
	{
		uint32_t w = pvp.g_buffer_depth_tex->Width(0);
		uint32_t h = pvp.g_buffer_depth_tex->Height(0);
		depth_to_min_max_pp_->SetParam(0, float2(0.5f / w, 0.5f / h));
		depth_to_min_max_pp_->SetParam(1, float2(static_cast<float>((w + 1) & ~1) / w,
			static_cast<float>((h + 1) & ~1) / h));
		depth_to_min_max_pp_->InputPin(0, pvp.g_buffer_depth_tex);
		depth_to_min_max_pp_->OutputPin(0, pvp.g_buffer_min_max_depth_texs[0]);
		depth_to_min_max_pp_->Apply();

		for (uint32_t i = 1; i < pvp.g_buffer_min_max_depth_texs.size(); ++ i)
		{
			w = pvp.g_buffer_min_max_depth_texs[i - 1]->Width(0);
			h = pvp.g_buffer_min_max_depth_texs[i - 1]->Height(0);
			reduce_min_max_pp_->SetParam(0, float2(0.5f / w, 0.5f / h));
			reduce_min_max_pp_->SetParam(1, float2(static_cast<float>((w + 1) & ~1) / w,
				static_cast<float>((h + 1) & ~1) / h));
			reduce_min_max_pp_->InputPin(0, pvp.g_buffer_min_max_depth_texs[i - 1]);
			reduce_min_max_pp_->OutputPin(0, pvp.g_buffer_min_max_depth_texs[i]);
			reduce_min_max_pp_->Apply();
		}
	}


	void DeferredRenderingLayer::UpdateTileBasedLighting(PerViewport const & pvp, uint32_t g_buffer_index)
	{
		RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();

		re.BindFrameBuffer(pvp.lighting_mask_fb);
		re.CurFrameBuffer()->Attached(FrameBuffer::ATT_Color0)->ClearColor(Color(0, 0, 0, 0));
		re.Render(*technique_tbdr_lighting_mask_, *rl_quad_);

		*min_max_depth_tex_param_ = pvp.g_buffer_min_max_depth_texs.back();

		uint32_t w = pvp.g_buffer_depth_tex->Width(0);
		uint32_t h = pvp.g_buffer_depth_tex->Height(0);
		float2 tile_scale(((w + TILE_SIZE - 1) & ~(TILE_SIZE - 1)) / (2.0f * TILE_SIZE),
			((h + TILE_SIZE - 1) & ~(TILE_SIZE - 1)) / (2.0f * TILE_SIZE));
		*tile_scale_param_ = float4(tile_scale.x(), tile_scale.y(), 0, 0);
		*camera_proj_01_param_ = float2(pvp.proj(0, 0) * tile_scale.x(), pvp.proj(1, 1) * tile_scale.y());

		*g_buffer_tex_param_ = pvp.g_buffer_rt0_tex;
		*g_buffer_1_tex_param_ = pvp.g_buffer_rt1_tex;
		*depth_tex_param_ = pvp.g_buffer_depth_tex;

		float3 upper_left = MathLib::transform_coord(float3(-1, +1, 1), pvp.inv_proj);
		float3 upper_right = MathLib::transform_coord(float3(+1, +1, 1), pvp.inv_proj);
		float3 lower_left = MathLib::transform_coord(float3(-1, -1, 1), pvp.inv_proj);
		*upper_left_param_ = upper_left;
		*x_dir_param_ = upper_right - upper_left;
		*y_dir_param_ = lower_left - upper_left;

		*inv_width_height_param_ = float2(1.0f / w, 1.0f / h);
		*width_height_param_ = uint2(w, h);

		*lighting_mask_tex_param_ = pvp.lighting_mask_tex;

		*skylight_diff_spec_mip_param_ = int3(0, 0, 0);
		*skylight_mip_bias_param_ = 0.0f;

		for (uint32_t li = 0; li < lights_.size();)
		{
			std::array<std::vector<uint32_t>, 11> available_lights;
			for (uint32_t batch = 0; (batch < light_batch_) && (li < lights_.size()); ++ li)
			{
				auto const & light = *lights_[li];
				if (light.Enabled() && pvp.light_visibles[li])
				{
					++ batch;

					LightSource::LightType type = light.Type();
					switch (type)
					{
					case LightSource::LT_Ambient:
						available_lights[0].push_back(li);
						if (light.SkylightTexY())
						{
							*skylight_y_cube_tex_param_ = light.SkylightTexY();
							*skylight_c_cube_tex_param_ = light.SkylightTexC();

							uint32_t const mip = light.SkylightTexY()->NumMipMaps();
							*skylight_diff_spec_mip_param_ = int3(mip - 1, mip - 2, 1);
							*skylight_mip_bias_param_ = mip / -2.0f;

							*inv_view_param_ = pvp.inv_view;
						}
						break;

					case LightSource::LT_Sun:
						available_lights[1].push_back(li);
						break;

					case LightSource::LT_Directional:
						available_lights[2].push_back(li);
						break;

					case LightSource::LT_Point:
						if (light.Attrib() & LightSource::LSA_NoShadow)
						{
							available_lights[3].push_back(li);
						}
						else
						{
							available_lights[4].push_back(li);
						}
						break;

					case LightSource::LT_Spot:
						if (light.Attrib() & LightSource::LSA_NoShadow)
						{
							available_lights[5].push_back(li);
						}
						else
						{
							available_lights[6].push_back(li);
						}
						break;

					case LightSource::LT_SphereArea:
						if (light.Attrib() & LightSource::LSA_NoShadow)
						{
							available_lights[7].push_back(li);
						}
						else
						{
							available_lights[8].push_back(li);
						}
						break;

					case LightSource::LT_TubeArea:
						if (light.Attrib() & LightSource::LSA_NoShadow)
						{
							available_lights[9].push_back(li);
						}
						else
						{
							available_lights[10].push_back(li);
						}
						break;

					default:
						BOOST_ASSERT(false);
						break;
					}
				}
			}

			uint8_t* lights_type = lights_type_param_->MemoryInCBuff<uint8_t>();
			*reinterpret_cast<uint32_t*>(lights_type + 0 * lights_type_param_->Stride()) = 0;
			for (size_t i = 1; i < available_lights.size() + 1; ++ i)
			{
				*reinterpret_cast<uint32_t*>(lights_type + i * lights_type_param_->Stride()) =
					*reinterpret_cast<uint32_t*>(lights_type + (i - 1) * lights_type_param_->Stride())
						+ static_cast<uint32_t>(available_lights[i - 1].size());
			}

			re.BindFrameBuffer(re.DefaultFrameBuffer());
			re.DefaultFrameBuffer()->Discard(FrameBuffer::CBM_Color | FrameBuffer::CBM_Depth
				| FrameBuffer::CBM_Stencil);

			uint8_t* lights_color = lights_color_param_->MemoryInCBuff<uint8_t>();
			uint8_t* lights_pos_es = lights_pos_es_param_->MemoryInCBuff<uint8_t>();
			uint8_t* lights_dir_es = lights_dir_es_param_->MemoryInCBuff<uint8_t>();
			uint8_t* lights_falloff_range = lights_falloff_range_param_->MemoryInCBuff<uint8_t>();
			uint8_t* lights_attrib = lights_attrib_param_->MemoryInCBuff<uint8_t>();
			uint8_t* lights_radius_extend = lights_radius_extend_param_->MemoryInCBuff<uint8_t>();
			uint8_t* lights_aabb_min = lights_aabb_min_param_->MemoryInCBuff<uint8_t>();
			uint8_t* lights_aabb_max = lights_aabb_max_param_->MemoryInCBuff<uint8_t>();
			std::array<int, 6> lights_shadowing_channel;
			lights_shadowing_channel.fill(-1);
			for (uint32_t t = 0; t < available_lights.size(); ++ t)
			{
				for (uint32_t i = 0; i < available_lights[t].size(); ++ i)
				{
					auto const & light = *lights_[available_lights[t][i]];
					LightSource::LightType type = light.Type();
					int32_t attr = light.Attrib();
					uint32_t offset = *reinterpret_cast<uint32_t*>(lights_type + t * lights_type_param_->Stride()) + i;

					*reinterpret_cast<float4*>(lights_color
						+ offset * lights_color_param_->Stride()) = light.Color();

					float3 const & p = light.Position();
					float3 loc_es = MathLib::transform_coord(p, pvp.view);
					*reinterpret_cast<float4*>(lights_pos_es
						+ offset * lights_pos_es_param_->Stride()) = float4(loc_es.x(), loc_es.y(), loc_es.z(), 1);

					float3 dir_es(0, 0, 0);
					switch (type)
					{
					case LightSource::LT_Ambient:
						dir_es = MathLib::transform_normal(float3(0, 1, 0), pvp.view);
						break;

					case LightSource::LT_Sun:
					case LightSource::LT_Directional:
						dir_es = MathLib::transform_normal(-light.Direction(), pvp.view);
						break;

					case LightSource::LT_Spot:
						dir_es = MathLib::transform_normal(light.Direction(), pvp.view);
						break;

					default:
						break;
					}
					*reinterpret_cast<float4*>(lights_dir_es
						+ offset * lights_dir_es_param_->Stride()) = float4(dir_es.x(), dir_es.y(), dir_es.z(), 0);

					if (LightSource::LT_Spot == type)
					{
						reinterpret_cast<float4*>(lights_pos_es
							+ offset * lights_pos_es_param_->Stride())->w() = light.CosOuterInner().x();
						reinterpret_cast<float4*>(lights_dir_es
							+ offset * lights_dir_es_param_->Stride())->w() = light.CosOuterInner().y();
					}

					*reinterpret_cast<float4*>(lights_attrib
						+ offset * lights_attrib_param_->Stride()) = float4(attr & LightSource::LSA_NoDiffuse ? 0.0f : 1.0f,
						attr & LightSource::LSA_NoSpecular ? 0.0f : 1.0f, 0, 0);

					float3 extend_es = MathLib::transform_normal(light.Extend(), pvp.view);
					*reinterpret_cast<float4*>(lights_radius_extend
						+ offset * lights_radius_extend_param_->Stride()) = float4(light.Radius(),
						extend_es.x(), extend_es.y(), extend_es.z());

					if (0 == (attr & LightSource::LSA_NoShadow))
					{
						int32_t channel = sm_light_indices_[available_lights[t][i]].second;
						switch (type)
						{
						case LightSource::LT_Sun:
							lights_shadowing_channel[0] = channel;
							break;

						case LightSource::LT_Point:
						case LightSource::LT_SphereArea:
						case LightSource::LT_TubeArea:
							lights_shadowing_channel[1] = channel;
							break;

						case LightSource::LT_Spot:
							lights_shadowing_channel[2 + i] = channel;
							break;

						default:
							break;
						}
					}

					float range = light.Range() * light_scale_;
					AABBox aabb(float3(0, 0, 0), float3(0, 0, 0));
					if (LightSource::LT_Spot == type)
					{
						float4x4 light_to_view = light.SMCamera(0)->InverseViewMatrix() * pvp.view;
						float const scale = light.CosOuterInner().w();
						float4x4 light_model = MathLib::scaling(range * 0.01f * float3(scale, scale, 1));
						float4x4 light_mv = light_model * light_to_view;
						aabb = MathLib::transform_aabb(cone_aabb_, light_mv);
					}
					*reinterpret_cast<float4*>(lights_falloff_range + offset * lights_falloff_range_param_->Stride())
						= float4(light.Falloff().x(), light.Falloff().y(), light.Falloff().z(), range);
					*reinterpret_cast<float4*>(lights_aabb_min + offset * lights_aabb_min_param_->Stride())
						= float4(aabb.Min().x(), aabb.Min().y(), aabb.Min().z(), 0);
					*reinterpret_cast<float4*>(lights_aabb_max + offset * lights_aabb_max_param_->Stride())
						= float4(aabb.Max().x(), aabb.Max().y(), aabb.Max().z(), 0);
				}
			}

			for (size_t i = 0; i < lights_shadowing_channel.size(); ++ i)
			{
				reinterpret_cast<float4*>(lights_attrib
					+ i * lights_attrib_param_->Stride())->z() = lights_shadowing_channel[i] + 0.5f;
			}

			lights_type_param_->CBuffer()->Dirty(true);
			lights_color_param_->CBuffer()->Dirty(true);
			lights_pos_es_param_->CBuffer()->Dirty(true);
			lights_dir_es_param_->CBuffer()->Dirty(true);
			lights_falloff_range_param_->CBuffer()->Dirty(true);
			lights_attrib_param_->CBuffer()->Dirty(true);
			lights_radius_extend_param_->CBuffer()->Dirty(true);
			lights_aabb_min_param_->CBuffer()->Dirty(true);
			lights_aabb_max_param_->CBuffer()->Dirty(true);

			if (available_lights[0].empty())
			{
				*shading_in_tex_param_ = (Opaque_GBuffer == g_buffer_index)
					? pvp.curr_merged_shading_tex : pvp.shading_tex;
				*shading_rw_tex_param_ = pvp.temp_shading_tex;
			}
			else
			{
				*shading_in_tex_param_ = TexturePtr();
				*shading_rw_tex_param_ = (Opaque_GBuffer == g_buffer_index)
					? pvp.curr_merged_shading_tex : pvp.shading_tex;
			}
			re.Dispatch(*technique_tbdr_unified_,
				(w + TILE_SIZE - 1) / TILE_SIZE, (h + TILE_SIZE - 1) / TILE_SIZE, 1);

			if (available_lights[0].empty())
			{
				copy_pp_->InputPin(0, pvp.temp_shading_tex);
				copy_pp_->OutputPin(0,
					(Opaque_GBuffer == g_buffer_index) ? pvp.curr_merged_shading_tex : pvp.shading_tex);
				copy_pp_->Apply();
			}
		}
	}

	void DeferredRenderingLayer::CreateDepthMinMaxMapCS(PerViewport const & pvp)
	{
		RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
		re.BindFrameBuffer(re.DefaultFrameBuffer());
		re.DefaultFrameBuffer()->Discard(FrameBuffer::CBM_Color | FrameBuffer::CBM_Depth | FrameBuffer::CBM_Stencil);

		TexturePtr const & in_tex = pvp.g_buffer_depth_tex;
		TexturePtr const & out_tex = pvp.g_buffer_min_max_depth_texs.back();
		*width_height_param_ = uint2(in_tex->Width(0) - 1, in_tex->Height(0) - 1);
		*depth_to_tiled_depth_in_tex_param_ = in_tex;
		*depth_to_tiled_min_max_depth_rw_tex_param_ = out_tex;

		re.Dispatch(*technique_depth_to_tiled_min_max_, out_tex->Width(0), out_tex->Height(0), 1);
	}
#endif

	void DeferredRenderingLayer::Display(DeferredRenderingLayer::DisplayType display_type)
	{
		checked_pointer_cast<DeferredRenderingDebugPostProcess>(dr_debug_pp_)->Display(display_type);
		display_type_ = display_type;
	}

	void DeferredRenderingLayer::DumpIntermediaTextures()
	{
		static int index = 0;

		std::shared_ptr<DeferredRenderingDebugPostProcess> pp = checked_pointer_cast<DeferredRenderingDebugPostProcess>(dr_debug_pp_);

		RenderFactory& rf = Context::Instance().RenderFactoryInstance();
		TexturePtr temp_tex = rf.MakeTexture2D(viewports_[0].g_buffer->Width(), viewports_[0].g_buffer->Height(),
			1, 1, EF_ABGR32F, 1, 0, EAH_GPU_Write, nullptr);
		pp->OutputPin(0, temp_tex);

		std::string index_str = boost::lexical_cast<std::string>(index);

		pp->Display(DT_Position);
		pp->Apply();
		SaveTexture(temp_tex, "DumpPosition" + index_str + ".dds");

		pp->Display(DT_Normal);
		pp->Apply();
		SaveTexture(temp_tex, "DumpNormal" + index_str  + ".dds");

		pp->Display(DT_Depth);
		pp->Apply();
		SaveTexture(temp_tex, "DumpDepth" + index_str  + ".dds");

		pp->Display(DT_Diffuse);
		pp->Apply();
		SaveTexture(temp_tex, "DumpDiffuse" + index_str + ".dds");

		pp->Display(DT_Specular);
		pp->Apply();
		SaveTexture(temp_tex, "DumpSpecular" + index_str + ".dds");

		pp->Display(DT_Shininess);
		pp->Apply();
		SaveTexture(temp_tex, "DumpShininess" + index_str + ".dds");

#if DEFAULT_DEFERRED == TRIDITIONAL_DEFERRED
		pp->Display(DT_DiffuseLighting);
		pp->Apply();
		SaveTexture(temp_tex, "DumpDiffuseLighting" + index_str + ".dds");

		pp->Display(DT_SpecularLighting);
		pp->Apply();
		SaveTexture(temp_tex, "DumpSpecularLighting" + index_str + ".dds");
#endif

		pp->Display(display_type_);
		pp->OutputPin(0, TexturePtr());

		++ index;
	}

	uint32_t DeferredRenderingLayer::NumObjectsRendered() const
	{
		return num_objects_rendered_;
	}

	uint32_t DeferredRenderingLayer::NumRenderablesRendered() const
	{
		return num_renderables_rendered_;
	}

	uint32_t DeferredRenderingLayer::NumPrimitivesRendered() const
	{
		return num_primitives_rendered_;
	}

	uint32_t DeferredRenderingLayer::NumVerticesRendered() const
	{
		return num_vertices_rendered_;
	}
}
