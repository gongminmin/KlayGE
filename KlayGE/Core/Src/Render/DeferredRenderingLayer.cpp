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
#include <KFL/ErrorHandling.hpp>
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
			: PostProcess(L"DeferredRenderingDebug", false,
				{},
				{ "g_buffer_tex", "g_buffer_1_tex", "depth_tex", "lighting_tex", "ssvo_tex" },
				{ "out_tex" },
				RenderEffectPtr(), nullptr)
		{
			auto effect = SyncLoadRenderEffect("DeferredRenderingDebug.fxml");
			this->Technique(effect, effect->TechniqueByName("ShowPosition"));
		}

		void Display(DeferredRenderingLayer::DisplayType display_type)
		{
			switch (display_type)
			{
			case DeferredRenderingLayer::DT_Final:
				break;

			case DeferredRenderingLayer::DT_Position:
				technique_ = effect_->TechniqueByName("ShowPosition");
				break;

			case DeferredRenderingLayer::DT_Normal:
				technique_ = effect_->TechniqueByName("ShowNormal");
				break;

			case DeferredRenderingLayer::DT_Depth:
				technique_ = effect_->TechniqueByName("ShowDepth");
				break;

			case DeferredRenderingLayer::DT_Diffuse:
				technique_ = effect_->TechniqueByName("ShowDiffuse");
				break;

			case DeferredRenderingLayer::DT_Specular:
				technique_ = effect_->TechniqueByName("ShowSpecular");
				break;

			case DeferredRenderingLayer::DT_Shininess:
				technique_ = effect_->TechniqueByName("ShowShininess");
				break;

			case DeferredRenderingLayer::DT_Edge:
				break;

			case DeferredRenderingLayer::DT_SSVO:
				technique_ = effect_->TechniqueByName("ShowSSVO");
				break;

#if DEFAULT_DEFERRED == TRIDITIONAL_DEFERRED
			case DeferredRenderingLayer::DT_DiffuseLighting:
				technique_ = effect_->TechniqueByName("ShowDiffuseLighting");
				break;

			case DeferredRenderingLayer::DT_SpecularLighting:
				technique_ = effect_->TechniqueByName("ShowSpecularLighting");
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
			*(effect_->ParameterByName("inv_proj")) = camera.InverseProjMatrix();
			*(effect_->ParameterByName("depth_near_far_invfar")) = float3(camera.NearPlane(), camera.FarPlane(), 1 / camera.FarPlane());
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

		tex_array_support_ = (caps.max_texture_array_length >= 4) && (caps.render_to_texture_array_support);

#if DEFAULT_DEFERRED == LIGHT_INDEXED_DEFERRED
		typed_uav_ = false;
		if ((caps.max_shader_model >= ShaderModel(5, 0)) && caps.cs_support)
		{
			static_assert(32 == TILE_SIZE, "TILE_SIZE must be 32.");

			cs_cldr_ = true;
			if (caps.uav_format_support(EF_ABGR16F) && caps.uav_format_support(EF_B10G11R11F) && caps.uav_format_support(EF_ABGR8))
			{
				typed_uav_ = true;
			}
		}
		else
		{
			cs_cldr_ = false;
		}
#endif

		for (size_t vpi = 0; vpi < viewports_.size(); ++ vpi)
		{
			PerViewport& pvp = viewports_[vpi];
			pvp.g_buffer = rf.MakeFrameBuffer();
#if DEFAULT_DEFERRED == TRIDITIONAL_DEFERRED
			pvp.lighting_fb = rf.MakeFrameBuffer();
#endif
			pvp.vdm_fb = rf.MakeFrameBuffer();
			pvp.shading_fb = rf.MakeFrameBuffer();
			pvp.shadowing_fb = rf.MakeFrameBuffer();
			pvp.projective_shadowing_fb = rf.MakeFrameBuffer();
			pvp.reflection_fb = rf.MakeFrameBuffer();
			for (size_t i = 0; i < pvp.merged_shading_fbs.size(); ++ i)
			{
				pvp.merged_shading_fbs[i] = rf.MakeFrameBuffer();
				pvp.merged_depth_fbs[i] = rf.MakeFrameBuffer();
			}
#if DEFAULT_DEFERRED == LIGHT_INDEXED_DEFERRED
			if (cs_cldr_)
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
				VertexElement(VEU_Position, 0, EF_BGR32F));

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
				VertexElement(VEU_Position, 0, EF_BGR32F));

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
				VertexElement(VEU_Position, 0, EF_BGR32F));

			rl_box_->BindIndexStream(rf.MakeIndexBuffer(BU_Static, EAH_GPU_Read | EAH_Immutable,
				static_cast<uint32_t>(index.size() * sizeof(index[0])), &index[0]), EF_R16UI);
		}
		{
			rl_quad_ = rf.MakeRenderLayout();
			rl_quad_->TopologyType(RenderLayout::TT_TriangleStrip);

			float3 const pos[] =
			{
				float3(+1, +1, 1),
				float3(-1, +1, 1),
				float3(+1, -1, 1),
				float3(-1, -1, 1)
			};

			rl_quad_->BindVertexStream(rf.MakeVertexBuffer(BU_Static, EAH_GPU_Read | EAH_Immutable,
				static_cast<uint32_t>(sizeof(pos)), pos),
				VertexElement(VEU_Position, 0, EF_BGR32F));
		}

		light_volume_rl_[LightSource::LT_Ambient] = rl_quad_;
		light_volume_rl_[LightSource::LT_Directional] = rl_quad_;
		light_volume_rl_[LightSource::LT_Point] = rl_box_;
		light_volume_rl_[LightSource::LT_Spot] = rl_cone_;
		light_volume_rl_[LightSource::LT_SphereArea] = rl_box_;
		light_volume_rl_[LightSource::LT_TubeArea] = rl_box_;

		default_ambient_light_ = MakeSharedPtr<AmbientLightSource>();
		merged_ambient_light_ = MakeSharedPtr<AmbientLightSource>();

		g_buffer_effect_ = SyncLoadRenderEffect("GBuffer.fxml");
		g_buffer_skinning_effect_ = SyncLoadRenderEffects({ "GBuffer.fxml", "GBufferSkinning.fxml" });
#if DEFAULT_DEFERRED == TRIDITIONAL_DEFERRED
		dr_effect_ = SyncLoadRenderEffect("DeferredRendering.fxml");
#elif DEFAULT_DEFERRED == LIGHT_INDEXED_DEFERRED
		if (cs_cldr_)
		{
			num_depth_slices_ = 4;
			depth_slices_.resize(num_depth_slices_ + 1);
			light_batch_ = 1024;
			dr_effect_ = SyncLoadRenderEffect("ClusteredDeferredRendering.fxml");
		}
		else
		{
			light_batch_ = 32;
			dr_effect_ = SyncLoadRenderEffect("LightIndexedDeferredRendering.fxml");
		}
#endif

		auto dr_effect = dr_effect_.get();

		technique_shadows_[LightSource::LT_Point][0] = dr_effect->TechniqueByName("DeferredShadowingPointR");
		technique_shadows_[LightSource::LT_Point][1] = dr_effect->TechniqueByName("DeferredShadowingPointG");
		technique_shadows_[LightSource::LT_Point][2] = dr_effect->TechniqueByName("DeferredShadowingPointB");
		technique_shadows_[LightSource::LT_Point][3] = dr_effect->TechniqueByName("DeferredShadowingPointA");
		technique_shadows_[LightSource::LT_Point][4] = dr_effect->TechniqueByName("DeferredShadowingPoint");
		technique_shadows_[LightSource::LT_Spot][0] = dr_effect->TechniqueByName("DeferredShadowingSpotR");
		technique_shadows_[LightSource::LT_Spot][1] = dr_effect->TechniqueByName("DeferredShadowingSpotG");
		technique_shadows_[LightSource::LT_Spot][2] = dr_effect->TechniqueByName("DeferredShadowingSpotB");
		technique_shadows_[LightSource::LT_Spot][3] = dr_effect->TechniqueByName("DeferredShadowingSpotA");
		technique_shadows_[LightSource::LT_Spot][4] = dr_effect->TechniqueByName("DeferredShadowingSpot");
		technique_shadows_[LightSource::LT_Directional][0] = dr_effect->TechniqueByName("DeferredShadowingDirectionalR");
		technique_shadows_[LightSource::LT_Directional][1] = dr_effect->TechniqueByName("DeferredShadowingDirectionalG");
		technique_shadows_[LightSource::LT_Directional][2] = dr_effect->TechniqueByName("DeferredShadowingDirectionalB");
		technique_shadows_[LightSource::LT_Directional][3] = dr_effect->TechniqueByName("DeferredShadowingDirectionalA");
		technique_shadows_[LightSource::LT_Directional][4] = dr_effect->TechniqueByName("DeferredShadowingDirectional");
		technique_shadows_[LightSource::LT_SphereArea][0] = dr_effect->TechniqueByName("DeferredShadowingPointR");
		technique_shadows_[LightSource::LT_SphereArea][1] = dr_effect->TechniqueByName("DeferredShadowingPointG");
		technique_shadows_[LightSource::LT_SphereArea][2] = dr_effect->TechniqueByName("DeferredShadowingPointB");
		technique_shadows_[LightSource::LT_SphereArea][3] = dr_effect->TechniqueByName("DeferredShadowingPointA");
		technique_shadows_[LightSource::LT_SphereArea][4] = dr_effect->TechniqueByName("DeferredShadowingPoint");
		technique_shadows_[LightSource::LT_TubeArea][0] = dr_effect->TechniqueByName("DeferredShadowingPointR");
		technique_shadows_[LightSource::LT_TubeArea][1] = dr_effect->TechniqueByName("DeferredShadowingPointG");
		technique_shadows_[LightSource::LT_TubeArea][2] = dr_effect->TechniqueByName("DeferredShadowingPointB");
		technique_shadows_[LightSource::LT_TubeArea][3] = dr_effect->TechniqueByName("DeferredShadowingPointA");
		technique_shadows_[LightSource::LT_TubeArea][4] = dr_effect->TechniqueByName("DeferredShadowingPoint");
#if DEFAULT_DEFERRED == TRIDITIONAL_DEFERRED
		technique_lights_[LightSource::LT_Ambient] = dr_effect->TechniqueByName("DeferredRenderingAmbient");
		technique_lights_[LightSource::LT_Directional] = dr_effect->TechniqueByName("DeferredRenderingDirectional");
		technique_lights_[LightSource::LT_Point] = dr_effect->TechniqueByName("DeferredRenderingPoint");
		technique_lights_[LightSource::LT_Spot] = dr_effect->TechniqueByName("DeferredRenderingSpot");
		technique_lights_[LightSource::LT_SphereArea] = dr_effect->TechniqueByName("DeferredRenderingSphereArea");
		technique_lights_[LightSource::LT_TubeArea] = dr_effect->TechniqueByName("DeferredRenderingTubeArea");
		technique_light_depth_only_ = dr_effect->TechniqueByName("DeferredRenderingLightDepthOnly");
		technique_light_stencil_ = dr_effect->TechniqueByName("DeferredRenderingLightStencil");
#endif
		technique_no_lighting_ = dr_effect->TechniqueByName("NoLightingTech");
		technique_shading_ = dr_effect->TechniqueByName("ShadingTech");
		technique_merge_shadings_[0] = dr_effect->TechniqueByName("MergeShadingTech");
		technique_merge_shadings_[1] = dr_effect->TechniqueByName("MergeShadingAlphaBlendTech");
		technique_merge_depths_[0] = dr_effect->TechniqueByName("MergeDepthTech");
		technique_merge_depths_[1] = dr_effect->TechniqueByName("MergeDepthAlphaBlendTech");
		technique_copy_shading_depth_ = dr_effect->TechniqueByName("CopyShadingDepthTech");
		technique_copy_depth_ = dr_effect->TechniqueByName("CopyDepthTech");
#if DEFAULT_DEFERRED == LIGHT_INDEXED_DEFERRED
		if (cs_cldr_)
		{
			technique_cldr_shadowing_unified_ = dr_effect->TechniqueByName("ClusteredDRShadowingUnified");
			technique_cldr_light_intersection_unified_ = dr_effect->TechniqueByName("ClusteredDRLightIntersection");
			technique_cldr_unified_ = dr_effect->TechniqueByName(typed_uav_ ? "ClusteredDRUnified" : "ClusteredDRUnifiedNoTypedUAV");
		}
		else
		{
			technique_draw_light_index_point_ = dr_effect->TechniqueByName("DrawLightIndexPoint");
			technique_draw_light_index_spot_ = dr_effect->TechniqueByName("DrawLightIndexSpot");
			technique_lidr_ambient_ = dr_effect->TechniqueByName("LIDRAmbient");
			technique_lidr_directional_shadow_ = dr_effect->TechniqueByName("LIDRDirectionalShadow");
			technique_lidr_directional_no_shadow_ = dr_effect->TechniqueByName("LIDRDirectionalNoShadow");
			technique_lidr_point_shadow_ = dr_effect->TechniqueByName("LIDRPointShadow");
			technique_lidr_point_no_shadow_ = dr_effect->TechniqueByName("LIDRPointNoShadow");
			technique_lidr_spot_shadow_ = dr_effect->TechniqueByName("LIDRSpotShadow");
			technique_lidr_spot_no_shadow_ = dr_effect->TechniqueByName("LIDRSpotNoShadow");
			technique_lidr_sphere_area_shadow_ = dr_effect->TechniqueByName("LIDRSphereAreaShadow");
			technique_lidr_sphere_area_no_shadow_ = dr_effect->TechniqueByName("LIDRSphereAreaNoShadow");
			technique_lidr_tube_area_shadow_ = dr_effect->TechniqueByName("LIDRTubeAreaShadow");
			technique_lidr_tube_area_no_shadow_ = dr_effect->TechniqueByName("LIDRTubeAreaNoShadow");
		}
#endif

		sm_fb_ = rf.MakeFrameBuffer();
		ElementFormat fmt;
		if (caps.rendertarget_format_support(EF_R32F, 1, 0))
		{
			fmt = EF_R32F;
		}
		else
		{
			BOOST_ASSERT(caps.rendertarget_format_support(EF_R16F, 1, 0));
			fmt = EF_R16F;
		}
		sm_tex_ = rf.MakeTexture2D(SM_SIZE, SM_SIZE, 1, 1, fmt, 1, 0, EAH_GPU_Read | EAH_GPU_Write);
		sm_fb_->Attach(FrameBuffer::ATT_Color0, rf.Make2DRenderView(*sm_tex_, 0, 1, 0));
		sm_depth_tex_ = rf.MakeTexture2D(SM_SIZE, SM_SIZE, 1, 1, EF_D24S8, 1, 0, EAH_GPU_Read | EAH_GPU_Write);
		RenderViewPtr sm_depth_view = rf.Make2DDepthStencilRenderView(*sm_depth_tex_, 0, 1, 0);
		sm_fb_->Attach(FrameBuffer::ATT_DepthStencil, sm_depth_view);

		csm_fb_ = rf.MakeFrameBuffer();
		csm_tex_ = rf.MakeTexture2D(SM_SIZE * 2, SM_SIZE * 2, 1, 1, fmt, 1, 0, EAH_GPU_Read | EAH_GPU_Write);
		csm_fb_->Attach(FrameBuffer::ATT_Color0, rf.Make2DRenderView(*csm_tex_, 0, 1, 0));
		csm_fb_->Attach(FrameBuffer::ATT_DepthStencil, rf.Make2DDepthStencilRenderView(SM_SIZE * 2, SM_SIZE * 2, EF_D24S8, 1, 0));

		for (auto& tex : unfiltered_sm_2d_texs_)
		{
			tex = rf.MakeTexture2D(SM_SIZE, SM_SIZE, 1, 1, sm_tex_->Format(), 1, 0, EAH_GPU_Read | EAH_GPU_Write);
		}
		if (tex_array_support_)
		{
			filtered_sm_2d_texs_[0] = rf.MakeTexture2D(SM_SIZE, SM_SIZE, 1, static_cast<uint32_t>(filtered_sm_2d_texs_.size()),
				sm_tex_->Format(), 1, 0, EAH_GPU_Read | EAH_GPU_Write);
		}
		else
		{
			for (auto& tex : filtered_sm_2d_texs_)
			{
				tex = rf.MakeTexture2D(SM_SIZE, SM_SIZE, 1, 1, sm_tex_->Format(), 1, 0, EAH_GPU_Read | EAH_GPU_Write);
			}
		}
		for (auto& tex : filtered_sm_cube_texs_)
		{
			tex = rf.MakeTextureCube(SM_SIZE, 1, 1, sm_tex_->Format(), 1, 0, EAH_GPU_Read | EAH_GPU_Write);
		}

		ssvo_pp_ = MakeSharedPtr<SSVOPostProcess>();
		auto effect_x = SyncLoadRenderEffect("SSVO.fxml");
		auto effect_y = effect_x->Clone();
		ssvo_blur_pp_ = MakeSharedPtr<BlurPostProcess<SeparableBilateralFilterPostProcess>>(8, 1.0f,
			effect_x, effect_x->TechniqueByName("SSVOBlurX"),
			effect_y, effect_y->TechniqueByName("SSVOBlurY"));
		ssr_pp_ = MakeSharedPtr<SSRPostProcess>();
		taa_pp_ = SyncLoadPostProcess("TAA.ppml", "taa");

		sss_blur_pp_ = MakeSharedPtr<SSSBlurPP>();
		sss_blur_pp_->SetParam(0, 1.0f);
		sss_blur_pp_->SetParam(1, 1.0f);

		translucency_pp_ = SyncLoadPostProcess("Translucency.ppml", "Translucency");
		translucency_pp_->SetParam(6, 100.0f);

		vdm_composition_pp_ = SyncLoadPostProcess("VarianceDepthMap.ppml", "VDMComposition");
		copy_to_depth_pp_ = SyncLoadPostProcess("Depth.ppml", "CopyToDepth");
		copy_to_depth_pp_->SetParam(0, 1);

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
		rsm_texs_[0] = rf.MakeTexture2D(SM_SIZE, SM_SIZE, MAX_RSM_MIPMAP_LEVELS, 1, fmt8, 1, 0,
			EAH_GPU_Read | EAH_GPU_Write | EAH_Generate_Mips);
		rsm_texs_[1] = rf.MakeTexture2D(SM_SIZE, SM_SIZE, MAX_RSM_MIPMAP_LEVELS, 1, fmt8, 1, 0,
			EAH_GPU_Read | EAH_GPU_Write | EAH_Generate_Mips);
		rsm_fb_->Attach(FrameBuffer::ATT_Color0, rf.Make2DRenderView(*rsm_texs_[0], 0, 1, 0)); // normal (light space)
		rsm_fb_->Attach(FrameBuffer::ATT_Color1, rf.Make2DRenderView(*rsm_texs_[1], 0, 1, 0)); // albedo
		rsm_fb_->Attach(FrameBuffer::ATT_DepthStencil, sm_depth_view);
			
		copy_to_light_buffer_pp_ = SyncLoadPostProcess("Copy2LightBuffer.ppml", "CopyToLightBuffer");
		copy_to_light_buffer_i_pp_ = SyncLoadPostProcess("Copy2LightBuffer.ppml", "CopyToLightBufferI");

		sm_filter_pp_ = MakeSharedPtr<LogGaussianBlurPostProcess>(4, true);
		sm_filter_pp_->InputPin(0, sm_tex_);
		csm_filter_pp_ = MakeSharedPtr<LogGaussianBlurPostProcess>(4, true);
		csm_filter_pp_->InputPin(0, csm_tex_);
		depth_to_esm_pp_ = SyncLoadPostProcess("Depth.ppml", "DepthToESM");
		depth_to_esm_pp_->InputPin(0, sm_depth_tex_);
		depth_to_esm_pp_->OutputPin(0, sm_tex_);
		depth_to_linear_pp_ = SyncLoadPostProcess("Depth.ppml", "DepthToLinear");
		depth_mipmap_pp_ = SyncLoadPostProcess("Depth.ppml", "DepthMipmapBilinear");

		g_buffer_tex_param_ = dr_effect->ParameterByName("g_buffer_tex");
		g_buffer_1_tex_param_ = dr_effect->ParameterByName("g_buffer_1_tex");
		depth_tex_param_ = dr_effect->ParameterByName("depth_tex");
#if DEFAULT_DEFERRED == TRIDITIONAL_DEFERRED
		lighting_tex_param_ = dr_effect->ParameterByName("lighting_tex");
#endif
		shading_tex_param_ = dr_effect->ParameterByName("shading_tex");
		light_attrib_param_ = dr_effect->ParameterByName("light_attrib");
		light_radius_extend_param_ = dr_effect->ParameterByName("light_radius_extend");
		light_color_param_ = dr_effect->ParameterByName("light_color");
		light_falloff_range_param_ = dr_effect->ParameterByName("light_falloff_range");
		light_view_proj_param_ = dr_effect->ParameterByName("light_view_proj");
		light_volume_mv_param_ = dr_effect->ParameterByName("light_volume_mv");
		light_volume_mvp_param_ = dr_effect->ParameterByName("light_volume_mvp");
		view_to_light_model_param_ = dr_effect->ParameterByName("view_to_light_model");
		light_pos_es_param_ = dr_effect->ParameterByName("light_pos_es");
		light_dir_es_param_ = dr_effect->ParameterByName("light_dir_es");
		projective_map_2d_tex_param_ = dr_effect->ParameterByName("projective_map_2d_tex");
		projective_map_cube_tex_param_ = dr_effect->ParameterByName("projective_map_cube_tex");
		filtered_sm_2d_tex_param_ = dr_effect->ParameterByName("filtered_sm_2d_tex");
		filtered_sm_2d_tex_array_param_ = dr_effect->ParameterByName("filtered_sm_2d_tex_array");
		filtered_sm_2d_light_index_param_ = dr_effect->ParameterByName("filtered_sm_2d_light_index");
		filtered_sm_cube_tex_param_ = dr_effect->ParameterByName("filtered_sm_cube_tex");
		inv_width_height_param_ = dr_effect->ParameterByName("inv_width_height");
		shadowing_tex_param_ = dr_effect->ParameterByName("shadowing_tex");
		projective_shadowing_tex_param_ = dr_effect->ParameterByName("projective_shadowing_tex");
		shadowing_channel_param_ = dr_effect->ParameterByName("shadowing_channel");
		esm_scale_factor_param_ = dr_effect->ParameterByName("esm_scale_factor");
		near_q_param_ = dr_effect->ParameterByName("near_q");
		cascade_intervals_param_ = dr_effect->ParameterByName("cascade_intervals");
		cascade_scale_bias_param_ = dr_effect->ParameterByName("cascade_scale_bias");
		num_cascades_param_ = dr_effect->ParameterByName("num_cascades");
		view_z_to_light_view_param_ = dr_effect->ParameterByName("view_z_to_light_view");
		if (tex_array_support_)
		{
			filtered_csm_texs_param_[0] = dr_effect->ParameterByName("filtered_csm_tex_array");
		}
		else
		{
			filtered_csm_texs_param_[0] = dr_effect->ParameterByName("filtered_csm_0_tex");
			filtered_csm_texs_param_[1] = dr_effect->ParameterByName("filtered_csm_1_tex");
			filtered_csm_texs_param_[2] = dr_effect->ParameterByName("filtered_csm_2_tex");
			filtered_csm_texs_param_[3] = dr_effect->ParameterByName("filtered_csm_3_tex");
		}
		skylight_diff_spec_mip_param_ = dr_effect->ParameterByName("skylight_diff_spec_mip");
		inv_view_param_ = dr_effect->ParameterByName("inv_view");
		skylight_y_cube_tex_param_ = dr_effect->ParameterByName("skylight_y_cube_tex");
		skylight_c_cube_tex_param_ = dr_effect->ParameterByName("skylight_c_cube_tex");
#if DEFAULT_DEFERRED == LIGHT_INDEXED_DEFERRED
		min_max_depth_tex_param_ = dr_effect->ParameterByName("min_max_depth_tex");
		lights_color_param_ = dr_effect->ParameterByName("lights_color");
		lights_pos_es_param_ = dr_effect->ParameterByName("lights_pos_es");
		lights_dir_es_param_ = dr_effect->ParameterByName("lights_dir_es");
		lights_falloff_range_param_ = dr_effect->ParameterByName("lights_falloff_range");
		lights_attrib_param_ = dr_effect->ParameterByName("lights_attrib");
		lights_radius_extend_param_ = dr_effect->ParameterByName("lights_radius_extend");
		lights_aabb_min_param_ = dr_effect->ParameterByName("lights_aabb_min");
		lights_aabb_max_param_ = dr_effect->ParameterByName("lights_aabb_max");
		tile_scale_param_ = dr_effect->ParameterByName("tile_scale");
		camera_proj_01_param_ = dr_effect->ParameterByName("camera_proj_01");

		if (cs_cldr_)
		{
			technique_depth_to_tiled_min_max_ = dr_effect->TechniqueByName("DepthToTiledMinMax");
			technique_cldr_lighting_mask_ = dr_effect->TechniqueByName("ClusteredDRLightingMask");

			near_q_far_param_ = dr_effect->ParameterByName("near_q_far");
			width_height_param_ = dr_effect->ParameterByName("width_height");
			depth_to_tiled_depth_in_tex_param_ = dr_effect->ParameterByName("depth_in_tex");
			depth_to_tiled_min_max_depth_rw_tex_param_ = dr_effect->ParameterByName("min_max_depth_rw_tex");
			linear_depth_rw_tex_param_ = dr_effect->ParameterByName("linear_depth_rw_tex");
			upper_left_param_ = dr_effect->ParameterByName("upper_left");
			x_dir_param_ = dr_effect->ParameterByName("x_dir");
			y_dir_param_ = dr_effect->ParameterByName("y_dir");
			lighting_mask_tex_param_ = dr_effect->ParameterByName("lighting_mask_tex");
			shading_in_tex_param_ = dr_effect->ParameterByName("shading_in_tex");
			shading_rw_tex_param_ = dr_effect->ParameterByName("shading_rw_tex");
			lights_type_param_ = dr_effect->ParameterByName("lights_type");
			lights_start_in_tex_param_ = dr_effect->ParameterByName("lights_start_in_tex");
			lights_start_rw_tex_param_ = dr_effect->ParameterByName("lights_start_rw_tex");
			intersected_light_indices_in_tex_param_ = dr_effect->ParameterByName("intersected_light_indices_in_tex");
			intersected_light_indices_rw_tex_param_ = dr_effect->ParameterByName("intersected_light_indices_rw_tex");
			depth_slices_param_ = dr_effect->ParameterByName("depth_slices");
			depth_slices_shading_param_ = dr_effect->ParameterByName("depth_slices_shading");

			projective_shadowing_rw_tex_param_ = dr_effect->ParameterByName("projective_shadowing_rw_tex");
			shadowing_rw_tex_param_ = dr_effect->ParameterByName("shadowing_rw_tex");
			lights_view_proj_param_ = dr_effect->ParameterByName("lights_view_proj");
			filtered_sms_2d_light_index_param_ = dr_effect->ParameterByName("filtered_sms_2d_light_index");
			esms_scale_factor_param_ = dr_effect->ParameterByName("esms_scale_factor");

			copy_pp_ = SyncLoadPostProcess("Copy.ppml", "copy");
		}
		else
		{
			light_index_tex_param_ = dr_effect->ParameterByName("light_index_tex");
		}

		depth_to_min_max_pp_ = SyncLoadPostProcess("Depth.ppml", "DepthToMinMax");
		reduce_min_max_pp_ = SyncLoadPostProcess("Depth.ppml", "ReduceMinMax");
#endif
		depth_to_max_pp_ = SyncLoadPostProcess("Depth.ppml", "DepthToMax");

		this->SetCascadedShadowType(CSLT_Auto);

#ifndef KLAYGE_SHIP
		PerfProfiler& profiler = PerfProfiler::Instance();
		shadow_map_perf_ = profiler.CreatePerfRange(0, "Gen shadow map");
		std::string buffer_name[] = { "Opaque", "Transparency back", "TransparencyFront" };
		for (uint32_t i = PTB_Opaque; i < PTB_None; ++ i)
		{
			gbuffer_perfs_[i] = profiler.CreatePerfRange(0, "GBuffer (" + buffer_name[i] + ")");
			shadowing_perfs_[i] = profiler.CreatePerfRange(0, "Shadowing (" + buffer_name[i] + ")");
			indirect_lighting_perfs_[i] = profiler.CreatePerfRange(0, "Indirect lighting (" + buffer_name[i] + ")");
#if DEFAULT_DEFERRED == LIGHT_INDEXED_DEFERRED
			clustering_perfs_[i] = profiler.CreatePerfRange(0, "Clustering (" + buffer_name[i] + ")");
#endif
			shading_perfs_[i] = profiler.CreatePerfRange(0, "Shading (" + buffer_name[i] + ")");
			reflection_perfs_[i] = profiler.CreatePerfRange(0, "Reflection (" + buffer_name[i] + ")");
			special_shading_perfs_[i] = profiler.CreatePerfRange(0, "Special shading (" + buffer_name[i] + ")");
		}
		sss_blur_pp_perf_ = profiler.CreatePerfRange(0, "SSS Blur PP");
		ssr_pp_perf_ = profiler.CreatePerfRange(0, "SSR PP");
		atmospheric_pp_perf_ = profiler.CreatePerfRange(0, "Atmospheric PP");
		taa_pp_perf_ = profiler.CreatePerfRange(0, "TAA PP");
		vdm_perf_ = profiler.CreatePerfRange(0, "VDM");
		vdm_composition_pp_perf_ = profiler.CreatePerfRange(0, "VDM composition PP");
#endif
	}

	bool DeferredRenderingLayer::ConfirmDevice()
	{
		RenderDeviceCaps const & caps = Context::Instance().RenderFactoryInstance().RenderEngineInstance().DeviceCaps();
		if ((caps.max_simultaneous_rts < 2)
			|| !caps.hw_instancing_support || !caps.instance_id_support
			|| !caps.depth_texture_support
			|| !caps.fp_color_support
			|| caps.pack_to_rgba_required
			|| !caps.texture_format_support(EF_D24S8) || !caps.rendertarget_format_support(EF_D24S8, 1, 0))
		{
			return false;
		}

		return true;
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
		if (caps.rendertarget_format_support(EF_R16F, 1, 0))
		{
			depth_fmt = EF_R16F;
		}
		else
		{
			BOOST_ASSERT(caps.rendertarget_format_support(EF_R32F, 1, 0));
			depth_fmt = EF_R32F;
		}

		pvp.g_buffer_ds_tex = rf.MakeTexture2D(width, height, 1, 1, EF_D24S8, 1, 0, EAH_GPU_Read | EAH_GPU_Write);
		RenderViewPtr ds_view = rf.Make2DDepthStencilRenderView(*pvp.g_buffer_ds_tex, 0, 1, 0);

		pvp.g_buffer_rt0_tex = rf.MakeTexture2D(width, height, MAX_IL_MIPMAP_LEVELS + 1, 1, fmt8, 1, 0,
			EAH_GPU_Read | EAH_GPU_Write | EAH_Generate_Mips);
		pvp.g_buffer_rt1_tex = rf.MakeTexture2D(width, height, 1, 1, fmt8, 1, 0,
			EAH_GPU_Read | EAH_GPU_Write);
		uint32_t hint = EAH_GPU_Read | EAH_GPU_Write | EAH_Generate_Mips;
#if DEFAULT_DEFERRED == LIGHT_INDEXED_DEFERRED
		if (cs_cldr_)
		{
			hint |= EAH_GPU_Unordered;
		}
#endif
		pvp.g_buffer_depth_tex = rf.MakeTexture2D(width, height, MAX_IL_MIPMAP_LEVELS + 1, 1, depth_fmt, 1, 0, hint);
		pvp.g_buffer_rt0_backup_tex = rf.MakeTexture2D(width, height, 1, 1, fmt8, 1, 0, EAH_GPU_Read);
#if DEFAULT_DEFERRED == LIGHT_INDEXED_DEFERRED
		{
			ElementFormat min_max_depth_fmt;
			if (EF_R16F == depth_fmt)
			{
				min_max_depth_fmt = EF_GR16F;
			}
			else
			{
				BOOST_ASSERT(EF_R32F == depth_fmt);
				min_max_depth_fmt = EF_GR32F;
			}

			pvp.g_buffer_min_max_depth_texs.clear();
			if (cs_cldr_)
			{
				uint32_t w = std::max(1U, (width + TILE_SIZE - 1) / TILE_SIZE);
				uint32_t h = std::max(1U, (height + TILE_SIZE - 1) / TILE_SIZE);
				pvp.g_buffer_min_max_depth_texs.push_back(rf.MakeTexture2D(w, h, 1, 1, min_max_depth_fmt, 1, 0,
					EAH_GPU_Read | EAH_GPU_Unordered));
			}
			else
			{
				uint32_t w = std::max(1U, (width + 1) / 2);
				uint32_t h = std::max(1U, (height + 1) / 2);
				for (uint32_t ts = TILE_SIZE; ts > 1; ts /= 2)
				{
					pvp.g_buffer_min_max_depth_texs.push_back(rf.MakeTexture2D(w, h, 1, 1, min_max_depth_fmt, 1, 0,
						EAH_GPU_Read | EAH_GPU_Write));
					w = std::max(1U, (w + 1) / 2);
					h = std::max(1U, (h + 1) / 2);
				}
			}
		}
#endif
		pvp.g_buffer_vdm_max_ds_texs.clear();
		{
			uint32_t w = std::max(1U, width / 2);
			uint32_t h = std::max(1U, height / 2);
			for (uint32_t i = 0; i < 2; ++ i)
			{
				TexturePtr tex = rf.MakeTexture2D(w, h, 1, 1, EF_D24S8, 1, 0, EAH_GPU_Read | EAH_GPU_Write);
				pvp.g_buffer_vdm_max_ds_texs.push_back(tex);
				pvp.g_buffer_vdm_max_ds_views.push_back(rf.Make2DDepthStencilRenderView(*tex, 0, 1, 0));
				w = std::max(1U, w / 2);
				h = std::max(1U, h / 2);
			}
		}

		RenderViewPtr g_buffer_rt0_view = rf.Make2DRenderView(*pvp.g_buffer_rt0_tex, 0, 1, 0);
		RenderViewPtr g_buffer_rt1_view = rf.Make2DRenderView(*pvp.g_buffer_rt1_tex, 0, 1, 0);

		pvp.g_buffer->Attach(FrameBuffer::ATT_Color0, g_buffer_rt0_view);
		pvp.g_buffer->Attach(FrameBuffer::ATT_Color1, g_buffer_rt1_view);
		pvp.g_buffer->Attach(FrameBuffer::ATT_DepthStencil, ds_view);

		this->SetupViewportGI(index, false);

		ElementFormat fmt;
		if (caps.rendertarget_format_support(EF_R32F, 1, 0))
		{
			fmt = EF_R32F;
		}
		else
		{
			BOOST_ASSERT(caps.rendertarget_format_support(EF_R16F, 1, 0));
			fmt = EF_R16F;
		}
		if (tex_array_support_)
		{
			pvp.filtered_csm_texs[0] = rf.MakeTexture2D(SM_SIZE * 2, SM_SIZE * 2, 3,
				CascadedShadowLayer::MAX_NUM_CASCADES, fmt, 1, 0, EAH_GPU_Read | EAH_GPU_Write | EAH_Generate_Mips);
		}
		else
		{
			for (size_t i = 0; i < pvp.filtered_csm_texs.size(); ++ i)
			{
				pvp.filtered_csm_texs[i] = rf.MakeTexture2D(SM_SIZE * 2, SM_SIZE * 2, 3, 1, fmt, 1, 0,
					EAH_GPU_Read | EAH_GPU_Write | EAH_Generate_Mips);
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
		hint = EAH_GPU_Read | EAH_GPU_Write;
#if DEFAULT_DEFERRED == LIGHT_INDEXED_DEFERRED
		if (cs_cldr_)
		{
			hint |= EAH_GPU_Unordered;
		}
#endif
		pvp.shadowing_tex = rf.MakeTexture2D(width / 2, height / 2, 1, 1, fmt, 1, 0, hint);
		pvp.shadowing_fb->Attach(FrameBuffer::ATT_Color0, rf.Make2DRenderView(*pvp.shadowing_tex, 0, 1, 0));

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

			ElementFormat fmt_srgb = MakeSRGB(fmt);
			if (caps.texture_format_support(fmt_srgb) && caps.rendertarget_format_support(fmt_srgb, 1, 0))
			{
				fmt = fmt_srgb;
			}
		}
		pvp.projective_shadowing_tex = rf.MakeTexture2D(width / 2, height / 2, 1, 1, fmt, 1, 0, hint);
		pvp.projective_shadowing_fb->Attach(FrameBuffer::ATT_Color0, rf.Make2DRenderView(*pvp.projective_shadowing_tex, 0, 1, 0));

		pvp.reflection_tex = rf.MakeTexture2D(width / 2, height / 2, 1, 1, fmt, 1, 0, EAH_GPU_Read | EAH_GPU_Write);
		pvp.reflection_fb->Attach(FrameBuffer::ATT_Color0, rf.Make2DRenderView(*pvp.reflection_tex, 0, 1, 0));
		pvp.reflection_fb->Attach(FrameBuffer::ATT_DepthStencil, rf.Make2DDepthStencilRenderView(
			pvp.reflection_tex->Width(0), pvp.reflection_tex->Height(0), ds_view->Format(), 1, 0));

		BOOST_ASSERT(caps.rendertarget_format_support(EF_ABGR16F, 1, 0));
		ElementFormat shading_fmt = EF_ABGR16F;

#if DEFAULT_DEFERRED == TRIDITIONAL_DEFERRED
		pvp.lighting_tex = rf.MakeTexture2D(width, height, 1, 1, shading_fmt, 1, 0, EAH_GPU_Read | EAH_GPU_Write);
		pvp.lighting_fb->Attach(FrameBuffer::ATT_Color0, rf.Make2DRenderView(*pvp.lighting_tex, 0, 1, 0));
		pvp.lighting_fb->Attach(FrameBuffer::ATT_DepthStencil, ds_view);
#endif

		uint32_t const vdm_width = std::max(1U, width / 4);
		uint32_t const vdm_height = std::max(1U, height / 4);
		pvp.vdm_color_tex = rf.MakeTexture2D(vdm_width, vdm_height, 1, 1, shading_fmt, 1, 0, EAH_GPU_Read | EAH_GPU_Write);
		pvp.vdm_transition_tex = rf.MakeTexture2D(vdm_width, vdm_height, 1, 1, EF_GR16F, 1, 0, EAH_GPU_Read | EAH_GPU_Write);
		pvp.vdm_count_tex = rf.MakeTexture2D(vdm_width, vdm_height, 1, 1, EF_GR16F, 1, 0, EAH_GPU_Read | EAH_GPU_Write);
		pvp.vdm_fb->Attach(FrameBuffer::ATT_Color0, rf.Make2DRenderView(*pvp.vdm_color_tex, 0, 1, 0));
		pvp.vdm_fb->Attach(FrameBuffer::ATT_Color1, rf.Make2DRenderView(*pvp.vdm_transition_tex, 0, 1, 0));
		pvp.vdm_fb->Attach(FrameBuffer::ATT_Color2, rf.Make2DRenderView(*pvp.vdm_count_tex, 0, 1, 0));
		pvp.vdm_fb->Attach(FrameBuffer::ATT_DepthStencil, pvp.g_buffer_vdm_max_ds_views[1]);

		if (caps.rendertarget_format_support(EF_B10G11R11F, 1, 0))
		{
			fmt = EF_B10G11R11F;
		}
		else
		{
			BOOST_ASSERT(caps.rendertarget_format_support(EF_ABGR16F, 1, 0));
			fmt = EF_ABGR16F;
		}
		hint = EAH_GPU_Read | EAH_GPU_Write;
#if DEFAULT_DEFERRED == LIGHT_INDEXED_DEFERRED
		if (cs_cldr_)
		{
			hint |= EAH_GPU_Unordered;
		}
#endif
		pvp.shading_tex = rf.MakeTexture2D(width, height, 1, 1, shading_fmt, 1, 0, hint);
		for (size_t i = 0; i < pvp.merged_shading_texs.size(); ++ i)
		{
			pvp.merged_shading_texs[i] = rf.MakeTexture2D(width, height, 1, 1, fmt, 1, 0, hint);
			pvp.merged_depth_texs[i] = rf.MakeTexture2D(width, height, 1, 1, depth_fmt, 1, 0, EAH_GPU_Read | EAH_GPU_Write);
		}
#if DEFAULT_DEFERRED == LIGHT_INDEXED_DEFERRED
		if (cs_cldr_)
		{
			if (!caps.uav_format_support(shading_fmt))
			{
				pvp.temp_shading_tex = rf.MakeTexture2D(width, height, 1, 1, shading_fmt, 1, 0, EAH_GPU_Read | EAH_GPU_Unordered);
			}

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
			pvp.lighting_mask_tex = rf.MakeTexture2D(width, height, 1, 1, lighting_mask_fmt, 1, 0, EAH_GPU_Read | EAH_GPU_Write);
			pvp.lighting_mask_fb->Attach(FrameBuffer::ATT_Color0, rf.Make2DRenderView(*pvp.lighting_mask_tex, 0, 1, 0));
			pvp.lighting_mask_fb->Attach(FrameBuffer::ATT_DepthStencil, ds_view);

			ElementFormat light_indices_fmt;
			if (caps.uav_format_support(EF_R16UI))
			{
				light_indices_fmt = EF_R16UI;
			}
			else
			{
				BOOST_ASSERT(caps.uav_format_support(EF_R32UI));

				light_indices_fmt = EF_R32UI;
			}
			pvp.lights_start_tex = rf.MakeTexture2D((width + (TILE_SIZE - 1)) / TILE_SIZE * 8,
				(height + (TILE_SIZE - 1)) / TILE_SIZE, 1, num_depth_slices_, light_indices_fmt, 1, 0,
				EAH_GPU_Read | EAH_GPU_Unordered);
			pvp.intersected_light_indices_tex = rf.MakeTexture2D((width + (TILE_SIZE - 1)) / TILE_SIZE * 32,
				(height + (TILE_SIZE - 1)) / TILE_SIZE * 32, 1, num_depth_slices_, light_indices_fmt, 1, 0,
				EAH_GPU_Read | EAH_GPU_Unordered);
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
			pvp.light_index_tex = rf.MakeTexture2D((width + (TILE_SIZE - 1)) / TILE_SIZE,
				(height + (TILE_SIZE - 1)) / TILE_SIZE, 1, 1, fmt, 1, 0, EAH_GPU_Read | EAH_GPU_Write);
			pvp.light_index_fb->Attach(FrameBuffer::ATT_Color0, rf.Make2DRenderView(*pvp.light_index_tex, 0, 1, 0));
		}
#endif

		pvp.shading_fb->Attach(FrameBuffer::ATT_Color0, rf.Make2DRenderView(*pvp.shading_tex, 0, 1, 0));
		pvp.shading_fb->Attach(FrameBuffer::ATT_DepthStencil, ds_view);

		for (size_t i = 0; i < pvp.merged_shading_texs.size(); ++ i)
		{
			pvp.merged_shading_fbs[i]->Attach(FrameBuffer::ATT_Color0, rf.Make2DRenderView(*pvp.merged_shading_texs[i], 0, 1, 0));
			pvp.merged_shading_fbs[i]->Attach(FrameBuffer::ATT_DepthStencil, ds_view);
			pvp.merged_depth_fbs[i]->Attach(FrameBuffer::ATT_Color0, rf.Make2DRenderView(*pvp.merged_depth_texs[i], 0, 1, 0));
			pvp.merged_depth_fbs[i]->Attach(FrameBuffer::ATT_DepthStencil, ds_view);
		}

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
		pvp.small_ssvo_tex = rf.MakeTexture2D(width / 2, height / 2, 1, 1, fmt, 1, 0, EAH_GPU_Read | EAH_GPU_Write);

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

			curr_job_iter_ = jobs_.begin();
		}

		scene_mgr.SmallObjectThreshold(0.0f);

		uint32_t urv = 0;
		while (urv == 0)
		{
			BOOST_ASSERT(curr_job_iter_ != jobs_.end());

			urv = (*curr_job_iter_)->Run();
			++ curr_job_iter_;
		}

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
				merged_ambient_light_->SkylightTex(light->SkylightTexY(), light->SkylightTexC());
				lights_.push_back(merged_ambient_light_.get());
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
						case LightSource::LT_Directional:
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
		has_vdm_objs_ = false;
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
				if (so->VDM())
				{
					has_vdm_objs_ = true;
				}
			}
		}
	}

	void DeferredRenderingLayer::BuildPassScanList(bool has_opaque_objs, bool has_transparency_back_objs, bool has_transparency_front_objs)
	{
		jobs_.clear();

#ifndef KLAYGE_SHIP
		jobs_.push_back(MakeSharedPtr<DeferredRenderingJob>(std::bind(&DeferredRenderingLayer::BeginPerfProfileDRJob,
			this, std::ref(*shadow_map_perf_))));
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
		jobs_.push_back(MakeSharedPtr<DeferredRenderingJob>(std::bind(&DeferredRenderingLayer::EndPerfProfileDRJob,
			this, std::ref(*shadow_map_perf_))));
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

				jobs_.push_back(MakeSharedPtr<DeferredRenderingJob>(std::bind(&DeferredRenderingLayer::SwitchViewportDRJob,
					this, vpi)));

				pvp.g_buffer_enables[PTB_Opaque] = (pvp.attrib & VPAM_NoOpaque) ? false : has_opaque_objs;
				pvp.g_buffer_enables[PTB_TransparencyBack] = (pvp.attrib & VPAM_NoTransparencyBack) ? false : has_transparency_back_objs;
				pvp.g_buffer_enables[PTB_TransparencyFront]
					= (pvp.attrib & VPAM_NoTransparencyFront) ? false : has_transparency_front_objs;

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

				for (uint32_t i = PTB_Opaque; i < PTB_None; ++ i)
				{
					PassTargetBuffer const pass_tb = static_cast<PassTargetBuffer>(i);

					if (pvp.g_buffer_enables[i])
					{
						this->AppendGBufferPassScanCode(vpi, pass_tb);
					}
					if (PTB_Opaque == i)
					{
						if (cascaded_shadow_index_ >= 0)
						{
							this->AppendCascadedShadowPassScanCode(vpi, cascaded_shadow_index_);
						}
					}

					if (pvp.g_buffer_enables[i])
					{
						jobs_.push_back(MakeSharedPtr<DeferredRenderingJob>(std::bind(&DeferredRenderingLayer::ShadowingDRJob,
							this, std::cref(pvp), pass_tb)));
						if (!(pvp.attrib & VPAM_NoGI))
						{
#ifndef KLAYGE_SHIP
							jobs_.push_back(MakeSharedPtr<DeferredRenderingJob>(std::bind(&DeferredRenderingLayer::BeginPerfProfileDRJob,
								this, std::ref(*indirect_lighting_perfs_[pass_tb]))));
#endif
							for (uint32_t li = 0; li < lights_.size(); ++ li)
							{
								auto const & light = *lights_[li];
								if (light.Enabled())
								{
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
							jobs_.push_back(MakeSharedPtr<DeferredRenderingJob>(std::bind(&DeferredRenderingLayer::EndPerfProfileDRJob,
								this, std::ref(*indirect_lighting_perfs_[pass_tb]))));
#endif
						}

						this->AppendShadingPassScanCode(vpi, pass_tb);
					}
				}

				jobs_.push_back(MakeSharedPtr<DeferredRenderingJob>(std::bind(&DeferredRenderingLayer::PostEffectsDRJob,
					this, std::ref(pvp))));
				if (has_simple_forward_objs_ && !(pvp.attrib & VPAM_NoSimpleForward))
				{
					jobs_.push_back(MakeSharedPtr<DeferredRenderingJob>(std::bind(&DeferredRenderingLayer::SimpleForwardDRJob, this)));
				}
			}
		}

		if ((DT_SSVO == display_type_)
#if DEFAULT_DEFERRED == TRIDITIONAL_DEFERRED
			|| (DT_DiffuseLighting == display_type_)
			|| (DT_SpecularLighting == display_type_)
#endif
			)
		{
			jobs_.push_back(MakeSharedPtr<DeferredRenderingJob>(std::bind(&DeferredRenderingLayer::VisualizeLightingDRJob, this)));
		}
		else
		{
			jobs_.push_back(MakeSharedPtr<DeferredRenderingJob>(std::bind(&DeferredRenderingLayer::FinishingDRJob, this)));
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

	void DeferredRenderingLayer::AppendGBufferPassScanCode(uint32_t vp_index, PassTargetBuffer pass_tb)
	{
#ifndef KLAYGE_SHIP
		jobs_.push_back(MakeSharedPtr<DeferredRenderingJob>(std::bind(&DeferredRenderingLayer::BeginPerfProfileDRJob,
			this, std::ref(*gbuffer_perfs_[pass_tb]))));
#endif
		jobs_.push_back(MakeSharedPtr<DeferredRenderingJob>(std::bind(&DeferredRenderingLayer::GBufferGenerationDRJob,
			this, std::ref(viewports_[vp_index]), ComposePassType(PRT_MRT, pass_tb, PC_GBuffer))));
		jobs_.push_back(MakeSharedPtr<DeferredRenderingJob>(std::bind(&DeferredRenderingLayer::RenderingStatsDRJob,
			this)));
		jobs_.push_back(MakeSharedPtr<DeferredRenderingJob>(std::bind(&DeferredRenderingLayer::GBufferProcessingDRJob,
			this, std::cref(viewports_[vp_index]))));
		if (pass_tb == PTB_Opaque)
		{
			jobs_.push_back(MakeSharedPtr<DeferredRenderingJob>(std::bind(&DeferredRenderingLayer::OpaqueGBufferProcessingDRJob,
				this, std::cref(viewports_[vp_index]))));
			if ((DeferredRenderingLayer::DT_Position == display_type_)
				|| (DeferredRenderingLayer::DT_Normal == display_type_)
				|| (DeferredRenderingLayer::DT_Depth == display_type_)
				|| (DeferredRenderingLayer::DT_Diffuse == display_type_)
				|| (DeferredRenderingLayer::DT_Specular == display_type_)
				|| (DeferredRenderingLayer::DT_Shininess == display_type_))
			{
				jobs_.push_back(MakeSharedPtr<DeferredRenderingJob>(std::bind(&DeferredRenderingLayer::VisualizeGBufferDRJob, this)));
			}
		}
#ifndef KLAYGE_SHIP
		jobs_.push_back(MakeSharedPtr<DeferredRenderingJob>(std::bind(&DeferredRenderingLayer::EndPerfProfileDRJob,
			this, std::ref(*gbuffer_perfs_[pass_tb]))));
#endif
	}

	void DeferredRenderingLayer::AppendShadowPassScanCode(uint32_t light_index)
	{
		PassType shadow_pt = PT_GenShadowMap;

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
					jobs_.push_back(MakeSharedPtr<DeferredRenderingJob>(std::bind(&DeferredRenderingLayer::ShadowMapGenerationDRJob,
						this, std::cref(viewports_[0]), shadow_pt, light_index, 0)));
					jobs_.push_back(MakeSharedPtr<DeferredRenderingJob>(std::bind(&DeferredRenderingLayer::ShadowMapGenerationDRJob,
						this, std::cref(viewports_[0]), shadow_pt, light_index, 1)));
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
					jobs_.push_back(MakeSharedPtr<DeferredRenderingJob>(std::bind(&DeferredRenderingLayer::ShadowMapGenerationDRJob,
						this, std::cref(viewports_[0]), shadow_pt, light_index, j)));
				}
			}
			break;

		case LightSource::LT_Ambient:
		case LightSource::LT_Directional:
			break;

		default:
			KFL_UNREACHABLE("Invalid light type");
		}
	}

	void DeferredRenderingLayer::AppendCascadedShadowPassScanCode(uint32_t vp_index, uint32_t light_index)
	{
		BOOST_ASSERT(LightSource::LT_Directional == lights_[light_index]->Type());

#ifndef KLAYGE_SHIP
		jobs_.push_back(MakeSharedPtr<DeferredRenderingJob>(std::bind(&DeferredRenderingLayer::BeginPerfProfileDRJob,
			this, std::ref(*shadow_map_perf_))));
#endif

		PerViewport& pvp = viewports_[vp_index];
		for (uint32_t i = 0; i < pvp.num_cascades + 1; ++ i)
		{
			jobs_.push_back(MakeSharedPtr<DeferredRenderingJob>(std::bind(&DeferredRenderingLayer::ShadowMapGenerationDRJob,
				this, std::cref(pvp), PT_GenCascadedShadowMap, light_index, i)));
		}

#ifndef KLAYGE_SHIP
		jobs_.push_back(MakeSharedPtr<DeferredRenderingJob>(std::bind(&DeferredRenderingLayer::EndPerfProfileDRJob,
			this, std::ref(*shadow_map_perf_))));
#endif
	}

	void DeferredRenderingLayer::AppendIndirectLightingPassScanCode(uint32_t vp_index, uint32_t light_index)
	{
		jobs_.push_back(MakeSharedPtr<DeferredRenderingJob>(std::bind(&DeferredRenderingLayer::IndirectLightingDRJob,
			this, std::cref(viewports_[vp_index]), light_index)));
	}

	void DeferredRenderingLayer::AppendShadingPassScanCode(uint32_t vp_index, PassTargetBuffer pass_tb)
	{
		jobs_.push_back(MakeSharedPtr<DeferredRenderingJob>(std::bind(&DeferredRenderingLayer::ShadingDRJob,
			this, std::cref(viewports_[vp_index]), ComposePassType(PRT_None, pass_tb, PC_Shading), 0)));

		if (has_reflective_objs_)
		{
#ifndef KLAYGE_SHIP
			jobs_.push_back(MakeSharedPtr<DeferredRenderingJob>(std::bind(&DeferredRenderingLayer::BeginPerfProfileDRJob,
				this, std::ref(*reflection_perfs_[pass_tb]))));
#endif
			jobs_.push_back(MakeSharedPtr<DeferredRenderingJob>(std::bind(&DeferredRenderingLayer::ReflectionDRJob,
				this, std::cref(viewports_[vp_index]), ComposePassType(PRT_None, pass_tb, PC_Reflection))));
#ifndef KLAYGE_SHIP
			jobs_.push_back(MakeSharedPtr<DeferredRenderingJob>(std::bind(&DeferredRenderingLayer::EndPerfProfileDRJob,
				this, std::ref(*reflection_perfs_[pass_tb]))));
#endif
		}

		if (has_vdm_objs_)
		{
#ifndef KLAYGE_SHIP
			jobs_.push_back(MakeSharedPtr<DeferredRenderingJob>(std::bind(&DeferredRenderingLayer::BeginPerfProfileDRJob,
				this, std::ref(*vdm_perf_))));
#endif
			jobs_.push_back(MakeSharedPtr<DeferredRenderingJob>(std::bind(&DeferredRenderingLayer::VDMDRJob,
				this, std::cref(viewports_[vp_index]))));
#ifndef KLAYGE_SHIP
			jobs_.push_back(MakeSharedPtr<DeferredRenderingJob>(std::bind(&DeferredRenderingLayer::EndPerfProfileDRJob,
				this, std::ref(*vdm_perf_))));
#endif
		}

#ifndef KLAYGE_SHIP
		jobs_.push_back(MakeSharedPtr<DeferredRenderingJob>(std::bind(&DeferredRenderingLayer::BeginPerfProfileDRJob,
			this, std::ref(*special_shading_perfs_[pass_tb]))));
#endif
		jobs_.push_back(MakeSharedPtr<DeferredRenderingJob>(std::bind(&DeferredRenderingLayer::SpecialShadingDRJob,
			this, std::ref(viewports_[vp_index]), ComposePassType(PRT_None, pass_tb, PC_SpecialShading))));
		jobs_.push_back(MakeSharedPtr<DeferredRenderingJob>(std::bind(&DeferredRenderingLayer::MergeShadingAndDepthDRJob,
			this, std::ref(viewports_[vp_index]), pass_tb)));
#ifndef KLAYGE_SHIP
		jobs_.push_back(MakeSharedPtr<DeferredRenderingJob>(std::bind(&DeferredRenderingLayer::EndPerfProfileDRJob,
			this, std::ref(*special_shading_perfs_[pass_tb]))));
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
	}

	void DeferredRenderingLayer::GenerateGBuffer(PerViewport const & pvp, PassTargetBuffer pass_tb)
	{
		RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();

		if (PTB_Opaque == pass_tb)
		{
			CameraPtr const & camera = pvp.frame_buffer->GetViewport()->camera;
			pvp.g_buffer->GetViewport()->camera = camera;
#if DEFAULT_DEFERRED == TRIDITIONAL_DEFERRED
			pvp.lighting_fb->GetViewport()->camera = camera;
#endif
			pvp.vdm_fb->GetViewport()->camera = camera;
			pvp.shading_fb->GetViewport()->camera = camera;
			for (size_t i = 0; i < pvp.merged_shading_fbs.size(); ++ i)
			{
				pvp.merged_shading_fbs[i]->GetViewport()->camera = camera;
				pvp.merged_depth_fbs[i]->GetViewport()->camera = camera;
			}
		}

		re.BindFrameBuffer(pvp.g_buffer);

		float depth = (PTB_TransparencyBack == pass_tb) ? 0.0f : 1.0f;
		int32_t stencil = (PTB_Opaque == pass_tb) ? 0 : 16;
		pvp.g_buffer->Clear(FrameBuffer::CBM_Color | FrameBuffer::CBM_Depth | FrameBuffer::CBM_Stencil,
			Color(0, 0, 0, 0), depth, stencil);
	}

	void DeferredRenderingLayer::PostGenerateGBuffer(PerViewport const & pvp)
	{
		pvp.g_buffer_rt0_tex->BuildMipSubLevels();

#if DEFAULT_DEFERRED == LIGHT_INDEXED_DEFERRED
		if (cs_cldr_)
		{
			RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();

			re.BindFrameBuffer(pvp.lighting_mask_fb);
			re.CurFrameBuffer()->Attached(FrameBuffer::ATT_Color0)->ClearColor(Color(0, 0, 0, 0));
			re.Render(*dr_effect_, *technique_cldr_lighting_mask_, *rl_quad_);

			this->CreateDepthMinMaxMapCS(pvp);
		}
		else
		{
			this->BuildLinearDepthMipmap(pvp);
			this->CreateDepthMinMaxMap(pvp);
		}
#else
		this->BuildLinearDepthMipmap(pvp);
#endif
	}

	void DeferredRenderingLayer::BuildLinearDepthMipmap(PerViewport const & pvp)
	{
		depth_to_linear_pp_->InputPin(0, pvp.g_buffer_ds_tex);
		depth_to_linear_pp_->OutputPin(0, pvp.g_buffer_depth_tex);
		depth_to_linear_pp_->Apply();

		pvp.g_buffer_depth_tex->BuildMipSubLevels();
	}

	void DeferredRenderingLayer::RenderDecals(PerViewport const & pvp, PassType pass_type)
	{
		RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();

		uint32_t const width = pvp.g_buffer_rt0_tex->Width(0);
		uint32_t const height = pvp.g_buffer_rt0_tex->Height(0);
		pvp.g_buffer_rt0_tex->CopyToSubTexture2D(*pvp.g_buffer_rt0_backup_tex, 0, 0,
			0, 0, width, height, 0, 0, 0, 0, width, height);

		re.BindFrameBuffer(pvp.g_buffer);
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
		case LightSource::LT_Directional:
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
				else if (LightSource::LT_Directional == type)
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

				if ((LightSource::LT_Directional == type) && (pass_cat != PC_Shadowing) && (pass_cat != PC_Shading))
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

				if ((index_in_pass > 0) && ((PT_GenShadowMap == pass_type) || (PT_GenReflectiveShadowMap == pass_type)))
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

				case LightSource::LT_Directional:
					*light_volume_mv_param_ = pvp.inv_proj;
					*light_volume_mvp_param_ = float4x4::Identity();
					break;

				default:
					KFL_UNREACHABLE("Invalid light type");
				}
					
				*light_pos_es_param_ = light_pos_es_actived;
				*light_dir_es_param_ = light_dir_es_actived;
			}
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
			KFL_UNREACHABLE("Invalid light type");
		}
	}

	void DeferredRenderingLayer::PostGenerateShadowMap(PerViewport const & pvp, int32_t org_no, int32_t index_in_pass)
	{
		LightSource::LightType const type = lights_[org_no]->Type();

		if (type != LightSource::LT_Directional)
		{
			depth_to_esm_pp_->Apply();
		}

		PostProcessChainPtr pp_chain;
		if (LightSource::LT_Directional == type)
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
				if (tex_array_support_)
				{
					pp_chain->OutputPin(0, filtered_sm_2d_texs_[0], 0, sm_light_indices_[org_no].first);
				}
				else
				{
					pp_chain->OutputPin(0, filtered_sm_2d_texs_[sm_light_indices_[org_no].first]);
				}
				if (has_sss_objs_ && translucency_enabled_)
				{
					sm_tex_->CopyToTexture(*unfiltered_sm_2d_texs_[sm_light_indices_[org_no].first]);
				}
			}
		}

		int2 kernel_size;
		if (LightSource::LT_Directional == type)
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

		if (LightSource::LT_Directional == type)
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

	void DeferredRenderingLayer::UpdateShadowing(PerViewport const & pvp)
	{
		for (uint32_t li = 0; li < lights_.size(); ++ li)
		{
			auto const & light = *lights_[li];
			int32_t const attr = light.Attrib();
			if (light.Enabled() && (0 == (attr & LightSource::LSA_NoShadow)) && pvp.light_visibles[li])
			{
				LightSource::LightType const type = light.Type();

				this->PrepareLightCamera(pvp, light, 0, PT_Shadowing);

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

				RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();

				Camera* sm_camera = nullptr;

				int32_t const light_index = sm_light_indices_[li].first;
				int32_t const shadowing_channel = sm_light_indices_[li].second;
				if ((light_index >= 0) || (LightSource::LT_Directional == type))
				{
					switch (type)
					{
					case LightSource::LT_Spot:
						sm_camera = light.SMCamera(0).get();
						if (tex_array_support_)
						{
							*filtered_sm_2d_tex_array_param_ = filtered_sm_2d_texs_[0];
							*filtered_sm_2d_light_index_param_ = light_index;
						}
						else
						{
							*filtered_sm_2d_tex_param_ = filtered_sm_2d_texs_[light_index];
						}
						break;

					case LightSource::LT_Point:
					case LightSource::LT_SphereArea:
					case LightSource::LT_TubeArea:
						sm_camera = light.SMCamera(0).get();
						*filtered_sm_cube_tex_param_ = filtered_sm_cube_texs_[light_index];
						break;

					case LightSource::LT_Directional:
						{
							sm_camera = lights_[cascaded_shadow_index_]->SMCamera(0).get();
							BOOST_ASSERT(sm_camera);
							KLAYGE_ASSUME(sm_camera);

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

				if (li == static_cast<uint32_t>(projective_light_index_))
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
				}

				re.Render(*dr_effect_, *technique_shadows_[type][shadowing_channel], *light_volume_rl_[type]);
			}
		}
	}

#if DEFAULT_DEFERRED == LIGHT_INDEXED_DEFERRED
	void DeferredRenderingLayer::UpdateShadowingCS(PerViewport const & pvp)
	{
		RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
		re.BindFrameBuffer(FrameBufferPtr());

		*min_max_depth_tex_param_ = pvp.g_buffer_min_max_depth_texs.back();
		*lighting_mask_tex_param_ = pvp.lighting_mask_tex;
		*projective_shadowing_rw_tex_param_ = pvp.projective_shadowing_tex;
		*shadowing_rw_tex_param_ = pvp.shadowing_tex;

		uint32_t w = pvp.shadowing_tex->Width(0);
		uint32_t h = pvp.shadowing_tex->Height(0);
		float2 tile_scale(((w + TILE_SIZE - 1) & ~(TILE_SIZE - 1)) / (2.0f * TILE_SIZE),
			((h + TILE_SIZE - 1) & ~(TILE_SIZE - 1)) / (2.0f * TILE_SIZE));
		*tile_scale_param_ = float4(tile_scale.x(), tile_scale.y(), 0, 0);
		*camera_proj_01_param_ = float2(pvp.proj(0, 0) * tile_scale.x(), pvp.proj(1, 1) * tile_scale.y());

		float3 upper_left = MathLib::transform_coord(float3(-1, +1, 1), pvp.inv_proj);
		float3 upper_right = MathLib::transform_coord(float3(+1, +1, 1), pvp.inv_proj);
		float3 lower_left = MathLib::transform_coord(float3(-1, -1, 1), pvp.inv_proj);
		*upper_left_param_ = upper_left;
		*x_dir_param_ = upper_right - upper_left;
		*y_dir_param_ = lower_left - upper_left;

		*inv_width_height_param_ = float2(1.0f / w, 1.0f / h);
		*width_height_param_ = uint2(w, h);

		uint8_t* lights_type = lights_type_param_->MemoryInCBuff<uint8_t>();
		uint8_t* lights_pos_es = lights_pos_es_param_->MemoryInCBuff<uint8_t>();
		uint8_t* lights_falloff_range = lights_falloff_range_param_->MemoryInCBuff<uint8_t>();
		uint8_t* lights_attrib = lights_attrib_param_->MemoryInCBuff<uint8_t>();
		uint8_t* lights_aabb_min = lights_aabb_min_param_->MemoryInCBuff<uint8_t>();
		uint8_t* lights_aabb_max = lights_aabb_max_param_->MemoryInCBuff<uint8_t>();
		uint8_t* lights_view_proj = lights_view_proj_param_->MemoryInCBuff<uint8_t>();
		uint8_t* filtered_sms_2d_light_index = filtered_sms_2d_light_index_param_->MemoryInCBuff<uint8_t>();
		uint8_t* esms_scale_factor = esms_scale_factor_param_->MemoryInCBuff<uint8_t>();

		for (uint32_t li = 0; li < lights_.size(); ++ li)
		{
			auto const & light = *lights_[li];
			int32_t const attr = light.Attrib();
			if (light.Enabled() && (0 == (attr & LightSource::LSA_NoShadow)) && pvp.light_visibles[li])
			{
				LightSource::LightType const type = light.Type();

				Camera* sm_camera = nullptr;

				int32_t const light_index = sm_light_indices_[li].first;
				int32_t const shadowing_channel = sm_light_indices_[li].second;
				if ((light_index >= 0) || (LightSource::LT_Directional == type))
				{
					switch (type)
					{
					case LightSource::LT_Spot:
						sm_camera = light.SMCamera(0).get();
						if (tex_array_support_)
						{
							*filtered_sm_2d_tex_array_param_ = filtered_sm_2d_texs_[0];
						}
						else
						{
							*filtered_sm_2d_tex_param_ = filtered_sm_2d_texs_[light_index];
						}
						break;

					case LightSource::LT_Point:
					case LightSource::LT_SphereArea:
					case LightSource::LT_TubeArea:
						sm_camera = light.SMCamera(0).get();
						*filtered_sm_cube_tex_param_ = filtered_sm_cube_texs_[light_index];
						break;

					case LightSource::LT_Directional:
						{
							sm_camera = lights_[cascaded_shadow_index_]->SMCamera(0).get();

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
						KFL_UNREACHABLE("Invalid light type");
					}
				}
				BOOST_ASSERT(sm_camera);
				KLAYGE_ASSUME(sm_camera);

				*reinterpret_cast<uint32_t*>(lights_type + shadowing_channel * lights_type_param_->Stride()) = type;

				*reinterpret_cast<float4x4*>(lights_view_proj + shadowing_channel * lights_view_proj_param_->Stride())
					= MathLib::transpose(pvp.inv_view * sm_camera->ViewProjMatrix());
				*reinterpret_cast<float*>(esms_scale_factor + shadowing_channel * esms_scale_factor_param_->Stride())
					= ESM_SCALE_FACTOR / (sm_camera->FarPlane() - sm_camera->NearPlane());

				float3 loc_es = MathLib::transform_coord(light.Position(), pvp.view);
				float4 light_pos_es_actived = float4(loc_es.x(), loc_es.y(), loc_es.z(), 1);

				float range = light.Range() * light_scale_;
				AABBox aabb(float3(0, 0, 0), float3(0, 0, 0));
				switch (type)
				{
				case LightSource::LT_Spot:
					{
						light_pos_es_actived.w() = light.CosOuterInner().x();

						*reinterpret_cast<int32_t*>(filtered_sms_2d_light_index
							+ shadowing_channel * filtered_sms_2d_light_index_param_->Stride()) = light_index;

						float4x4 const light_to_view = light.SMCamera(0)->InverseViewMatrix() * pvp.view;
						float const scale = light.CosOuterInner().w();
						float4x4 const light_model = MathLib::scaling(range * 0.01f * float3(scale, scale, 1));
						float4x4 const light_mv = light_model * light_to_view;
						aabb = MathLib::transform_aabb(cone_aabb_, light_mv);
					}
					break;

				case LightSource::LT_Point:
				case LightSource::LT_SphereArea:
				case LightSource::LT_TubeArea:
					{
						float light_scale = std::min(light.Range() * 0.01f, 1.0f) * light_scale_;
						float4x4 const light_model = MathLib::scaling(light_scale, light_scale, light_scale)
							* MathLib::to_matrix(light.Rotation()) * MathLib::translation(light.Position());
						*view_to_light_model_param_ = pvp.inv_view * MathLib::inverse(light_model);
					}
					break;

				case LightSource::LT_Directional:
					break;

				default:
					KFL_UNREACHABLE("Invalid light type");
				}

				*reinterpret_cast<float4*>(lights_pos_es + shadowing_channel * lights_pos_es_param_->Stride()) = light_pos_es_actived;

				*reinterpret_cast<float4*>(lights_attrib + shadowing_channel * lights_attrib_param_->Stride())
					= float4((attr & LightSource::LSA_NoDiffuse) ? 0.0f : 1.0f, (attr & LightSource::LSA_NoSpecular) ? 0.0f : 1.0f,
						(attr & LightSource::LSA_NoShadow) ? -1.0f : 1.0f,
						light.ProjectiveTexture() ? 1.0f : -1.0f);

				*reinterpret_cast<float4*>(lights_falloff_range + shadowing_channel * lights_falloff_range_param_->Stride())
					= float4(light.Falloff().x(), light.Falloff().y(), light.Falloff().z(), range);
				*reinterpret_cast<float4*>(lights_aabb_min + shadowing_channel * lights_aabb_min_param_->Stride())
					= float4(aabb.Min().x(), aabb.Min().y(), aabb.Min().z(), 0);
				*reinterpret_cast<float4*>(lights_aabb_max + shadowing_channel * lights_aabb_max_param_->Stride())
					= float4(aabb.Max().x(), aabb.Max().y(), aabb.Max().z(), 0);

				if (4 == shadowing_channel)
				{
					if ((LightSource::LT_Point == type) || (LightSource::LT_SphereArea == type)
						|| (LightSource::LT_TubeArea == type))
					{
						*projective_map_cube_tex_param_ = light.ProjectiveTexture();
					}
					else
					{
						*projective_map_2d_tex_param_ = light.ProjectiveTexture();
					}
				}
			}
		}

		lights_type_param_->CBuffer().Dirty(true);
		lights_pos_es_param_->CBuffer().Dirty(true);
		lights_falloff_range_param_->CBuffer().Dirty(true);
		lights_attrib_param_->CBuffer().Dirty(true);
		lights_aabb_min_param_->CBuffer().Dirty(true);
		lights_aabb_max_param_->CBuffer().Dirty(true);
		lights_view_proj_param_->CBuffer().Dirty(true);
		filtered_sms_2d_light_index_param_->CBuffer().Dirty(true);
		esms_scale_factor_param_->CBuffer().Dirty(true);

		re.Dispatch(*dr_effect_, *technique_cldr_shadowing_unified_, (w + TILE_SIZE - 1) / TILE_SIZE, (h + TILE_SIZE - 1) / TILE_SIZE, 1);
	}
#endif

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
			re.Render(*dr_effect_, *technique_light_stencil_, *rl);
		}

		if (LightSource::LT_Ambient == type)
		{
			if (light.SkylightTexY())
			{
				*skylight_y_cube_tex_param_ = light.SkylightTexY();
				*skylight_c_cube_tex_param_ = light.SkylightTexC();

				uint32_t const mip = light.SkylightTexY()->NumMipMaps();
				*skylight_diff_spec_mip_param_ = int3(mip - 1, mip - 2, 1);

				*inv_view_param_ = pvp.inv_view;
			}
			else
			{
				*skylight_diff_spec_mip_param_ = int3(0, 0, 0);
			}
		}

		re.Render(*dr_effect_, *technique_lights_[type], *rl);
	}

	void DeferredRenderingLayer::UpdateShading(PerViewport const & pvp, PassTargetBuffer pass_tb)
	{
		RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();

		*g_buffer_tex_param_ = pvp.g_buffer_rt0_tex;
		*g_buffer_1_tex_param_ = pvp.g_buffer_rt1_tex;
		*depth_tex_param_ = pvp.g_buffer_depth_tex;
		*lighting_tex_param_ = pvp.lighting_tex;
		*light_volume_mv_param_ = pvp.inv_proj;

		if (PTB_Opaque == pass_tb)
		{
			re.BindFrameBuffer(pvp.merged_shading_fbs[pvp.curr_merged_buffer_index]);
			re.Render(*dr_effect_, *technique_no_lighting_, *rl_quad_);
		}
		else
		{
			re.BindFrameBuffer(pvp.shading_fb);
			re.CurFrameBuffer()->Attached(FrameBuffer::ATT_Color0)->Discard();
		}
		re.Render(*dr_effect_, *technique_shading_, *rl_quad_);
	}
#endif

	void DeferredRenderingLayer::MergeIndirectLighting(PerViewport const & pvp, PassTargetBuffer pass_tb)
	{
		if ((indirect_lighting_enabled_ && !(pvp.attrib & VPAM_NoGI)) && (illum_ != 1))
		{
			pvp.il_layer->CalcIndirectLighting(pvp.merged_shading_texs[!pvp.curr_merged_buffer_index], pvp.proj_to_prev);
			this->AccumulateToLightingTex(pvp, pass_tb);
		}
	}

	void DeferredRenderingLayer::MergeSSVO(PerViewport const & pvp, PassTargetBuffer pass_tb)
	{
		if (pvp.ssvo_enabled && !(pvp.attrib & VPAM_NoSSVO))
		{
			ssvo_pp_->InputPin(0, pvp.g_buffer_rt0_tex);
			ssvo_pp_->InputPin(1, pvp.g_buffer_depth_tex);
			ssvo_pp_->OutputPin(0, pvp.small_ssvo_tex);
			ssvo_pp_->Apply();

			ssvo_blur_pp_->InputPin(0, pvp.small_ssvo_tex);
			ssvo_blur_pp_->InputPin(1, pvp.g_buffer_depth_tex);
			ssvo_blur_pp_->OutputPin(0,
				(PTB_Opaque == pass_tb) ? pvp.merged_shading_texs[pvp.curr_merged_buffer_index] : pvp.shading_tex);
			ssvo_blur_pp_->Apply();
		}
	}

	void DeferredRenderingLayer::AddTranslucency(uint32_t org_no, PerViewport const & pvp, PassTargetBuffer pass_tb)
	{
		auto & light = lights_[org_no];
		LightSource::LightType const type = light->Type();
		int32_t const light_index = sm_light_indices_[org_no].first;
		if (light->Enabled() && pvp.light_visibles[org_no] && (0 == (light->Attrib() & LightSource::LSA_NoShadow))
			&& ((light_index >= 0) || (LightSource::LT_Directional == type)))
		{
			Camera* light_camera = nullptr;
			switch (type)
			{
			case LightSource::LT_Spot:
				light_camera = light->SMCamera(0).get();
				translucency_pp_->InputPin(3, unfiltered_sm_2d_texs_[light_index]);
				break;

			case LightSource::LT_Directional:
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
					(PTB_Opaque == pass_tb) ? pvp.merged_shading_texs[pvp.curr_merged_buffer_index] : pvp.shading_tex);

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
		sss_blur_pp_->InputPin(0, pvp.merged_shading_texs[pvp.curr_merged_buffer_index]);
		sss_blur_pp_->InputPin(1, pvp.g_buffer_depth_tex);
		sss_blur_pp_->OutputPin(0, pvp.merged_shading_texs[pvp.curr_merged_buffer_index]);
		sss_blur_pp_->Apply();
	}

	void DeferredRenderingLayer::AddSSR(PerViewport const & pvp)
	{
		RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();

		re.BindFrameBuffer(pvp.merged_shading_fbs[pvp.curr_merged_buffer_index]);
		ssr_pp_->InputPin(0, pvp.g_buffer_rt0_tex);
		ssr_pp_->InputPin(1, pvp.g_buffer_rt1_tex);
		ssr_pp_->InputPin(2, pvp.g_buffer_depth_tex);
		ssr_pp_->InputPin(3, pvp.merged_shading_texs[!pvp.curr_merged_buffer_index]);
		ssr_pp_->InputPin(4, pvp.merged_depth_texs[pvp.curr_merged_buffer_index]);
		ssr_pp_->Apply();
	}

	void DeferredRenderingLayer::AddVDM(PerViewport const & pvp)
	{
		RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();

		re.BindFrameBuffer(pvp.merged_shading_fbs[pvp.curr_merged_buffer_index]);
		vdm_composition_pp_->InputPin(0, pvp.vdm_color_tex);
		vdm_composition_pp_->InputPin(1, pvp.vdm_transition_tex);
		vdm_composition_pp_->InputPin(2, pvp.vdm_count_tex);
		vdm_composition_pp_->InputPin(3, pvp.merged_depth_texs[pvp.curr_merged_buffer_index]);
		vdm_composition_pp_->Render();
	}

	void DeferredRenderingLayer::AddAtmospheric(PerViewport const & pvp)
	{
		RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();

		re.BindFrameBuffer(pvp.merged_shading_fbs[pvp.curr_merged_buffer_index]);
		atmospheric_pp_->SetParam(0, pvp.inv_proj);
		atmospheric_pp_->InputPin(0, pvp.merged_depth_texs[pvp.curr_merged_buffer_index]);
		atmospheric_pp_->Render();
	}

	void DeferredRenderingLayer::AddTAA(PerViewport const & pvp)
	{
		RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();

		re.BindFrameBuffer(pvp.frame_buffer);
		pvp.frame_buffer->Discard(FrameBuffer::CBM_Color | FrameBuffer::CBM_Depth | FrameBuffer::CBM_Stencil);
		{
			*depth_tex_param_ = pvp.merged_depth_texs[pvp.curr_merged_buffer_index];

			Camera const & camera = *pvp.frame_buffer->GetViewport()->camera;
			float q = camera.FarPlane() / (camera.FarPlane() - camera.NearPlane());
			float2 near_q(camera.NearPlane() * q, q);
			*near_q_param_ = near_q;
		}
		App3DFramework& app = Context::Instance().AppInstance();
		if ((app.FrameTime() < 1.0f / 30) && taa_enabled_)
		{
			taa_pp_->InputPin(0, pvp.merged_shading_texs[pvp.curr_merged_buffer_index]);
			taa_pp_->InputPin(1, pvp.merged_shading_texs[!pvp.curr_merged_buffer_index]);
			taa_pp_->Render();
			re.Render(*dr_effect_, *technique_copy_depth_, *rl_quad_);
		}
		else
		{
			*shading_tex_param_ = pvp.merged_shading_texs[pvp.curr_merged_buffer_index];
			re.Render(*dr_effect_, *technique_copy_shading_depth_, *rl_quad_);
		}
	}

	void DeferredRenderingLayer::MergeShadingAndDepth(PerViewport const & pvp, PassTargetBuffer pass_tb)
	{
		RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();

		if (pass_tb != PTB_Opaque)
		{
			re.BindFrameBuffer(pvp.merged_shading_fbs[pvp.curr_merged_buffer_index]);
			*shading_tex_param_ = pvp.shading_tex;
			re.Render(*dr_effect_, *technique_merge_shadings_[pass_tb != PTB_Opaque], *rl_quad_);
		}

		re.BindFrameBuffer(pvp.merged_depth_fbs[pvp.curr_merged_buffer_index]);
		*depth_tex_param_ = pvp.g_buffer_depth_tex;
		re.Render(*dr_effect_, *technique_merge_depths_[pass_tb != PTB_Opaque], *rl_quad_);
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

	void DeferredRenderingLayer::AccumulateToLightingTex(PerViewport const & pvp, PassTargetBuffer pass_tb)
	{
		PostProcessPtr const & copy_to_light_buffer_pp = (0 == illum_) ? copy_to_light_buffer_pp_ : copy_to_light_buffer_i_pp_;
		copy_to_light_buffer_pp->SetParam(0, indirect_scale_ * 256 / VPL_COUNT);
		copy_to_light_buffer_pp->SetParam(1, float2(1.0f / pvp.g_buffer_rt0_tex->Width(0), 1.0f / pvp.g_buffer_rt0_tex->Height(0)));
		copy_to_light_buffer_pp->SetParam(2, pvp.inv_proj);
		copy_to_light_buffer_pp->InputPin(0, pvp.il_layer->IndirectLightingTex());
		copy_to_light_buffer_pp->InputPin(1, pvp.g_buffer_rt0_tex);
		copy_to_light_buffer_pp->InputPin(2, pvp.g_buffer_rt1_tex);
		copy_to_light_buffer_pp->InputPin(3, pvp.g_buffer_depth_tex);
		copy_to_light_buffer_pp->OutputPin(0,
			(PTB_Opaque == pass_tb) ? pvp.merged_shading_texs[pvp.curr_merged_buffer_index] : pvp.shading_tex);
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

	void DeferredRenderingLayer::CreateVDMDepthMaxMap(PerViewport const & pvp)
	{
		for (uint32_t i = 0; i < 2; ++ i)
		{
			TexturePtr input_tex = (0 == i) ? pvp.g_buffer_ds_tex : pvp.g_buffer_vdm_max_ds_texs[i - 1];

			uint32_t const & w = input_tex->Width(0);
			uint32_t const & h = input_tex->Height(0);
			depth_to_max_pp_->SetParam(0, float2(0.5f / w, 0.5f / h));
			depth_to_max_pp_->SetParam(1, float2(static_cast<float>((w + 1) & ~1) / w,
				static_cast<float>((h + 1) & ~1) / h));
			depth_to_max_pp_->InputPin(0, input_tex);
			// Borrow the small_ssvo_tex
			depth_to_max_pp_->OutputPin(0, (0 == i) ? pvp.small_ssvo_tex : pvp.vdm_color_tex);
			depth_to_max_pp_->OutputFrameBuffer()->Attach(FrameBuffer::ATT_DepthStencil, pvp.g_buffer_vdm_max_ds_views[i]);
			depth_to_max_pp_->Apply();
		}
	}

#if DEFAULT_DEFERRED == LIGHT_INDEXED_DEFERRED
	void DeferredRenderingLayer::UpdateLightIndexedLighting(PerViewport const & pvp, PassTargetBuffer pass_tb)
	{
		*g_buffer_1_tex_param_ = pvp.g_buffer_rt1_tex;
		*light_volume_mv_param_ = pvp.inv_proj;

		RenderFactory& rf = Context::Instance().RenderFactoryInstance();
		RenderEngine& re = rf.RenderEngineInstance();
		re.BindFrameBuffer((PTB_Opaque == pass_tb) ? pvp.merged_shading_fbs[pvp.curr_merged_buffer_index] : pvp.shading_fb);
		re.Render(*dr_effect_, *technique_no_lighting_, *rl_quad_);

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
					this->UpdateLightIndexedLightingAmbientSun(pvp, type, li, pass_tb);
					break;

				case LightSource::LT_Directional:
					if (light.Attrib() & LightSource::LSA_NoShadow)
					{
						directional_lights.push_back(li);
					}
					else
					{
						this->UpdateLightIndexedLightingAmbientSun(pvp, type, li, pass_tb);
					}
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
					KFL_UNREACHABLE("Invalid light type");
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
				this->UpdateLightIndexedLightingDirectional(pvp, pass_tb, iter_beg, iter_end);
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
				this->UpdateLightIndexedLightingPointSpotArea(pvp, pass_tb, iter_beg, iter_end);
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
				this->UpdateLightIndexedLightingPointSpotArea(pvp, pass_tb, iter_beg, iter_end);
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
				this->UpdateLightIndexedLightingPointSpotArea(pvp, pass_tb, iter_beg, iter_end);
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
				this->UpdateLightIndexedLightingPointSpotArea(pvp, pass_tb, iter_beg, iter_end);
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
				this->UpdateLightIndexedLightingPointSpotArea(pvp, pass_tb, iter_beg, iter_end);
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
				this->UpdateLightIndexedLightingPointSpotArea(pvp, pass_tb, iter_beg, iter_end);
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
				this->UpdateLightIndexedLightingPointSpotArea(pvp, pass_tb, iter_beg, iter_end);
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
				this->UpdateLightIndexedLightingPointSpotArea(pvp, pass_tb, iter_beg, iter_end);
				li += nl;
			}
		}
	}

	void DeferredRenderingLayer::UpdateLightIndexedLightingAmbientSun(PerViewport const & pvp, LightSource::LightType type,
		int32_t org_no, PassTargetBuffer pass_tb)
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

		RenderTechnique* tech;
		if (LightSource::LT_Directional == type)
		{
			std::vector<float4> lights_color;
			std::vector<float4> lights_dir_es;
			std::vector<float4> lights_attrib;

			lights_color.push_back(light.Color());

			float3 dir_es = MathLib::transform_normal(-light.Direction(), pvp.view);
			lights_dir_es.push_back(float4(dir_es.x(), dir_es.y(), dir_es.z(), 0));

			lights_attrib.push_back(float4((attr & LightSource::LSA_NoDiffuse) ? 0.0f : 1.0f,
				(attr & LightSource::LSA_NoSpecular) ? 0.0f : 1.0f, 0, 1.5f));

			*lights_color_param_ = lights_color;
			*lights_dir_es_param_ = lights_dir_es;
			*lights_attrib_param_ = lights_attrib;

			tech = technique_lidr_directional_shadow_;
		}
		else
		{
			BOOST_ASSERT(LightSource::LT_Ambient == type);

			float3 dir_es = MathLib::transform_normal(float3(0, 1, 0), pvp.view);
			*light_dir_es_param_ = float4(dir_es.x(), dir_es.y(), dir_es.z(), 0);

			*light_attrib_param_ = float4((attr & LightSource::LSA_NoDiffuse) ? 0.0f : 1.0f,
				(attr & LightSource::LSA_NoSpecular) ? 0.0f : 1.0f, 0, 0);
			*light_color_param_ = light.Color();

			if (light.SkylightTexY())
			{
				*skylight_y_cube_tex_param_ = light.SkylightTexY();
				*skylight_c_cube_tex_param_ = light.SkylightTexC();

				uint32_t const mip = light.SkylightTexY()->NumMipMaps();
				*skylight_diff_spec_mip_param_ = int3(mip - 1, mip - 2, 1);

				*inv_view_param_ = pvp.inv_view;
			}
			else
			{
				*skylight_diff_spec_mip_param_ = int3(0, 0, 0);
			}

			tech = technique_lidr_ambient_;
		}

		*g_buffer_tex_param_ = pvp.g_buffer_rt0_tex;
		*g_buffer_1_tex_param_ = pvp.g_buffer_rt1_tex;
		*light_volume_mv_param_ = pvp.inv_proj;

		if (PTB_Opaque == pass_tb)
		{
			re.BindFrameBuffer(pvp.merged_shading_fbs[pvp.curr_merged_buffer_index]);
		}
		else
		{
			re.BindFrameBuffer(pvp.shading_fb);
		}
		re.Render(*dr_effect_, *tech, *rl_quad_);
	}

	void DeferredRenderingLayer::UpdateLightIndexedLightingDirectional(PerViewport const & pvp,
		PassTargetBuffer pass_tb, std::vector<uint32_t>::const_iterator iter_beg, std::vector<uint32_t>::const_iterator iter_end)
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

			lights_attrib.push_back(float4((attr & LightSource::LSA_NoDiffuse) ? 0.0f : 1.0f,
				(attr & LightSource::LSA_NoSpecular) ? 0.0f : 1.0f, 0, 0));
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
		re.BindFrameBuffer((PTB_Opaque == pass_tb) ? pvp.merged_shading_fbs[pvp.curr_merged_buffer_index] : pvp.shading_fb);
		re.Render(*dr_effect_, *technique_lidr_directional_no_shadow_, *rl_quad_);
	}

	void DeferredRenderingLayer::UpdateLightIndexedLightingPointSpotArea(PerViewport const & pvp, PassTargetBuffer pass_tb,
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

			lights_attrib.push_back(float4((attr & LightSource::LSA_NoDiffuse) ? 0.0f : 1.0f,
				(attr & LightSource::LSA_NoSpecular) ? 0.0f : 1.0f, channel + 0.5f, 0));

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

		RenderTechnique* tech;
		if ((LightSource::LT_Point == type) || (LightSource::LT_SphereArea == type)
			|| (LightSource::LT_TubeArea == type))
		{
			tech = technique_draw_light_index_point_;
		}
		else
		{
			tech = technique_draw_light_index_spot_;
		}
		re.Render(*dr_effect_, *tech, *rl_quad_);

		*light_index_tex_param_ = pvp.light_index_tex;
		*g_buffer_tex_param_ = pvp.g_buffer_rt0_tex;
		*g_buffer_1_tex_param_ = pvp.g_buffer_rt1_tex;
		*depth_tex_param_ = pvp.g_buffer_depth_tex;
		*light_volume_mv_param_ = pvp.inv_proj;

		if (PTB_Opaque == pass_tb)
		{
			re.BindFrameBuffer(pvp.merged_shading_fbs[pvp.curr_merged_buffer_index]);
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
			KFL_UNREACHABLE("Invalid light type");
		}
		re.Render(*dr_effect_, *tech, *rl_quad_);
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

	void DeferredRenderingLayer::UpdateClusteredLighting(PerViewport const & pvp, PassTargetBuffer pass_tb)
	{
		RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();

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

							*inv_view_param_ = pvp.inv_view;
						}
						break;

					case LightSource::LT_Directional:
						if (light.Attrib() & LightSource::LSA_NoShadow)
						{
							available_lights[1].push_back(li);
						}
						else
						{
							available_lights[2].push_back(li);
						}
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
						KFL_UNREACHABLE("Invalid light type");
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

					int32_t shadowing_channel;
					if (0 == (light.Attrib() & LightSource::LSA_NoShadow))
					{
						shadowing_channel = sm_light_indices_[available_lights[t][i]].second;
					}
					else
					{
						shadowing_channel = -1;
					}

					*reinterpret_cast<float4*>(lights_attrib
						+ offset * lights_attrib_param_->Stride()) = float4((attr & LightSource::LSA_NoDiffuse) ? 0.0f : 1.0f,
						(attr & LightSource::LSA_NoSpecular) ? 0.0f : 1.0f, shadowing_channel + 0.5f, 0);

					float3 extend_es = MathLib::transform_normal(light.Extend(), pvp.view);
					*reinterpret_cast<float4*>(lights_radius_extend
						+ offset * lights_radius_extend_param_->Stride()) = float4(light.Radius(),
						extend_es.x(), extend_es.y(), extend_es.z());

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

			lights_type_param_->CBuffer().Dirty(true);
			lights_color_param_->CBuffer().Dirty(true);
			lights_pos_es_param_->CBuffer().Dirty(true);
			lights_dir_es_param_->CBuffer().Dirty(true);
			lights_falloff_range_param_->CBuffer().Dirty(true);
			lights_attrib_param_->CBuffer().Dirty(true);
			lights_radius_extend_param_->CBuffer().Dirty(true);
			lights_aabb_min_param_->CBuffer().Dirty(true);
			lights_aabb_max_param_->CBuffer().Dirty(true);

			if (pvp.temp_shading_tex)
			{
				if (available_lights[0].empty())
				{
					*shading_in_tex_param_
						= (PTB_Opaque == pass_tb) ? pvp.merged_shading_texs[pvp.curr_merged_buffer_index] : pvp.shading_tex;
					*shading_rw_tex_param_ = pvp.temp_shading_tex;
				}
				else
				{
					*shading_in_tex_param_ = TexturePtr();
					*shading_rw_tex_param_
						= (PTB_Opaque == pass_tb) ? pvp.merged_shading_texs[pvp.curr_merged_buffer_index] : pvp.shading_tex;
				}
			}
			else
			{
				*shading_rw_tex_param_
					= (PTB_Opaque == pass_tb) ? pvp.merged_shading_texs[pvp.curr_merged_buffer_index] : pvp.shading_tex;
			}

			{
				CameraPtr const & camera = pvp.frame_buffer->GetViewport()->camera;

				float const near_plane = camera->NearPlane();
				float const far_plane = camera->FarPlane();
				depth_slices_[0] = near_plane;
				depth_slices_[1] = depth_slices_[0] + (far_plane - near_plane) * 0.02f;
				float const base = far_plane / depth_slices_[1];
				for (uint32_t i = 2; i < num_depth_slices_; ++ i)
				{
					depth_slices_[i] = depth_slices_[1] * pow(base, static_cast<float>(i) / num_depth_slices_);
				}
				depth_slices_[num_depth_slices_] = far_plane;

				{
					uint8_t* depth_slices = depth_slices_param_->MemoryInCBuff<uint8_t>();
					for (size_t i = 0; i < depth_slices_.size(); ++ i)
					{
						*reinterpret_cast<float*>(depth_slices + i * depth_slices_param_->Stride()) = depth_slices_[i];
					}
					depth_slices_param_->CBuffer().Dirty(true);
				}

				float const log_depth_1 = std::log(depth_slices_[1]);
				*depth_slices_shading_param_ = float3(depth_slices_[1], log_depth_1, std::log(far_plane) - log_depth_1);
			}

#ifndef KLAYGE_SHIP
			clustering_perfs_[pass_tb]->Begin();
#endif

			*lights_start_rw_tex_param_ = pvp.lights_start_tex;
			*intersected_light_indices_rw_tex_param_ = pvp.intersected_light_indices_tex;
			re.Dispatch(*dr_effect_, *technique_cldr_light_intersection_unified_,
				(w + TILE_SIZE - 1) / TILE_SIZE, (h + TILE_SIZE - 1) / TILE_SIZE, num_depth_slices_);

#ifndef KLAYGE_SHIP
			clustering_perfs_[pass_tb]->End();
#endif

			uint32_t const BLOCK_X = 16;
			uint32_t const BLOCK_Y = 16;
			*lights_start_in_tex_param_ = pvp.lights_start_tex;
			*intersected_light_indices_in_tex_param_ = pvp.intersected_light_indices_tex;
			re.Dispatch(*dr_effect_, *technique_cldr_unified_, (w + BLOCK_X - 1) / BLOCK_X, (h + BLOCK_Y - 1) / BLOCK_Y, 1);

			if (available_lights[0].empty() && pvp.temp_shading_tex)
			{
				copy_pp_->InputPin(0, pvp.temp_shading_tex);
				copy_pp_->OutputPin(0,
					(PTB_Opaque == pass_tb) ? pvp.merged_shading_texs[pvp.curr_merged_buffer_index] : pvp.shading_tex);
				copy_pp_->Apply();
			}
		}
	}

	void DeferredRenderingLayer::CreateDepthMinMaxMapCS(PerViewport const & pvp)
	{
		RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
		re.BindFrameBuffer(re.DefaultFrameBuffer());
		re.DefaultFrameBuffer()->Discard(FrameBuffer::CBM_Color | FrameBuffer::CBM_Depth | FrameBuffer::CBM_Stencil);

		TexturePtr const & in_tex = pvp.g_buffer_ds_tex;
		TexturePtr const & out_tex = pvp.g_buffer_min_max_depth_texs.back();
		*width_height_param_ = uint2(in_tex->Width(0) - 1, in_tex->Height(0) - 1);
		*depth_to_tiled_depth_in_tex_param_ = in_tex;
		*depth_to_tiled_min_max_depth_rw_tex_param_ = out_tex;
		*linear_depth_rw_tex_param_ = pvp.g_buffer_depth_tex;

		CameraPtr const & camera = pvp.frame_buffer->GetViewport()->camera;
		float q = camera->FarPlane() / (camera->FarPlane() - camera->NearPlane());
		float4 near_q_far(camera->NearPlane() * q, q, camera->FarPlane(), 1 / camera->FarPlane());
		*near_q_far_param_ = near_q_far;

		re.Dispatch(*dr_effect_, *technique_depth_to_tiled_min_max_, out_tex->Width(0), out_tex->Height(0), 1);

		pvp.g_buffer_depth_tex->BuildMipSubLevels();
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
			1, 1, EF_ABGR32F, 1, 0, EAH_GPU_Write);
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


	uint32_t DeferredRenderingLayer::BeginPerfProfileDRJob(PerfRange& perf)
	{
		perf.Begin();
		return 0;
	}

	uint32_t DeferredRenderingLayer::EndPerfProfileDRJob(PerfRange& perf)
	{
		perf.End();
		return 0;
	}

	uint32_t DeferredRenderingLayer::RenderingStatsDRJob()
	{
		auto& scene_mgr = Context::Instance().SceneManagerInstance();

		num_objects_rendered_ += scene_mgr.NumObjectsRendered();
		num_renderables_rendered_ += scene_mgr.NumRenderablesRendered();
		num_primitives_rendered_ += scene_mgr.NumPrimitivesRendered();
		num_vertices_rendered_ += scene_mgr.NumVerticesRendered();

		return 0;
	}

	uint32_t DeferredRenderingLayer::GBufferGenerationDRJob(PerViewport& pvp, PassType pass_type)
	{
		auto& rf = Context::Instance().RenderFactoryInstance();
		auto& re = rf.RenderEngineInstance();

		auto const pass_tb = GetPassTargetBuffer(pass_type);

		for (auto const & deo : visible_scene_objs_)
		{
			deo->Pass(pass_type);
		}

		re.ForceLineMode(force_line_mode_);

		CameraPtr const & camera = pvp.frame_buffer->GetViewport()->camera;
		pvp.shadowing_fb->GetViewport()->camera = camera;
		pvp.projective_shadowing_fb->GetViewport()->camera = camera;
		pvp.reflection_fb->GetViewport()->camera = camera;

		this->PreparePVP(pvp);

		float q = camera->FarPlane() / (camera->FarPlane() - camera->NearPlane());
		float4 near_q_far(camera->NearPlane() * q, q, camera->FarPlane(), 1 / camera->FarPlane());
		depth_to_linear_pp_->SetParam(0, near_q_far);

		*g_buffer_tex_param_ = pvp.g_buffer_rt0_tex;
		*depth_tex_param_ = pvp.g_buffer_depth_tex;
		*inv_width_height_param_ = float2(1.0f / pvp.frame_buffer->GetViewport()->width,
			1.0f / pvp.frame_buffer->GetViewport()->height);
		*shadowing_tex_param_ = pvp.shadowing_tex;
		*projective_shadowing_tex_param_ = pvp.projective_shadowing_tex;

		this->GenerateGBuffer(pvp, pass_tb);

		return App3DFramework::URV_NeedFlush | (App3DFramework::URV_OpaqueOnly << pass_tb);
	}

	uint32_t DeferredRenderingLayer::GBufferProcessingDRJob(PerViewport const & pvp)
	{
		auto& rf = Context::Instance().RenderFactoryInstance();
		auto& re = rf.RenderEngineInstance();

		re.ForceLineMode(false);

		this->PostGenerateGBuffer(pvp);

		return 0;
	}

	uint32_t DeferredRenderingLayer::OpaqueGBufferProcessingDRJob(PerViewport const & pvp)
	{
		if (indirect_lighting_enabled_ && !(pvp.attrib & VPAM_NoGI))
		{
			pvp.il_layer->UpdateGBuffer(*pvp.frame_buffer->GetViewport()->camera);
		}

		if (cascaded_shadow_index_ >= 0)
		{
			Camera const & scene_camera = *pvp.frame_buffer->GetViewport()->camera;
			Camera const & light_camera = *lights_[cascaded_shadow_index_]->SMCamera(0);

			checked_cast<DirectionalLightSource*>(lights_[cascaded_shadow_index_])->UpdateSMCamera(scene_camera);

			float const BLUR_FACTOR = 0.2f;
			blur_size_light_space_.x() = BLUR_FACTOR * 0.5f * light_camera.ProjMatrix()(0, 0);
			blur_size_light_space_.y() = BLUR_FACTOR * 0.5f * light_camera.ProjMatrix()(1, 1);

			float3 cascade_border(blur_size_light_space_.x(), blur_size_light_space_.y(), light_camera.ProjMatrix()(2, 2));
			cascaded_shadow_layer_->NumCascades(pvp.num_cascades);
			if (CSLT_SDSM == cascaded_shadow_layer_->Type())
			{
				checked_pointer_cast<SDSMCascadedShadowLayer>(cascaded_shadow_layer_)->DepthTexture(pvp.g_buffer_depth_tex);
			}
			cascaded_shadow_layer_->UpdateCascades(scene_camera, light_camera.ViewProjMatrix(), cascade_border);
		}

		if (!decals_.empty())
		{
			this->RenderDecals(pvp, PT_OpaqueGBufferMRT);
		}

		return 0;
	}

	uint32_t DeferredRenderingLayer::ShadowMapGenerationDRJob(PerViewport const & pvp, PassType pass_type, int32_t org_no, 
		int32_t index_in_pass)
	{
		auto& rf = Context::Instance().RenderFactoryInstance();
		auto& re = rf.RenderEngineInstance();
		auto& scene_mgr = Context::Instance().SceneManagerInstance();

		for (auto const & deo : visible_scene_objs_)
		{
			deo->Pass(pass_type);
		}

		auto const & light = *lights_[org_no];
		this->PrepareLightCamera(pvp, light, index_in_pass, pass_type);

		if (index_in_pass > 0)
		{
			this->PostGenerateShadowMap(pvp, org_no, index_in_pass);
		}

		uint32_t urv;
		if ((((LightSource::LT_Point == light.Type()) || (LightSource::LT_SphereArea == light.Type())
			|| (LightSource::LT_TubeArea == light.Type())) && (6 == index_in_pass))
			|| ((LightSource::LT_Spot == light.Type()) && (1 == index_in_pass))
			|| ((LightSource::LT_Directional == light.Type()) && (static_cast<int32_t>(pvp.num_cascades) == index_in_pass)))
		{
			curr_cascade_index_ = -1;
			urv = 0;
		}
		else
		{
			scene_mgr.SmallObjectThreshold(0.002f);

			PassRT const pass_rt = GetPassRT(pass_type);

			urv = App3DFramework::URV_NeedFlush | App3DFramework::URV_OpaqueOnly;
			switch (pass_rt)
			{
			case PRT_ShadowMap:
				re.BindFrameBuffer(sm_fb_);
				sm_fb_->Attached(FrameBuffer::ATT_Color0)->Discard();
				sm_fb_->Attached(FrameBuffer::ATT_DepthStencil)->ClearDepth(1.0f);
				break;

			case PRT_CascadedShadowMap:
				{
					CameraPtr const & light_camera = sm_fb_->GetViewport()->camera;
					csm_fb_->GetViewport()->camera = light_camera;
					re.BindFrameBuffer(csm_fb_);
					float const far_plane = light_camera->FarPlane();
					re.CurFrameBuffer()->Clear(FrameBuffer::CBM_Color | FrameBuffer::CBM_Depth, Color(far_plane, 0, 0, 0), 1.0f, 0);
				}
				break;

			default:
				BOOST_ASSERT(PRT_ReflectiveShadowMap == pass_rt);

				rsm_fb_->GetViewport()->camera = sm_fb_->GetViewport()->camera;
				re.BindFrameBuffer(rsm_fb_);
				rsm_fb_->Attached(FrameBuffer::ATT_Color0)->Discard();
				rsm_fb_->Attached(FrameBuffer::ATT_Color1)->Discard();
				rsm_fb_->Attached(FrameBuffer::ATT_DepthStencil)->ClearDepthStencil(1.0f, 0);
				break;
			}
		}

		return urv;
	}

	uint32_t DeferredRenderingLayer::IndirectLightingDRJob(PerViewport const & pvp, int32_t org_no)
	{
		depth_to_esm_pp_->Apply();
		pvp.il_layer->UpdateRSM(*rsm_fb_->GetViewport()->camera, *lights_[org_no]);
		return 0;
	}

	uint32_t DeferredRenderingLayer::ShadowingDRJob(PerViewport const & pvp, PassTargetBuffer pass_tb)
	{
#ifndef KLAYGE_SHIP
		shadowing_perfs_[pass_tb]->Begin();
#else
		KFL_UNUSED(pass_tb);
#endif

#if DEFAULT_DEFERRED == LIGHT_INDEXED_DEFERRED
		if (cs_cldr_ && typed_uav_)
		{
			this->UpdateShadowingCS(pvp);
		}
		else
		{
			this->UpdateShadowing(pvp);
		}
#else
		this->UpdateShadowing(pvp);
#endif

#ifndef KLAYGE_SHIP
		shadowing_perfs_[pass_tb]->End();
#endif

		return 0;
	}

	uint32_t DeferredRenderingLayer::ShadingDRJob(PerViewport const & pvp, PassType pass_type, int32_t index_in_pass)
	{
		auto const pass_tb = GetPassTargetBuffer(pass_type);

#ifndef KLAYGE_SHIP
		shading_perfs_[pass_tb]->Begin();
#endif

#if DEFAULT_DEFERRED == LIGHT_INDEXED_DEFERRED
		KFL_UNUSED(index_in_pass);

		if (cs_cldr_)
		{
			this->UpdateClusteredLighting(pvp, pass_tb);
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

#ifndef KLAYGE_SHIP
		shading_perfs_[pass_tb]->End();
#endif
		return 0;
	}

	uint32_t DeferredRenderingLayer::ReflectionDRJob(PerViewport const & pvp, PassType pass_type)
	{
		auto& rf = Context::Instance().RenderFactoryInstance();
		auto& re = rf.RenderEngineInstance();

		auto const pass_tb = GetPassTargetBuffer(pass_type);

		for (auto const & deo : visible_scene_objs_)
		{
			deo->Pass(pass_type);
		}

		re.BindFrameBuffer(pvp.reflection_fb);
		re.CurFrameBuffer()->Attached(FrameBuffer::ATT_DepthStencil)->ClearDepth(1.0f);
		return App3DFramework::URV_NeedFlush | App3DFramework::URV_ReflectionOnly | (App3DFramework::URV_OpaqueOnly << pass_tb);
	}

	uint32_t DeferredRenderingLayer::VDMDRJob(PerViewport const & pvp)
	{
		auto& rf = Context::Instance().RenderFactoryInstance();
		auto& re = rf.RenderEngineInstance();

		BOOST_ASSERT(has_vdm_objs_);

		for (auto const & deo : visible_scene_objs_)
		{
			if (deo->VDM())
			{
				deo->Pass(PT_VDM);
			}
		}

		this->CreateVDMDepthMaxMap(pvp);

		re.BindFrameBuffer(pvp.vdm_fb);
		re.CurFrameBuffer()->Clear(FrameBuffer::CBM_Color, Color(0, 0, 0, 0), 0, 0);

		return App3DFramework::URV_NeedFlush | App3DFramework::URV_VDMOnly;
	}

	uint32_t DeferredRenderingLayer::SpecialShadingDRJob(PerViewport& pvp, PassType pass_type)
	{
		auto& rf = Context::Instance().RenderFactoryInstance();
		auto& re = rf.RenderEngineInstance();

		auto const pass_tb = GetPassTargetBuffer(pass_type);
		
		for (auto const & deo : visible_scene_objs_)
		{
			deo->Pass(pass_type);
		}

		if (PTB_Opaque == pass_tb)
		{
			re.BindFrameBuffer(pvp.merged_shading_fbs[pvp.curr_merged_buffer_index]);
		}
		else
		{
			re.BindFrameBuffer(pvp.shading_fb);
		}
		
		return App3DFramework::URV_NeedFlush | App3DFramework::URV_SpecialShadingOnly | (App3DFramework::URV_OpaqueOnly << pass_tb);
	}

	uint32_t DeferredRenderingLayer::MergeShadingAndDepthDRJob(PerViewport& pvp, PassTargetBuffer pass_tb)
	{
		this->MergeShadingAndDepth(pvp, pass_tb);
		return 0;
	}

	uint32_t DeferredRenderingLayer::PostEffectsDRJob(PerViewport& pvp)
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

		if (has_vdm_objs_)
		{
#ifndef KLAYGE_SHIP
			vdm_composition_pp_perf_->Begin();
#endif
			this->AddVDM(pvp);
#ifndef KLAYGE_SHIP
			vdm_composition_pp_perf_->End();
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

		pvp.curr_merged_buffer_index = !pvp.curr_merged_buffer_index;

		return 0;
	}

	uint32_t DeferredRenderingLayer::SimpleForwardDRJob()
	{
		for (auto const & deo : visible_scene_objs_)
		{
			if (deo->SimpleForward())
			{
				deo->Pass(PT_SimpleForward);
			}
		}

		return App3DFramework::URV_NeedFlush | App3DFramework::URV_SimpleForwardOnly;
	}

	uint32_t DeferredRenderingLayer::FinishingDRJob()
	{
		return App3DFramework::URV_Finished;
	}

	uint32_t DeferredRenderingLayer::SwitchViewportDRJob(uint32_t vp_index)
	{
		active_viewport_ = vp_index;
		return 0;
	}

	uint32_t DeferredRenderingLayer::VisualizeGBufferDRJob()
	{
		auto& rf = Context::Instance().RenderFactoryInstance();
		auto& re = rf.RenderEngineInstance();

		re.BindFrameBuffer(FrameBufferPtr());
		re.CurFrameBuffer()->Attached(FrameBuffer::ATT_DepthStencil)->ClearDepth(1.0f);
		dr_debug_pp_->Apply();

		return App3DFramework::URV_SkipPostProcess | App3DFramework::URV_Finished;
	}

	uint32_t DeferredRenderingLayer::VisualizeLightingDRJob()
	{
		auto& rf = Context::Instance().RenderFactoryInstance();
		auto& re = rf.RenderEngineInstance();

		re.BindFrameBuffer(FrameBufferPtr());
		re.CurFrameBuffer()->Attached(FrameBuffer::ATT_DepthStencil)->ClearDepth(1.0f);
		dr_debug_pp_->Apply();

		return App3DFramework::URV_Finished | App3DFramework::URV_SkipPostProcess;
	}
}
