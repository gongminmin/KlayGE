// InfTerrain.cpp
// KlayGE Infinite Terrain implement file
// Ver 3.11.0
// Copyright(C) Minmin Gong, 2010
// Homepage: http://www.klayge.org
//
// 3.11.0
// First release (2010.8.21)
//
// CHANGE LIST
/////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KFL/Util.hpp>
#include <KlayGE/RenderLayout.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/RenderEffect.hpp>
#include <KlayGE/Camera.hpp>
#include <KlayGE/PostProcess.hpp>
#include <KlayGE/FrameBuffer.hpp>
#include <KFL/Half.hpp>

#include <iterator>

#include <KlayGE/InfTerrain.hpp>

namespace KlayGE
{
	InfTerrainRenderable::InfTerrainRenderable(std::wstring_view name, uint32_t num_grids, float stride, float increate_rate)
		: Renderable(name)
	{
		RenderFactory& rf = Context::Instance().RenderFactoryInstance();

		rls_[0] = rf.MakeRenderLayout();
		rls_[0]->TopologyType(RenderLayout::TT_TriangleList);

		RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
		Camera const& camera = *re.DefaultFrameBuffer()->Viewport()->Camera();

		float far_plane = camera.FarPlane();

		float angle = atan(tan(camera.FOV() / 2) * camera.Aspect());
		x_dir_ = float2(-sin(angle), cos(angle));
		y_dir_ = float2(-x_dir_.x(), x_dir_.y());

		float2 addr(0, 0);
		float2 increment(stride, stride);
		std::vector<float2> vertices;
		for (uint32_t y = 0; y < num_grids - 1; ++ y, addr.y() += increment.y())
		{
			increment.x() = stride;
			addr.x() = 0;
			for (uint32_t x = 0; x < num_grids - 1; ++ x, addr.x() += increment.x())
			{
				vertices.emplace_back(addr.x() * x_dir_ * 0.5f + addr.y() * y_dir_ * 0.5f);
				increment.x() *= increate_rate;
			}
			vertices.emplace_back((addr.x() + far_plane) * x_dir_ * 0.5f + addr.y() * y_dir_ * 0.5f);

			increment.y() *= increate_rate;
		}
		{
			increment.x() = stride;
			addr.x() = 0;
			for (uint32_t x = 0; x < num_grids - 1; ++ x, addr.x() += increment.x())
			{
				vertices.emplace_back(addr.x() * x_dir_ * 0.5f + (addr.y() + far_plane) * y_dir_ * 0.5f);
				increment.x() *= increate_rate;
			}
			vertices.emplace_back((addr.x() + far_plane) * x_dir_ * 0.5f + (addr.y() + far_plane) * y_dir_ * 0.5f);
		}

		GraphicsBufferPtr pos_vb = rf.MakeVertexBuffer(BU_Static, EAH_GPU_Read | EAH_Immutable,
			static_cast<uint32_t>(vertices.size() * sizeof(vertices[0])), &vertices[0]);
		rls_[0]->BindVertexStream(pos_vb, VertexElement(VEU_Position, 0, EF_GR32F));

		std::vector<uint32_t> indices;
		for (uint32_t y = 0; y < num_grids - 1; ++ y)
		{
			for (uint32_t x = 0; x < num_grids - 1; ++ x)
			{
				indices.push_back((y + 0) * num_grids + (x + 0));
				indices.push_back((y + 0) * num_grids + (x + 1));
				indices.push_back((y + 1) * num_grids + (x + 0));

				indices.push_back((y + 1) * num_grids + (x + 0));
				indices.push_back((y + 0) * num_grids + (x + 1));
				indices.push_back((y + 1) * num_grids + (x + 1));
			}
		}

		GraphicsBufferPtr ib = rf.MakeIndexBuffer(BU_Static, EAH_GPU_Read | EAH_Immutable,
			static_cast<uint32_t>(indices.size() * sizeof(indices[0])), &indices[0]);
		rls_[0]->BindIndexStream(ib, EF_R32UI);
	}

	void InfTerrainRenderable::SetStretch(float stretch)
	{
		*(effect_->ParameterByName("stretch")) = stretch;
	}

	void InfTerrainRenderable::SetBaseLevel(float base_level)
	{
		*(effect_->ParameterByName("base_level")) = base_level;
	}

	void InfTerrainRenderable::OffsetY(float y)
	{
		*(effect_->ParameterByName("offset_y")) = y;
	}

	void InfTerrainRenderable::OnRenderBegin()
	{
		Context& context = Context::Instance();
		RenderEngine& re = context.RenderFactoryInstance().RenderEngineInstance();
		Camera const& camera = *re.DefaultFrameBuffer()->Viewport()->Camera();

		if (context.DeferredRenderingLayerValid())
		{
			Renderable::OnRenderBegin();
		}
		else
		{
			*(effect_->ParameterByName("mvp")) = camera.ViewProjMatrix();
		}

		float3 look_at_vec = float3(camera.LookAt().x() - camera.EyePos().x(), 0, camera.LookAt().z() - camera.EyePos().z());
		if (MathLib::dot(look_at_vec, look_at_vec) < 1e-6f)
		{
			look_at_vec = float3(0, 0, 1);
		}
		float4x4 virtual_view = MathLib::look_at_lh(camera.EyePos(), camera.EyePos() + look_at_vec);
		float4x4 inv_virtual_view = MathLib::inverse(virtual_view);

		*(effect_->ParameterByName("inv_virtual_view")) = inv_virtual_view;
		*(effect_->ParameterByName("culling_eye_pos")) = camera.EyePos();
	}


	InfTerrainRenderableComponent::InfTerrainRenderableComponent(RenderablePtr const& renderable) : RenderableComponent(renderable)
	{
		this->OnMainThreadUpdate().Connect(
			[](SceneComponent& component, [[maybe_unused]] float app_time, [[maybe_unused]] float elapsed_time)
			{
				auto& inf_terrain = checked_cast<InfTerrainRenderableComponent&>(component);
				auto& inf_terrain_renderable = inf_terrain.BoundRenderableOfType<InfTerrainRenderable>();

				RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
				Camera const& camera = *re.DefaultFrameBuffer()->Viewport()->Camera();

				float3 look_at_vec = float3(camera.LookAt().x() - camera.EyePos().x(), 0, camera.LookAt().z() - camera.EyePos().z());
				if (MathLib::dot(look_at_vec, look_at_vec) < 1e-6f)
				{
					look_at_vec = float3(0, 0, 1);
				}
				float4x4 virtual_view = MathLib::look_at_lh(camera.EyePos(), camera.EyePos() + look_at_vec);

				float4x4 proj_to_virtual_view = camera.InverseViewProjMatrix() * virtual_view;

				float2 const & x_dir_2d = inf_terrain_renderable.XDir();
				float2 const & y_dir_2d = inf_terrain_renderable.YDir();
				float3 x_dir(x_dir_2d.x(), -camera.EyePos().y(), x_dir_2d.y());
				float3 y_dir(y_dir_2d.x(), -camera.EyePos().y(), y_dir_2d.y());

				float3 const frustum[8] =
				{
					MathLib::transform_coord(float3(-1, +1, 1), proj_to_virtual_view),
					MathLib::transform_coord(float3(+1, +1, 1), proj_to_virtual_view),
					MathLib::transform_coord(float3(-1, -1, 1), proj_to_virtual_view),
					MathLib::transform_coord(float3(+1, -1, 1), proj_to_virtual_view),
					MathLib::transform_coord(float3(-1, +1, 0), proj_to_virtual_view),
					MathLib::transform_coord(float3(+1, +1, 0), proj_to_virtual_view),
					MathLib::transform_coord(float3(-1, -1, 0), proj_to_virtual_view),
					MathLib::transform_coord(float3(+1, -1, 0), proj_to_virtual_view)
				};

				int const view_cube[24] =
				{
					0, 1, 1, 3, 3, 2, 2, 0,
					4, 5, 5, 7, 7, 6, 6, 4,
					0, 4, 1, 5, 3, 7, 2, 6
				};

				Plane const lower_bound = MathLib::from_point_normal(
					float3(0, inf_terrain.base_level_ - camera.EyePos().y() - inf_terrain.strength_, 0), float3(0, 1, 0));

				bool intersect = false;
				float sy = 0;
				for (int i = 0; i < 12; ++ i)
				{
					int src = view_cube[i * 2 + 0];
					int dst = view_cube[i * 2 + 1];
					if (MathLib::dot_coord(lower_bound, frustum[src]) / MathLib::dot_coord(lower_bound, frustum[dst]) < 0)
					{
						float t = MathLib::intersect_ray(lower_bound, frustum[src], frustum[dst] - frustum[src]);
						float3 p = MathLib::lerp(frustum[src], frustum[dst], t);
						sy = std::max(sy, std::max((x_dir.z() * p.x() - x_dir.x() * p.z()) / x_dir.x(),
							(y_dir.z() * p.x() - y_dir.x() * p.z()) / y_dir.x()));
						intersect = true;
					}
				}
				inf_terrain_renderable.OffsetY(sy);

				component.Enabled(intersect);
			});
	}


	int const COARSE_HEIGHT_MAP_SIZE = 1024;
	uint32_t const VERTEX_PER_TILE_EDGE = 9;				// overlap => -2
	uint32_t const PATCHES_PER_TILE_EDGE = VERTEX_PER_TILE_EDGE - 1;
	uint32_t const NON_TESS_INDEX_COUNT = (VERTEX_PER_TILE_EDGE - 1) * (2 * VERTEX_PER_TILE_EDGE + 2);
	uint32_t const TESS_INDEX_COUNT = (VERTEX_PER_TILE_EDGE - 1) * (VERTEX_PER_TILE_EDGE - 1) * 4;
	[[maybe_unused]] uint32_t const MAX_RINGS = 10;

	HQTerrainRenderable::TileRing::TileRing(int hole_width, int outer_width, float tile_size,
		GraphicsBufferPtr const & tile_non_tess_ib, GraphicsBufferPtr const & tile_non_tess_vid_vb,
		GraphicsBufferPtr const & tile_tess_ib)
		: tile_non_tess_ib_(tile_non_tess_ib), tile_non_tess_vid_vb_(tile_non_tess_vid_vb),
			tile_tess_ib_(tile_tess_ib),
			hole_width_(hole_width), outer_width_(outer_width),
			ring_width_((outer_width - hole_width) / 2),
			num_tiles_(outer_width * outer_width - hole_width * hole_width),
			tile_size_(tile_size)
	{
		BOOST_ASSERT(0 == ((outer_width - hole_width) % 2));

		this->CreateInstanceDataVB();
	}

	bool HQTerrainRenderable::TileRing::InRing(int x, int y) const
	{
		BOOST_ASSERT((x >= 0) && (x < outer_width_));
		BOOST_ASSERT((y >= 0) && (y < outer_width_));

		return (x < ring_width_) || (y < ring_width_) || (x >= outer_width_ - ring_width_)
			|| (y >= outer_width_ - ring_width_);
	}

	void HQTerrainRenderable::TileRing::AssignNeighbourSizes(int x, int y, Adjacency& adj) const
	{
		adj.neighbor_minus_x = 1;
		adj.neighbor_minus_y = 1;
		adj.neighbor_plus_x = 1;
		adj.neighbor_plus_y = 1;

		// TBD: these aren't necessarily 2x different. Depends on the relative tiles sizes supplied to ring ctors.
		float const inner_neighbour_size = 0.5f;
		float const outer_neighbour_size = 2.0f;

		// Inner edges abut tiles that are smaller. (But not on the inner-most.)
		if (hole_width_ > 0)
		{
			if ((y >= ring_width_) && (y < outer_width_ - ring_width_))
			{
				if (x == ring_width_ - 1)
				{
					adj.neighbor_plus_x = inner_neighbour_size;
				}
				if (x == outer_width_ - ring_width_)
				{
					adj.neighbor_minus_x = inner_neighbour_size;
				}
			}
			if ((x >= ring_width_) && (x < outer_width_ - ring_width_))
			{
				if (y == ring_width_ - 1)
				{
					adj.neighbor_plus_y = inner_neighbour_size;
				}
				if (y == outer_width_ - ring_width_)
				{
					adj.neighbor_minus_y = inner_neighbour_size;
				}
			}
		}

		// Outer edges abut tiles that are larger. We could skip this on the outer-most ring. But it will
		// make almost zero visual or perf difference.
		if (0 == x)
		{
			adj.neighbor_minus_x = outer_neighbour_size;
		}
		if (0 == y)
		{
			adj.neighbor_minus_y = outer_neighbour_size;
		}
		if (x == outer_width_ - 1)
		{
			adj.neighbor_plus_x = outer_neighbour_size;
		}
		if (y == outer_width_ - 1)
		{
			adj.neighbor_plus_y = outer_neighbour_size;
		}
	}

	void HQTerrainRenderable::TileRing::CreateInstanceDataVB()
	{
		int index = 0;
		std::vector<InstanceData> vb_data(num_tiles_);

		float const half_width = 0.5f * outer_width_;
		for (int y = 0; y < outer_width_; ++ y)
		{
			for (int x = 0; x < outer_width_; ++ x)
			{
				if (this->InRing(x, y))
				{
					vb_data[index].x = tile_size_ * (x - half_width);
					vb_data[index].y = tile_size_ * (y - half_width);
					this->AssignNeighbourSizes(x, y, vb_data[index].adjacency);
					++ index;
				}
			}
		}
		BOOST_ASSERT(index == num_tiles_);

		RenderFactory& rf = Context::Instance().RenderFactoryInstance();
		vb_ = rf.MakeVertexBuffer(BU_Static, EAH_GPU_Read | EAH_Immutable, num_tiles_ * sizeof(InstanceData), &vb_data[0]);

		tile_non_tess_rl_ = rf.MakeRenderLayout();
		tile_non_tess_rl_->TopologyType(RenderLayout::TT_TriangleStrip);
		tile_non_tess_rl_->BindIndexStream(tile_non_tess_ib_, EF_R16UI);
		tile_non_tess_rl_->NumIndices(NON_TESS_INDEX_COUNT);
		tile_non_tess_rl_->BindVertexStream(vb_,
			MakeSpan({VertexElement(VEU_TextureCoord, 0, EF_GR32F), VertexElement(VEU_TextureCoord, 1, EF_ABGR32F)}),
			RenderLayout::ST_Instance);
		tile_non_tess_rl_->BindVertexStream(tile_non_tess_vid_vb_, VertexElement(VEU_TextureCoord, 2, EF_R32F));
		tile_non_tess_rl_->NumInstances(num_tiles_);

		tile_tess_rl_ = rf.MakeRenderLayout();
		tile_tess_rl_->TopologyType(RenderLayout::TT_4_Ctrl_Pt_PatchList);
		tile_tess_rl_->BindIndexStream(tile_tess_ib_, EF_R16UI);
		tile_tess_rl_->NumIndices(TESS_INDEX_COUNT);
		tile_tess_rl_->BindVertexStream(vb_,
			MakeSpan({VertexElement(VEU_TextureCoord, 0, EF_GR32F), VertexElement(VEU_TextureCoord, 1, EF_ABGR32F)}),
			RenderLayout::ST_Instance);
		tile_tess_rl_->NumInstances(num_tiles_);
	}


	HQTerrainRenderable::HQTerrainRenderable(RenderEffectPtr const & effect,
			float world_scale, float vertical_scale, int world_uv_repeats)
		: Renderable(L"HQTerrain"),
			world_scale_(world_scale), vertical_scale_(vertical_scale), world_uv_repeats_(world_uv_repeats),
			ridge_octaves_(3), fBm_octaves_(3), tex_twist_octaves_(1), detail_noise_scale_(0.02f),
			tessellated_tri_size_(6), wireframe_(false), show_patches_(false), show_tiles_(false)
	{
		RenderFactory& rf = Context::Instance().RenderFactoryInstance();
		RenderEngine& re = rf.RenderEngineInstance();

		hw_tessellation_ = re.DeviceCaps().ds_support;

		this->CreateNonTessIB();
		this->CreateNonTessVIDVB();
		this->CreateTessIB();

		int widths[] = { 0, 16, 16, 16, 16 };
		uint32_t const rings = static_cast<uint32_t>(std::size(widths)) - 1;
		BOOST_ASSERT(rings <= MAX_RINGS);

		tile_rings_.resize(rings);
		float tile_width = 0.125f;
		for (uint32_t i = 0; i < rings; ++ i)
		{
			tile_rings_[i] = MakeUniquePtr<TileRing>(widths[i] / 2, widths[i + 1], tile_width,
				tile_non_tess_ib_, tile_non_tess_vid_vb_, tile_tess_ib_);
			tile_width *= 2.0f;
		}

		snap_grid_size_ = world_scale_ * tile_rings_.back()->TileSize() / PATCHES_PER_TILE_EDGE;

		this->BindDeferredEffect(effect);
	}

	void HQTerrainRenderable::Tessellation(bool tess)
	{
		RenderFactory& rf = Context::Instance().RenderFactoryInstance();
		RenderEngine& re = rf.RenderEngineInstance();

		hw_tessellation_ = re.DeviceCaps().ds_support & tess;
		this->UpdateTechniques();
	}

	void HQTerrainRenderable::ShowPatches(bool sp)
	{
		show_patches_ = sp;
	}

	void HQTerrainRenderable::ShowTiles(bool st)
	{
		show_tiles_ = st;
	}

	void HQTerrainRenderable::Wireframe(bool wf)
	{
		wireframe_ = wf;
		this->UpdateTechniques();
	}

	void HQTerrainRenderable::DetailNoiseScale(float scale)
	{
		detail_noise_scale_ = scale;
	}

	void HQTerrainRenderable::TessellatedTriSize(int size)
	{
		tessellated_tri_size_ = size;
	}

	void HQTerrainRenderable::Render()
	{
		this->OnRenderBegin();
		this->RenderTerrain();
		this->OnRenderEnd();
	}

	void HQTerrainRenderable::ModelMatrix([[maybe_unused]] float4x4 const & mat)
	{
		// Calculate matrix in SetMatrices.
	}

	void HQTerrainRenderable::UpdateTechniques()
	{
		Renderable::UpdateTechniques();

		auto* deferred_effect = effect_.get();

		auto& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
		if (re.DeviceCaps().vp_rt_index_at_every_stage_support)
		{
			terrain_gbuffer_techs_[0] = deferred_effect->TechniqueByName("GBufferTessTerrainFillTech");
			terrain_gbuffer_techs_[1] = deferred_effect->TechniqueByName("GBufferTessTerrainLineTech");
			terrain_gbuffer_techs_[2] = deferred_effect->TechniqueByName("GBufferNoTessTerrainFillTech");
			terrain_gbuffer_techs_[3] = deferred_effect->TechniqueByName("GBufferNoTessTerrainLineTech");
			gen_shadow_map_tech_ = deferred_effect->TechniqueByName("GenNoTessTerrainShadowMapTech");
			gen_csm_tech_ = deferred_effect->TechniqueByName("GenNoTessTerrainCascadedShadowMapTech");
			gen_rsm_tech_ = deferred_effect->TechniqueByName("GenNoTessTerrainReflectiveShadowMapTech");
		}
		else
		{
			terrain_gbuffer_techs_[0] = deferred_effect->TechniqueByName("GBufferTessTerrainFillNoVpRtTech");
			terrain_gbuffer_techs_[1] = deferred_effect->TechniqueByName("GBufferTessTerrainLineNoVpRtTech");
			terrain_gbuffer_techs_[2] = deferred_effect->TechniqueByName("GBufferNoTessTerrainFillNoVpRtTech");
			terrain_gbuffer_techs_[3] = deferred_effect->TechniqueByName("GBufferNoTessTerrainLineNoVpRtTech");
			gen_shadow_map_tech_ = deferred_effect->TechniqueByName("GenNoTessTerrainShadowMapNoVpRtTech");
			gen_csm_tech_ = deferred_effect->TechniqueByName("GenNoTessTerrainCascadedShadowMapNoVpRtTech");
			gen_rsm_tech_ = deferred_effect->TechniqueByName("GenNoTessTerrainReflectiveShadowMapNoVpRtTech");
		}
		gen_shadow_map_multi_view_tech_ = gen_shadow_map_tech_;
		gen_csm_multi_view_tech_ = gen_csm_tech_;
		gen_rsm_multi_view_tech_ = gen_rsm_tech_;

		*deferred_effect->ParameterByName("vertex_per_tile_edge") = static_cast<int32_t>(VERTEX_PER_TILE_EDGE);
		*deferred_effect->ParameterByName("patches_per_tile_edge") = int2(PATCHES_PER_TILE_EDGE, PATCHES_PER_TILE_EDGE - 1);
		*deferred_effect->ParameterByName("inv_patches_per_tile_edge") = 1.0f / PATCHES_PER_TILE_EDGE;
		*deferred_effect->ParameterByName("inv_vertex_per_tile_edge") = 1.0f / VERTEX_PER_TILE_EDGE;
		*deferred_effect->ParameterByName("world_scale") = world_scale_;
		*deferred_effect->ParameterByName("vertical_scale") = float2(vertical_scale_, world_scale_ * vertical_scale_);
		*deferred_effect->ParameterByName("world_uv_repeats") = float2(static_cast<float>(world_uv_repeats_), 1.0f / world_uv_repeats_);

		height_map_param_ = deferred_effect->ParameterByName("coarse_height_map");
		gradient_map_param_ = deferred_effect->ParameterByName("coarse_gradient_map");
		mask_map_param_ = deferred_effect->ParameterByName("coarse_mask_map");

		culling_eye_pos_param_ = deferred_effect->ParameterByName("culling_eye_pos");
		view_dir_param_ = deferred_effect->ParameterByName("view_dir");
		proj_mat_param_ = deferred_effect->ParameterByName("proj_mat");
		texture_world_offset_param_ = deferred_effect->ParameterByName("texture_world_offset");
		tri_size_param_ = deferred_effect->ParameterByName("tri_size");
		tile_size_param_ = deferred_effect->ParameterByName("tile_size");
		debug_show_patches_param_ = deferred_effect->ParameterByName("show_patches");
		debug_show_tiles_param_ = deferred_effect->ParameterByName("show_tiles");
		detail_noise_param_ = deferred_effect->ParameterByName("detail_noise_scale");
		detail_uv_param_ = deferred_effect->ParameterByName("detail_uv_scale");
		sample_spacing_param_ = deferred_effect->ParameterByName("coarse_sample_spacing");
		frame_size_param_ = deferred_effect->ParameterByName("frame_size");

		terrain_tex_layer_params_[0] = deferred_effect->ParameterByName("terrain_tex_layer_0");
		terrain_tex_layer_params_[1] = deferred_effect->ParameterByName("terrain_tex_layer_1");
		terrain_tex_layer_params_[2] = deferred_effect->ParameterByName("terrain_tex_layer_2");
		terrain_tex_layer_params_[3] = deferred_effect->ParameterByName("terrain_tex_layer_3");

		terrain_tex_layer_scale_params_[0] = deferred_effect->ParameterByName("terrain_tex_layer_scale_0");
		terrain_tex_layer_scale_params_[1] = deferred_effect->ParameterByName("terrain_tex_layer_scale_1");
		terrain_tex_layer_scale_params_[2] = deferred_effect->ParameterByName("terrain_tex_layer_scale_2");
		terrain_tex_layer_scale_params_[3] = deferred_effect->ParameterByName("terrain_tex_layer_scale_3");

		uint32_t tech_index;
		if (hw_tessellation_)
		{
			tech_index = 0;
		}
		else
		{
			tech_index = 2;
		}

		if (wireframe_)
		{
			tech_index += 1;
		}

		gbuffer_tech_ = terrain_gbuffer_techs_[tech_index];
		gbuffer_multi_view_tech_ = gbuffer_tech_;
		technique_ = gbuffer_tech_;
	}

	void HQTerrainRenderable::CreateNonTessIB()
	{
		uint16_t index = 0;
		uint16_t indices[NON_TESS_INDEX_COUNT];

		for (uint16_t y = 0; y < VERTEX_PER_TILE_EDGE - 1; ++ y)
		{
			uint16_t const row_start = y * VERTEX_PER_TILE_EDGE;

			for (uint16_t x = 0; x < VERTEX_PER_TILE_EDGE; ++ x)
			{
				indices[index] = row_start + x;
				++ index;
				indices[index] = row_start + x + VERTEX_PER_TILE_EDGE;
				++ index;
			}

			indices[index] = indices[index - 1];
			++ index;
			indices[index] = row_start + VERTEX_PER_TILE_EDGE;
			++ index;
		}
		BOOST_ASSERT(NON_TESS_INDEX_COUNT == index);

		RenderFactory& rf = Context::Instance().RenderFactoryInstance();
		tile_non_tess_ib_ = rf.MakeIndexBuffer(BU_Static, EAH_GPU_Read | EAH_Immutable, sizeof(indices), &indices[0]);
	}

	void HQTerrainRenderable::CreateNonTessVIDVB()
	{
		float vid[VERTEX_PER_TILE_EDGE * VERTEX_PER_TILE_EDGE];

		for (uint32_t y = 0; y < VERTEX_PER_TILE_EDGE; ++ y)
		{
			uint32_t const row_start = y * VERTEX_PER_TILE_EDGE;

			for (uint32_t x = 0; x < VERTEX_PER_TILE_EDGE; ++ x)
			{
				vid[row_start + x] = row_start + x + 0.5f;
			}
		}

		RenderFactory& rf = Context::Instance().RenderFactoryInstance();
		tile_non_tess_vid_vb_ = rf.MakeVertexBuffer(BU_Static, EAH_GPU_Read | EAH_Immutable, sizeof(vid), &vid[0]);
	}

	void HQTerrainRenderable::CreateTessIB()
	{
		uint16_t index = 0;
		uint16_t indices[TESS_INDEX_COUNT];

		for (uint16_t y = 0; y < VERTEX_PER_TILE_EDGE - 1; ++ y)
		{
			uint16_t const row_start = y * VERTEX_PER_TILE_EDGE;

			for (uint16_t x = 0; x < VERTEX_PER_TILE_EDGE - 1; ++ x)
			{
				indices[index] = row_start + x;
				++ index;
				indices[index] = row_start + x + VERTEX_PER_TILE_EDGE;
				++ index;
				indices[index] = row_start + x + VERTEX_PER_TILE_EDGE + 1;
				++ index;
				indices[index] = row_start + x + 1;
				++ index;
			}
		}
		BOOST_ASSERT(TESS_INDEX_COUNT == index);

		RenderFactory& rf = Context::Instance().RenderFactoryInstance();
		tile_tess_ib_ = rf.MakeIndexBuffer(BU_Static, EAH_GPU_Read | EAH_Immutable, sizeof(indices), &indices[0]);
	}

	void HQTerrainRenderable::TextureLayer(uint32_t layer, TexturePtr const & tex)
	{
		BOOST_ASSERT(layer < terrain_tex_layer_params_.size());
		*terrain_tex_layer_params_[layer] = tex;
	}

	void HQTerrainRenderable::TextureScale(uint32_t layer, float2 const & scale)
	{
		BOOST_ASSERT(layer < terrain_tex_layer_scale_params_.size());
		*terrain_tex_layer_scale_params_[layer] = scale;
	}

	float3 HQTerrainRenderable::CalcUVOffset(Camera const & camera) const
	{
		float3 eye = camera.EyePos();
		eye.y() = 0;
		if (snap_grid_size_ > 0)
		{
			eye.x() = floor(eye.x() / snap_grid_size_) * snap_grid_size_;
			eye.z() = floor(eye.z() / snap_grid_size_) * snap_grid_size_;
		}
		eye /= world_scale_;
		return eye;
	}

	void HQTerrainRenderable::SetMatrices(Camera const & camera)
	{
		float4x4 const & proj = camera.ProjMatrix();

		texture_world_offset_ = this->CalcUVOffset(camera);

		float3 const & eye = camera.EyePos();
		snapped_x_ = texture_world_offset_.x() * world_scale_;
		snapped_z_ = texture_world_offset_.z() * world_scale_;
		float const dx = eye.x() - snapped_x_;
		float const dz = eye.z() - snapped_z_;
		snapped_x_ = eye.x() - 2 * dx;				// TODO: Figure out why the 2x works
		snapped_z_ = eye.z() - 2 * dz;
		float4x4 const trans = MathLib::translation(snapped_x_, 0.0f, snapped_z_);
		float4x4 const scale = MathLib::scaling(world_scale_, world_scale_, world_scale_);
		float4x4 const model_mat = scale * trans;
		float4x4 const prev_model_mat = model_mat_;
		Renderable::ModelMatrix(model_mat);
		Renderable::PrevModelMatrix(prev_model_mat);

		*proj_mat_param_ = proj;

		*tri_size_param_ = 2 * tessellated_tri_size_;

		*debug_show_patches_param_ = show_patches_;
		*debug_show_tiles_param_ = show_tiles_;

		*detail_noise_param_ = detail_noise_scale_;
		*sample_spacing_param_ = world_scale_ * tile_rings_.back()->OuterWidth() / COARSE_HEIGHT_MAP_SIZE;

		float const detail_uv_scale = pow(2.0f, std::max(ridge_octaves_, tex_twist_octaves_) + fBm_octaves_ - 4.0f);
		*detail_uv_param_ = float2(detail_uv_scale, 1.0f / detail_uv_scale);

		*texture_world_offset_param_ = texture_world_offset_;

		float3 culling_eye = camera.EyePos();
		culling_eye.x() -= snapped_x_;
		culling_eye.z() -= snapped_z_;
		*culling_eye_pos_param_ = culling_eye;
		*view_dir_param_ = camera.ForwardVec();
	}

	void HQTerrainRenderable::RenderTerrain()
	{
		RenderFactory& rf = Context::Instance().RenderFactoryInstance();
		RenderEngine& re = rf.RenderEngineInstance();

		*height_map_param_ = height_map_srv_;
		*gradient_map_param_ = gradient_map_srv_;
		*mask_map_param_ = mask_map_srv_;
		*frame_size_param_ = int2(static_cast<int>(re.CurFrameBuffer()->Width()),
			static_cast<int>(re.CurFrameBuffer()->Height()));

		bool need_tess = false;
		if (hw_tessellation_)
		{
			switch (type_)
			{
			case PT_OpaqueGBuffer:
			case PT_OpaqueShading:
			case PT_OpaqueSpecialShading:
				need_tess = true;
				break;

			default:
				break;
			}
		}

		for (size_t i = 0; i < tile_rings_.size(); ++ i)
		{
			TileRing const& ring = *tile_rings_[i];

			if (need_tess)
			{
				rls_[0] = ring.GetTessRL();
			}
			else
			{
				rls_[0] = ring.GetNonTessRL();
			}

			*tile_size_param_ = ring.TileSize();
			re.Render(*effect_, *technique_, *rls_[0]);
		}
	}

	float HQTerrainRenderable::GetHeight(float x, float z)
	{
		uint32_t const width = height_map_cpu_tex_->Width(0);
		uint32_t const height = height_map_cpu_tex_->Height(0);

		float2 uv((x - snapped_x_) / world_scale_ / (world_uv_repeats_ * 2) + 0.5f,
			(z - snapped_z_) / world_scale_ / (world_uv_repeats_ * 2) + 0.5f);
		uv.x() = MathLib::clamp(uv.x(), 0.0f, 1.0f);
		uv.y() = MathLib::clamp(uv.y(), 0.0f, 1.0f);

		float fu = uv.x() * width;
		float fv = uv.y() * height;
		uint32_t iu0 = MathLib::clamp(static_cast<uint32_t>(fu), 0U, width - 1);
		uint32_t iv0 = MathLib::clamp(static_cast<uint32_t>(fv), 0U, height - 1);
		uint32_t iu1 = MathLib::clamp(iu0 + 1, 0U, width - 1);
		uint32_t iv1 = MathLib::clamp(iv0 + 1, 0U, height - 1);
		float wu = fu - iu0;
		float wv = fv - iv0;
		Texture::Mapper mapper(*height_map_cpu_tex_, 0, 0, TMA_Read_Only, 0, 0, width, height);
		half const * src = mapper.Pointer<half>();
		float t0 = static_cast<float>(src[iv0 * mapper.RowPitch() / sizeof(half) + iu0]);
		float t1 = static_cast<float>(src[iv0 * mapper.RowPitch() / sizeof(half) + iu1]);
		float t2 = static_cast<float>(src[iv1 * mapper.RowPitch() / sizeof(half) + iu0]);
		float t3 = static_cast<float>(src[iv1 * mapper.RowPitch() / sizeof(half) + iu1]);
		return MathLib::lerp(MathLib::lerp(t0, t1, wu), MathLib::lerp(t2, t3, wu), wv) * world_scale_ * vertical_scale_;
	}


	HQTerrainRenderableComponent::HQTerrainRenderableComponent(RenderablePtr const& renderable)
		: RenderableComponent(renderable),
			reset_terrain_(true)
	{
		last_eye_pos_ =
			Context::Instance().RenderFactoryInstance().RenderEngineInstance().DefaultFrameBuffer()->Viewport()->Camera()->EyePos();

		BOOST_ASSERT(dynamic_cast<HQTerrainRenderable*>(renderable.get()) != nullptr);

		this->OnMainThreadUpdate().Connect(
			[](SceneComponent& component, [[maybe_unused]] float app_time, [[maybe_unused]] float elapsed_time)
			{
				auto& hq_inf_terrain = checked_cast<HQTerrainRenderableComponent&>(component);
				auto& hq_inf_terrain_renderable = hq_inf_terrain.BoundRenderableOfType<HQTerrainRenderable>();

				RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
				Camera const& camera = *re.DefaultFrameBuffer()->Viewport()->Camera();

				hq_inf_terrain_renderable.SetMatrices(camera);

				hq_inf_terrain.reset_terrain_ = hq_inf_terrain.reset_terrain_ || (hq_inf_terrain.last_eye_pos_ != camera.EyePos());
				if (hq_inf_terrain.reset_terrain_)
				{
					hq_inf_terrain_renderable.FlushTerrainData();
					hq_inf_terrain.reset_terrain_ = false;
					hq_inf_terrain.last_eye_pos_ = camera.EyePos();
				}
			});
	}
}
