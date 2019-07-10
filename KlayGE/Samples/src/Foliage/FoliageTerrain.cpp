#include <KlayGE/KlayGE.hpp>
#include <KFL/CXX17/iterator.hpp>
#include <KlayGE/RenderEffect.hpp>
#include <KlayGE/PostProcess.hpp>
#include <KlayGE/Camera.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/SceneManager.hpp>
#include <KlayGE/Query.hpp>
#include <KlayGE/Imposter.hpp>
#include <KlayGE/Mesh.hpp>

#include "FoliageTerrain.hpp"

namespace KlayGE
{
	uint32_t const COARSE_HEIGHT_MAP_SIZE = 1024;

	struct FoliagePlantParameters
	{
		std::string mesh_name;
		std::string imposter_name;

		uint32_t probability_channel;
		float plant_spacing_width;
		float plant_spacing_height;
		float impostor_distance;
		float fade_out_distance;

		float impostor_distance_sq;
		float fade_out_distance_sq;
	};

	FoliagePlantParameters plant_parameters[] = 
	{
		{ "Grass1/grass.glb", "Grass1/grass.impml", 0, 1.5f, 1.5f, 150, 600, 0, 0 },
		{ "Grass2/WC_Euphorbia-larica_2.glb", "Grass2/WC_Euphorbia-larica_2.impml", 0, 2, 2, 150, 600, 0, 0 },
		{ "Tree2/tree2a.meshml", "Tree2/tree2a.impml", 1, 11, 11, 300, -1, 0, 0 },
		{ "Tree1/tree1a.meshml", "Tree1/tree1a.impml", 1, 7, 7, 300, -1, 0, 0 },
	};

	class FoliageMesh : public StaticMesh
	{
	public:
		explicit FoliageMesh(std::wstring_view name)
			: StaticMesh(name)
		{
		}

		void DoBuildMeshInfo(RenderModel const & model) override
		{
			StaticMesh::DoBuildMeshInfo(model);

			std::string g_buffer_files[2];
			g_buffer_files[0] = "GBufferFoliage.fxml";
			uint32_t num = 1;
			if (mtl_->TwoSided())
			{
				g_buffer_files[1] = "GBufferTwoSided.fxml";
				++ num;
			}
			this->BindDeferredEffect(SyncLoadRenderEffects(MakeSpan(g_buffer_files, num)));
		}

		void InstanceBuffer(uint32_t lod, GraphicsBufferPtr const & vb)
		{
			rls_[lod]->BindVertexStream(vb,
				MakeSpan({VertexElement(VEU_TextureCoord, 1, EF_ABGR32F), VertexElement(VEU_TextureCoord, 2, EF_GR32F)}),
				RenderLayout::ST_Instance);
		}

		void ForceNumInstances(uint32_t lod, uint32_t num)
		{
			rls_[lod]->NumInstances(num);
		}
	};

	class FoliageImpostorMesh : public Renderable
	{
	public:
		explicit FoliageImpostorMesh(AABBox const & aabbox)
			: Renderable(L"FoliageImpostor")
		{
			RenderFactory& rf = Context::Instance().RenderFactoryInstance();

			rls_[0] = rf.MakeRenderLayout();
			rls_[0]->TopologyType(RenderLayout::TT_TriangleStrip);

			float2 pos[] = 
			{
				float2(-1, +1),
				float2(+1, +1),
				float2(-1, -1),
				float2(+1, -1)
			};
			GraphicsBufferPtr vb = rf.MakeVertexBuffer(BU_Static, EAH_GPU_Read | EAH_Immutable, sizeof(pos), pos);
			rls_[0]->BindVertexStream(vb, VertexElement(VEU_Position, 0, EF_GR32F));

			this->BindDeferredEffect(SyncLoadRenderEffect("GBufferFoliageImpostor.fxml"));
			gbuffer_mrt_tech_ = effect_->TechniqueByName("GBufferAlphaTestMRTTech");
			technique_ = gbuffer_mrt_tech_;

			pos_aabb_ = aabbox;

			mtl_ = MakeSharedPtr<RenderMaterial>();
			mtl_->Albedo(float4(1, 1, 1, 1));
		}

		void ImpostorTexture(TexturePtr const & rt0_tex, TexturePtr const & rt1_tex, float2 const & extent)
		{
			auto& rf = Context::Instance().RenderFactoryInstance();
			mtl_->Texture(RenderMaterial::TS_Normal, rf.MakeTextureSrv(rt0_tex));
			mtl_->Texture(RenderMaterial::TS_Albedo, rf.MakeTextureSrv(rt1_tex));

			tc_aabb_.Min() = float3(-extent.x(), -extent.y(), 0);
			tc_aabb_.Max() = float3(+extent.x(), +extent.y(), 0);

			this->UpdateBoundBox();
		}

		void InstanceBuffer(GraphicsBufferPtr const & vb)
		{
			rls_[0]->BindVertexStream(vb, MakeSpan({VertexElement(VEU_TextureCoord, 1, EF_ABGR32F), VertexElement(VEU_TextureCoord, 2, EF_GR32F)}),
				RenderLayout::ST_Instance);
		}

		void ForceNumInstances(uint32_t num)
		{
			rls_[0]->NumInstances(num);
		}

		void OnRenderBegin() override
		{
			RenderFactory& rf = Context::Instance().RenderFactoryInstance();
			RenderEngine& re = rf.RenderEngineInstance();
			Camera const& camera = *re.CurFrameBuffer()->Viewport()->Camera();

			float4x4 billboard_mat = camera.InverseViewMatrix();
			billboard_mat(3, 0) = 0;
			billboard_mat(3, 1) = 0;
			billboard_mat(3, 2) = 0;
			*(effect_->ParameterByName("billboard_mat")) = billboard_mat;

			Renderable::OnRenderBegin();
		}
	};


	ProceduralTerrain::ProceduralTerrain()
		: HQTerrainRenderable(SyncLoadRenderEffect("ProceduralTerrain.fxml"), 800, 1),
			num_3d_plants_(0), num_impostor_plants_(0)
	{
		for (size_t i = 0; i < std::size(plant_parameters); ++ i)
		{
			if (plant_parameters[i].impostor_distance > 0)
			{
				plant_parameters[i].impostor_distance_sq = plant_parameters[i].impostor_distance * plant_parameters[i].impostor_distance;
			}
			else
			{
				plant_parameters[i].impostor_distance_sq = -1;
			}

			if (plant_parameters[i].fade_out_distance > 0)
			{
				plant_parameters[i].fade_out_distance_sq = plant_parameters[i].fade_out_distance * plant_parameters[i].fade_out_distance;
			}
			else
			{
				plant_parameters[i].fade_out_distance_sq = -1;
			}
		}

		RenderFactory& rf = Context::Instance().RenderFactoryInstance();
		RenderEngine& re = rf.RenderEngineInstance();
		RenderDeviceCaps const & caps = re.DeviceCaps();

		use_draw_indirect_ = caps.draw_indirect_support && caps.uavs_at_every_stage_support;

		foliage_dist_effect_ = SyncLoadRenderEffect("FoliageDistribution.fxml");
		foliage_dist_tech_ = foliage_dist_effect_->TechniqueByName("FoliageDistribution");
		foliage_impostor_dist_tech_ = foliage_dist_effect_->TechniqueByName("FoliageImpostorDistribution");
		if (use_draw_indirect_)
		{
			foliage_dist_rw_tech_ = foliage_dist_effect_->TechniqueByName("FoliageDistributionRw");
			foliage_impostor_dist_rw_tech_ = foliage_dist_effect_->TechniqueByName("FoliageImpostorDistributionRw");
		}

		foliage_dist_rl_ = rf.MakeRenderLayout();
		foliage_dist_rl_->TopologyType(RenderLayout::TT_PointList);

		auto const height_fmt = caps.BestMatchTextureRenderTargetFormat(
			caps.pack_to_rgba_required ? MakeSpan({EF_ABGR8, EF_ARGB8}) : MakeSpan({EF_R16F, EF_R32F}), 1, 0);
		BOOST_ASSERT(height_fmt != EF_Unknown);
		height_map_tex_ = rf.MakeTexture2D(COARSE_HEIGHT_MAP_SIZE, COARSE_HEIGHT_MAP_SIZE, 1, 1, height_fmt,
			1, 0, EAH_GPU_Read | EAH_GPU_Write);
		height_map_srv_ = rf.MakeTextureSrv(height_map_tex_);
		height_map_rtv_ = rf.Make2DRtv(height_map_tex_, 0, 1, 0);
		height_map_cpu_tex_ = rf.MakeTexture2D(height_map_tex_->Width(0), height_map_tex_->Height(0),
			1, 1, height_map_tex_->Format(), 1, 0, EAH_CPU_Read);

		ElementFormat gradient_fmt;
		if (EF_R16F == height_fmt)
		{
			gradient_fmt = EF_GR16F;
		}
		else if (EF_R32F == height_fmt)
		{
			gradient_fmt = EF_GR32F;
		}
		else
		{
			gradient_fmt = height_fmt;
		}
		gradient_map_tex_ = rf.MakeTexture2D(COARSE_HEIGHT_MAP_SIZE, COARSE_HEIGHT_MAP_SIZE, 1, 1, gradient_fmt,
			1, 0, EAH_GPU_Read | EAH_GPU_Write);
		gradient_map_srv_ = rf.MakeTextureSrv(gradient_map_tex_);
		gradient_map_rtv_ = rf.Make2DRtv(gradient_map_tex_, 0, 1, 0);
		gradient_map_cpu_tex_ = rf.MakeTexture2D(gradient_map_tex_->Width(0), gradient_map_tex_->Height(0),
			1, 1, gradient_map_tex_->Format(), 1, 0, EAH_CPU_Read);

		auto const mask_fmt = caps.BestMatchTextureRenderTargetFormat(MakeSpan({EF_ABGR8, EF_ARGB8}), 1, 0);
		BOOST_ASSERT(mask_fmt != EF_Unknown);

		mask_map_tex_ = rf.MakeTexture2D(COARSE_HEIGHT_MAP_SIZE, COARSE_HEIGHT_MAP_SIZE, 1, 1, mask_fmt,
			1, 0, EAH_GPU_Read | EAH_GPU_Write);
		mask_map_srv_ = rf.MakeTextureSrv(mask_map_tex_);
		mask_map_rtv_ = rf.Make2DRtv(mask_map_tex_, 0, 1, 0);
		mask_map_cpu_tex_ = rf.MakeTexture2D(mask_map_tex_->Width(0), mask_map_tex_->Height(0),
			1, 1, mask_map_tex_->Format(), 1, 0, EAH_CPU_Read);

		height_pp_ = SyncLoadPostProcess("ProceduralTerrain.ppml", "height");
		gradient_pp_ = SyncLoadPostProcess("ProceduralTerrain.ppml", "gradient");
		mask_pp_ = SyncLoadPostProcess("ProceduralTerrain.ppml", "mask");

		height_pp_->OutputPin(0, height_map_rtv_);

		gradient_pp_->InputPin(0, height_map_srv_);
		gradient_pp_->OutputPin(0, gradient_map_rtv_);

		mask_pp_->InputPin(0, height_map_srv_);
		mask_pp_->InputPin(1, gradient_map_srv_);
		mask_pp_->OutputPin(0, mask_map_rtv_);

		float const total_size = tile_rings_.back()->TileSize() * tile_rings_.back()->OuterWidth();
		float const tile_size = tile_rings_[0]->TileSize();
		num_tiles_edge_ = static_cast<uint32_t>(total_size / tile_size + 0.5f);

		plant_meshes_.resize(std::size(plant_parameters));
		plant_impostor_meshes_.resize(plant_meshes_.size());
		plant_imposters_.resize(plant_meshes_.size());
		plant_lod_instance_buffers_.resize(plant_meshes_.size());
		plant_lod_instance_rls_.resize(plant_meshes_.size());
		plant_impostor_instance_buffers_.resize(plant_meshes_.size());
		plant_impostor_instance_rls_.resize(plant_meshes_.size());
		num_tile_plants_.resize(plant_meshes_.size());
		tile_addr_offset_width_.resize(plant_meshes_.size());
		if (use_draw_indirect_)
		{
			uint32_t indirect_args[] = { 0, 0, 0, 0 };
			plant_primitive_written_buff_ = rf.MakeVertexBuffer(BU_Dynamic, EAH_GPU_Write | EAH_GPU_Unordered | EAH_Raw,
				sizeof(indirect_args), indirect_args, sizeof(uint32_t));

			plant_primitive_written_fb_ = rf.MakeFrameBuffer();
			plant_primitive_written_buff_uav_ = rf.MakeBufferUav(plant_primitive_written_buff_, EF_R32UI);
			plant_primitive_written_fb_->Attach(0, plant_primitive_written_buff_uav_);
		}
		else
		{
			plant_lod_primitive_written_query_.resize(plant_meshes_.size());
			plant_impostor_primitive_written_query_.resize(plant_meshes_.size());
		}
		for (size_t plant_type = 0; plant_type < plant_meshes_.size(); ++ plant_type)
		{
			plant_meshes_[plant_type] = SyncLoadModel(plant_parameters[plant_type].mesh_name, EAH_GPU_Read | EAH_Immutable,
				SceneNode::SOA_Cullable, nullptr,
				CreateModelFactory<RenderModel>, CreateMeshFactory<FoliageMesh>);
			plant_impostor_meshes_[plant_type] = MakeSharedPtr<FoliageImpostorMesh>(plant_meshes_[plant_type]->RootNode()->PosBoundOS());

			plant_imposters_[plant_type] = SyncLoadImposter(plant_parameters[plant_type].imposter_name);
			checked_pointer_cast<FoliageImpostorMesh>(plant_impostor_meshes_[plant_type])->ImpostorTexture(
				plant_imposters_[plant_type]->RT0Texture(), plant_imposters_[plant_type]->RT1Texture(),
				float2(0.5f / plant_imposters_[plant_type]->NumAzimuth(), 0.5f));

			uint32_t const num_x
				= static_cast<uint32_t>(tile_size * world_scale_ / plant_parameters[plant_type].plant_spacing_width + 0.5f);
			uint32_t const num_y
				= static_cast<uint32_t>(tile_size * world_scale_ / plant_parameters[plant_type].plant_spacing_height + 0.5f);
			num_tile_plants_[plant_type] = uint4(num_x * num_y, num_x, num_y, 0);

			uint32_t plant_3d_tiles;
			if (plant_parameters[plant_type].impostor_distance > 0)
			{
				plant_3d_tiles
					= static_cast<uint32_t>(plant_parameters[plant_type].impostor_distance / (tile_size * world_scale_) * 2 + 4.5f);
				plant_3d_tiles = std::min(plant_3d_tiles, num_tiles_edge_);
			}
			else
			{
				plant_3d_tiles = num_tiles_edge_;
			}
			tile_addr_offset_width_[plant_type].x() = static_cast<uint32_t>((num_tiles_edge_ - plant_3d_tiles + 0.5f) / 2);
			tile_addr_offset_width_[plant_type].y() = plant_3d_tiles;

			uint32_t plant_visible_tiles;
			if (plant_parameters[plant_type].fade_out_distance > 0)
			{
				plant_visible_tiles
					= static_cast<uint32_t>(plant_parameters[plant_type].fade_out_distance / (tile_size * world_scale_) * 2 + 4.5f);
				plant_visible_tiles = std::min(plant_visible_tiles, num_tiles_edge_);
			}
			else
			{
				plant_visible_tiles = num_tiles_edge_;
			}
			tile_addr_offset_width_[plant_type].z() = static_cast<uint32_t>((num_tiles_edge_ - plant_visible_tiles + 0.5f) / 2);
			tile_addr_offset_width_[plant_type].w() = plant_visible_tiles;

			plant_lod_instance_buffers_[plant_type].resize(plant_meshes_[plant_type]->NumLods());
			plant_lod_instance_rls_[plant_type].resize(plant_lod_instance_buffers_[plant_type].size());
			if (!use_draw_indirect_)
			{
				plant_lod_primitive_written_query_[plant_type].resize(plant_lod_instance_buffers_[plant_type].size());
			}
			for (uint32_t lod = 0; lod < static_cast<uint32_t>(plant_lod_instance_buffers_[plant_type].size()); ++ lod)
			{
				plant_lod_instance_buffers_[plant_type][lod] = rf.MakeVertexBuffer(BU_Dynamic, EAH_GPU_Read | EAH_GPU_Write,
					plant_3d_tiles * plant_3d_tiles * num_tile_plants_[plant_type].x() * sizeof(PlantInstanceData), nullptr);
				for (uint32_t i = 0; i < plant_meshes_[plant_type]->NumMeshes(); ++ i)
				{
					auto& mesh = checked_cast<FoliageMesh&>(*plant_meshes_[plant_type]->Mesh(i));
					mesh.InstanceBuffer(lod, plant_lod_instance_buffers_[plant_type][lod]);
				}

				plant_lod_instance_rls_[plant_type][lod] = rf.MakeRenderLayout();
				plant_lod_instance_rls_[plant_type][lod]->BindVertexStream(plant_lod_instance_buffers_[plant_type][lod],
					MakeSpan({VertexElement(VEU_TextureCoord, 1, EF_ABGR32F), VertexElement(VEU_TextureCoord, 2, EF_GR32F)}));

				if (!use_draw_indirect_)
				{
					plant_lod_primitive_written_query_[plant_type][lod] = rf.MakeSOStatisticsQuery();
				}
			}

			plant_impostor_instance_buffers_[plant_type] = rf.MakeVertexBuffer(BU_Dynamic, EAH_GPU_Read | EAH_GPU_Write,
				plant_visible_tiles * plant_visible_tiles * num_tile_plants_[plant_type].x() * sizeof(PlantInstanceData), nullptr);
			{
				auto& mesh = checked_cast<FoliageImpostorMesh&>(*plant_impostor_meshes_[plant_type]);
				mesh.InstanceBuffer(plant_impostor_instance_buffers_[plant_type]);
			}

			plant_impostor_instance_rls_[plant_type] = rf.MakeRenderLayout();
			plant_impostor_instance_rls_[plant_type]->BindVertexStream(plant_impostor_instance_buffers_[plant_type],
				MakeSpan({VertexElement(VEU_TextureCoord, 1, EF_ABGR32F), VertexElement(VEU_TextureCoord, 2, EF_GR32F)}));

			if (!use_draw_indirect_)
			{
				plant_impostor_primitive_written_query_[plant_type] = rf.MakeSOStatisticsQuery();
			}
		}

		if (use_draw_indirect_)
		{
			std::vector<uint32_t> lod_indirect_args;
			std::vector<uint32_t> imposter_indirect_args(plant_meshes_.size() * 4);
			for (size_t plant_type = 0; plant_type < plant_meshes_.size(); ++ plant_type)
			{
				for (uint32_t lod = 0; lod < plant_meshes_[plant_type]->NumLods(); ++ lod)
				{
					for (uint32_t i = 0; i < plant_meshes_[plant_type]->NumMeshes(); ++ i)
					{
						auto const& mesh = checked_cast<FoliageMesh&>(*plant_meshes_[plant_type]->Mesh(i));
						auto const& rl = mesh.GetRenderLayout(lod);

						lod_indirect_args.insert(lod_indirect_args.end(),
							{ rl.NumIndices(), 0, rl.StartIndexLocation(), rl.StartVertexLocation(), rl.StartInstanceLocation() });
					}
				}

				{
					auto const& mesh = checked_cast<FoliageImpostorMesh&>(*plant_impostor_meshes_[plant_type]);
					auto const& rl = mesh.GetRenderLayout();

					imposter_indirect_args[plant_type * 4 + 0] = rl.NumVertices();
					imposter_indirect_args[plant_type * 4 + 1] = 0;
					imposter_indirect_args[plant_type * 4 + 2] = rl.StartVertexLocation();
					imposter_indirect_args[plant_type * 4 + 3] = rl.StartInstanceLocation();
				}
			}
			plant_lod_primitive_indirect_args_ = rf.MakeVertexBuffer(BU_Dynamic, EAH_DrawIndirectArgs,
				static_cast<uint32_t>(lod_indirect_args.size() * sizeof(uint32_t)), lod_indirect_args.data(), sizeof(uint32_t));
			plant_impostor_primitive_indirect_args_ = rf.MakeVertexBuffer(BU_Dynamic, EAH_DrawIndirectArgs,
				static_cast<uint32_t>(imposter_indirect_args.size() * sizeof(uint32_t)), imposter_indirect_args.data(), sizeof(uint32_t));

			uint32_t offset = 0;
			for (size_t plant_type = 0; plant_type < plant_meshes_.size(); ++ plant_type)
			{
				for (uint32_t lod = 0; lod < plant_meshes_[plant_type]->NumLods(); ++ lod)
				{
					for (uint32_t i = 0; i < plant_meshes_[plant_type]->NumMeshes(); ++ i)
					{
						auto& mesh = checked_cast<FoliageMesh&>(*plant_meshes_[plant_type]->Mesh(i));
						auto& rl = mesh.GetRenderLayout(lod);

						rl.BindIndirectArgs(plant_lod_primitive_indirect_args_);
						rl.IndirectArgsOffset(offset);
						offset += 5 * sizeof(uint32_t);
					}
				}

				{
					auto& mesh = checked_cast<FoliageImpostorMesh&>(*plant_impostor_meshes_[plant_type]);
					auto& rl = mesh.GetRenderLayout();

					rl.BindIndirectArgs(plant_impostor_primitive_indirect_args_);
					rl.IndirectArgsOffset(static_cast<uint32_t>(plant_type * 4 * sizeof(uint32_t)));
				}
			}
		}

		*(foliage_dist_effect_->ParameterByName("foliage_tile_size")) = tile_size;
		*(foliage_dist_effect_->ParameterByName("half_num_tiles_edge")) = num_tiles_edge_ * 0.5f;

		*(foliage_dist_effect_->ParameterByName("foliage_world_uv_repeats"))
			= float2(static_cast<float>(world_uv_repeats_), 1.0f / world_uv_repeats_);
		*(foliage_dist_effect_->ParameterByName("foliage_vertical_scale")) = vertical_scale_;
		*(foliage_dist_effect_->ParameterByName("height_map_tex")) = height_map_tex_;
	}

	void ProceduralTerrain::FlushTerrainData()
	{
		RenderFactory& rf = Context::Instance().RenderFactoryInstance();
		RenderEngine& re = rf.RenderEngineInstance();

		int3 octaves(ridge_octaves_, fBm_octaves_, tex_twist_octaves_);
		height_pp_->SetParam(0, octaves);
		height_pp_->SetParam(1, texture_world_offset_);
		height_pp_->SetParam(2, float2(static_cast<float>(world_uv_repeats_), 1.0f / world_uv_repeats_));

		mask_pp_->SetParam(0, texture_world_offset_);
		mask_pp_->SetParam(1, world_scale_ * tile_rings_.back()->OuterWidth() / COARSE_HEIGHT_MAP_SIZE);
		mask_pp_->SetParam(2, float2(static_cast<float>(world_uv_repeats_), 1.0f / world_uv_repeats_));
		mask_pp_->SetParam(3, float2(vertical_scale_, 1.0f / vertical_scale_));

		FrameBufferPtr fb = re.CurFrameBuffer();

		height_pp_->Apply();
		gradient_pp_->Apply();
		//mask_pp_->Apply();

		if (use_draw_indirect_)
		{
			re.BindFrameBuffer(plant_primitive_written_fb_);

			*(foliage_dist_effect_->ParameterByName("rw_primitive_buff")) = plant_primitive_written_buff_uav_;
		}
		else
		{
			re.BindFrameBuffer(fb);
		}

		height_map_tex_->CopyToTexture(*height_map_cpu_tex_);

		Camera const& camera = *fb->Viewport()->Camera();
		
		auto const & frustum = camera.ViewFrustum();
		std::vector<float4> view_frustum_planes(6);
		for (size_t i = 0; i < view_frustum_planes.size(); ++ i)
		{
			view_frustum_planes[i] = float4(&frustum.FrustumPlane(static_cast<uint32_t>(i)).a());
		}
		*(foliage_dist_effect_->ParameterByName("model_mat")) = model_mat_;
		*(foliage_dist_effect_->ParameterByName("eye_pos")) = camera.EyePos();
		*(foliage_dist_effect_->ParameterByName("view_frustum_planes")) = view_frustum_planes;
		*(foliage_dist_effect_->ParameterByName("foliage_texture_world_offset")) = texture_world_offset_;
		*(foliage_dist_effect_->ParameterByName("fov_scale")) = camera.ProjMatrix()(0, 0);

		uint32_t offset = sizeof(uint32_t);
		for (size_t plant_type = 0; plant_type < plant_meshes_.size(); ++ plant_type)
		{
			*(foliage_dist_effect_->ParameterByName("num_tile_plants")) = num_tile_plants_[plant_type];
			*(foliage_dist_effect_->ParameterByName("tile_addr_offset_width"))
				= uint2(tile_addr_offset_width_[plant_type].x(), tile_addr_offset_width_[plant_type].y());

			auto const & aabbox = plant_meshes_[plant_type]->RootNode()->PosBoundOS();
			*(foliage_dist_effect_->ParameterByName("plant_aabb_min")) = aabbox.Min();
			*(foliage_dist_effect_->ParameterByName("plant_aabb_max")) = aabbox.Max();
			*(foliage_dist_effect_->ParameterByName("probability_channel")) = plant_parameters[plant_type].probability_channel;
			*(foliage_dist_effect_->ParameterByName("plant_spacing"))
				= float4(plant_parameters[plant_type].plant_spacing_width / world_scale_,
					plant_parameters[plant_type].plant_spacing_height / world_scale_,
					world_scale_ / plant_parameters[plant_type].plant_spacing_width,
					world_scale_ / plant_parameters[plant_type].plant_spacing_height);
			*(foliage_dist_effect_->ParameterByName("impostor_dist"))
				= float2(plant_parameters[plant_type].impostor_distance_sq, plant_parameters[plant_type].fade_out_distance_sq);
			*(foliage_dist_effect_->ParameterByName("num_lods")) = static_cast<int32_t>(plant_meshes_[plant_type]->NumLods());

			for (uint32_t lod = 0; lod < static_cast<uint32_t>(plant_lod_instance_buffers_[plant_type].size()); ++ lod)
			{
				*(foliage_dist_effect_->ParameterByName("lod")) = static_cast<int32_t>(lod);

				uint32_t const num_vertices = plant_lod_instance_buffers_[plant_type][lod]->Size() / sizeof(PlantInstanceData);
				foliage_dist_rl_->NumVertices(num_vertices);
				re.BindSOBuffers(plant_lod_instance_rls_[plant_type][lod]);
				if (use_draw_indirect_)
				{
					uint32_t constexpr zero = 0;
					plant_primitive_written_buff_->UpdateSubresource(0, 4, &zero);

					re.Render(*foliage_dist_effect_, *foliage_dist_rw_tech_, *foliage_dist_rl_);

					for (uint32_t i = 0; i < plant_meshes_[plant_type]->NumMeshes(); ++ i)
					{
						plant_primitive_written_buff_->CopyToSubBuffer(*plant_lod_primitive_indirect_args_, offset, 0, sizeof(uint32_t));
						offset += 5 * sizeof(uint32_t);
					}
				}
				else
				{
					plant_lod_primitive_written_query_[plant_type][lod]->Begin();
					re.Render(*foliage_dist_effect_, *foliage_dist_tech_, *foliage_dist_rl_);
					plant_lod_primitive_written_query_[plant_type][lod]->End();
				}
			}
		}

		for (size_t plant_type = 0; plant_type < plant_meshes_.size(); ++ plant_type)
		{
			*(foliage_dist_effect_->ParameterByName("num_tile_plants")) = num_tile_plants_[plant_type];
			*(foliage_dist_effect_->ParameterByName("tile_addr_offset_width"))
				= uint2(tile_addr_offset_width_[plant_type].z(), tile_addr_offset_width_[plant_type].w());

			auto const & aabbox = plant_meshes_[plant_type]->RootNode()->PosBoundOS();
			*(foliage_dist_effect_->ParameterByName("plant_aabb_min")) = aabbox.Min();
			*(foliage_dist_effect_->ParameterByName("plant_aabb_max")) = aabbox.Max();
			*(foliage_dist_effect_->ParameterByName("probability_channel")) = plant_parameters[plant_type].probability_channel;
			*(foliage_dist_effect_->ParameterByName("plant_spacing"))
				= float4(plant_parameters[plant_type].plant_spacing_width / world_scale_,
					plant_parameters[plant_type].plant_spacing_height / world_scale_,
					world_scale_ / plant_parameters[plant_type].plant_spacing_width,
					world_scale_ / plant_parameters[plant_type].plant_spacing_height);
			*(foliage_dist_effect_->ParameterByName("impostor_dist"))
				= float2(plant_parameters[plant_type].impostor_distance_sq, plant_parameters[plant_type].fade_out_distance_sq);
			*(foliage_dist_effect_->ParameterByName("center_tc_angles_step"))
				= float4(0.5f / plant_imposters_[plant_type]->NumAzimuth(), 0.5f,
					1.0f / plant_imposters_[plant_type]->AzimuthAngleStep(), 1.0f / plant_imposters_[plant_type]->NumAzimuth());

			uint32_t const num_vertices = plant_impostor_instance_buffers_[plant_type]->Size() / sizeof(PlantInstanceData);
			foliage_dist_rl_->NumVertices(num_vertices);
			re.BindSOBuffers(plant_impostor_instance_rls_[plant_type]);
			if (use_draw_indirect_)
			{
				uint32_t constexpr zero = 0;
				plant_primitive_written_buff_->UpdateSubresource(0, 4, &zero);

				re.Render(*foliage_dist_effect_, *foliage_impostor_dist_rw_tech_, *foliage_dist_rl_);

				plant_primitive_written_buff_->CopyToSubBuffer(*plant_impostor_primitive_indirect_args_,
					static_cast<uint32_t>((plant_type * 4 + 1) * sizeof(uint32_t)), 0, sizeof(uint32_t));
			}
			else
			{
				plant_impostor_primitive_written_query_[plant_type]->Begin();
				re.Render(*foliage_dist_effect_, *foliage_impostor_dist_tech_, *foliage_dist_rl_);
				plant_impostor_primitive_written_query_[plant_type]->End();
			}
		}
		re.BindSOBuffers(RenderLayoutPtr());

		num_3d_plants_ = 0;
		num_impostor_plants_ = 0;
		if (use_draw_indirect_)
		{
			re.BindFrameBuffer(fb);
		}
		else
		{
			for (size_t plant_type = 0; plant_type < plant_meshes_.size(); ++ plant_type)
			{
				for (uint32_t lod = 0; lod < static_cast<uint32_t>(plant_lod_primitive_written_query_[plant_type].size()); ++ lod)
				{
					uint32_t const num_instances = static_cast<uint32_t>(
						checked_pointer_cast<SOStatisticsQuery>(plant_lod_primitive_written_query_[plant_type][lod])->NumPrimitivesWritten());
					for (uint32_t i = 0; i < plant_meshes_[plant_type]->NumMeshes(); ++ i)
					{
						auto& mesh = checked_cast<FoliageMesh&>(*plant_meshes_[plant_type]->Mesh(i));
						mesh.ForceNumInstances(lod, num_instances);
					}
					num_3d_plants_ += num_instances;
				}
			}

			for (size_t plant_type = 0; plant_type < plant_meshes_.size(); ++ plant_type)
			{
				uint32_t const num_instances = static_cast<uint32_t>(
					checked_pointer_cast<SOStatisticsQuery>(plant_impostor_primitive_written_query_[plant_type])->NumPrimitivesWritten());
				{
					auto& mesh = checked_cast<FoliageImpostorMesh&>(*plant_impostor_meshes_[plant_type]);
					mesh.ForceNumInstances(num_instances);
				}
				num_impostor_plants_ += num_instances;
			}
		}
	}

	void ProceduralTerrain::Render()
	{
		RenderFactory& rf = Context::Instance().RenderFactoryInstance();
		RenderEngine& re = rf.RenderEngineInstance();

		Camera const& camera = *re.CurFrameBuffer()->Viewport()->Camera();
		float2 const eye_pos_xz(camera.EyePos().x(), camera.EyePos().z());

		*(effect_->ParameterByName("model_mat")) = model_mat_;
		*(effect_->ParameterByName("eye_pos_xz")) = eye_pos_xz;

		HQTerrainRenderable::Render();

		for (size_t plant_type = 0; plant_type < plant_meshes_.size(); ++ plant_type)
		{
			for (uint32_t i = 0; i < plant_meshes_[plant_type]->NumMeshes(); ++ i)
			{
				auto& mesh = checked_cast<FoliageMesh&>(*plant_meshes_[plant_type]->Mesh(i));
				for (uint32_t lod = 0; lod < mesh.NumLods(); ++ lod)
				{
					mesh.Pass(type_);
					mesh.ActiveLod(lod);
					mesh.Render();
				}
			}
		}

		if (GetPassCategory(type_) != PC_ShadowMap)
		{
			for (size_t plant_type = 0; plant_type < plant_impostor_meshes_.size(); ++ plant_type)
			{
				auto& mesh = checked_cast<FoliageImpostorMesh&>(*plant_impostor_meshes_[plant_type]);
				mesh.Pass(type_);
				mesh.Render();
			}
		}
	}
}
