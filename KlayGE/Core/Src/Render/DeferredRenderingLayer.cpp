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
#include <KlayGE/DepthOfField.hpp>
#include <KlayGE/ResLoader.hpp>
#include <KlayGE/Renderable.hpp>
#include <KlayGE/RenderableHelper.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/RenderEffect.hpp>
#include <KlayGE/RenderView.hpp>
#include <KlayGE/FrameBuffer.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/SceneManager.hpp>
#include <KlayGE/SceneNode.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/App3D.hpp>
#include <KlayGE/PostProcess.hpp>
#include <KlayGE/Camera.hpp>
#include <KlayGE/Mesh.hpp>
#include <KlayGE/MotionBlur.hpp>
#include <KlayGE/PPRPostProcess.hpp>
#include <KlayGE/SSVOPostProcess.hpp>
#include <KlayGE/SSRPostProcess.hpp>
#include <KlayGE/SSSBlur.hpp>
#include <KlayGE/PerfProfiler.hpp>

#include <string>

#include <KlayGE/DeferredRenderingLayer.hpp>

namespace
{
	using namespace KlayGE;

	int const SHADOW_MAP_SIZE = 512;

	int const MAX_IL_MIPMAP_LEVELS = 3;

	int const MAX_RSM_MIPMAP_LEVELS = 7; // (log(512)-log(4))/log(2) + 1
	int const BEGIN_RSM_SAMPLING_LIGHT_LEVEL = 5;
	int const SAMPLE_LEVEL_CNT = MAX_RSM_MIPMAP_LEVELS - BEGIN_RSM_SAMPLING_LIGHT_LEVEL;
	int const VPL_COUNT = 64 * ((1UL << (SAMPLE_LEVEL_CNT * 2)) - 1) / (4 - 1);

	float const ESM_SCALE_FACTOR = 300.0f;

#if DEFAULT_DEFERRED == LIGHT_INDEXED_DEFERRED
	uint32_t const TILE_SIZE = 32;
#endif

	union EffectIndex
	{
#ifdef KLAYGE_HAS_STRUCT_PACK
#pragma pack(push, 1)
#endif
		struct Flags
		{
			uint8_t line : 1;
			uint8_t sss : 1;
			uint8_t two_sided : 1;
			uint8_t skinning : 1;
			uint8_t detail_mode : 2;
		} flags;
		static_assert(sizeof(Flags) == 1);
#ifdef KLAYGE_HAS_STRUCT_PACK
#pragma pack(pop)
#endif
		size_t index;
	};

	template <typename T>
	void CreateConeMesh(std::vector<T>& vb, std::vector<uint16_t>& ib, uint16_t vertex_base, float radius, float height, uint16_t n)
	{
		for (int i = 0; i < n; ++ i)
		{
			T& v = vb.emplace_back();
			v.x() = v.y() = v.z() = 0;
		}

		float outer_radius = radius / cos(PI / n);
		for (int i = 0; i < n; ++ i)
		{
			T& v = vb.emplace_back();
			float angle = i * 2 * PI / n;
			v.x() = outer_radius * cos(angle);
			v.y() = outer_radius * sin(angle);
			v.z() = height;
		}

		{
			T& v = vb.emplace_back();
			v.x() = v.y() = 0;
			v.z() = height;
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
			T& v = vb.emplace_back();
			v.x() = v.y() = v.z() = 0;
		}

		float outer_radius = radius * sqrt(2.0f);
		{
			T& v = vb.emplace_back();
			v.x() = -outer_radius;
			v.y() = -outer_radius;
			v.z() = height;
		}
		{
			T& v = vb.emplace_back();
			v.x() = +outer_radius;
			v.y() = -outer_radius;
			v.z() = height;
		}
		{
			T& v = vb.emplace_back();
			v.x() = +outer_radius;
			v.y() = +outer_radius;
			v.z() = height;
		}
		{
			T& v = vb.emplace_back();
			v.x() = -outer_radius;
			v.y() = +outer_radius;
			v.z() = height;
		}

		{
			T& v = vb.emplace_back();
			v.x() = v.y() = 0;
			v.z() = height;
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
			T& v = vb.emplace_back();
			v.x() = -half_length;
			v.y() = +half_length;
			v.z() = -half_length;
		}
		{
			T& v = vb.emplace_back();
			v.x() = +half_length;
			v.y() = +half_length;
			v.z() = -half_length;
		}
		{
			T& v = vb.emplace_back();
			v.x() = +half_length;
			v.y() = -half_length;
			v.z() = -half_length;
		}
		{
			T& v = vb.emplace_back();
			v.x() = -half_length;
			v.y() = -half_length;
			v.z() = -half_length;
		}

		{
			T& v = vb.emplace_back();
			v.x() = -half_length;
			v.y() = +half_length;
			v.z() = +half_length;
		}
		{
			T& v = vb.emplace_back();
			v.x() = +half_length;
			v.y() = +half_length;
			v.z() = +half_length;
		}
		{
			T& v = vb.emplace_back();
			v.x() = +half_length;
			v.y() = -half_length;
			v.z() = +half_length;
		}
		{
			T& v = vb.emplace_back();
			v.x() = -half_length;
			v.y() = -half_length;
			v.z() = +half_length;
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
			: PostProcess(L"DeferredRenderingDebug", false, MakeSpan<std::string>(),
				  MakeSpan<std::string>(
					  {"g_buffer_rt0_tex", "g_buffer_rt1_tex", "g_buffer_rt2_tex", "depth_tex", "lighting_tex", "ssvo_tex"}),
				  MakeSpan<std::string>({"out_tex"}), RenderEffectPtr(), nullptr)
		{
			auto effect = ASyncLoadRenderEffect("DeferredRenderingDebug.fxml");
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

			case DeferredRenderingLayer::DT_MotionVec:
				technique_ = effect_->TechniqueByName("ShowMotionVec");
				break;

			case DeferredRenderingLayer::DT_Occlusion:
				technique_ = effect_->TechniqueByName("ShowOcclusion");
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
}

namespace KlayGE
{
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

		tex_array_support_ = (caps.max_texture_array_length >= 6) && caps.render_to_texture_array_support;
		flexible_srvs_support_ = caps.flexible_srvs_support;

#if DEFAULT_DEFERRED == LIGHT_INDEXED_DEFERRED
		typed_uav_ = false;
		if ((caps.max_shader_model >= ShaderModel(5, 0)) && caps.cs_support)
		{
			static_assert(32 == TILE_SIZE, "TILE_SIZE must be 32.");

			cs_cldr_ = true;
#ifdef KLAYGE_PLATFORM_WINDOWS_STORE
			// Shaders are compiled to d3d11_0 for Windows store apps. No typed UAV support.
			typed_uav_ = false;
#else
			if (caps.UavFormatSupport(EF_ABGR16F) && caps.UavFormatSupport(EF_B10G11R11F) && caps.UavFormatSupport(EF_ABGR8))
			{
				typed_uav_ = true;
			}
#endif
		}
		else
		{
			cs_cldr_ = false;
		}
#endif

		ppr_enabled_ = cs_cldr_;

		for (size_t vpi = 0; vpi < viewports_.size(); ++ vpi)
		{
			PerViewport& pvp = viewports_[vpi];
			pvp.g_buffer_fb = rf.MakeFrameBuffer();
			pvp.g_buffer_resolved_fb = rf.MakeFrameBuffer();
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
				pvp.merged_depth_resolved_fbs[i] = rf.MakeFrameBuffer();
			}
#if DEFAULT_DEFERRED == LIGHT_INDEXED_DEFERRED
			if (!cs_cldr_)
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

#if DEFAULT_DEFERRED == TRIDITIONAL_DEFERRED
		dr_effect_ = ASyncLoadRenderEffect("DeferredRendering.fxml");
#elif DEFAULT_DEFERRED == LIGHT_INDEXED_DEFERRED
		if (cs_cldr_)
		{
			num_depth_slices_ = 4;
			depth_slices_.resize(num_depth_slices_ + 1);
			light_batch_ = 1024;
			dr_effect_ = ASyncLoadRenderEffect("ClusteredDeferredRendering.fxml");
		}
		else
		{
			light_batch_ = 32;
			dr_effect_ = ASyncLoadRenderEffect("LightIndexedDeferredRendering.fxml");
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
		technique_merge_shading_[0] = dr_effect->TechniqueByName("MergeShadingAlphaBlendTech");
		technique_merge_shading_[1] = dr_effect->TechniqueByName("MergeShadingAlphaBlendMSTech");
		technique_merge_depth_[0] = dr_effect->TechniqueByName("MergeDepthAlphaBlendTech");
		technique_merge_depth_[1] = dr_effect->TechniqueByName("MergeDepthAlphaBlendMSTech");
		technique_copy_shading_depth_ = dr_effect->TechniqueByName("CopyShadingDepthTech");
#if DEFAULT_DEFERRED == LIGHT_INDEXED_DEFERRED
		if (cs_cldr_)
		{
			technique_cldr_shadowing_unified_[0] = dr_effect->TechniqueByName("ClusteredDRShadowingUnified");
			technique_cldr_shadowing_unified_[1] = dr_effect->TechniqueByName("ClusteredDRShadowingUnifiedMS");
			technique_cldr_light_intersection_unified_ = dr_effect->TechniqueByName("ClusteredDRLightIntersection");
			technique_cldr_unified_[0] = dr_effect->TechniqueByName(typed_uav_ ? "ClusteredDRUnified" : "ClusteredDRUnifiedNoTypedUAV");
			technique_cldr_unified_[1] = dr_effect->TechniqueByName(typed_uav_ ? "ClusteredDRUnifiedMS" : "ClusteredDRUnifiedNoTypedUAVMS");

			technique_depth_to_tiled_min_max_[0] = dr_effect->TechniqueByName("DepthToTiledMinMax");
			technique_depth_to_tiled_min_max_[1] = dr_effect->TechniqueByName("DepthToTiledMinMaxMS");
			technique_resolve_g_buffers_ = dr_effect->TechniqueByName("ResolveGBuffers");
			technique_resolve_merged_depth_ = dr_effect->TechniqueByName("ResolveMergedDepth");
			technique_array_to_multiSample_ = dr_effect->TechniqueByName("ArrayToMultiSample");
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

		auto const shadow_map_fmt = caps.BestMatchTextureRenderTargetFormat(MakeSpan({EF_R32F, EF_R16F}), 1, 0);
		BOOST_ASSERT(shadow_map_fmt != EF_Unknown);

		shadow_map_fb_ = rf.MakeFrameBuffer();
		shadow_map_tex_ = rf.MakeTexture2D(SHADOW_MAP_SIZE, SHADOW_MAP_SIZE, 1, 1, shadow_map_fmt, 1, 0, EAH_GPU_Read | EAH_GPU_Write);
		KLAYGE_TEXTURE_DEBUG_NAME(shadow_map_tex_);
		shadow_map_rtv_ = rf.Make2DRtv(shadow_map_tex_, 0, 1, 0);
		shadow_map_fb_->Attach(FrameBuffer::Attachment::Color0, shadow_map_rtv_);
		shadow_map_depth_tex_ = rf.MakeTexture2D(SHADOW_MAP_SIZE, SHADOW_MAP_SIZE, 1, 1, EF_D24S8, 1, 0, EAH_GPU_Read | EAH_GPU_Write);
		KLAYGE_TEXTURE_DEBUG_NAME(shadow_map_depth_tex_);
		auto shadow_map_depth_dsv = rf.Make2DDsv(shadow_map_depth_tex_, 0, 1, 0);
		shadow_map_fb_->Attach(shadow_map_depth_dsv);
		shadow_map_depth_srv_ = rf.MakeTextureSrv(shadow_map_depth_tex_);

		if (tex_array_support_)
		{
			shadow_map_array_fb_ = rf.MakeFrameBuffer();
			shadow_map_array_fb_->Viewport()->NumCameras(6);
			shadow_map_array_tex_ =
				rf.MakeTexture2D(SHADOW_MAP_SIZE, SHADOW_MAP_SIZE, 1, 6, shadow_map_fmt, 1, 0, EAH_GPU_Read | EAH_GPU_Write);
			KLAYGE_TEXTURE_DEBUG_NAME(shadow_map_array_tex_);
			shadow_map_array_rtv_ = rf.Make2DRtv(shadow_map_array_tex_, 0, 6, 0);
			shadow_map_array_fb_->Attach(FrameBuffer::Attachment::Color0, shadow_map_array_rtv_);
			shadow_map_array_depth_tex_ =
				rf.MakeTexture2D(SHADOW_MAP_SIZE, SHADOW_MAP_SIZE, 1, 6, EF_D24S8, 1, 0, EAH_GPU_Read | EAH_GPU_Write);
			KLAYGE_TEXTURE_DEBUG_NAME(shadow_map_array_depth_tex_);
			auto shadow_map_array_depth_dsv = rf.Make2DDsv(shadow_map_array_depth_tex_, 0, 6, 0);
			shadow_map_array_fb_->Attach(shadow_map_array_depth_dsv);
			if (flexible_srvs_support_)
			{
				for (uint32_t i = 0; i < 6; ++i)
				{
					shadow_map_array_depth_srvs_[i] = rf.MakeTextureSrv(shadow_map_array_depth_tex_, i, 1, 0, 1);
				}
			}
		}

		csm_fb_ = rf.MakeFrameBuffer();
		csm_tex_ = rf.MakeTexture2D(SHADOW_MAP_SIZE * 2, SHADOW_MAP_SIZE * 2, 1, 1, shadow_map_fmt, 1, 0, EAH_GPU_Read | EAH_GPU_Write);
		KLAYGE_TEXTURE_DEBUG_NAME(csm_tex_);
		csm_fb_->Attach(FrameBuffer::Attachment::Color0, rf.Make2DRtv(csm_tex_, 0, 1, 0));
		csm_fb_->Attach(rf.Make2DDsv(SHADOW_MAP_SIZE * 2, SHADOW_MAP_SIZE * 2, EF_D24S8, 1, 0));

		for (size_t i = 0; i < std::size(unfiltered_shadow_map_2d_texs_); ++i)
		{
			unfiltered_shadow_map_2d_texs_[i] =
				rf.MakeTexture2D(SHADOW_MAP_SIZE, SHADOW_MAP_SIZE, 1, 1, shadow_map_tex_->Format(), 1, 0, EAH_GPU_Read | EAH_GPU_Write);
			KLAYGE_TEXTURE_DEBUG_NAME(unfiltered_shadow_map_2d_texs_[i]);
			unfiltered_shadow_map_2d_srvs_[i] = rf.MakeTextureSrv(unfiltered_shadow_map_2d_texs_[i]);
		}
		if (tex_array_support_)
		{
			filtered_shadow_map_2d_texs_[0] =
				rf.MakeTexture2D(SHADOW_MAP_SIZE, SHADOW_MAP_SIZE, 1, static_cast<uint32_t>(std::size(filtered_shadow_map_2d_texs_)),
					shadow_map_tex_->Format(), 1, 0, EAH_GPU_Read | EAH_GPU_Write);
			KLAYGE_TEXTURE_DEBUG_NAME(filtered_shadow_map_2d_texs_[0]);
			filtered_shadow_map_2d_srvs_[0] = rf.MakeTextureSrv(filtered_shadow_map_2d_texs_[0]);
			for (uint32_t slice = 0; slice < filtered_shadow_map_2d_texs_[0]->ArraySize(); ++slice)
			{
				filtered_shadow_map_2d_slice_rtvs_[slice] = rf.Make2DRtv(filtered_shadow_map_2d_texs_[0], slice, 1, 0);
			}
		}
		else
		{
			for (size_t i = 0; i < std::size(filtered_shadow_map_2d_texs_); ++i)
			{
				filtered_shadow_map_2d_texs_[i] =
					rf.MakeTexture2D(SHADOW_MAP_SIZE, SHADOW_MAP_SIZE, 1, 1, shadow_map_tex_->Format(), 1, 0, EAH_GPU_Read | EAH_GPU_Write);
				KLAYGE_TEXTURE_DEBUG_NAME(filtered_shadow_map_2d_texs_[i]);
				filtered_shadow_map_2d_srvs_[i] = rf.MakeTextureSrv(filtered_shadow_map_2d_texs_[i]);
				filtered_shadow_map_2d_slice_rtvs_[i] = rf.Make2DRtv(filtered_shadow_map_2d_texs_[i], 0, 1, 0);
			}
		}
		for (size_t i = 0; i < std::size(filtered_shadow_map_cube_texs_); ++i)
		{
			filtered_shadow_map_cube_texs_[i] =
				rf.MakeTextureCube(SHADOW_MAP_SIZE, 1, 1, shadow_map_tex_->Format(), 1, 0, EAH_GPU_Read | EAH_GPU_Write);
			KLAYGE_TEXTURE_DEBUG_NAME(filtered_shadow_map_cube_texs_[i]);
			filtered_shadow_map_cube_srvs_[i] = rf.MakeTextureSrv(filtered_shadow_map_cube_texs_[i]);
			for (uint32_t face = 0; face < 6; ++face)
			{
				filtered_shadow_map_cube_face_rtvs_[i * 6 + face] =
					rf.Make2DRtv(filtered_shadow_map_cube_texs_[i], 0, static_cast<Texture::CubeFaces>(face), 0);
			}
		}

		ssvo_pp_ = MakeSharedPtr<SSVOPostProcess>();
		auto effect_x = SyncLoadRenderEffect("SSVO.fxml");
		auto effect_y = effect_x->Clone();
		ssvo_blur_pp_ = MakeSharedPtr<BlurPostProcess<SeparableBilateralFilterPostProcess>>(3, 1.0f,
			effect_x, effect_x->TechniqueByName("SSVOBlurX"),
			effect_y, effect_y->TechniqueByName("SSVOBlurY"));
		ssvo_upsample_pp_ = SyncLoadPostProcess("SSVO.ppml", "SSVOUpsample");
		for (int i = 0; i < 2; ++ i)
		{
			ssr_pps_[i] = MakeSharedPtr<SSRPostProcess>(i != 0);

			if (cs_cldr_)
			{
				ppr_pps_[i] = MakeSharedPtr<PPRPostProcess>(i != 0);
			}

			sss_blur_pps_[i] = MakeSharedPtr<SSSBlurPP>(i != 0);
			sss_blur_pps_[i]->SetParam(0, 1.0f);
			sss_blur_pps_[i]->SetParam(1, 1.0f);

			translucency_pps_[i] = SyncLoadPostProcess("Translucency.ppml", (i == 0) ? "Translucency" : "TranslucencyMS");
			translucency_pps_[i]->SetParam(6, 100.0f);
		}
		vdm_composition_pp_ = SyncLoadPostProcess("VarianceDepthMap.ppml", "VDMComposition");
		taa_pp_ = SyncLoadPostProcess("TAA.ppml", "Taa");

		if (caps.fp_color_support && caps.TextureRenderTargetFormatSupport(EF_ABGR32F, 1, 0))
		{
			depth_of_field_pp_ = MakeSharedPtr<DepthOfField>();
			bokeh_filter_pp_ = MakeSharedPtr<BokehFilter>();
		}

		motion_blur_pp_ = MakeSharedPtr<MotionBlurPostProcess>();
		motion_blur_pp_->SetParam(3, static_cast<uint32_t>(MotionBlurPostProcess::VT_Result));

		rsm_fb_ = rf.MakeFrameBuffer();

		auto const fmt8 = caps.BestMatchTextureRenderTargetFormat(MakeSpan({EF_ABGR8, EF_ARGB8}), 1, 0);
		BOOST_ASSERT(fmt8 != EF_Unknown);
		rsm_texs_[0] = rf.MakeTexture2D(SHADOW_MAP_SIZE, SHADOW_MAP_SIZE, MAX_RSM_MIPMAP_LEVELS, 1, fmt8, 1, 0,
			EAH_GPU_Read | EAH_GPU_Write | EAH_Generate_Mips);
		KLAYGE_TEXTURE_DEBUG_NAME(rsm_texs_[0]);
		rsm_texs_[1] = rf.MakeTexture2D(SHADOW_MAP_SIZE, SHADOW_MAP_SIZE, MAX_RSM_MIPMAP_LEVELS, 1, fmt8, 1, 0,
			EAH_GPU_Read | EAH_GPU_Write | EAH_Generate_Mips);
		KLAYGE_TEXTURE_DEBUG_NAME(rsm_texs_[1]);
		rsm_fb_->Attach(FrameBuffer::Attachment::Color0, rf.Make2DRtv(rsm_texs_[0], 0, 1, 0)); // normal (light space)
		rsm_fb_->Attach(FrameBuffer::Attachment::Color1, rf.Make2DRtv(rsm_texs_[1], 0, 1, 0)); // albedo
		rsm_fb_->Attach(shadow_map_depth_dsv);
			
		copy_to_light_buffer_pp_ = SyncLoadPostProcess("Copy2LightBuffer.ppml", "CopyToLightBuffer");
		copy_to_light_buffer_i_pp_ = SyncLoadPostProcess("Copy2LightBuffer.ppml", "CopyToLightBufferI");

		shadow_map_filter_pp_ = MakeSharedPtr<LogGaussianBlurPostProcess>(4, true);
		shadow_map_filter_pp_->InputPin(0, rf.MakeTextureSrv(shadow_map_tex_));
		csm_filter_pp_ = MakeSharedPtr<LogGaussianBlurPostProcess>(4, true);
		csm_filter_pp_->InputPin(0, rf.MakeTextureSrv(csm_tex_));
		depth_to_esm_pp_ = SyncLoadPostProcess("Depth.ppml", "DepthToESM");
		depth_to_esm_pp_->OutputPin(0, shadow_map_rtv_);
		for (int i = 0; i < 2; ++ i)
		{
			depth_to_linear_pps_[i] = SyncLoadPostProcess("Depth.ppml", (i == 0) ? "DepthToLinear" : "DepthToLinearMS");
		}
		depth_mipmap_pp_ = SyncLoadPostProcess("Depth.ppml", "DepthMipmapBilinear");

		g_buffer_rt0_tex_param_ = dr_effect->ParameterByName("g_buffer_rt0_tex");
		g_buffer_rt1_tex_param_ = dr_effect->ParameterByName("g_buffer_rt1_tex");
		g_buffer_rt2_tex_param_ = dr_effect->ParameterByName("g_buffer_rt2_tex");
		depth_tex_param_ = dr_effect->ParameterByName("depth_tex");
		depth_tex_ms_param_ = dr_effect->ParameterByName("depth_tex_ms");
#if DEFAULT_DEFERRED == TRIDITIONAL_DEFERRED
		lighting_tex_param_ = dr_effect->ParameterByName("lighting_tex");
#endif
		shading_tex_param_ = dr_effect->ParameterByName("shading_tex");
		shading_tex_ms_param_ = dr_effect->ParameterByName("shading_tex_ms");
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
		filtered_shadow_map_2d_tex_param_ = dr_effect->ParameterByName("filtered_shadow_map_2d_tex");
		filtered_shadow_map_2d_tex_array_param_ = dr_effect->ParameterByName("filtered_shadow_map_2d_tex_array");
		filtered_shadow_map_2d_light_index_param_ = dr_effect->ParameterByName("filtered_shadow_map_2d_light_index");
		filtered_shadow_map_cube_tex_param_ = dr_effect->ParameterByName("filtered_shadow_map_cube_tex");
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
			g_buffer_rt0_tex_ms_param_ = dr_effect->ParameterByName("g_buffer_rt0_tex_ms");
			g_buffer_rt1_tex_ms_param_ = dr_effect->ParameterByName("g_buffer_rt1_tex_ms");
			g_buffer_rt2_tex_ms_param_ = dr_effect->ParameterByName("g_buffer_rt2_tex_ms");
			g_buffer_ds_tex_ms_param_ = dr_effect->ParameterByName("g_buffer_ds_tex_ms");
			g_buffer_depth_tex_ms_param_ = dr_effect->ParameterByName("g_buffer_depth_tex_ms");
			g_buffer_stencil_tex_param_ = (dr_effect_->ParameterByName("g_buffer_stencil_tex"));
			g_buffer_stencil_tex_ms_param_ = (dr_effect_->ParameterByName("g_buffer_stencil_tex_ms"));
			src_2d_tex_array_param_ = dr_effect->ParameterByName("src_2d_tex_array");

			near_q_far_param_ = dr_effect->ParameterByName("near_q_far");
			width_height_param_ = dr_effect->ParameterByName("width_height");
			depth_to_tiled_ds_in_tex_param_ = dr_effect->ParameterByName("ds_in_tex");
			depth_to_tiled_linear_depth_in_tex_ms_param_ = dr_effect->ParameterByName("linear_depth_in_tex_ms");
			depth_to_tiled_min_max_depth_rw_tex_param_ = dr_effect->ParameterByName("min_max_depth_rw_tex");
			linear_depth_rw_tex_param_ = dr_effect->ParameterByName("linear_depth_rw_tex");
			upper_left_param_ = dr_effect->ParameterByName("upper_left");
			x_dir_param_ = dr_effect->ParameterByName("x_dir");
			y_dir_param_ = dr_effect->ParameterByName("y_dir");
			multi_sample_mask_tex_param_ = dr_effect->ParameterByName("multi_sample_mask_tex");
			shading_in_tex_param_ = dr_effect->ParameterByName("shading_in_tex");
			shading_in_tex_ms_param_ = dr_effect->ParameterByName("shading_in_tex_ms");
			shading_rw_tex_param_ = dr_effect->ParameterByName("shading_rw_tex");
			shading_rw_tex_array_param_ = dr_effect->ParameterByName("shading_rw_tex_array");
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
			filtered_shadow_maps_2d_light_index_param_ = dr_effect->ParameterByName("filtered_shadow_maps_2d_light_index");
			esms_scale_factor_param_ = dr_effect->ParameterByName("esms_scale_factor");

			for (int i = 0; i < 2; ++ i)
			{
				copy_pps_[i] = SyncLoadPostProcess("Copy.ppml", (i == 0) ? "Copy" : "CopyMS");
			}
		}
		else
		{
			light_index_tex_param_ = dr_effect->ParameterByName("light_index_tex");
		}

		depth_to_min_max_pp_ = SyncLoadPostProcess("Depth.ppml", "DepthToMinMax");
		reduce_min_max_pp_ = SyncLoadPostProcess("Depth.ppml", "ReduceMinMax");
#endif
		for (int i = 0; i < 2; ++ i)
		{
			depth_to_max_pps_[i] = SyncLoadPostProcess("Depth.ppml", (i == 0) ? "DepthToMax" : "DepthToMaxMS");
		}

		this->SetCascadedShadowType(CSLT_Auto);

#ifndef KLAYGE_SHIP
		PerfProfiler& profiler = PerfProfiler::Instance();
		shadow_map_perf_ = profiler.CreatePerfRegion(0, "Gen shadow map");
		std::string buffer_name[] = { "Opaque", "Transparency back", "TransparencyFront" };
		for (uint32_t i = PTB_Opaque; i < PTB_None; ++ i)
		{
			gbuffer_perfs_[i] = profiler.CreatePerfRegion(0, "GBuffer (" + buffer_name[i] + ")");
			shadowing_perfs_[i] = profiler.CreatePerfRegion(0, "Shadowing (" + buffer_name[i] + ")");
			indirect_lighting_perfs_[i] = profiler.CreatePerfRegion(0, "Indirect lighting (" + buffer_name[i] + ")");
#if DEFAULT_DEFERRED == LIGHT_INDEXED_DEFERRED
			clustering_perfs_[i] = profiler.CreatePerfRegion(0, "Clustering (" + buffer_name[i] + ")");
#endif
			shading_perfs_[i] = profiler.CreatePerfRegion(0, "Shading (" + buffer_name[i] + ")");
			reflection_perfs_[i] = profiler.CreatePerfRegion(0, "Reflection (" + buffer_name[i] + ")");
			special_shading_perfs_[i] = profiler.CreatePerfRegion(0, "Special shading (" + buffer_name[i] + ")");
		}
		sss_blur_pp_perf_ = profiler.CreatePerfRegion(0, "SSS Blur PP");
		ssr_pp_perf_ = profiler.CreatePerfRegion(0, "SSR PP");
		if (cs_cldr_)
		{
			ppr_pp_perf_ = profiler.CreatePerfRegion(0, "PPR PP");
		}
		atmospheric_pp_perf_ = profiler.CreatePerfRegion(0, "Atmospheric PP");
		taa_pp_perf_ = profiler.CreatePerfRegion(0, "TAA PP");
		vdm_perf_ = profiler.CreatePerfRegion(0, "VDM");
		vdm_composition_pp_perf_ = profiler.CreatePerfRegion(0, "VDM composition PP");
		depth_of_field_perf_ = profiler.CreatePerfRegion(0, "Depth of Field PP");
		bokeh_filter_perf_ = profiler.CreatePerfRegion(0, "Bokeh Filter PP");
#endif
	}

	void DeferredRenderingLayer::Register()
	{
		Context::Instance().AppInstance().OnConfirmDevice().Connect(DeferredRenderingLayer::ConfirmDevice);
	}

	bool DeferredRenderingLayer::ConfirmDevice()
	{
		RenderDeviceCaps const & caps = Context::Instance().RenderFactoryInstance().RenderEngineInstance().DeviceCaps();
		if ((caps.max_simultaneous_rts < 2)
			|| !caps.depth_texture_support
			|| !caps.fp_color_support
			|| caps.pack_to_rgba_required
			|| !caps.TextureRenderTargetFormatSupport(EF_D24S8, 1, 0))
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

	void DeferredRenderingLayer::SSSEnabled(bool sss)
	{
		sss_enabled_ = sss;
	}

	void DeferredRenderingLayer::SSSStrength(float strength)
	{
		for (int i = 0; i < 2; ++ i)
		{
			sss_blur_pps_[i]->SetParam(0, strength);
		}
	}

	void DeferredRenderingLayer::SSSCorrection(float correction)
	{
		for (int i = 0; i < 2; ++ i)
		{
			sss_blur_pps_[i]->SetParam(1, correction);
		}
	}

	void DeferredRenderingLayer::TranslucencyEnabled(bool trans)
	{
		translucency_enabled_ = trans;
	}

	void DeferredRenderingLayer::TranslucencyStrength(float strength)
	{
		for (int i = 0; i < 2; ++ i)
		{
			translucency_pps_[i]->SetParam(6, strength);
		}
	}

	void DeferredRenderingLayer::SSREnabled(bool ssr)
	{
		ssr_enabled_ = ssr;
	}

	void DeferredRenderingLayer::PPREnabled(bool ppr)
	{
		if (cs_cldr_)
		{
			ppr_enabled_ = ppr;
		}
		else
		{
			ppr_enabled_ = false;
		}
	}

	void DeferredRenderingLayer::PPRPlane(Plane const& plane)
	{
		for (int i = 0; i < 2; ++i)
		{
			if (ppr_pps_[i])
			{
				ppr_pps_[i]->SetParam(0, reinterpret_cast<float4 const&>(plane));
			}
		}
	}

	void DeferredRenderingLayer::TemporalAAEnabled(bool taa)
	{
		taa_enabled_ = taa;
	}

	void DeferredRenderingLayer::DepthOfFieldEnabled(bool dof, bool bokeh)
	{
		if (depth_of_field_pp_)
		{
			depth_of_field_enabled_ = dof;
		}
		if (bokeh_filter_pp_)
		{
			bokeh_filter_enabled_ = dof && bokeh;
		}
	}

	void DeferredRenderingLayer::DepthFocus(float plane, float range)
	{
		if (depth_of_field_pp_)
		{
			auto& dof_pp = checked_cast<DepthOfField&>(*depth_of_field_pp_);
			dof_pp.FocusPlane(plane);
			dof_pp.FocusRange(range);
		}
		if (bokeh_filter_pp_)
		{
			auto& bokeh_pp = checked_cast<BokehFilter&>(*bokeh_filter_pp_);
			bokeh_pp.FocusPlane(plane);
			bokeh_pp.FocusRange(range);
		}
	}

	void DeferredRenderingLayer::BokehLuminanceThreshold(float lum_threshold)
	{
		if (bokeh_filter_pp_)
		{
			auto& bokeh_pp = checked_cast<BokehFilter&>(*bokeh_filter_pp_);
			bokeh_pp.LuminanceThreshold(lum_threshold);
		}
	}

	void DeferredRenderingLayer::MotionBlurEnabled(bool mb)
	{
		if (motion_blur_pp_)
		{
			motion_blur_enabled_ = mb;
		}
	}

	void DeferredRenderingLayer::MotionBlurExposure(float exposure)
	{
		if (motion_blur_pp_)
		{
			auto& mb_pp = checked_cast<MotionBlurPostProcess&>(*motion_blur_pp_);
			mb_pp.Exposure(exposure);
		}
	}

	float DeferredRenderingLayer::MotionBlurExposure() const
	{
		float exposure = 0;
		if (motion_blur_pp_)
		{
			auto& mb_pp = checked_cast<MotionBlurPostProcess&>(*motion_blur_pp_);
			exposure = mb_pp.Exposure();
		}
		return exposure;
	}

	void DeferredRenderingLayer::MotionBlurRadius(uint32_t blur_radius)
	{
		if (motion_blur_pp_)
		{
			auto& mb_pp = checked_cast<MotionBlurPostProcess&>(*motion_blur_pp_);
			mb_pp.BlurRadius(blur_radius);
		}
	}

	uint32_t DeferredRenderingLayer::MotionBlurRadius() const
	{
		uint32_t blur_radius = 0;
		if (motion_blur_pp_)
		{
			auto& mb_pp = checked_cast<MotionBlurPostProcess&>(*motion_blur_pp_);
			blur_radius = mb_pp.BlurRadius();
		}
		return blur_radius;
	}

	void DeferredRenderingLayer::MotionBlurReconstructionSamples(uint32_t reconstruction_samples)
	{
		if (motion_blur_pp_)
		{
			auto& mb_pp = checked_cast<MotionBlurPostProcess&>(*motion_blur_pp_);
			mb_pp.ReconstructionSamples(reconstruction_samples);
		}
	}

	void DeferredRenderingLayer::AddDecal(RenderDecalPtr const & decal)
	{
		decals_.push_back(decal);
	}

	void DeferredRenderingLayer::SetupViewport(uint32_t index, FrameBufferPtr const & fb, uint32_t attrib)
	{
		this->SetupViewport(index, fb, attrib, 1, 0);
	}

	void DeferredRenderingLayer::SetupViewport(uint32_t index, FrameBufferPtr const & fb, uint32_t attrib,
		uint32_t sample_count, uint32_t sample_quality)
	{
#if DEFAULT_DEFERRED == LIGHT_INDEXED_DEFERRED
		if (!cs_cldr_)
		{
			// TODO: Supports MSAA in LIDR
			sample_count = 1;
			sample_quality = 0;
		}
#elif DEFAULT_DEFERRED == TRIDITIONAL_DEFERRED
		// Triditional deferred rendering doesn't suport MSAA
		sample_count = 1;
		sample_quality = 0;
#endif

		if (!cs_cldr_)
		{
			attrib |= VPAM_NoPPR;
		}

		PerViewport& pvp = viewports_[index];
		pvp.attrib = attrib;
		pvp.frame_buffer = fb;
		pvp.sample_count = sample_count;
		pvp.sample_quality = sample_quality;

		if (fb)
		{
			pvp.attrib |= VPAM_Enabled;

			BOOST_ASSERT(fb->AttachedRtv(FrameBuffer::Attachment::Color0)->SampleCount() == 1);
		}

		uint32_t const width = pvp.frame_buffer->Viewport()->Width();
		uint32_t const height = pvp.frame_buffer->Viewport()->Height();

		RenderFactory& rf = Context::Instance().RenderFactoryInstance();
		RenderDeviceCaps const & caps = rf.RenderEngineInstance().DeviceCaps();

		auto const fmt8 = caps.BestMatchTextureRenderTargetFormat(MakeSpan({EF_ABGR8, EF_ARGB8}), 1, 0);
		BOOST_ASSERT(fmt8 != EF_Unknown);
		auto const depth_fmt = caps.BestMatchTextureRenderTargetFormat(MakeSpan({EF_R16F, EF_R32F}), 1, 0);
		BOOST_ASSERT(depth_fmt != EF_Unknown);

		pvp.g_buffer_ds_tex = rf.MakeTexture2D(width, height, 1, 1, EF_D24S8, sample_count, sample_quality, EAH_GPU_Read | EAH_GPU_Write);
		KLAYGE_TEXTURE_DEBUG_NAME(pvp.g_buffer_ds_tex);
		pvp.g_buffer_ds_srv = rf.MakeTextureSrv(pvp.g_buffer_ds_tex);
#if DEFAULT_DEFERRED == LIGHT_INDEXED_DEFERRED
		if (cs_cldr_)
		{
			pvp.g_buffer_stencil_srv = rf.MakeTextureSrv(pvp.g_buffer_ds_tex, EF_X24G8, 0, 1, 0, 1);
		}
#endif
		auto ds_view = rf.Make2DDsv(pvp.g_buffer_ds_tex, 0, 1, 0);

		pvp.g_buffer_resolved_rt0_tex = rf.MakeTexture2D(width, height, MAX_IL_MIPMAP_LEVELS + 1, 1, fmt8, 1, 0,
			EAH_GPU_Read | EAH_GPU_Write | EAH_Generate_Mips);
		KLAYGE_TEXTURE_DEBUG_NAME(pvp.g_buffer_resolved_rt0_tex);
		pvp.g_buffer_resolved_rt0_srv = rf.MakeTextureSrv(pvp.g_buffer_resolved_rt0_tex);
		pvp.g_buffer_resolved_rt1_tex = rf.MakeTexture2D(width, height, 1, 1, fmt8, 1, 0,
			EAH_GPU_Read | EAH_GPU_Write);
		KLAYGE_TEXTURE_DEBUG_NAME(pvp.g_buffer_resolved_rt1_tex);
		pvp.g_buffer_resolved_rt1_srv = rf.MakeTextureSrv(pvp.g_buffer_resolved_rt1_tex);
		pvp.g_buffer_resolved_rt2_tex = rf.MakeTexture2D(width, height, 1, 1, fmt8, 1, 0,
			EAH_GPU_Read | EAH_GPU_Write);
		KLAYGE_TEXTURE_DEBUG_NAME(pvp.g_buffer_resolved_rt2_tex);
		pvp.g_buffer_resolved_rt2_srv = rf.MakeTextureSrv(pvp.g_buffer_resolved_rt2_tex);
		if (sample_count == 1)
		{
			pvp.g_buffer_rt0_tex = pvp.g_buffer_resolved_rt0_tex;
			pvp.g_buffer_rt0_srv = pvp.g_buffer_resolved_rt0_srv;
			pvp.g_buffer_rt1_tex = pvp.g_buffer_resolved_rt1_tex;
			pvp.g_buffer_rt1_srv = pvp.g_buffer_resolved_rt1_srv;
			pvp.g_buffer_rt2_tex = pvp.g_buffer_resolved_rt2_tex;
			pvp.g_buffer_rt2_srv = pvp.g_buffer_resolved_rt2_srv;
		}
		else
		{
			pvp.g_buffer_rt0_tex = rf.MakeTexture2D(width, height, 1, 1, fmt8, sample_count, sample_quality,
				EAH_GPU_Read | EAH_GPU_Write);
			KLAYGE_TEXTURE_DEBUG_NAME(pvp.g_buffer_rt0_tex);
			pvp.g_buffer_rt0_srv = rf.MakeTextureSrv(pvp.g_buffer_rt0_tex);
			pvp.g_buffer_rt1_tex = rf.MakeTexture2D(width, height, 1, 1, fmt8, sample_count, sample_quality,
				EAH_GPU_Read | EAH_GPU_Write);
			KLAYGE_TEXTURE_DEBUG_NAME(pvp.g_buffer_rt1_tex);
			pvp.g_buffer_rt1_srv = rf.MakeTextureSrv(pvp.g_buffer_rt1_tex);
			pvp.g_buffer_rt2_tex = rf.MakeTexture2D(width, height, 1, 1, fmt8, sample_count, sample_quality,
				EAH_GPU_Read | EAH_GPU_Write);
			KLAYGE_TEXTURE_DEBUG_NAME(pvp.g_buffer_rt2_tex);
			pvp.g_buffer_rt2_srv = rf.MakeTextureSrv(pvp.g_buffer_rt2_tex);
		}

		uint32_t hint = EAH_GPU_Read | EAH_GPU_Write | EAH_Generate_Mips;
#if DEFAULT_DEFERRED == LIGHT_INDEXED_DEFERRED
		if (cs_cldr_)
		{
			hint |= EAH_GPU_Unordered;
		}
#endif
		pvp.g_buffer_resolved_depth_tex = rf.MakeTexture2D(width, height, MAX_IL_MIPMAP_LEVELS + 1, 1, depth_fmt, 1, 0, hint);
		KLAYGE_TEXTURE_DEBUG_NAME(pvp.g_buffer_resolved_depth_tex);
		pvp.g_buffer_resolved_depth_srv = rf.MakeTextureSrv(pvp.g_buffer_resolved_depth_tex);
		pvp.g_buffer_resolved_depth_rtv = rf.Make2DRtv(pvp.g_buffer_resolved_depth_tex, 0, 1, 0);
		if (sample_count == 1)
		{
			pvp.g_buffer_depth_tex = pvp.g_buffer_resolved_depth_tex;
			pvp.g_buffer_depth_srv = pvp.g_buffer_resolved_depth_srv;
			pvp.g_buffer_depth_rtv = pvp.g_buffer_resolved_depth_rtv;
		}
		else
		{
			pvp.g_buffer_depth_tex = rf.MakeTexture2D(width, height, 1, 1, depth_fmt, sample_count, sample_quality,
				EAH_GPU_Read | EAH_GPU_Write);
			KLAYGE_TEXTURE_DEBUG_NAME(pvp.g_buffer_depth_tex);
			pvp.g_buffer_depth_srv = rf.MakeTextureSrv(pvp.g_buffer_depth_tex);
			pvp.g_buffer_depth_rtv = rf.Make2DRtv(pvp.g_buffer_depth_tex, 0, 1, 0);
		}
		pvp.g_buffer_rt0_backup_tex = rf.MakeTexture2D(width, height, 1, 1, fmt8, 1, 0, EAH_GPU_Read);
		KLAYGE_TEXTURE_DEBUG_NAME(pvp.g_buffer_rt0_backup_tex);
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
				KLAYGE_TEXTURE_DEBUG_NAME(pvp.g_buffer_min_max_depth_texs.back());
			}
			else
			{
				uint32_t w = std::max(1U, (width + 1) / 2);
				uint32_t h = std::max(1U, (height + 1) / 2);
				for (uint32_t ts = TILE_SIZE; ts > 1; ts /= 2)
				{
					pvp.g_buffer_min_max_depth_texs.push_back(rf.MakeTexture2D(w, h, 1, 1, min_max_depth_fmt, 1, 0,
						EAH_GPU_Read | EAH_GPU_Write));
					KLAYGE_TEXTURE_DEBUG_NAME(pvp.g_buffer_min_max_depth_texs.back());
					w = std::max(1U, (w + 1) / 2);
					h = std::max(1U, (h + 1) / 2);
				}

				pvp.g_buffer_min_max_depth_rtvs.clear();
				pvp.g_buffer_min_max_depth_rtvs.reserve(pvp.g_buffer_min_max_depth_texs.size());
				for (auto const& tex : pvp.g_buffer_min_max_depth_texs)
				{
					pvp.g_buffer_min_max_depth_rtvs.push_back(rf.Make2DRtv(tex, 0, 1, 0));
				}
			}

			pvp.g_buffer_min_max_depth_srvs.clear();
			pvp.g_buffer_min_max_depth_srvs.reserve(pvp.g_buffer_min_max_depth_texs.size());
			for (auto const& tex : pvp.g_buffer_min_max_depth_texs)
			{
				pvp.g_buffer_min_max_depth_srvs.push_back(rf.MakeTextureSrv(tex));
			}
		}
#endif
		pvp.g_buffer_vdm_max_ds_texs.clear();
		pvp.g_buffer_vdm_max_ds_srvs.clear();
		pvp.g_buffer_vdm_max_ds_dsvs.clear();
		{
			uint32_t w = std::max(1U, width / 2);
			uint32_t h = std::max(1U, height / 2);
			for (uint32_t i = 0; i < 2; ++ i)
			{
				TexturePtr tex = rf.MakeTexture2D(w, h, 1, 1, EF_D24S8, 1, 0, EAH_GPU_Read | EAH_GPU_Write);
				pvp.g_buffer_vdm_max_ds_texs.push_back(tex);
				KLAYGE_TEXTURE_DEBUG_NAME(pvp.g_buffer_vdm_max_ds_texs.back());
				pvp.g_buffer_vdm_max_ds_srvs.push_back(rf.MakeTextureSrv(tex));
				pvp.g_buffer_vdm_max_ds_dsvs.push_back(rf.Make2DDsv(tex, 0, 1, 0));
				w = std::max(1U, w / 2);
				h = std::max(1U, h / 2);
			}
		}

		pvp.g_buffer_fb->Attach(FrameBuffer::Attachment::Color0, rf.Make2DRtv(pvp.g_buffer_rt0_tex, 0, 1, 0));
		pvp.g_buffer_fb->Attach(FrameBuffer::Attachment::Color1, rf.Make2DRtv(pvp.g_buffer_rt1_tex, 0, 1, 0));
		pvp.g_buffer_fb->Attach(FrameBuffer::Attachment::Color2, rf.Make2DRtv(pvp.g_buffer_rt2_tex, 0, 1, 0));
		pvp.g_buffer_fb->Attach(ds_view);

		this->SetupViewportGI(index, false);

		auto fmt = caps.BestMatchTextureRenderTargetFormat(MakeSpan({EF_R32F, EF_R16F}), 1, 0);
		BOOST_ASSERT(fmt != EF_Unknown);
		if (tex_array_support_)
		{
			pvp.filtered_csm_texs[0] = rf.MakeTexture2D(SHADOW_MAP_SIZE * 2, SHADOW_MAP_SIZE * 2, 3,
				CascadedShadowLayer::MAX_NUM_CASCADES, fmt, 1, 0, EAH_GPU_Read | EAH_GPU_Write | EAH_Generate_Mips);
			KLAYGE_TEXTURE_DEBUG_NAME(pvp.filtered_csm_texs[0]);
			pvp.filtered_csm_srvs[0] = rf.MakeTextureSrv(pvp.filtered_csm_texs[0]);
			for (uint32_t slice = 0; slice < pvp.filtered_csm_texs[0]->ArraySize(); ++slice)
			{
				pvp.filtered_csm_slice_rtvs[slice] = rf.Make2DRtv(pvp.filtered_csm_texs[0], slice, 1, 0);
			}
		}
		else
		{
			for (size_t i = 0; i < pvp.filtered_csm_texs.size(); ++ i)
			{
				pvp.filtered_csm_texs[i] = rf.MakeTexture2D(SHADOW_MAP_SIZE * 2, SHADOW_MAP_SIZE * 2, 3, 1, fmt, 1, 0,
					EAH_GPU_Read | EAH_GPU_Write | EAH_Generate_Mips);
				KLAYGE_TEXTURE_DEBUG_NAME(pvp.filtered_csm_texs[i]);
				pvp.filtered_csm_srvs[i] = rf.MakeTextureSrv(pvp.filtered_csm_texs[i]);
				pvp.filtered_csm_slice_rtvs[i] = rf.Make2DRtv(pvp.filtered_csm_texs[i], 0, 1, 0);
			}
		}

		fmt = caps.BestMatchTextureRenderTargetFormat(MakeSpan({EF_ABGR8, EF_ARGB8}), 1, 0);
		BOOST_ASSERT(fmt != EF_Unknown);
		hint = EAH_GPU_Read | EAH_GPU_Write;
#if DEFAULT_DEFERRED == LIGHT_INDEXED_DEFERRED
		if (cs_cldr_)
		{
			hint |= EAH_GPU_Unordered;
		}
#endif
		pvp.shadowing_tex = rf.MakeTexture2D(width / 2, height / 2, 1, 1, fmt, 1, 0, hint);
		KLAYGE_TEXTURE_DEBUG_NAME(pvp.shadowing_tex);
		pvp.shadowing_fb->Attach(FrameBuffer::Attachment::Color0, rf.Make2DRtv(pvp.shadowing_tex, 0, 1, 0));

		fmt = caps.BestMatchTextureRenderTargetFormat(MakeSpan({EF_B10G11R11F, EF_ABGR8_SRGB, EF_ARGB8_SRGB, EF_ABGR8, EF_ARGB8}), 1, 0);
		BOOST_ASSERT(fmt != EF_Unknown);
		pvp.projective_shadowing_tex = rf.MakeTexture2D(width / 2, height / 2, 1, 1, fmt, 1, 0, hint);
		KLAYGE_TEXTURE_DEBUG_NAME(pvp.projective_shadowing_tex);
		pvp.projective_shadowing_fb->Attach(FrameBuffer::Attachment::Color0, rf.Make2DRtv(pvp.projective_shadowing_tex, 0, 1, 0));

		pvp.reflection_tex = rf.MakeTexture2D(width / 2, height / 2, 1, 1, fmt, 1, 0, EAH_GPU_Read | EAH_GPU_Write);
		KLAYGE_TEXTURE_DEBUG_NAME(pvp.reflection_tex);
		pvp.reflection_fb->Attach(FrameBuffer::Attachment::Color0, rf.Make2DRtv(pvp.reflection_tex, 0, 1, 0));
		pvp.reflection_fb->Attach(rf.Make2DDsv(pvp.reflection_tex->Width(0), pvp.reflection_tex->Height(0), ds_view->Format(), 1, 0));

		BOOST_ASSERT(caps.TextureRenderTargetFormatSupport(EF_ABGR16F, 1, 0));
		ElementFormat shading_fmt = EF_ABGR16F;

#if DEFAULT_DEFERRED == TRIDITIONAL_DEFERRED
		pvp.lighting_tex = rf.MakeTexture2D(width, height, 1, 1, shading_fmt, 1, 0, EAH_GPU_Read | EAH_GPU_Write);
		KLAYGE_TEXTURE_DEBUG_NAME(pvp.lighting_tex);
		pvp.lighting_srv = rf.MakeTextureSrv(pvp.lighting_tex);
		pvp.lighting_fb->Attach(FrameBuffer::Attachment::Color0, rf.Make2DRtv(pvp.lighting_tex, 0, 1, 0));
		pvp.lighting_fb->Attach(ds_view);
#endif

		uint32_t const vdm_width = std::max(1U, width / 4);
		uint32_t const vdm_height = std::max(1U, height / 4);
		pvp.vdm_color_tex = rf.MakeTexture2D(vdm_width, vdm_height, 1, 1, shading_fmt, 1, 0, EAH_GPU_Read | EAH_GPU_Write);
		KLAYGE_TEXTURE_DEBUG_NAME(pvp.vdm_color_tex);
		pvp.vdm_color_srv = rf.MakeTextureSrv(pvp.vdm_color_tex);
		pvp.vdm_color_rtv = rf.Make2DRtv(pvp.vdm_color_tex, 0, 1, 0);
		pvp.vdm_transition_tex = rf.MakeTexture2D(vdm_width, vdm_height, 1, 1, EF_GR16F, 1, 0, EAH_GPU_Read | EAH_GPU_Write);
		KLAYGE_TEXTURE_DEBUG_NAME(pvp.vdm_transition_tex);
		pvp.vdm_transition_srv = rf.MakeTextureSrv(pvp.vdm_transition_tex);
		pvp.vdm_count_tex = rf.MakeTexture2D(vdm_width, vdm_height, 1, 1, EF_GR16F, 1, 0, EAH_GPU_Read | EAH_GPU_Write);
		KLAYGE_TEXTURE_DEBUG_NAME(pvp.vdm_count_tex);
		pvp.vdm_count_srv = rf.MakeTextureSrv(pvp.vdm_count_tex);
		pvp.vdm_fb->Attach(FrameBuffer::Attachment::Color0, pvp.vdm_color_rtv);
		pvp.vdm_fb->Attach(FrameBuffer::Attachment::Color1, rf.Make2DRtv(pvp.vdm_transition_tex, 0, 1, 0));
		pvp.vdm_fb->Attach(FrameBuffer::Attachment::Color2, rf.Make2DRtv(pvp.vdm_count_tex, 0, 1, 0));
		pvp.vdm_fb->Attach(pvp.g_buffer_vdm_max_ds_dsvs[1]);

		fmt = caps.BestMatchTextureRenderTargetFormat(MakeSpan({EF_B10G11R11F, EF_ABGR16F}), 1, 0);
		BOOST_ASSERT(fmt != EF_Unknown);
		hint = EAH_GPU_Read | EAH_GPU_Write;
#if DEFAULT_DEFERRED == LIGHT_INDEXED_DEFERRED
		if (cs_cldr_)
		{
			hint |= EAH_GPU_Unordered;
		}
#endif
		pvp.shading_tex = rf.MakeTexture2D(width, height, 1, 1, shading_fmt, sample_count, sample_quality,
			(sample_count == 1) ? hint : (EAH_GPU_Read | EAH_GPU_Write));
		KLAYGE_TEXTURE_DEBUG_NAME(pvp.shading_tex);
		pvp.shading_srv = rf.MakeTextureSrv(pvp.shading_tex);
		pvp.shading_rtv = rf.Make2DRtv(pvp.shading_tex, 0, 1, 0);
		for (size_t i = 0; i < pvp.merged_shading_texs.size(); ++ i)
		{
			pvp.merged_shading_texs[i] = rf.MakeTexture2D(width, height, 1, 1, fmt, sample_count, sample_quality,
				(sample_count == 1) ? hint : (EAH_GPU_Read | EAH_GPU_Write));
			KLAYGE_TEXTURE_DEBUG_NAME(pvp.merged_shading_texs[i]);
			pvp.merged_shading_srvs[i] = rf.MakeTextureSrv(pvp.merged_shading_texs[i]);
			pvp.merged_shading_rtvs[i] = rf.Make2DRtv(pvp.merged_shading_texs[i], 0, 1, 0);
			pvp.merged_depth_texs[i] = rf.MakeTexture2D(width, height, 1, 1, depth_fmt, sample_count, sample_quality,
				EAH_GPU_Read | EAH_GPU_Write);
			KLAYGE_TEXTURE_DEBUG_NAME(pvp.merged_depth_texs[i]);
			pvp.merged_depth_srvs[i] = rf.MakeTextureSrv(pvp.merged_depth_texs[i]);
			pvp.merged_depth_rtvs[i] = rf.Make2DRtv(pvp.merged_depth_texs[i], 0, 1, 0);
		}
		if (sample_count == 1)
		{
			for (size_t i = 0; i < pvp.merged_shading_resolved_texs.size(); ++ i)
			{
				pvp.merged_shading_resolved_texs[i] = pvp.merged_shading_texs[i];
				pvp.merged_shading_resolved_srvs[i] = pvp.merged_shading_srvs[i];
				pvp.merged_shading_resolved_rtvs[i] = pvp.merged_shading_rtvs[i];
				pvp.merged_depth_resolved_texs[i] = pvp.merged_depth_texs[i];
				pvp.merged_depth_resolved_srvs[i] = pvp.merged_depth_srvs[i];
				pvp.merged_depth_resolved_rtvs[i] = pvp.merged_depth_rtvs[i];
			}
		}
		else
		{
			for (size_t i = 0; i < pvp.merged_shading_resolved_texs.size(); ++ i)
			{
				pvp.merged_shading_resolved_texs[i] = rf.MakeTexture2D(width, height, 1, 1, fmt, 1, 0, EAH_GPU_Read | EAH_GPU_Write);
				KLAYGE_TEXTURE_DEBUG_NAME(pvp.merged_shading_resolved_texs[i]);
				pvp.merged_shading_resolved_srvs[i] = rf.MakeTextureSrv(pvp.merged_shading_resolved_texs[i]);
				pvp.merged_shading_resolved_rtvs[i] = rf.Make2DRtv(pvp.merged_shading_resolved_texs[i], 0, 1, 0);
				pvp.merged_depth_resolved_texs[i] = rf.MakeTexture2D(width, height, 1, 1, depth_fmt, 1, 0, EAH_GPU_Read | EAH_GPU_Write);
				KLAYGE_TEXTURE_DEBUG_NAME(pvp.merged_depth_resolved_texs[i]);
				pvp.merged_depth_resolved_srvs[i] = rf.MakeTextureSrv(pvp.merged_depth_resolved_texs[i]);
				pvp.merged_depth_resolved_rtvs[i] = rf.Make2DRtv(pvp.merged_depth_resolved_texs[i], 0, 1, 0);
			}
		}

		if (!(attrib & VPAM_NoDoF))
		{
			pvp.dof_tex = rf.MakeTexture2D(width, height, 1, 1, fmt, 1, 0, EAH_GPU_Read | EAH_GPU_Write);
			KLAYGE_TEXTURE_DEBUG_NAME(pvp.dof_tex);
			pvp.dof_srv = rf.MakeTextureSrv(pvp.dof_tex);
			pvp.dof_rtv = rf.Make2DRtv(pvp.dof_tex, 0, 1, 0);
		}

		if (!(attrib & VPAM_NoMotionBlur))
		{
			pvp.motion_blur_tex = rf.MakeTexture2D(width, height, 1, 1, fmt, 1, 0, EAH_GPU_Read | EAH_GPU_Write);
			KLAYGE_TEXTURE_DEBUG_NAME(pvp.motion_blur_tex);
			pvp.motion_blur_srv = rf.MakeTextureSrv(pvp.motion_blur_tex);
			pvp.motion_blur_rtv = rf.Make2DRtv(pvp.motion_blur_tex, 0, 1, 0);
		}

		if (!(attrib & VPAM_NoSSR) && !(attrib & VPAM_NoPPR))
		{
			pvp.merged_shading_resolved_before_ssr_tex = rf.MakeTexture2D(width, height, 1, 1, fmt, 1, 0, EAH_GPU_Read | EAH_GPU_Write);
			KLAYGE_TEXTURE_DEBUG_NAME(pvp.merged_shading_resolved_before_ssr_tex);
			pvp.merged_shading_resolved_before_ssr_srv = rf.MakeTextureSrv(pvp.merged_shading_resolved_before_ssr_tex);
		}

#if DEFAULT_DEFERRED == LIGHT_INDEXED_DEFERRED
		if (cs_cldr_)
		{
			if (!typed_uav_)
			{
				pvp.temp_shading_tex = rf.MakeTexture2D(width, height, 1, 1, shading_fmt, sample_count, sample_quality,
					EAH_GPU_Read | ((sample_count == 1) ? EAH_GPU_Unordered : EAH_GPU_Write));
				KLAYGE_TEXTURE_DEBUG_NAME(pvp.temp_shading_tex);
				pvp.temp_shading_srv = rf.MakeTextureSrv(pvp.temp_shading_tex);
				if (sample_count != 1)
				{
					pvp.temp_shading_fb = rf.MakeFrameBuffer();
					pvp.temp_shading_fb->Attach(FrameBuffer::Attachment::Color0, rf.Make2DRtv(pvp.temp_shading_tex, 0, 1, 0));
				}
			}

			if (sample_count != 1)
			{
				pvp.temp_shading_tex_array = rf.MakeTexture2D(width, height, 1, sample_count, fmt, 1, 0,
					EAH_GPU_Read | EAH_GPU_Write | EAH_GPU_Unordered);
				KLAYGE_TEXTURE_DEBUG_NAME(pvp.temp_shading_tex_array);

				auto const multi_sample_mask_fmt = caps.BestMatchTextureRenderTargetFormat(MakeSpan({EF_R8, EF_ABGR8, EF_ARGB8}), 1, 0);
				pvp.multi_sample_mask_tex = rf.MakeTexture2D(width, height, 1, 1, multi_sample_mask_fmt, 1, 0,
					EAH_GPU_Read | EAH_GPU_Write);
				KLAYGE_TEXTURE_DEBUG_NAME(pvp.multi_sample_mask_tex);

				pvp.g_buffer_resolved_fb->Attach(FrameBuffer::Attachment::Color0, rf.Make2DRtv(pvp.g_buffer_resolved_rt0_tex, 0, 1, 0));
				pvp.g_buffer_resolved_fb->Attach(FrameBuffer::Attachment::Color1, rf.Make2DRtv(pvp.g_buffer_resolved_rt1_tex, 0, 1, 0));
				pvp.g_buffer_resolved_fb->Attach(FrameBuffer::Attachment::Color2, rf.Make2DRtv(pvp.g_buffer_resolved_rt2_tex, 0, 1, 0));
				pvp.g_buffer_resolved_fb->Attach(FrameBuffer::Attachment::Color3, pvp.g_buffer_resolved_depth_rtv);
				pvp.g_buffer_resolved_fb->Attach(FrameBuffer::Attachment::Color4, rf.Make2DRtv(pvp.multi_sample_mask_tex, 0, 1, 0));
			}

			auto const light_indices_fmt = caps.BestMatchUavFormat(MakeSpan({EF_R16UI, EF_R32UI}));
			BOOST_ASSERT(light_indices_fmt != EF_Unknown);
			pvp.lights_start_tex = rf.MakeTexture2D((width + (TILE_SIZE - 1)) / TILE_SIZE * 8,
				(height + (TILE_SIZE - 1)) / TILE_SIZE, 1, num_depth_slices_, light_indices_fmt, 1, 0,
				EAH_GPU_Read | EAH_GPU_Unordered);
			KLAYGE_TEXTURE_DEBUG_NAME(pvp.lights_start_tex);
			pvp.intersected_light_indices_tex = rf.MakeTexture2D((width + (TILE_SIZE - 1)) / TILE_SIZE * 32,
				(height + (TILE_SIZE - 1)) / TILE_SIZE * 32, 1, num_depth_slices_, light_indices_fmt, 1, 0,
				EAH_GPU_Read | EAH_GPU_Unordered);
			KLAYGE_TEXTURE_DEBUG_NAME(pvp.intersected_light_indices_tex);
		}
		else
		{
			fmt = caps.BestMatchTextureRenderTargetFormat(MakeSpan({EF_ABGR8, EF_ARGB8}), 1, 0);
			BOOST_ASSERT(fmt != EF_Unknown);
			pvp.light_index_tex = rf.MakeTexture2D((width + (TILE_SIZE - 1)) / TILE_SIZE,
				(height + (TILE_SIZE - 1)) / TILE_SIZE, 1, 1, fmt, 1, 0, EAH_GPU_Read | EAH_GPU_Write);
			KLAYGE_TEXTURE_DEBUG_NAME(pvp.light_index_tex);
			pvp.light_index_fb->Attach(FrameBuffer::Attachment::Color0, rf.Make2DRtv(pvp.light_index_tex, 0, 1, 0));
		}
#endif

		pvp.shading_fb->Attach(FrameBuffer::Attachment::Color0, pvp.shading_rtv);
		pvp.shading_fb->Attach(ds_view);

		for (size_t i = 0; i < pvp.merged_shading_texs.size(); ++ i)
		{
			pvp.merged_shading_fbs[i]->Attach(FrameBuffer::Attachment::Color0, pvp.merged_shading_rtvs[i]);
			pvp.merged_shading_fbs[i]->Attach(ds_view);
			pvp.merged_depth_fbs[i]->Attach(FrameBuffer::Attachment::Color0, pvp.merged_depth_rtvs[i]);
			pvp.merged_depth_fbs[i]->Attach(ds_view);
		}
		if (pvp.sample_count != 1)
		{
			for (size_t i = 0; i < pvp.merged_depth_resolved_fbs.size(); ++ i)
			{
				pvp.merged_depth_resolved_fbs[i]->Attach(FrameBuffer::Attachment::Color0, pvp.merged_depth_resolved_rtvs[i]);
			}
		}

		fmt = caps.BestMatchTextureRenderTargetFormat(MakeSpan({EF_R8, EF_R16F, EF_ABGR8, EF_ARGB8}), 1, 0);
		BOOST_ASSERT(fmt != EF_Unknown);
		pvp.small_ssvo_tex = rf.MakeTexture2D(width / 2, height / 2, 1, 1, fmt, 1, 0, EAH_GPU_Read | EAH_GPU_Write);
		KLAYGE_TEXTURE_DEBUG_NAME(pvp.small_ssvo_tex);
		pvp.small_ssvo_srv = rf.MakeTextureSrv(pvp.small_ssvo_tex);
		pvp.small_ssvo_rtv = rf.Make2DRtv(pvp.small_ssvo_tex, 0, 1, 0);

		if (0 == index)
		{
			dr_debug_pp_->InputPin(0, this->GBufferResolvedRT0Srv(index));
			dr_debug_pp_->InputPin(1, this->GBufferResolvedRT1Srv(index));
			dr_debug_pp_->InputPin(2, this->GBufferResolvedRT2Srv(index));
			dr_debug_pp_->InputPin(3, this->ResolvedDepthSrv(index));
#if DEFAULT_DEFERRED == TRIDITIONAL_DEFERRED
			dr_debug_pp_->InputPin(4,this->LightingSrv(index));
#endif
			dr_debug_pp_->InputPin(5, this->SmallSSVOSrv(index));
		}
	}

	RenderEffectPtr const & DeferredRenderingLayer::GBufferEffect(RenderMaterial const * material, bool line, bool skinning) const
	{
		EffectIndex effect_index;
		effect_index.index = 0;
		if (material)
		{
			effect_index.flags.sss = material->Sss();
			effect_index.flags.two_sided = material->TwoSided();
			effect_index.flags.detail_mode = static_cast<uint8_t>(material->DetailMode());
		}
		effect_index.flags.line = line;
		effect_index.flags.skinning = skinning;

		if (!g_buffer_effects_[effect_index.index])
		{
			std::string g_buffer_files[6];
			g_buffer_files[0] = "GBuffer.fxml";

			uint32_t num = 1;
			if (effect_index.flags.line)
			{
				g_buffer_files[num] = "GBufferLine.fxml";
				++ num;
			}
			if (effect_index.flags.sss)
			{
				g_buffer_files[num] = "GBufferSSS.fxml";
				++ num;
			}
			if (effect_index.flags.two_sided)
			{
				g_buffer_files[num] = "GBufferTwoSided.fxml";
				++ num;
			}
			if (effect_index.flags.skinning)
			{
				g_buffer_files[num] = "GBufferSkinning.fxml";
				++ num;
			}
			switch (static_cast<RenderMaterial::SurfaceDetailMode>(effect_index.flags.detail_mode))
			{
			case RenderMaterial::SurfaceDetailMode::ParallaxMapping:
				break;

			case RenderMaterial::SurfaceDetailMode::ParallaxOcclusionMapping:
				g_buffer_files[num] = "GBufferParallaxOcclusionMapping.fxml";
				++num;
				break;

			case RenderMaterial::SurfaceDetailMode::FlatTessellation:
				g_buffer_files[num] = "GBufferFlatTess.fxml";
				++ num;
				break;

			case RenderMaterial::SurfaceDetailMode::SmoothTessellation:
				g_buffer_files[num] = "GBufferSmoothTess.fxml";
				++ num;
				break;
			}

			g_buffer_effects_[effect_index.index] = SyncLoadRenderEffects(MakeSpan(g_buffer_files, num));
		}

		return g_buffer_effects_[effect_index.index];
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
		shadow_map_light_indices_.clear();

		uint32_t const num_lights = scene_mgr.NumFrameLights();
		
		for (uint32_t i = 0; i < num_lights; ++ i)
		{
			auto* light = scene_mgr.GetFrameLight(i);
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
		shadow_map_light_indices_.emplace_back(-1, 0);

		uint32_t num_ambient_lights = 0;
		float3 ambient_clr(0, 0, 0);

		projective_light_index_ = -1;
		cascaded_shadow_index_ = -1;
		uint32_t num_shadow_map_lights = 0;
		uint32_t num_shadow_map_2d_lights = 0;
		uint32_t num_shadow_map_cube_lights = 0;
		for (uint32_t i = 0; i < num_lights; ++ i)
		{
			auto* light = scene_mgr.GetFrameLight(i);
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
							shadow_map_light_indices_.emplace_back(0, num_shadow_map_lights);
							++num_shadow_map_lights;
							cascaded_shadow_index_ = static_cast<int32_t>(i + 1 - num_ambient_lights);
							break;

						case LightSource::LT_Spot:
							if ((projective_light_index_ < 0) && light->ProjectiveTexture())
							{
								projective_light_index_ = static_cast<int32_t>(i + 1 - num_ambient_lights);
								shadow_map_light_indices_.emplace_back(0, 4);
							}
							else if ((num_shadow_map_2d_lights < MAX_NUM_SHADOWED_SPOT_LIGHTS)
								&& (num_shadow_map_lights < MAX_NUM_SHADOWED_LIGHTS))
							{
								shadow_map_light_indices_.emplace_back(num_shadow_map_2d_lights, num_shadow_map_lights);
								++num_shadow_map_2d_lights;
								++num_shadow_map_lights;
							}
							else
							{
								shadow_map_light_indices_.emplace_back(-1, 0);
							}
							break;

						case LightSource::LT_Point:
						case LightSource::LT_SphereArea:
						case LightSource::LT_TubeArea:
							if ((projective_light_index_ < 0) && light->ProjectiveTexture())
							{
								projective_light_index_ = static_cast<int32_t>(i + 1 - num_ambient_lights);
								shadow_map_light_indices_.emplace_back(0, 4);
							}
							else if ((num_shadow_map_cube_lights < MAX_NUM_SHADOWED_POINT_LIGHTS)
								&& (num_shadow_map_lights < MAX_NUM_SHADOWED_LIGHTS))
							{
								shadow_map_light_indices_.emplace_back(num_shadow_map_cube_lights, num_shadow_map_lights);
								++num_shadow_map_cube_lights;
								++num_shadow_map_lights;
							}
							else
							{
								shadow_map_light_indices_.emplace_back(-1, 0);
							}
							break;

						default:
							shadow_map_light_indices_.emplace_back(-1, 0);
							break;
						}
					}
					else
					{
						shadow_map_light_indices_.emplace_back(-1, 0);
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

		has_sss_objs_ = false;
		has_reflective_objs_ = false;
		has_simple_forward_objs_ = false;
		has_vdm_objs_ = false;
		has_ssr_objs_ = false;
		has_ppr_objs_ = false;
		visible_scene_nodes_.clear();
		scene_mgr.SceneRootNode().Traverse(
			[this, &has_opaque_objs, &has_transparency_back_objs, &has_transparency_front_objs](SceneNode& node)
			{
				if (node.Visible())
				{
					if (node.NumComponents() > 0)
					{
						visible_scene_nodes_.push_back(&node);

						has_opaque_objs = true;

						if (node.TransparencyBackFace())
						{
							has_transparency_back_objs = true;
						}
						if (node.TransparencyFrontFace())
						{
							has_transparency_front_objs = true;
						}
						if (node.SSS())
						{
							has_sss_objs_ = true;
						}
						if (node.ObjectReflection())
						{
							has_reflective_objs_ = true;
						}
						if (node.SimpleForward())
						{
							has_simple_forward_objs_ = true;
						}
						if (node.VDM())
						{
							has_vdm_objs_ = true;
						}
						if (node.ScreenSpaceReflection())
						{
							has_ssr_objs_ = true;
						}
						if (node.PixelProjectedReflection())
						{
							has_ppr_objs_ = true;
						}
					}

					return true;
				}

				return false;
			});
	}

	void DeferredRenderingLayer::BuildPassScanList(bool has_opaque_objs, bool has_transparency_back_objs, bool has_transparency_front_objs)
	{
		jobs_.clear();

		if (dr_effect_->HWResourceReady())
		{
#ifndef KLAYGE_SHIP
			jobs_.push_back(MakeUniquePtr<DeferredRenderingJob>([this] { return this->BeginPerfProfileDRJob(*shadow_map_perf_); }));
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
			jobs_.push_back(MakeUniquePtr<DeferredRenderingJob>([this] { return this->EndPerfProfileDRJob(*shadow_map_perf_); }));
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

					jobs_.push_back(MakeUniquePtr<DeferredRenderingJob>([this, vpi] { return this->SwitchViewportDRJob(vpi); }));

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
							jobs_.push_back(MakeUniquePtr<DeferredRenderingJob>(
								[this, vpi, pass_tb]
							{
								return this->ShadowingDRJob(viewports_[vpi], pass_tb);
							}));
							if (!(pvp.attrib & VPAM_NoGI))
							{
#ifndef KLAYGE_SHIP
								jobs_.push_back(MakeUniquePtr<DeferredRenderingJob>(
									[this, pass_tb]
								{
									return this->BeginPerfProfileDRJob(*indirect_lighting_perfs_[pass_tb]);
								}));
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
								jobs_.push_back(MakeUniquePtr<DeferredRenderingJob>(
									[this, pass_tb]
								{
									return this->EndPerfProfileDRJob(*indirect_lighting_perfs_[pass_tb]);
								}));
#endif
							}

							this->AppendShadingPassScanCode(vpi, pass_tb);
						}
					}

					jobs_.push_back(MakeUniquePtr<DeferredRenderingJob>(
						[this, vpi]
					{
						return this->PostEffectsDRJob(viewports_[vpi]);
					}));
					if (has_simple_forward_objs_ && !(pvp.attrib & VPAM_NoSimpleForward))
					{
						jobs_.push_back(MakeUniquePtr<DeferredRenderingJob>([this, vpi] { return this->SimpleForwardDRJob(viewports_[vpi]); }));
						jobs_.push_back(MakeUniquePtr<DeferredRenderingJob>([this, vpi] {
							return this->PostSimpleForwardDRJob(viewports_[vpi]); }));
					}

					jobs_.push_back(
						MakeUniquePtr<DeferredRenderingJob>([this, vpi] { return this->FinishingViewportDRJob(viewports_[vpi]); }));
				}
			}

			if ((DT_SSVO == display_type_)
#if DEFAULT_DEFERRED == TRIDITIONAL_DEFERRED
				|| (DT_DiffuseLighting == display_type_)
				|| (DT_SpecularLighting == display_type_)
#endif
				)
			{
				jobs_.push_back(MakeUniquePtr<DeferredRenderingJob>([this] { return this->VisualizeLightingDRJob(); }));
			}
			else
			{
				jobs_.push_back(MakeUniquePtr<DeferredRenderingJob>([this] { return this->FinishingDRJob(); }));
			}

#ifdef KLAYGE_DEBUG
			if (no_viewport)
			{
				LogDebug() << "No viewport available." << std::endl;
			}
#endif
		}
		else
		{
			jobs_.push_back(MakeUniquePtr<DeferredRenderingJob>([this] { return this->ClearOnlyDRJob(); }));
		}
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
				float const scale = light.CosOuterInner().w();
				float4x4 mat = MathLib::scaling(scale * light_scale, scale * light_scale, light_scale);
				float4x4 light_model = mat * light.BoundSceneNode()->TransformToWorld();
				pvp.light_visibles[light_index] =
					(scene_mgr.AABBVisible(MathLib::transform_aabb(cone_aabb_, light_model)) != BoundOverlap::No);
			}
			break;

		case LightSource::LT_Point:
		case LightSource::LT_SphereArea:
		case LightSource::LT_TubeArea:
			{
				float3 const & p = light.Position();
				float4x4 light_model = MathLib::scaling(light_scale, light_scale, light_scale)
					* MathLib::translation(p);
				pvp.light_visibles[light_index] =
					(scene_mgr.AABBVisible(MathLib::transform_aabb(box_aabb_, light_model)) != BoundOverlap::No);
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
		jobs_.push_back(MakeUniquePtr<DeferredRenderingJob>(
			[this, pass_tb]
			{
				return this->BeginPerfProfileDRJob(*gbuffer_perfs_[pass_tb]);
			}));
#endif
		jobs_.push_back(MakeUniquePtr<DeferredRenderingJob>(
			[this, vp_index, pass_tb]
			{
				return this->GBufferGenerationDRJob(viewports_[vp_index], ComposePassType(PRT_GBuffer, pass_tb, PC_GBuffer));
			}));
		jobs_.push_back(MakeUniquePtr<DeferredRenderingJob>([this] { return this->RenderingStatsDRJob(); }));
		jobs_.push_back(MakeUniquePtr<DeferredRenderingJob>(
			[this, vp_index]
			{
				return this->GBufferProcessingDRJob(viewports_[vp_index]);
			}));
		if (pass_tb == PTB_Opaque)
		{
			jobs_.push_back(MakeUniquePtr<DeferredRenderingJob>(
				[this, vp_index]
				{
					return this->OpaqueGBufferProcessingDRJob(viewports_[vp_index]);
				}));
			if ((DeferredRenderingLayer::DT_Position == display_type_)
				|| (DeferredRenderingLayer::DT_Normal == display_type_)
				|| (DeferredRenderingLayer::DT_Depth == display_type_)
				|| (DeferredRenderingLayer::DT_Diffuse == display_type_)
				|| (DeferredRenderingLayer::DT_Specular == display_type_)
				|| (DeferredRenderingLayer::DT_Shininess == display_type_)
				|| (DeferredRenderingLayer::DT_MotionVec == display_type_)
				|| (DeferredRenderingLayer::DT_Occlusion == display_type_))
			{
				jobs_.push_back(MakeUniquePtr<DeferredRenderingJob>([this] { return this->VisualizeGBufferDRJob(); }));
			}
		}
#ifndef KLAYGE_SHIP
		jobs_.push_back(MakeUniquePtr<DeferredRenderingJob>(
			[this, pass_tb]
			{
				return this->EndPerfProfileDRJob(*gbuffer_perfs_[pass_tb]);
			}));
#endif
	}

	void DeferredRenderingLayer::AppendShadowPassScanCode(uint32_t light_index)
	{
		PassType shadow_pt = PT_GenShadowMap;
		int passes = 0;

		auto const & light = *lights_[light_index];
		LightSource::LightType const type = light.Type();
		int32_t const attr = light.Attrib();
		switch (type)
		{
		case LightSource::LT_Spot:
			{
				bool gen_sm;
				if (attr & LightSource::LSA_IndirectLighting)
				{
					gen_sm = true;
					if (rsm_fb_ && (illum_ != 1))
					{
						shadow_pt = PT_GenReflectiveShadowMap;
					}
				}
				else
				{
					gen_sm = (0 == (attr & LightSource::LSA_NoShadow));
				}

				if (gen_sm)
				{
					passes = 2;
				}
			}
			break;

		case LightSource::LT_Point:
		case LightSource::LT_SphereArea:
		case LightSource::LT_TubeArea:
			if (0 == (attr & LightSource::LSA_NoShadow))
			{
				if (tex_array_support_)
				{
					shadow_pt = PT_GenShadowMapMultiView;
					passes = 2;
				}
				else
				{
					passes = 7;
				}
			}
			break;

		case LightSource::LT_Ambient:
		case LightSource::LT_Directional:
			break;

		default:
			KFL_UNREACHABLE("Invalid light type");
		}

		for (int i = 0; i < passes; ++i)
		{
			jobs_.push_back(MakeUniquePtr<DeferredRenderingJob>(
				[this, shadow_pt, light_index, i]
				{
					return this->ShadowMapGenerationDRJob(viewports_[0], shadow_pt, light_index, i);
				}));
		}
	}

	void DeferredRenderingLayer::AppendCascadedShadowPassScanCode(uint32_t vp_index, uint32_t light_index)
	{
		BOOST_ASSERT(LightSource::LT_Directional == lights_[light_index]->Type());

#ifndef KLAYGE_SHIP
		jobs_.push_back(MakeUniquePtr<DeferredRenderingJob>([this] { return this->BeginPerfProfileDRJob(*shadow_map_perf_); }));
#endif

		PerViewport& pvp = viewports_[vp_index];
		for (uint32_t i = 0; i < pvp.num_cascades + 1; ++ i)
		{
			jobs_.push_back(MakeUniquePtr<DeferredRenderingJob>(
				[this, vp_index, light_index, i]
				{
					return this->ShadowMapGenerationDRJob(viewports_[vp_index], PT_GenCascadedShadowMap, light_index, i);
				}));
		}

#ifndef KLAYGE_SHIP
		jobs_.push_back(MakeUniquePtr<DeferredRenderingJob>([this] { return this->EndPerfProfileDRJob(*shadow_map_perf_); }));
#endif
	}

	void DeferredRenderingLayer::AppendIndirectLightingPassScanCode(uint32_t vp_index, uint32_t light_index)
	{
		jobs_.push_back(MakeUniquePtr<DeferredRenderingJob>(
			[this, vp_index, light_index]
			{
				return this->IndirectLightingDRJob(viewports_[vp_index], light_index);
			}));
	}

	void DeferredRenderingLayer::AppendShadingPassScanCode(uint32_t vp_index, PassTargetBuffer pass_tb)
	{
		jobs_.push_back(MakeUniquePtr<DeferredRenderingJob>(
			[this, vp_index, pass_tb]
			{
				return this->ShadingDRJob(viewports_[vp_index], ComposePassType(PRT_None, pass_tb, PC_Shading), 0);
			}));

		if (has_reflective_objs_)
		{
#ifndef KLAYGE_SHIP
			jobs_.push_back(MakeUniquePtr<DeferredRenderingJob>(
				[this, pass_tb]
				{
					return this->BeginPerfProfileDRJob(*reflection_perfs_[pass_tb]);
				}));
#endif
			jobs_.push_back(MakeUniquePtr<DeferredRenderingJob>(
				[this, vp_index, pass_tb]
				{
					return this->ReflectionDRJob(viewports_[vp_index], ComposePassType(PRT_None, pass_tb, PC_ObjectReflection));
				}));
#ifndef KLAYGE_SHIP
			jobs_.push_back(MakeUniquePtr<DeferredRenderingJob>(
				[this, pass_tb]
				{
					return this->EndPerfProfileDRJob(*reflection_perfs_[pass_tb]);
				}));
#endif
		}

		if (has_vdm_objs_)
		{
#ifndef KLAYGE_SHIP
			jobs_.push_back(MakeUniquePtr<DeferredRenderingJob>([this] { return this->BeginPerfProfileDRJob(*vdm_perf_); }));
#endif
			jobs_.push_back(MakeUniquePtr<DeferredRenderingJob>(
				[this, vp_index]
				{
					return this->VDMDRJob(viewports_[vp_index]);
				}));
#ifndef KLAYGE_SHIP
			jobs_.push_back(MakeUniquePtr<DeferredRenderingJob>([this] { return this->EndPerfProfileDRJob(*vdm_perf_); }));
#endif
		}

#ifndef KLAYGE_SHIP
		jobs_.push_back(MakeUniquePtr<DeferredRenderingJob>(
			[this, pass_tb]
			{
				return this->BeginPerfProfileDRJob(*special_shading_perfs_[pass_tb]);
			}));
#endif
		jobs_.push_back(MakeUniquePtr<DeferredRenderingJob>(
			[this, vp_index, pass_tb]
			{
				return this->SpecialShadingDRJob(viewports_[vp_index] , ComposePassType(PRT_None, pass_tb, PC_SpecialShading));
			}));
		jobs_.push_back(MakeUniquePtr<DeferredRenderingJob>(
			[this, vp_index, pass_tb]
			{
				return this->MergeShadingAndDepthDRJob(viewports_[vp_index] , pass_tb);
			}));
#ifndef KLAYGE_SHIP
		jobs_.push_back(MakeUniquePtr<DeferredRenderingJob>(
			[this, pass_tb]
			{
				return this->EndPerfProfileDRJob(*special_shading_perfs_[pass_tb]);
			}));
#endif
	}

	void DeferredRenderingLayer::PreparePVP(PerViewport& pvp)
	{
		Camera const & camera = *pvp.frame_buffer->Viewport()->Camera();
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
			CameraPtr const & camera = pvp.frame_buffer->Viewport()->Camera();
			pvp.g_buffer_fb->Viewport()->Camera(camera);
#if DEFAULT_DEFERRED == TRIDITIONAL_DEFERRED
			pvp.lighting_fb->Viewport()->Camera(camera);
#endif
			pvp.vdm_fb->Viewport()->Camera(camera);
			pvp.shading_fb->Viewport()->Camera(camera);
			for (size_t i = 0; i < pvp.merged_shading_fbs.size(); ++ i)
			{
				pvp.merged_shading_fbs[i]->Viewport()->Camera(camera);
				pvp.merged_depth_fbs[i]->Viewport()->Camera(camera);
				pvp.merged_depth_resolved_fbs[i]->Viewport()->Camera(camera);
			}
		}

		re.BindFrameBuffer(pvp.g_buffer_fb);

		float depth = (PTB_TransparencyBack == pass_tb) ? 0.0f : 1.0f;
		int32_t stencil = (PTB_Opaque == pass_tb) ? 0 : 16;
		pvp.g_buffer_fb->Clear(FrameBuffer::CBM_Color | FrameBuffer::CBM_Depth | FrameBuffer::CBM_Stencil,
			Color(0, 0, 0, 0), depth, stencil);
	}

	void DeferredRenderingLayer::PostGenerateGBuffer(PerViewport const & pvp)
	{
#if DEFAULT_DEFERRED == LIGHT_INDEXED_DEFERRED
		auto& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();

		if (pvp.sample_count != 1)
		{
			*g_buffer_rt0_tex_ms_param_ = pvp.g_buffer_rt0_tex;
			*g_buffer_rt1_tex_ms_param_ = pvp.g_buffer_rt1_tex;
			*g_buffer_rt2_tex_ms_param_ = pvp.g_buffer_rt2_tex;
			*g_buffer_ds_tex_ms_param_ = pvp.g_buffer_ds_srv;

			*near_q_far_param_ = pvp.frame_buffer->Viewport()->Camera()->NearQFarParam();

			re.BindFrameBuffer(pvp.g_buffer_resolved_fb);
			re.CurFrameBuffer()->AttachedRtv(FrameBuffer::Attachment::Color3)->ClearColor(Color(0, 0, 0, 0));
			re.Render(*dr_effect_, *technique_resolve_g_buffers_, *rl_quad_);
		}
#endif

		pvp.g_buffer_resolved_rt0_tex->BuildMipSubLevels(TextureFilter::Linear);

#if DEFAULT_DEFERRED == LIGHT_INDEXED_DEFERRED
		if (cs_cldr_)
		{
			this->CreateDepthMinMaxMapCS(pvp);
		}
		else
		{
			this->BuildLinearDepthMipmap(pvp);
			this->CreateDepthMinMaxMap(pvp);
		}
#elif DEFAULT_DEFERRED == TRIDITIONAL_DEFERRED
		this->BuildLinearDepthMipmap(pvp);
#endif
	}

	void DeferredRenderingLayer::BuildLinearDepthMipmap(PerViewport const & pvp)
	{
		depth_to_linear_pps_[0]->InputPin(0, pvp.g_buffer_ds_srv);
		depth_to_linear_pps_[0]->OutputPin(0, pvp.g_buffer_depth_rtv);
		depth_to_linear_pps_[0]->Apply();

		pvp.g_buffer_depth_tex->BuildMipSubLevels(TextureFilter::Linear);
	}

	void DeferredRenderingLayer::RenderDecals(PerViewport const & pvp, PassType pass_type)
	{
		RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();

		uint32_t const width = pvp.g_buffer_resolved_rt0_tex->Width(0);
		uint32_t const height = pvp.g_buffer_resolved_rt0_tex->Height(0);
		pvp.g_buffer_resolved_rt0_tex->CopyToSubTexture2D(
			*pvp.g_buffer_rt0_backup_tex, 0, 0, 0, 0, width, height, 0, 0, 0, 0, width, height, TextureFilter::Point);

		re.BindFrameBuffer(pvp.g_buffer_fb);
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
		int32_t const attr = light.Attrib();

		switch (type)
		{
		case LightSource::LT_Point:
		case LightSource::LT_Spot:
		case LightSource::LT_Directional:
		case LightSource::LT_SphereArea:
		case LightSource::LT_TubeArea:
			{
				int32_t const face = std::min(index_in_pass, 5);
				float3 dir_es(0, 0, 0);
				float3 lookat, up;
				if ((type == LightSource::LT_Spot) || (type == LightSource::LT_Directional))
				{
					lookat = float3(0, 0, 1);
					up = float3(0, 1, 0);
					dir_es = MathLib::transform_normal((type == LightSource::LT_Spot) ? light.Direction() : -light.Direction(), pvp.view);
				}
				else
				{
					std::tie(lookat, up) = CubeMapViewVector<float>(static_cast<Texture::CubeFaces>(face));
					dir_es = MathLib::transform_normal(MathLib::transform_quat(lookat, light.Rotation()), pvp.view);
				}
				float4 light_dir_es_actived = float4(dir_es.x(), dir_es.y(), dir_es.z(), 0);

				float4x4 inv_light_camera_view;
				if (attr & LightSource::LSA_NoShadow)
				{
					lookat = MathLib::transform_quat(lookat, light.Rotation());
					up = MathLib::transform_quat(up, light.Rotation());
					inv_light_camera_view = MathLib::inverse(MathLib::look_at_lh(light.Position(), light.Position() + lookat, up));
				}
				else
				{
					auto const& shadow_map_camera =
						light.SMCamera(((type == LightSource::LT_Spot) || (type == LightSource::LT_Directional)) ? 0 : face);
					*light_view_proj_param_ = pvp.inv_view * shadow_map_camera->ViewProjMatrix();
					inv_light_camera_view = shadow_map_camera->InverseViewMatrix();
				}

				float4x4 light_to_view = inv_light_camera_view * pvp.view;
				float4x4 light_to_proj = light_to_view * pvp.proj;

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

	void DeferredRenderingLayer::PostGenerateShadowMap(PerViewport const & pvp, int32_t light_index, int32_t index_in_pass)
	{
		LightSource::LightType const type = lights_[light_index]->Type();

		if (tex_array_support_ &&
			((LightSource::LT_Point == type) || (LightSource::LT_SphereArea == type) || (LightSource::LT_TubeArea == type)))
		{
			auto const& shadow_map_camera = lights_[light_index]->SMCamera(0);
			depth_to_esm_pp_->SetParam(0, shadow_map_camera->NearQFarParam());
			depth_to_esm_pp_->SetParam(1, shadow_map_camera->InverseProjMatrix());

			PostProcessChainPtr pp_chain = checked_pointer_cast<PostProcessChain>(shadow_map_filter_pp_);
			checked_pointer_cast<SeparableLogGaussianFilterPostProcess>(pp_chain->GetPostProcess(0))->KernelRadius(4);
			checked_pointer_cast<SeparableLogGaussianFilterPostProcess>(pp_chain->GetPostProcess(1))->KernelRadius(4);
			checked_pointer_cast<LogGaussianBlurPostProcess>(pp_chain)->ESMScaleFactor(ESM_SCALE_FACTOR, *shadow_map_camera);

			for (uint32_t i = 0; i < 6; ++i)
			{
				if (flexible_srvs_support_)
				{
					depth_to_esm_pp_->InputPin(0, shadow_map_array_depth_srvs_[i]);
				}
				else
				{
					shadow_map_array_depth_tex_->CopyToSubTexture2D(*shadow_map_depth_tex_, 0, 0, 0, 0, shadow_map_depth_tex_->Width(0),
						shadow_map_depth_tex_->Height(0), i, 0, 0, 0, shadow_map_depth_tex_->Width(0), shadow_map_depth_tex_->Height(0),
						TextureFilter::Point);
					depth_to_esm_pp_->InputPin(0, shadow_map_depth_srv_);
				}
				depth_to_esm_pp_->Apply();

				pp_chain->OutputPin(
					0, filtered_shadow_map_cube_face_rtvs_[shadow_map_light_indices_[light_index].first * 6 + i]);

				pp_chain->Apply();
			}
		}
		else
		{
			auto const& shadow_map_camera = lights_[light_index]->SMCamera(
				((type == LightSource::LT_Spot) || (type == LightSource::LT_Directional)) ? 0 : index_in_pass - 1);

			PostProcessChainPtr pp_chain;
			if (LightSource::LT_Directional == type)
			{
				pp_chain = checked_pointer_cast<PostProcessChain>(csm_filter_pp_);
				pp_chain->OutputPin(0, pvp.filtered_csm_slice_rtvs[index_in_pass - 1]);
			}
			else
			{
				depth_to_esm_pp_->InputPin(0, shadow_map_depth_srv_);
				depth_to_esm_pp_->SetParam(0, shadow_map_camera->NearQFarParam());
				depth_to_esm_pp_->SetParam(1, shadow_map_camera->InverseProjMatrix());
				depth_to_esm_pp_->Apply();

				pp_chain = checked_pointer_cast<PostProcessChain>(shadow_map_filter_pp_);
				if ((LightSource::LT_Point == type) || (LightSource::LT_SphereArea == type) || (LightSource::LT_TubeArea == type))
				{
					pp_chain->OutputPin(
						0, filtered_shadow_map_cube_face_rtvs_[shadow_map_light_indices_[light_index].first * 6 + index_in_pass - 1]);
				}
				else
				{
					pp_chain->OutputPin(0, filtered_shadow_map_2d_slice_rtvs_[shadow_map_light_indices_[light_index].first]);
					if (has_sss_objs_ && translucency_enabled_)
					{
						shadow_map_tex_->CopyToTexture(
							*unfiltered_shadow_map_2d_texs_[shadow_map_light_indices_[light_index].first], TextureFilter::Point);
					}
				}
			}

			int2 kernel_size;
			if (LightSource::LT_Directional == type)
			{
				float3 const& scale = cascaded_shadow_layer_->CascadeScales()[index_in_pass - 1];
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

			checked_pointer_cast<LogGaussianBlurPostProcess>(pp_chain)->ESMScaleFactor(ESM_SCALE_FACTOR, *shadow_map_camera);
			pp_chain->Apply();

			if (LightSource::LT_Directional == type)
			{
				if (tex_array_support_)
				{
					if (static_cast<int32_t>(pvp.num_cascades) == index_in_pass)
					{
						pvp.filtered_csm_texs[0]->BuildMipSubLevels(TextureFilter::Linear);
					}
				}
				else
				{
					pvp.filtered_csm_texs[index_in_pass - 1]->BuildMipSubLevels(TextureFilter::Linear);
				}
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

				Camera* shadow_map_camera = nullptr;

				int32_t const light_index = shadow_map_light_indices_[li].first;
				int32_t const shadowing_channel = shadow_map_light_indices_[li].second;
				if ((light_index >= 0) || (LightSource::LT_Directional == type))
				{
					switch (type)
					{
					case LightSource::LT_Spot:
						shadow_map_camera = light.SMCamera(0).get();
						if (tex_array_support_)
						{
							*filtered_shadow_map_2d_tex_array_param_ = filtered_shadow_map_2d_srvs_[0];
							*filtered_shadow_map_2d_light_index_param_ = light_index;
						}
						else
						{
							*filtered_shadow_map_2d_tex_param_ = filtered_shadow_map_2d_srvs_[light_index];
						}
						break;

					case LightSource::LT_Point:
					case LightSource::LT_SphereArea:
					case LightSource::LT_TubeArea:
						shadow_map_camera = light.SMCamera(0).get();
						*filtered_shadow_map_cube_tex_param_ = filtered_shadow_map_cube_srvs_[light_index];
						break;

					case LightSource::LT_Directional:
						{
							shadow_map_camera = lights_[cascaded_shadow_index_]->SMCamera(0).get();
							BOOST_ASSERT(shadow_map_camera);
							KLAYGE_ASSUME(shadow_map_camera);

							*light_view_proj_param_ = pvp.inv_view * shadow_map_camera->ViewProjMatrix();

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

							float4x4 light_view = pvp.inv_view * shadow_map_camera->ViewMatrix();
							*view_z_to_light_view_param_ = light_view.Col(2);
						}
						if (tex_array_support_)
						{
							*filtered_csm_texs_param_[0] = pvp.filtered_csm_srvs[0];
						}
						else
						{
							for (uint32_t i = 0; i < pvp.num_cascades; ++ i)
							{
								*filtered_csm_texs_param_[i] = pvp.filtered_csm_srvs[i];
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
					pvp.projective_shadowing_fb->AttachedRtv(FrameBuffer::Attachment::Color0)->ClearColor(Color(1, 1, 1, 1));
				}
				else
				{
					re.BindFrameBuffer(pvp.shadowing_fb);
					if (shadowing_channel <= 0)
					{
						pvp.shadowing_fb->AttachedRtv(FrameBuffer::Attachment::Color0)->ClearColor(Color(1, 1, 1, 1));
					}
				}

				if (shadow_map_camera != nullptr)
				{
					*esm_scale_factor_param_ = ESM_SCALE_FACTOR / (shadow_map_camera->FarPlane() - shadow_map_camera->NearPlane());
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
		if (pvp.sample_count == 1)
		{
			*depth_tex_param_ = pvp.g_buffer_depth_tex;
			*g_buffer_stencil_tex_param_ = pvp.g_buffer_stencil_srv;
		}
		else
		{
			*g_buffer_depth_tex_ms_param_ = pvp.g_buffer_depth_tex;
			*g_buffer_stencil_tex_ms_param_ = pvp.g_buffer_stencil_srv;
			*multi_sample_mask_tex_param_ = pvp.multi_sample_mask_tex;
		}
		*projective_shadowing_rw_tex_param_ = pvp.projective_shadowing_tex;
		*shadowing_rw_tex_param_ = pvp.shadowing_tex;

		uint32_t w = pvp.shadowing_tex->Width(0);
		uint32_t h = pvp.shadowing_tex->Height(0);
		float2 tile_scale(((w + TILE_SIZE - 1) & ~(TILE_SIZE - 1)) / (2.0f * TILE_SIZE),
			((h + TILE_SIZE - 1) & ~(TILE_SIZE - 1)) / (2.0f * TILE_SIZE));
		*tile_scale_param_ = float4(tile_scale.x(), tile_scale.y(), 0, 0);
		*camera_proj_01_param_ = float2(pvp.proj(0, 0) * tile_scale.x(), pvp.proj(1, 1) * tile_scale.y());

		float const flipping = re.RequiresFlipping() ? -1.0f : +1.0f;
		float3 const upper_left = MathLib::transform_coord(float3(-1, -flipping, 1), pvp.inv_proj);
		float3 const upper_right = MathLib::transform_coord(float3(+1, -flipping, 1), pvp.inv_proj);
		float3 const lower_left = MathLib::transform_coord(float3(-1, flipping, 1), pvp.inv_proj);
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
		uint8_t* filtered_shadow_maps_2d_light_index = filtered_shadow_maps_2d_light_index_param_->MemoryInCBuff<uint8_t>();
		uint8_t* esms_scale_factor = esms_scale_factor_param_->MemoryInCBuff<uint8_t>();

		for (uint32_t li = 0; li < lights_.size(); ++ li)
		{
			auto const & light = *lights_[li];
			int32_t const attr = light.Attrib();
			if (light.Enabled() && (0 == (attr & LightSource::LSA_NoShadow)) && pvp.light_visibles[li])
			{
				LightSource::LightType const type = light.Type();

				Camera* shadow_map_camera = nullptr;

				int32_t const light_index = shadow_map_light_indices_[li].first;
				int32_t const shadowing_channel = shadow_map_light_indices_[li].second;
				if ((light_index >= 0) || (LightSource::LT_Directional == type))
				{
					switch (type)
					{
					case LightSource::LT_Spot:
						shadow_map_camera = light.SMCamera(0).get();
						if (tex_array_support_)
						{
							*filtered_shadow_map_2d_tex_array_param_ = filtered_shadow_map_2d_srvs_[0];
						}
						else
						{
							*filtered_shadow_map_2d_tex_param_ = filtered_shadow_map_2d_srvs_[light_index];
						}
						break;

					case LightSource::LT_Point:
					case LightSource::LT_SphereArea:
					case LightSource::LT_TubeArea:
						shadow_map_camera = light.SMCamera(0).get();
						*filtered_shadow_map_cube_tex_param_ = filtered_shadow_map_cube_srvs_[light_index];
						break;

					case LightSource::LT_Directional:
						{
							shadow_map_camera = lights_[cascaded_shadow_index_]->SMCamera(0).get();

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

							float4x4 light_view = pvp.inv_view * shadow_map_camera->ViewMatrix();
							*view_z_to_light_view_param_ = light_view.Col(2);
						}
						if (tex_array_support_)
						{
							*filtered_csm_texs_param_[0] = pvp.filtered_csm_srvs[0];
						}
						else
						{
							for (uint32_t i = 0; i < pvp.num_cascades; ++ i)
							{
								*filtered_csm_texs_param_[i] = pvp.filtered_csm_srvs[i];
							}
						}
						break;

					default:
						KFL_UNREACHABLE("Invalid light type");
					}
				}
				BOOST_ASSERT(shadow_map_camera);
				KLAYGE_ASSUME(shadow_map_camera);

				*reinterpret_cast<uint32_t*>(lights_type + shadowing_channel * lights_type_param_->Stride()) = type;

				*reinterpret_cast<float4x4*>(lights_view_proj + shadowing_channel * lights_view_proj_param_->Stride())
					= MathLib::transpose(pvp.inv_view * shadow_map_camera->ViewProjMatrix());
				*reinterpret_cast<float*>(esms_scale_factor + shadowing_channel * esms_scale_factor_param_->Stride())
					= ESM_SCALE_FACTOR / (shadow_map_camera->FarPlane() - shadow_map_camera->NearPlane());

				float3 loc_es = MathLib::transform_coord(light.Position(), pvp.view);
				float4 light_pos_es_actived = float4(loc_es.x(), loc_es.y(), loc_es.z(), 1);

				float range = light.Range() * light_scale_;
				AABBox aabb(float3(0, 0, 0), float3(0, 0, 0));
				switch (type)
				{
				case LightSource::LT_Spot:
					{
						light_pos_es_actived.w() = light.CosOuterInner().x();

						*reinterpret_cast<int32_t*>(filtered_shadow_maps_2d_light_index
							+ shadowing_channel * filtered_shadow_maps_2d_light_index_param_->Stride()) = light_index;

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
		filtered_shadow_maps_2d_light_index_param_->CBuffer().Dirty(true);
		esms_scale_factor_param_->CBuffer().Dirty(true);

		re.Dispatch(*dr_effect_, *technique_cldr_shadowing_unified_[pvp.sample_count != 1],
			(w + TILE_SIZE - 1) / TILE_SIZE, (h + TILE_SIZE - 1) / TILE_SIZE, 1);
	}
#endif

#if DEFAULT_DEFERRED == TRIDITIONAL_DEFERRED
	void DeferredRenderingLayer::UpdateLighting(PerViewport const & pvp, LightSource::LightType type, int32_t light_index)
	{
		RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();

		LightSource const & light = *lights_[light_index];
		int32_t shadowing_channel;
		if (0 == (light.Attrib() & LightSource::LSA_NoShadow))
		{
			shadowing_channel = shadow_map_light_indices_[light_index].second;
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

		*g_buffer_rt0_tex_param_ = pvp.g_buffer_rt0_srv;
		*g_buffer_rt1_tex_param_ = pvp.g_buffer_rt1_srv;
		*g_buffer_rt2_tex_param_ = pvp.g_buffer_rt2_srv;
		*depth_tex_param_ = pvp.g_buffer_depth_srv;
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
			re.CurFrameBuffer()->AttachedRtv(FrameBuffer::Attachment::Color0)->Discard();
		}
		re.Render(*dr_effect_, *technique_shading_, *rl_quad_);
	}
#endif

	void DeferredRenderingLayer::MergeIndirectLighting(PerViewport const & pvp, PassTargetBuffer pass_tb)
	{
		if ((indirect_lighting_enabled_ && !(pvp.attrib & VPAM_NoGI)) && (illum_ != 1))
		{
			pvp.il_layer->CalcIndirectLighting(pvp.merged_shading_resolved_texs[!pvp.curr_merged_buffer_index], pvp.proj_to_prev);
			this->AccumulateToLightingTex(pvp, pass_tb);
		}
	}

	void DeferredRenderingLayer::MergeSSVO(PerViewport const & pvp, PassTargetBuffer pass_tb)
	{
		if (pvp.ssvo_enabled && !(pvp.attrib & VPAM_NoSSVO))
		{
			ssvo_pp_->InputPin(0, pvp.g_buffer_resolved_rt0_srv);
			ssvo_pp_->InputPin(1, pvp.g_buffer_resolved_depth_srv);
			ssvo_pp_->OutputPin(0, pvp.small_ssvo_rtv);
			ssvo_pp_->Apply();

			ssvo_blur_pp_->InputPin(0, pvp.small_ssvo_srv);
			ssvo_blur_pp_->InputPin(1, pvp.g_buffer_resolved_depth_srv);
			ssvo_blur_pp_->OutputPin(0, pvp.small_ssvo_rtv);
			ssvo_blur_pp_->Apply();

			ssvo_upsample_pp_->InputPin(0, pvp.small_ssvo_srv);
			ssvo_upsample_pp_->OutputPin(0,
				(PTB_Opaque == pass_tb) ? pvp.merged_shading_rtvs[pvp.curr_merged_buffer_index] : pvp.shading_rtv);
			ssvo_upsample_pp_->Apply();
		}
	}

	void DeferredRenderingLayer::AddTranslucency(uint32_t light_index, PerViewport const & pvp, PassTargetBuffer pass_tb)
	{
		auto const* light = lights_[light_index];
		LightSource::LightType const type = light->Type();
		int32_t const shadow_map_light_index = shadow_map_light_indices_[light_index].first;
		if (light->Enabled() && pvp.light_visibles[light_index] && (0 == (light->Attrib() & LightSource::LSA_NoShadow))
			&& ((shadow_map_light_index >= 0) || (LightSource::LT_Directional == type)))
		{
			auto* trans_pp = translucency_pps_[pvp.sample_count != 1].get();
			Camera* light_camera = nullptr;
			switch (type)
			{
			case LightSource::LT_Spot:
				light_camera = light->SMCamera(0).get();
				trans_pp->InputPin(3, unfiltered_shadow_map_2d_srvs_[shadow_map_light_index]);
				break;

			case LightSource::LT_Directional:
				// TODO
				break;

			default:
				break;
			}

			if (light_camera)
			{
				trans_pp->OutputFrameBuffer()->Attach(pvp.g_buffer_fb->AttachedDsv());
				trans_pp->InputPin(0, pvp.g_buffer_rt0_srv);
				trans_pp->InputPin(1, pvp.g_buffer_rt1_srv);
				trans_pp->InputPin(2, pvp.g_buffer_depth_srv);
				trans_pp->OutputPin(0,
					(PTB_Opaque == pass_tb) ? pvp.merged_shading_rtvs[pvp.curr_merged_buffer_index] : pvp.shading_rtv);

				Camera const& scene_camera = *pvp.frame_buffer->Viewport()->Camera();

				trans_pp->SetParam(0, pvp.inv_view * light_camera->ViewProjMatrix());
				trans_pp->SetParam(1, pvp.inv_view * light_camera->ViewMatrix());
				trans_pp->SetParam(2, pvp.inv_proj);
				trans_pp->SetParam(3, MathLib::transform_coord(light->Position(), pvp.view));
				trans_pp->SetParam(4, float3(light->Color()));
				trans_pp->SetParam(5, light->Falloff());
				trans_pp->SetParam(7, scene_camera.FarPlane());
				trans_pp->SetParam(8, light_camera->FarPlane());
				trans_pp->Apply();
			}
		}
	}

	void DeferredRenderingLayer::AddSSS(PerViewport const & pvp)
	{
		if (!(pvp.attrib & VPAM_NoSSS))
		{
			auto* sss_pp = sss_blur_pps_[pvp.sample_count != 1].get();

			sss_pp->OutputFrameBuffer()->Attach(pvp.g_buffer_fb->AttachedDsv());
			sss_pp->InputPin(0, pvp.merged_shading_srvs[pvp.curr_merged_buffer_index]);
			sss_pp->InputPin(1, pvp.g_buffer_depth_srv);
			sss_pp->OutputPin(0, pvp.merged_shading_rtvs[pvp.curr_merged_buffer_index]);
			sss_pp->Apply();
		}
	}

	void DeferredRenderingLayer::AddSSR(PerViewport const & pvp)
	{
		if (!(pvp.attrib & VPAM_NoSSR))
		{
			auto* ssr_pp = ssr_pps_[pvp.sample_count != 1].get();

			RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();

			pvp.merged_shading_texs[pvp.curr_merged_buffer_index]->CopyToTexture(
				*pvp.merged_shading_resolved_before_ssr_tex, TextureFilter::Point);

			re.BindFrameBuffer(pvp.merged_shading_fbs[pvp.curr_merged_buffer_index]);
			ssr_pp->InputPin(0, pvp.g_buffer_rt0_srv);
			ssr_pp->InputPin(1, pvp.g_buffer_rt1_srv);
			ssr_pp->InputPin(2, pvp.g_buffer_resolved_depth_srv);
			ssr_pp->InputPin(3, pvp.merged_shading_resolved_before_ssr_srv);
			ssr_pp->InputPin(4, pvp.merged_depth_srvs[pvp.curr_merged_buffer_index]);

			ShaderResourceViewPtr skybox_tex;
			ShaderResourceViewPtr skybox_C_tex;
			skylight_y_cube_tex_param_->Value(skybox_tex);
			skylight_c_cube_tex_param_->Value(skybox_C_tex);

			ssr_pp->InputPin(5, skybox_tex);
			ssr_pp->InputPin(6, skybox_C_tex);

			ssr_pp->Apply();
		}
	}

	void DeferredRenderingLayer::AddPPR(PerViewport const& pvp)
	{
		if (!(pvp.attrib & VPAM_NoPPR))
		{
			auto* ppr_pp = ppr_pps_[pvp.sample_count != 1].get();

			auto& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();

			pvp.merged_shading_texs[pvp.curr_merged_buffer_index]->CopyToTexture(
				*pvp.merged_shading_resolved_before_ssr_tex, TextureFilter::Point);

			re.BindFrameBuffer(pvp.merged_shading_fbs[pvp.curr_merged_buffer_index]);
			ppr_pp->InputPin(0, pvp.g_buffer_rt0_srv);
			ppr_pp->InputPin(1, pvp.g_buffer_rt1_srv);
			ppr_pp->InputPin(2, pvp.merged_shading_resolved_before_ssr_srv);
			ppr_pp->InputPin(3, pvp.merged_depth_srvs[pvp.curr_merged_buffer_index]);

			ShaderResourceViewPtr skybox_tex;
			ShaderResourceViewPtr skybox_C_tex;
			skylight_y_cube_tex_param_->Value(skybox_tex);
			skylight_c_cube_tex_param_->Value(skybox_C_tex);

			ppr_pp->InputPin(4, skybox_tex);
			ppr_pp->InputPin(5, skybox_C_tex);

			ppr_pp->Apply();
		}
	}

	void DeferredRenderingLayer::AddVDM(PerViewport const & pvp)
	{
		if (!(pvp.attrib & VPAM_NoVDM))
		{
			RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();

			re.BindFrameBuffer(pvp.merged_shading_fbs[pvp.curr_merged_buffer_index]);
			vdm_composition_pp_->InputPin(0, pvp.vdm_color_srv);
			vdm_composition_pp_->InputPin(1, pvp.vdm_transition_srv);
			vdm_composition_pp_->InputPin(2, pvp.vdm_count_srv);
			vdm_composition_pp_->InputPin(3, pvp.merged_depth_resolved_srvs[pvp.curr_merged_buffer_index]);
			vdm_composition_pp_->Render();
		}
	}

	void DeferredRenderingLayer::AddAtmospheric(PerViewport const & pvp)
	{
		if (!(pvp.attrib & VPAM_NoAtmospheric))
		{
			RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();

			re.BindFrameBuffer(pvp.merged_shading_fbs[pvp.curr_merged_buffer_index]);
			atmospheric_pp_->SetParam(0, pvp.inv_proj);
			atmospheric_pp_->InputPin(0, pvp.merged_depth_srvs[pvp.curr_merged_buffer_index]);
			atmospheric_pp_->Render();
		}
	}

	void DeferredRenderingLayer::AddTAA(PerViewport const & pvp)
	{
		if (!(pvp.attrib & VPAM_NoTAA))
		{
			App3DFramework& app = Context::Instance().AppInstance();
			if ((app.FrameTime() < 1.0f / 30) && taa_enabled_)
			{
				taa_pp_->InputPin(0, pvp.merged_shading_resolved_srvs[!pvp.curr_merged_buffer_index]);
				taa_pp_->Render();
			}
		}
	}

	void DeferredRenderingLayer::MergeShadingAndDepth(PerViewport const & pvp, PassTargetBuffer pass_tb)
	{
		RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();

		if (pass_tb != PTB_Opaque)
		{
			re.BindFrameBuffer(pvp.merged_shading_fbs[pvp.curr_merged_buffer_index]);
			if (pvp.sample_count == 1)
			{
				*shading_tex_param_ = pvp.shading_srv;
			}
			else
			{
				*shading_tex_ms_param_ = pvp.shading_srv;
			}
			re.Render(*dr_effect_, *technique_merge_shading_[pvp.sample_count != 1], *rl_quad_);
		}

		if (pass_tb == PTB_Opaque)
		{
			uint32_t const w = pvp.g_buffer_depth_tex->Width(0);
			uint32_t const h = pvp.g_buffer_depth_tex->Height(0);
			pvp.g_buffer_depth_tex->CopyToSubTexture2D(
				*pvp.merged_depth_texs[pvp.curr_merged_buffer_index], 0, 0, 0, 0, w, h, 0, 0, 0, 0, w, h, TextureFilter::Point);
		}
		else
		{
			re.BindFrameBuffer(pvp.merged_depth_fbs[pvp.curr_merged_buffer_index]);
			if (pvp.sample_count == 1)
			{
				*depth_tex_param_ = pvp.g_buffer_depth_tex;
			}
			else
			{
				*depth_tex_ms_param_ = pvp.g_buffer_depth_tex;
			}
			re.Render(*dr_effect_, *technique_merge_depth_[pvp.sample_count != 1], *rl_quad_);
		}
	}

	void DeferredRenderingLayer::SetupViewportGI(uint32_t vp, bool ssgi_enable)
	{
		PerViewport& pvp = viewports_[vp];
		if (ssgi_enable)
		{
			pvp.il_layer = MakeUniquePtr<SSGILayer>();
		}
		else
		{
			if (rsm_fb_)
			{
				pvp.il_layer = MakeUniquePtr<MultiResSILLayer>();
			}
			else
			{
				pvp.il_layer.reset();
			}
		}

		if (pvp.il_layer && pvp.g_buffer_resolved_rt0_tex && rsm_texs_[0])
		{
			pvp.il_layer->GBuffer(pvp.g_buffer_resolved_rt0_tex, pvp.g_buffer_resolved_rt1_tex,
				pvp.g_buffer_resolved_depth_tex);
			pvp.il_layer->RSM(rsm_texs_[0], rsm_texs_[1], shadow_map_tex_);
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
					cascaded_shadow_layer_ = MakeUniquePtr<SDSMCascadedShadowLayer>();
				}
				else
				{
					cascaded_shadow_layer_ = MakeUniquePtr<PSSMCascadedShadowLayer>();
				}
			}
			break;

		case CSLT_PSSM:
			cascaded_shadow_layer_ = MakeUniquePtr<PSSMCascadedShadowLayer>();
			break;

		case CSLT_SDSM:
		default:
			cascaded_shadow_layer_ = MakeUniquePtr<SDSMCascadedShadowLayer>();
			break;
		}
	}

	void DeferredRenderingLayer::SetViewportCascades(uint32_t vp, uint32_t num_cascades, float pssm_lambda)
	{
		PerViewport& pvp = viewports_[vp];
		pvp.num_cascades = num_cascades;
		if (CSLT_PSSM == cascaded_shadow_layer_->Type())
		{
			checked_cast<PSSMCascadedShadowLayer&>(*cascaded_shadow_layer_).Lambda(pssm_lambda);
		}
	}

	void DeferredRenderingLayer::AccumulateToLightingTex(PerViewport const & pvp, PassTargetBuffer pass_tb)
	{
		PostProcessPtr const & copy_to_light_buffer_pp = (0 == illum_) ? copy_to_light_buffer_pp_ : copy_to_light_buffer_i_pp_;
		copy_to_light_buffer_pp->SetParam(0, indirect_scale_ * 256 / VPL_COUNT);
		copy_to_light_buffer_pp->SetParam(1, float2(1.0f / pvp.g_buffer_resolved_rt0_tex->Width(0),
			1.0f / pvp.g_buffer_resolved_rt0_tex->Height(0)));
		copy_to_light_buffer_pp->SetParam(2, pvp.inv_proj);
		copy_to_light_buffer_pp->InputPin(0, pvp.il_layer->IndirectLightingSrv());
		copy_to_light_buffer_pp->InputPin(1, pvp.g_buffer_resolved_rt0_srv);
		copy_to_light_buffer_pp->InputPin(2, pvp.g_buffer_resolved_rt1_srv);
		copy_to_light_buffer_pp->InputPin(3, pvp.g_buffer_resolved_depth_srv);
		copy_to_light_buffer_pp->OutputPin(0,
			(PTB_Opaque == pass_tb) ? pvp.merged_shading_rtvs[pvp.curr_merged_buffer_index] : pvp.shading_rtv);
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
		int32_t light_index, int32_t index_in_pass, bool is_profile) const
	{
		return (vp_index << 28) | (pass_type << 18) | (light_index << 6) | (index_in_pass << 1) | (is_profile ? 1 : 0);
	}

	void DeferredRenderingLayer::DecomposePassScanCode(uint32_t& vp_index, PassType& pass_type,
		int32_t& light_index, int32_t& index_in_pass, bool& is_profile, uint32_t code) const
	{
		vp_index = code >> 28;				//  4 bits, [31 - 28]
		pass_type = static_cast<PassType>((code >> 18) & 0x03FF);	//  10 bits, [27 - 18]
		light_index = (code >> 6) & 0x0FFF;		// 12 bits, [17 - 6]
		index_in_pass = (code >> 1) & 0x1F;		//  5 bits, [5 -  1]
		is_profile = (code & 1) ? true : false;
	}

	void DeferredRenderingLayer::CreateVDMDepthMaxMap(PerViewport const & pvp)
	{
		for (uint32_t i = 0; i < 2; ++ i)
		{
			auto* depth_to_max_pp = depth_to_max_pps_[(i == 0) ? (pvp.sample_count != 1) : 0].get();

			auto const& input_srv = (0 == i) ? pvp.g_buffer_ds_srv : pvp.g_buffer_vdm_max_ds_srvs[i - 1];
			auto const* input_tex = input_srv->TextureResource().get();

			uint32_t const & w = input_tex->Width(0);
			uint32_t const & h = input_tex->Height(0);
			depth_to_max_pp->SetParam(0, float2(0.5f / w, 0.5f / h));
			depth_to_max_pp->SetParam(1, float2(static_cast<float>((w + 1) & ~1) / w,
				static_cast<float>((h + 1) & ~1) / h));
			depth_to_max_pp->InputPin(0, input_srv);
			// Borrow the small_ssvo_tex
			depth_to_max_pp->OutputPin(0, (0 == i) ? pvp.small_ssvo_rtv : pvp.vdm_color_rtv);
			depth_to_max_pp->OutputFrameBuffer()->Attach(pvp.g_buffer_vdm_max_ds_dsvs[i]);
			depth_to_max_pp->Apply();
		}
	}

#if DEFAULT_DEFERRED == LIGHT_INDEXED_DEFERRED
	void DeferredRenderingLayer::UpdateLightIndexedLighting(PerViewport const & pvp, PassTargetBuffer pass_tb)
	{
		*g_buffer_rt1_tex_param_ = pvp.g_buffer_resolved_rt1_tex;
		*g_buffer_rt2_tex_param_ = pvp.g_buffer_resolved_rt2_tex;
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
		int32_t light_index, PassTargetBuffer pass_tb)
	{
		RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();

		auto const & light = *lights_[light_index];
		int32_t const attr = light.Attrib();
		
		int32_t shadowing_channel;
		if (0 == (attr & LightSource::LSA_NoShadow))
		{
			shadowing_channel = shadow_map_light_indices_[light_index].second;
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

		*g_buffer_rt0_tex_param_ = pvp.g_buffer_rt0_srv;
		*g_buffer_rt1_tex_param_ = pvp.g_buffer_rt1_srv;
		*g_buffer_rt2_tex_param_ = pvp.g_buffer_rt2_srv;
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

		*g_buffer_rt0_tex_param_ = pvp.g_buffer_rt0_srv;
		*g_buffer_rt1_tex_param_ = pvp.g_buffer_rt1_srv;
		*g_buffer_rt2_tex_param_ = pvp.g_buffer_rt2_srv;
		*depth_tex_param_ = pvp.g_buffer_depth_srv;
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
		pvp.light_index_fb->AttachedRtv(FrameBuffer::Attachment::Color0)->ClearColor(Color(0, 0, 0, 0));

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
				channel = shadow_map_light_indices_[*iter].second;
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
				float4x4 light_to_view = light.BoundSceneNode()->TransformToWorld() * pvp.view;
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
		*g_buffer_rt0_tex_param_ = pvp.g_buffer_rt0_srv;
		*g_buffer_rt1_tex_param_ = pvp.g_buffer_rt1_srv;
		*g_buffer_rt2_tex_param_ = pvp.g_buffer_rt2_srv;
		*depth_tex_param_ = pvp.g_buffer_depth_srv;
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
		depth_to_min_max_pp_->InputPin(0, pvp.g_buffer_depth_srv);
		depth_to_min_max_pp_->OutputPin(0, pvp.g_buffer_min_max_depth_rtvs[0]);
		depth_to_min_max_pp_->Apply();

		for (uint32_t i = 1; i < pvp.g_buffer_min_max_depth_texs.size(); ++ i)
		{
			w = pvp.g_buffer_min_max_depth_texs[i - 1]->Width(0);
			h = pvp.g_buffer_min_max_depth_texs[i - 1]->Height(0);
			reduce_min_max_pp_->SetParam(0, float2(0.5f / w, 0.5f / h));
			reduce_min_max_pp_->SetParam(1, float2(static_cast<float>((w + 1) & ~1) / w,
				static_cast<float>((h + 1) & ~1) / h));
			reduce_min_max_pp_->InputPin(0, pvp.g_buffer_min_max_depth_srvs[i - 1]);
			reduce_min_max_pp_->OutputPin(0, pvp.g_buffer_min_max_depth_rtvs[i]);
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

		if (pvp.sample_count == 1)
		{
			*g_buffer_rt0_tex_param_ = pvp.g_buffer_rt0_srv;
			*g_buffer_rt1_tex_param_ = pvp.g_buffer_rt1_srv;
			*g_buffer_rt2_tex_param_ = pvp.g_buffer_rt2_srv;
			*g_buffer_stencil_tex_param_ = pvp.g_buffer_stencil_srv;
			*depth_tex_param_ = pvp.g_buffer_depth_srv;
		}
		else
		{
			*g_buffer_rt0_tex_ms_param_ = pvp.g_buffer_rt0_srv;
			*g_buffer_rt1_tex_ms_param_ = pvp.g_buffer_rt1_srv;
			*g_buffer_rt2_tex_ms_param_ = pvp.g_buffer_rt2_srv;
			*g_buffer_depth_tex_ms_param_ = pvp.g_buffer_depth_srv;
			*g_buffer_stencil_tex_ms_param_ = pvp.g_buffer_stencil_srv;
			*multi_sample_mask_tex_param_ = pvp.multi_sample_mask_tex;
		}

		float const flipping = re.RequiresFlipping() ? -1.0f : +1.0f;
		float3 const upper_left = MathLib::transform_coord(float3(-1, -flipping, 1), pvp.inv_proj);
		float3 const upper_right = MathLib::transform_coord(float3(+1, -flipping, 1), pvp.inv_proj);
		float3 const lower_left = MathLib::transform_coord(float3(-1, flipping, 1), pvp.inv_proj);
		*upper_left_param_ = upper_left;
		*x_dir_param_ = upper_right - upper_left;
		*y_dir_param_ = lower_left - upper_left;

		*inv_width_height_param_ = float2(1.0f / w, 1.0f / h);
		*width_height_param_ = uint2(w, h);

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

					float3 loc_es;
					if (type != LightSource::LT_Ambient)
					{
						float3 const& p = light.Position();
						loc_es = MathLib::transform_coord(p, pvp.view);
					}
					else
					{
						loc_es = float3(0, 0, 0);
					}
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
						shadowing_channel = shadow_map_light_indices_[available_lights[t][i]].second;
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
						float4x4 light_to_view = light.BoundSceneNode()->TransformToWorld() * pvp.view;
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

			bool const first_pass = !available_lights[0].empty();
			if (pvp.sample_count == 1)
			{
				if (typed_uav_)
				{
					*shading_rw_tex_param_
						= (PTB_Opaque == pass_tb) ? pvp.merged_shading_texs[pvp.curr_merged_buffer_index] : pvp.shading_tex;
				}
				else
				{
					if (first_pass)
					{
						*shading_in_tex_param_ = TexturePtr();
						*shading_rw_tex_param_
							= (PTB_Opaque == pass_tb) ? pvp.merged_shading_texs[pvp.curr_merged_buffer_index] : pvp.shading_tex;
					}
					else
					{
						*shading_in_tex_param_
							= (PTB_Opaque == pass_tb) ? pvp.merged_shading_texs[pvp.curr_merged_buffer_index] : pvp.shading_tex;
						*shading_rw_tex_param_ = pvp.temp_shading_tex;
					}
				}
			}
			else
			{
				if (!typed_uav_)
				{
					if (first_pass)
					{
						*shading_in_tex_ms_param_ = TexturePtr();
					}
					else
					{
						*shading_in_tex_ms_param_
							= (PTB_Opaque == pass_tb) ? pvp.merged_shading_texs[pvp.curr_merged_buffer_index] : pvp.shading_tex;
					}
				}

				*shading_rw_tex_array_param_ = pvp.temp_shading_tex_array;
			}

			{
				CameraPtr const& camera = pvp.frame_buffer->Viewport()->Camera();

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
			re.Dispatch(*dr_effect_, *technique_cldr_unified_[pvp.sample_count != 1],
				(w + BLOCK_X - 1) / BLOCK_X, (h + BLOCK_Y - 1) / BLOCK_Y, 1);

			if (pvp.sample_count != 1)
			{
				if (!first_pass && !typed_uav_)
				{
					re.BindFrameBuffer(pvp.temp_shading_fb);
				}
				else
				{
					re.BindFrameBuffer(
						(PTB_Opaque == pass_tb) ? pvp.merged_shading_fbs[pvp.curr_merged_buffer_index] : pvp.shading_fb);
				}
				*src_2d_tex_array_param_ = pvp.temp_shading_tex_array;
				re.Render(*dr_effect_, *technique_array_to_multiSample_, *rl_quad_);
			}

			if (!first_pass && !typed_uav_)
			{
				copy_pps_[pvp.sample_count != 1]->InputPin(0, pvp.temp_shading_srv);
				copy_pps_[pvp.sample_count != 1]->OutputPin(0,
					(PTB_Opaque == pass_tb) ? pvp.merged_shading_rtvs[pvp.curr_merged_buffer_index] : pvp.shading_rtv);
				copy_pps_[pvp.sample_count != 1]->Apply();
			}
		}
	}

	void DeferredRenderingLayer::CreateDepthMinMaxMapCS(PerViewport const & pvp)
	{
		auto& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();

		if (pvp.sample_count != 1)
		{
			depth_to_linear_pps_[1]->InputPin(0, pvp.g_buffer_ds_srv);
			depth_to_linear_pps_[1]->OutputPin(0, pvp.g_buffer_depth_rtv);
			depth_to_linear_pps_[1]->Apply();
		}

		re.BindFrameBuffer(re.DefaultFrameBuffer());
		re.DefaultFrameBuffer()->Discard(FrameBuffer::CBM_Color | FrameBuffer::CBM_Depth | FrameBuffer::CBM_Stencil);

		TexturePtr const & in_tex = pvp.g_buffer_ds_tex;
		TexturePtr const & out_tex = pvp.g_buffer_min_max_depth_texs.back();
		*width_height_param_ = uint2(in_tex->Width(0) - 1, in_tex->Height(0) - 1);
		if (pvp.sample_count == 1)
		{
			*depth_to_tiled_ds_in_tex_param_ = in_tex;
		}
		else
		{
			*depth_to_tiled_linear_depth_in_tex_ms_param_ = pvp.g_buffer_depth_srv;
		}
		*depth_to_tiled_min_max_depth_rw_tex_param_ = out_tex;
		*linear_depth_rw_tex_param_ = pvp.g_buffer_resolved_depth_tex;
		*near_q_far_param_ = pvp.frame_buffer->Viewport()->Camera()->NearQFarParam();

		re.Dispatch(*dr_effect_, *technique_depth_to_tiled_min_max_[pvp.sample_count != 1], out_tex->Width(0), out_tex->Height(0), 1);

		pvp.g_buffer_resolved_depth_tex->BuildMipSubLevels(TextureFilter::Point);
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
		TexturePtr temp_tex = rf.MakeTexture2D(viewports_[0].g_buffer_fb->Width(), viewports_[0].g_buffer_fb->Height(),
			1, 1, EF_ABGR32F, 1, 0, EAH_GPU_Write);
		KLAYGE_TEXTURE_DEBUG_NAME(temp_tex);
		pp->OutputPin(0, rf.Make2DRtv(temp_tex, 0, 1, 0));

		std::string const index_str = std::to_string(index);

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
		pp->OutputPin(0, RenderTargetViewPtr());

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


	uint32_t DeferredRenderingLayer::BeginPerfProfileDRJob(PerfRegion& perf)
	{
		perf.Begin();
		return 0;
	}

	uint32_t DeferredRenderingLayer::EndPerfProfileDRJob(PerfRegion& perf)
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

		for (auto const & node : visible_scene_nodes_)
		{
			node->Pass(pass_type);
		}

		re.ForceLineMode(force_line_mode_);

		CameraPtr const& camera = pvp.frame_buffer->Viewport()->Camera();
		pvp.shadowing_fb->Viewport()->Camera(camera);
		pvp.projective_shadowing_fb->Viewport()->Camera(camera);
		pvp.reflection_fb->Viewport()->Camera(camera);

		this->PreparePVP(pvp);

		depth_to_linear_pps_[pvp.sample_count != 1]->SetParam(0, camera->NearQFarParam());

		*g_buffer_rt0_tex_param_ = pvp.g_buffer_resolved_rt0_srv;
		*depth_tex_param_ = pvp.g_buffer_resolved_depth_srv;
		*inv_width_height_param_ = float2(1.0f / pvp.frame_buffer->Viewport()->Width(),
			1.0f / pvp.frame_buffer->Viewport()->Height());
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
			pvp.il_layer->UpdateGBuffer(*pvp.frame_buffer->Viewport()->Camera());
		}

		if (cascaded_shadow_index_ >= 0)
		{
			Camera const & scene_camera = *pvp.frame_buffer->Viewport()->Camera();
			Camera const & light_camera = *lights_[cascaded_shadow_index_]->SMCamera(0);

			float const BLUR_FACTOR = 0.2f;
			blur_size_light_space_.x() = BLUR_FACTOR * 0.5f * light_camera.ProjMatrix()(0, 0);
			blur_size_light_space_.y() = BLUR_FACTOR * 0.5f * light_camera.ProjMatrix()(1, 1);

			float3 cascade_border(blur_size_light_space_.x(), blur_size_light_space_.y(), light_camera.ProjMatrix()(2, 2));
			cascaded_shadow_layer_->NumCascades(pvp.num_cascades);
			if (CSLT_SDSM == cascaded_shadow_layer_->Type())
			{
				checked_cast<SDSMCascadedShadowLayer&>(*cascaded_shadow_layer_).DepthTexture(pvp.g_buffer_resolved_depth_tex);
			}
			cascaded_shadow_layer_->UpdateCascades(scene_camera, light_camera.ViewProjMatrix(), cascade_border);
		}

		if (!decals_.empty())
		{
			this->RenderDecals(pvp, PT_OpaqueGBuffer);
		}

		return 0;
	}

	uint32_t DeferredRenderingLayer::ShadowMapGenerationDRJob(PerViewport const & pvp, PassType pass_type, int32_t light_index,
		int32_t index_in_pass)
	{
		auto& rf = Context::Instance().RenderFactoryInstance();
		auto& re = rf.RenderEngineInstance();
		auto& scene_mgr = Context::Instance().SceneManagerInstance();

		for (auto const & node : visible_scene_nodes_)
		{
			node->Pass(pass_type);
		}

		if (index_in_pass > 0)
		{
			this->PostGenerateShadowMap(pvp, light_index, index_in_pass);
		}

		auto const& light = *lights_[light_index];
		auto const light_type = light.Type();

		int32_t last_pass_index;
		switch (light_type)
		{
		case LightSource::LT_Point:
		case LightSource::LT_SphereArea:
		case LightSource::LT_TubeArea:
			last_pass_index = tex_array_support_ ? 1 : 6;
			break;

		case LightSource::LT_Spot:
			last_pass_index = 1;
			break;

		case LightSource::LT_Directional:
			last_pass_index = static_cast<int32_t>(pvp.num_cascades);
			break;

		default:
			KFL_UNREACHABLE("Invalid light type");
		}

		curr_cascade_index_ = -1;

		uint32_t urv;
		if (index_in_pass == last_pass_index)
		{
			urv = 0;
		}
		else
		{
			urv = App3DFramework::URV_NeedFlush | App3DFramework::URV_OpaqueOnly;

			PassCategory const pass_cat = GetPassCategory(pass_type);
			if ((light_type == LightSource::LT_Directional) && (pass_cat != PC_Shadowing) && (pass_cat != PC_Shading))
			{
				curr_cascade_index_ = index_in_pass;
			}

			scene_mgr.SmallObjectThreshold(0.002f);

			auto const& shadow_map_camera = light.SMCamera(
				((light_type == LightSource::LT_Spot) || (light_type == LightSource::LT_Directional)) ? 0 : index_in_pass);

			switch (GetPassRT(pass_type))
			{
			case PRT_ShadowMap:
				shadow_map_fb_->Viewport()->Camera(shadow_map_camera);
				re.BindFrameBuffer(shadow_map_fb_);
				shadow_map_fb_->AttachedRtv(FrameBuffer::Attachment::Color0)->Discard();
				shadow_map_fb_->AttachedDsv()->ClearDepth(1.0f);
				break;

			case PRT_ShadowMapMultiView:
				for (uint32_t i = 0; i < 6; ++i)
				{
					shadow_map_array_fb_->Viewport()->Camera(i, light.SMCamera(i));
				}
				re.BindFrameBuffer(shadow_map_array_fb_);
				shadow_map_array_fb_->AttachedRtv(FrameBuffer::Attachment::Color0)->Discard();
				shadow_map_array_fb_->AttachedDsv()->ClearDepth(1.0f);
				break;

			case PRT_CascadedShadowMap:
				csm_fb_->Viewport()->Camera(shadow_map_camera);
				re.BindFrameBuffer(csm_fb_);
				csm_fb_->Clear(FrameBuffer::CBM_Color | FrameBuffer::CBM_Depth, Color(shadow_map_camera->FarPlane(), 0, 0, 0), 1.0f, 0);
				break;

			case PRT_ReflectiveShadowMap:
				rsm_fb_->Viewport()->Camera(shadow_map_camera);
				re.BindFrameBuffer(rsm_fb_);
				rsm_fb_->AttachedRtv(FrameBuffer::Attachment::Color0)->Discard();
				rsm_fb_->AttachedRtv(FrameBuffer::Attachment::Color1)->Discard();
				rsm_fb_->AttachedDsv()->ClearDepthStencil(1.0f, 0);
				break;

			default:
				KFL_UNREACHABLE("Invalid pass render target");
			}
		}

		return urv;
	}

	uint32_t DeferredRenderingLayer::IndirectLightingDRJob(PerViewport const & pvp, int32_t light_index)
	{
		depth_to_esm_pp_->InputPin(0, shadow_map_depth_srv_);

		auto const& shadow_map_camera = rsm_fb_->Viewport()->Camera();
		depth_to_esm_pp_->SetParam(0, shadow_map_camera->NearQFarParam());
		depth_to_esm_pp_->SetParam(1, shadow_map_camera->InverseProjMatrix());
		depth_to_esm_pp_->Apply();

		pvp.il_layer->UpdateRSM(*shadow_map_camera, *lights_[light_index]);

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
#elif DEFAULT_DEFERRED == TRIDITIONAL_DEFERRED
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

		for (auto const & node : visible_scene_nodes_)
		{
			node->Pass(pass_type);
		}

		re.BindFrameBuffer(pvp.reflection_fb);
		re.CurFrameBuffer()->AttachedDsv()->ClearDepth(1.0f);
		return App3DFramework::URV_NeedFlush | App3DFramework::URV_ReflectionOnly | (App3DFramework::URV_OpaqueOnly << pass_tb);
	}

	uint32_t DeferredRenderingLayer::VDMDRJob(PerViewport const & pvp)
	{
		auto& rf = Context::Instance().RenderFactoryInstance();
		auto& re = rf.RenderEngineInstance();

		BOOST_ASSERT(has_vdm_objs_);

		for (auto const & node : visible_scene_nodes_)
		{
			if (node->VDM())
			{
				node->Pass(PT_VDM);
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

		for (auto const & node : visible_scene_nodes_)
		{
			node->Pass(pass_type);
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

		if (has_ssr_objs_ && ssr_enabled_)
		{
#ifndef KLAYGE_SHIP
			ssr_pp_perf_->Begin();
#endif
			this->AddSSR(pvp);
#ifndef KLAYGE_SHIP
			ssr_pp_perf_->End();
#endif
		}

		if (has_ppr_objs_ && ppr_enabled_)
		{
#ifndef KLAYGE_SHIP
			ppr_pp_perf_->Begin();
#endif
			this->AddPPR(pvp);
#ifndef KLAYGE_SHIP
			ppr_pp_perf_->End();
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

		return 0;
	}

	uint32_t DeferredRenderingLayer::SimpleForwardDRJob(PerViewport& pvp)
	{
		auto& rf = Context::Instance().RenderFactoryInstance();
		auto& re = rf.RenderEngineInstance();

		re.BindFrameBuffer(pvp.merged_shading_fbs[pvp.curr_merged_buffer_index]);
		for (auto const & node : visible_scene_nodes_)
		{
			if (node->SimpleForward())
			{
				node->Pass(PT_SimpleForward);
			}
		}

		return App3DFramework::URV_NeedFlush | App3DFramework::URV_SimpleForwardOnly;
	}

	uint32_t DeferredRenderingLayer::PostSimpleForwardDRJob(PerViewport& pvp)
	{
		uint32_t const index = (pvp.sample_count != 1);
		depth_to_linear_pps_[index]->InputPin(0, pvp.g_buffer_ds_srv);
		depth_to_linear_pps_[index]->OutputPin(0, pvp.merged_depth_rtvs[pvp.curr_merged_buffer_index]);
		depth_to_linear_pps_[index]->Apply();

		pvp.g_buffer_depth_tex->BuildMipSubLevels(TextureFilter::Point);

		return 0;
	}

	uint32_t DeferredRenderingLayer::FinishingViewportDRJob(PerViewport& pvp)
	{
		auto& rf = Context::Instance().RenderFactoryInstance();
		auto& re = rf.RenderEngineInstance();

#if DEFAULT_DEFERRED == LIGHT_INDEXED_DEFERRED
		if (pvp.sample_count != 1)
		{
			pvp.merged_shading_texs[pvp.curr_merged_buffer_index]->CopyToTexture(
				*pvp.merged_shading_resolved_texs[pvp.curr_merged_buffer_index], TextureFilter::Point);

			// Borrow g_buffer_ds_tex_ms_param_
			*g_buffer_ds_tex_ms_param_ = pvp.merged_depth_srvs[pvp.curr_merged_buffer_index];

			re.BindFrameBuffer(pvp.merged_depth_resolved_fbs[pvp.curr_merged_buffer_index]);
			re.Render(*dr_effect_, *technique_resolve_merged_depth_, *rl_quad_);
		}
#endif

		auto color_srv = pvp.merged_shading_resolved_srvs[pvp.curr_merged_buffer_index];
		auto const& depth_srv = pvp.merged_depth_resolved_srvs[pvp.curr_merged_buffer_index];

		if (!(pvp.attrib & VPAM_NoDoF) && depth_of_field_enabled_)
		{
			depth_of_field_pp_->InputPin(0, color_srv);
			depth_of_field_pp_->InputPin(1, depth_srv);
			depth_of_field_pp_->OutputPin(0, pvp.dof_rtv);
			depth_of_field_pp_->Apply();

			if (bokeh_filter_enabled_)
			{
				bokeh_filter_pp_->InputPin(0, color_srv);
				bokeh_filter_pp_->InputPin(1, depth_srv);
				bokeh_filter_pp_->OutputPin(0, pvp.dof_rtv);
				bokeh_filter_pp_->Apply();
			}

			color_srv = pvp.dof_srv;
		}

		if (!(pvp.attrib & VPAM_NoMotionBlur) && motion_blur_enabled_)
		{
			motion_blur_pp_->InputPin(0, color_srv);
			motion_blur_pp_->InputPin(1, depth_srv);
			motion_blur_pp_->InputPin(2, pvp.g_buffer_rt2_srv);
			motion_blur_pp_->OutputPin(0, pvp.motion_blur_rtv);
			motion_blur_pp_->Apply();

			color_srv = pvp.motion_blur_srv;
		}

		re.BindFrameBuffer(pvp.frame_buffer);
		pvp.frame_buffer->Discard(FrameBuffer::CBM_Color | FrameBuffer::CBM_Depth | FrameBuffer::CBM_Stencil);
		{
			*depth_tex_param_ = depth_srv;

			Camera const& camera = *pvp.frame_buffer->Viewport()->Camera();
			float q = camera.FarPlane() / (camera.FarPlane() - camera.NearPlane());
			float2 near_q(camera.NearPlane() * q, q);
			*near_q_param_ = near_q;

			*shading_tex_param_ = color_srv;
			re.Render(*dr_effect_, *technique_copy_shading_depth_, *rl_quad_);
		}

		pvp.curr_merged_buffer_index = !pvp.curr_merged_buffer_index;

		return 0;
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
		re.CurFrameBuffer()->AttachedDsv()->ClearDepth(1.0f);
		dr_debug_pp_->Apply();

		return App3DFramework::URV_SkipPostProcess | App3DFramework::URV_Finished;
	}

	uint32_t DeferredRenderingLayer::VisualizeLightingDRJob()
	{
		auto& rf = Context::Instance().RenderFactoryInstance();
		auto& re = rf.RenderEngineInstance();

		re.BindFrameBuffer(FrameBufferPtr());
		re.CurFrameBuffer()->AttachedDsv()->ClearDepth(1.0f);
		dr_debug_pp_->Apply();

		return App3DFramework::URV_Finished | App3DFramework::URV_SkipPostProcess;
	}

	uint32_t DeferredRenderingLayer::ClearOnlyDRJob()
	{
		auto& rf = Context::Instance().RenderFactoryInstance();
		auto& re = rf.RenderEngineInstance();

		re.BindFrameBuffer(FrameBufferPtr());
		re.CurFrameBuffer()->Clear(FrameBuffer::CBM_Color | FrameBuffer::CBM_Depth, Color(0, 0, 0, 1), 1, 0);

		return App3DFramework::URV_Finished;
	}
}
