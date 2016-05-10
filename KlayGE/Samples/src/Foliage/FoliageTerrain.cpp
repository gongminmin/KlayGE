#include <KlayGE/KlayGE.hpp>
#include <KlayGE/RenderEffect.hpp>
#include <KlayGE/PostProcess.hpp>
#include <KlayGE/Camera.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/SceneManager.hpp>
#include <KlayGE/Query.hpp>

#include "FoliageTerrain.hpp"

namespace KlayGE
{
	uint32_t const COARSE_HEIGHT_MAP_SIZE = 1024;
	uint32_t const NUM_IMPOSTOR_ANGLES = 6;
	uint32_t const IMPOSTOR_SIZE = 256;

	struct FoliagePlantParameters
	{
		std::string mesh_name;

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
		{ "Grass1/grass.meshml", 0, 1.5f, 1.5f, 150, 600, 0, 0 },
		{ "Grass2/WC_Euphorbia-larica_2.meshml", 0, 2, 2, 150, 600, 0, 0 },
		{ "Tree2/tree2a_lod0.meshml", 1, 11, 11, 300, -1, 0, 0 },
		{ "Tree1/tree1a_lod0.meshml", 1, 7, 7, 300, -1, 0, 0 },
	};

	class FoliageMesh : public StaticMesh
	{
	public:
		FoliageMesh(RenderModelPtr const & model, std::wstring const & name)
			: StaticMesh(model, name)
		{
			this->BindDeferredEffect(SyncLoadRenderEffect("Foliage.fxml"));
			technique_ = gbuffer_mrt_tech_;
		}

		void DoBuildMeshInfo() override
		{
			StaticMesh::DoBuildMeshInfo();

			if ((effect_attrs_ & EA_AlphaTest) || (effect_attrs_ & EA_SSS))
			{
				gbuffer_mrt_tech_ = deferred_effect_->TechniqueByName("FoliageGBufferAlphaTestMRT");
			}
			else
			{
				gbuffer_mrt_tech_ = deferred_effect_->TechniqueByName("FoliageGBufferMRT");
			}
			technique_ = gbuffer_mrt_tech_;
		}

		void InstanceBuffer(GraphicsBufferPtr const & vb)
		{
			rl_->BindVertexStream(vb, std::make_tuple(vertex_element(VEU_TextureCoord, 1, EF_ABGR32F),
				vertex_element(VEU_TextureCoord, 2, EF_GR32F)),
				RenderLayout::ST_Instance);
		}

		void ForceNumInstances(uint32_t num)
		{
			rl_->NumInstances(num);
		}
	};

	class FoliageImpostorMesh : public RenderableHelper
	{
	public:
		explicit FoliageImpostorMesh(AABBox const & aabbox)
			: RenderableHelper(L"FoliageImpostor")
		{
			RenderFactory& rf = Context::Instance().RenderFactoryInstance();

			rl_ = rf.MakeRenderLayout();
			rl_->TopologyType(RenderLayout::TT_TriangleStrip);

			float2 pos[] = 
			{
				float2(-1, +1),
				float2(+1, +1),
				float2(-1, -1),
				float2(+1, -1)
			};
			GraphicsBufferPtr vb = rf.MakeVertexBuffer(BU_Static, EAH_GPU_Read | EAH_Immutable, sizeof(pos), pos);
			rl_->BindVertexStream(vb, std::make_tuple(vertex_element(VEU_Position, 0, EF_GR32F)));

			this->BindDeferredEffect(SyncLoadRenderEffect("Foliage.fxml"));
			gbuffer_mrt_tech_ = deferred_effect_->TechniqueByName("FoliageImpostorGBufferAlphaTestMRT");
			technique_ = gbuffer_mrt_tech_;

			pos_aabb_ = aabbox;
		}

		void ImpostorTexture(TexturePtr const & rt0_tex, TexturePtr const & rt1_tex, float2 const & extent)
		{
			normal_tex_ = rt0_tex;
			diffuse_tex_ = rt1_tex;

			tc_aabb_.Min() = float3(-extent.x(), -extent.y(), 0);
			tc_aabb_.Max() = float3(+extent.x(), +extent.y(), 0);
		}

		void InstanceBuffer(GraphicsBufferPtr const & vb)
		{
			rl_->BindVertexStream(vb, std::make_tuple(vertex_element(VEU_TextureCoord, 1, EF_ABGR32F),
				vertex_element(VEU_TextureCoord, 2, EF_GR32F)),
				RenderLayout::ST_Instance);
		}

		void ForceNumInstances(uint32_t num)
		{
			rl_->NumInstances(num);
		}

		void OnRenderBegin() override
		{
			RenderFactory& rf = Context::Instance().RenderFactoryInstance();
			RenderEngine& re = rf.RenderEngineInstance();
			Camera const * camera = re.CurFrameBuffer()->GetViewport()->camera.get();

			float4x4 billboard_mat = camera->InverseViewMatrix();
			billboard_mat(3, 0) = 0;
			billboard_mat(3, 1) = 0;
			billboard_mat(3, 2) = 0;
			*(deferred_effect_->ParameterByName("billboard_mat")) = billboard_mat;

			RenderableHelper::OnRenderBegin();
		}
	};


	ProceduralTerrain::ProceduralTerrain()
		: HQTerrainRenderable(SyncLoadRenderEffect("ProceduralTerrain.fxml"), 800, 1),
			num_3d_plants_(0), num_impostor_plants_(0)
	{
		for (size_t i = 0; i < sizeof(plant_parameters) / sizeof(plant_parameters[0]); ++ i)
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

		foliage_dist_effect_ = SyncLoadRenderEffect("Foliage.fxml");
		foliage_dist_tech_ = foliage_dist_effect_->TechniqueByName("FoliageDistribution");
		foliage_impostor_dist_tech_ = foliage_dist_effect_->TechniqueByName("FoliageImpostorDistribution");

		foliage_dist_rl_ = rf.MakeRenderLayout();
		foliage_dist_rl_->TopologyType(RenderLayout::TT_PointList);

		ElementFormat height_fmt;
		if (caps.pack_to_rgba_required)
		{
			if (caps.rendertarget_format_support(EF_ABGR8, 1, 0))
			{
				height_fmt = EF_ABGR8;
			}
			else
			{
				BOOST_ASSERT(caps.rendertarget_format_support(EF_ARGB8, 1, 0));
				height_fmt = EF_ARGB8;
			}
		}
		else
		{
			if (caps.rendertarget_format_support(EF_R16F, 1, 0))
			{
				height_fmt = EF_R16F;
			}
			else
			{
				BOOST_ASSERT(caps.rendertarget_format_support(EF_R32F, 1, 0));
				height_fmt = EF_R32F;
			}
		}
		height_map_tex_ = rf.MakeTexture2D(COARSE_HEIGHT_MAP_SIZE, COARSE_HEIGHT_MAP_SIZE, 1, 1, height_fmt,
			1, 0, EAH_GPU_Read | EAH_GPU_Write, nullptr);
		height_map_cpu_tex_ = rf.MakeTexture2D(height_map_tex_->Width(0), height_map_tex_->Height(0),
			1, 1, height_map_tex_->Format(), 1, 0, EAH_CPU_Read, nullptr);

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
			1, 0, EAH_GPU_Read | EAH_GPU_Write, nullptr);
		gradient_map_cpu_tex_ = rf.MakeTexture2D(gradient_map_tex_->Width(0), gradient_map_tex_->Height(0),
			1, 1, gradient_map_tex_->Format(), 1, 0, EAH_CPU_Read, nullptr);

		ElementFormat mask_fmt;
		if (caps.texture_format_support(EF_ABGR8))
		{
			mask_fmt = EF_ABGR8;
		}
		else
		{
			BOOST_ASSERT(caps.texture_format_support(EF_ARGB8));
			mask_fmt = EF_ARGB8;
		}
		mask_map_tex_ = rf.MakeTexture2D(COARSE_HEIGHT_MAP_SIZE, COARSE_HEIGHT_MAP_SIZE, 1, 1, mask_fmt,
			1, 0, EAH_GPU_Read | EAH_GPU_Write, nullptr);
		mask_map_cpu_tex_ = rf.MakeTexture2D(mask_map_tex_->Width(0), mask_map_tex_->Height(0),
			1, 1, mask_map_tex_->Format(), 1, 0, EAH_CPU_Read, nullptr);

		height_pp_ = SyncLoadPostProcess("ProceduralTerrain.ppml", "height");
		gradient_pp_ = SyncLoadPostProcess("ProceduralTerrain.ppml", "gradient");
		mask_pp_ = SyncLoadPostProcess("ProceduralTerrain.ppml", "mask");

		height_pp_->OutputPin(0, height_map_tex_);

		gradient_pp_->InputPin(0, height_map_tex_);
		gradient_pp_->OutputPin(0, gradient_map_tex_);

		mask_pp_->InputPin(0, height_map_tex_);
		mask_pp_->InputPin(1, gradient_map_tex_);
		mask_pp_->OutputPin(0, mask_map_tex_);

		float const total_size = tile_rings_.back()->TileSize() * tile_rings_.back()->OuterWidth();
		float const tile_size = tile_rings_[0]->TileSize();
		num_tiles_edge_ = static_cast<uint32_t>(total_size / tile_size + 0.5f);

		plant_meshes_.resize(sizeof(plant_parameters) / sizeof(plant_parameters[0]));
		plant_impostor_meshes_.resize(plant_meshes_.size());
		plant_instance_buffers_.resize(plant_meshes_.size());
		plant_instance_rls_.resize(plant_meshes_.size());
		plant_impostor_instance_buffers_.resize(plant_meshes_.size());
		plant_impostor_instance_rls_.resize(plant_meshes_.size());
		num_tile_plants_.resize(plant_meshes_.size());
		tile_addr_offset_width_.resize(plant_meshes_.size());
		plant_primitive_written_query_.resize(plant_meshes_.size());
		plant_impostor_primitive_written_query_.resize(plant_meshes_.size());
		for (size_t plant_type = 0; plant_type < plant_meshes_.size(); ++ plant_type)
		{
			plant_meshes_[plant_type] = SyncLoadModel(plant_parameters[plant_type].mesh_name, EAH_GPU_Read | EAH_Immutable,
				CreateModelFactory<RenderModel>(), CreateMeshFactory<FoliageMesh>());
			plant_impostor_meshes_[plant_type] = MakeSharedPtr<FoliageImpostorMesh>(plant_meshes_[plant_type]->PosBound());

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

			plant_instance_buffers_[plant_type] = rf.MakeVertexBuffer(BU_Dynamic, EAH_GPU_Read | EAH_GPU_Write,
				plant_3d_tiles * plant_3d_tiles * num_tile_plants_[plant_type].x() * sizeof(PlantInstanceData), nullptr);
			for (uint32_t i = 0; i < plant_meshes_[plant_type]->NumSubrenderables(); ++ i)
			{
				auto mesh = checked_cast<FoliageMesh*>(plant_meshes_[plant_type]->Subrenderable(i).get());
				mesh->InstanceBuffer(plant_instance_buffers_[plant_type]);
			}

			plant_instance_rls_[plant_type] = rf.MakeRenderLayout();
			plant_instance_rls_[plant_type]->BindVertexStream(plant_instance_buffers_[plant_type],
				std::make_tuple(vertex_element(VEU_TextureCoord, 1, EF_ABGR32F), vertex_element(VEU_TextureCoord, 2, EF_GR32F)));

			plant_impostor_instance_buffers_[plant_type] = rf.MakeVertexBuffer(BU_Dynamic, EAH_GPU_Read | EAH_GPU_Write,
				plant_visible_tiles * plant_visible_tiles * num_tile_plants_[plant_type].x() * sizeof(PlantInstanceData), nullptr);
			{
				auto mesh = checked_cast<FoliageImpostorMesh*>(plant_impostor_meshes_[plant_type].get());
				mesh->InstanceBuffer(plant_impostor_instance_buffers_[plant_type]);
			}

			plant_impostor_instance_rls_[plant_type] = rf.MakeRenderLayout();
			plant_impostor_instance_rls_[plant_type]->BindVertexStream(plant_impostor_instance_buffers_[plant_type],
				std::make_tuple(vertex_element(VEU_TextureCoord, 1, EF_ABGR32F), vertex_element(VEU_TextureCoord, 2, EF_GR32F)));

			plant_primitive_written_query_[plant_type] = rf.MakeSOStatisticsQuery();
			plant_impostor_primitive_written_query_[plant_type] = rf.MakeSOStatisticsQuery();
		}

		*(foliage_dist_effect_->ParameterByName("tile_size")) = tile_size;
		*(foliage_dist_effect_->ParameterByName("half_num_tiles_edge")) = num_tiles_edge_ * 0.5f;

		*(foliage_dist_effect_->ParameterByName("world_uv_repeats"))
			= float2(static_cast<float>(world_uv_repeats_), 1.0f / world_uv_repeats_);
		*(foliage_dist_effect_->ParameterByName("vertical_scale")) = vertical_scale_;
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

		re.BindFrameBuffer(fb);

		height_map_tex_->CopyToTexture(*height_map_cpu_tex_);

		if (!plant_impostors_g_buffer_rt0_)
		{
			BOOST_ASSERT(!plant_impostors_g_buffer_rt1_);

			this->GeneratesImposters();
		}

		Camera const * camera = re.CurFrameBuffer()->GetViewport()->camera.get();
		float2 eye_pos_xz(camera->EyePos().x(), camera->EyePos().z());
		
		auto const & frustum = camera->ViewFrustum();
		std::vector<float4> view_frustum_planes(6);
		for (size_t i = 0; i < view_frustum_planes.size(); ++i)
		{
			view_frustum_planes[i] = float4(&frustum.FrustumPlane(static_cast<uint32_t>(i)).a());
		}
		*(foliage_dist_effect_->ParameterByName("model_mat")) = model_mat_;
		*(foliage_dist_effect_->ParameterByName("eye_pos_xz")) = eye_pos_xz;
		*(foliage_dist_effect_->ParameterByName("view_frustum_planes")) = view_frustum_planes;
		*(foliage_dist_effect_->ParameterByName("texture_world_offset")) = texture_world_offset_;

		for (size_t plant_type = 0; plant_type < plant_meshes_.size(); ++ plant_type)
		{
			*(foliage_dist_effect_->ParameterByName("num_tile_plants")) = num_tile_plants_[plant_type];
			*(foliage_dist_effect_->ParameterByName("tile_addr_offset_width"))
				= uint2(tile_addr_offset_width_[plant_type].x(), tile_addr_offset_width_[plant_type].y());

			auto const & aabbox = plant_meshes_[plant_type]->PosBound();
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

			uint32_t const num_vertices = plant_instance_buffers_[plant_type]->Size() / sizeof(PlantInstanceData);
			foliage_dist_rl_->NumVertices(num_vertices);
			re.BindSOBuffers(plant_instance_rls_[plant_type]);
			plant_primitive_written_query_[plant_type]->Begin();
			re.Render(*foliage_dist_effect_, *foliage_dist_tech_, *foliage_dist_rl_);
			plant_primitive_written_query_[plant_type]->End();
		}

		for (size_t plant_type = 0; plant_type < plant_meshes_.size(); ++ plant_type)
		{
			*(foliage_dist_effect_->ParameterByName("num_tile_plants")) = num_tile_plants_[plant_type];
			*(foliage_dist_effect_->ParameterByName("tile_addr_offset_width"))
				= uint2(tile_addr_offset_width_[plant_type].z(), tile_addr_offset_width_[plant_type].w());

			auto const & aabbox = plant_meshes_[plant_type]->PosBound();
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
				= float4(plant_imposters_center_tc_[plant_type * NUM_IMPOSTOR_ANGLES].x(),
					plant_imposters_center_tc_[plant_type * NUM_IMPOSTOR_ANGLES].y(), 
					NUM_IMPOSTOR_ANGLES / PI, 1.0f / NUM_IMPOSTOR_ANGLES);

			uint32_t const num_vertices = plant_impostor_instance_buffers_[plant_type]->Size() / sizeof(PlantInstanceData);
			foliage_dist_rl_->NumVertices(num_vertices);
			re.BindSOBuffers(plant_impostor_instance_rls_[plant_type]);
			plant_impostor_primitive_written_query_[plant_type]->Begin();
			re.Render(*foliage_dist_effect_, *foliage_impostor_dist_tech_, *foliage_dist_rl_);
			plant_impostor_primitive_written_query_[plant_type]->End();
		}
		re.BindSOBuffers(RenderLayoutPtr());

		num_3d_plants_ = 0;
		for (size_t plant_type = 0; plant_type < plant_meshes_.size(); ++ plant_type)
		{
			uint32_t instance_index = static_cast<uint32_t>(
				checked_pointer_cast<SOStatisticsQuery>(plant_primitive_written_query_[plant_type])->NumPrimitivesWritten());
			for (uint32_t i = 0; i < plant_meshes_[plant_type]->NumSubrenderables(); ++ i)
			{
				auto mesh = checked_cast<FoliageMesh*>(plant_meshes_[plant_type]->Subrenderable(i).get());
				mesh->ForceNumInstances(instance_index);
			}
			num_3d_plants_ += instance_index;
		}

		num_impostor_plants_ = 0;
		for (size_t plant_type = 0; plant_type < plant_meshes_.size(); ++ plant_type)
		{
			uint32_t instance_index = static_cast<uint32_t>(
				checked_pointer_cast<SOStatisticsQuery>(plant_impostor_primitive_written_query_[plant_type])->NumPrimitivesWritten());
			{
				auto mesh = checked_cast<FoliageImpostorMesh*>(plant_impostor_meshes_[plant_type].get());
				mesh->ForceNumInstances(instance_index);
			}
			num_impostor_plants_ += instance_index;
		}
	}

	void ProceduralTerrain::Render()
	{
		RenderFactory& rf = Context::Instance().RenderFactoryInstance();
		RenderEngine& re = rf.RenderEngineInstance();

		Camera const * camera = re.CurFrameBuffer()->GetViewport()->camera.get();
		float2 eye_pos_xz(camera->EyePos().x(), camera->EyePos().z());

		*(effect_->ParameterByName("model_mat")) = model_mat_;
		*(effect_->ParameterByName("eye_pos_xz")) = eye_pos_xz;

		HQTerrainRenderable::Render();

		for (size_t plant_type = 0; plant_type < plant_meshes_.size(); ++ plant_type)
		{
			for (uint32_t i = 0; i < plant_meshes_[plant_type]->NumSubrenderables(); ++ i)
			{
				auto mesh = checked_cast<FoliageMesh*>(plant_meshes_[plant_type]->Subrenderable(i).get());
				mesh->Pass(type_);
				mesh->Render();
			}
		}

		for (size_t plant_type = 0; plant_type < plant_impostor_meshes_.size(); ++ plant_type)
		{
			auto mesh = checked_cast<FoliageImpostorMesh*>(plant_impostor_meshes_[plant_type].get());
			mesh->Pass(type_);
			mesh->Render();
		}
	}

	void ProceduralTerrain::GeneratesImposters()
	{
		RenderFactory& rf = Context::Instance().RenderFactoryInstance();
		RenderEngine& re = rf.RenderEngineInstance();

		FrameBufferPtr old_fb = re.CurFrameBuffer();

		plant_imposters_center_tc_.resize(plant_meshes_.size() * NUM_IMPOSTOR_ANGLES);

		plant_impostors_g_buffer_rt0_ = rf.MakeTexture2D(static_cast<uint32_t>(IMPOSTOR_SIZE * plant_meshes_.size()),
			IMPOSTOR_SIZE * NUM_IMPOSTOR_ANGLES, 0, 1, EF_ABGR8, 1, 0, EAH_GPU_Read | EAH_GPU_Write | EAH_Generate_Mips, nullptr);
		plant_impostors_g_buffer_rt1_ = rf.MakeTexture2D(static_cast<uint32_t>(IMPOSTOR_SIZE * plant_meshes_.size()),
			IMPOSTOR_SIZE * NUM_IMPOSTOR_ANGLES, 0, 1, EF_ABGR8, 1, 0, EAH_GPU_Read | EAH_GPU_Write | EAH_Generate_Mips, nullptr);

		FrameBufferPtr imposter_fb = rf.MakeFrameBuffer();
		imposter_fb->Attach(FrameBuffer::ATT_Color0, rf.Make2DRenderView(*plant_impostors_g_buffer_rt0_, 0, 1, 0));
		imposter_fb->Attach(FrameBuffer::ATT_Color1, rf.Make2DRenderView(*plant_impostors_g_buffer_rt1_, 0, 1, 0));
		imposter_fb->Attach(FrameBuffer::ATT_DepthStencil, rf.Make2DDepthStencilRenderView(
			static_cast<uint32_t>(IMPOSTOR_SIZE * plant_meshes_.size()), IMPOSTOR_SIZE * NUM_IMPOSTOR_ANGLES, EF_D24S8, 1, 0));
		auto const & imposter_camera = imposter_fb->GetViewport()->camera;
		imposter_fb->GetViewport()->width = IMPOSTOR_SIZE;
		imposter_fb->GetViewport()->height = IMPOSTOR_SIZE;

		imposter_fb->Clear(FrameBuffer::CBM_Color | FrameBuffer::CBM_Depth | FrameBuffer::CBM_Stencil,
			Color(0, 0, 0, 0), 1, 0);

		GraphicsBufferPtr imposter_instance_vb = rf.MakeVertexBuffer(BU_Dynamic, EAH_CPU_Write | EAH_GPU_Read,
			sizeof(PlantInstanceData), nullptr);

		float const angle_step = PI * 2 / NUM_IMPOSTOR_ANGLES;
		for (size_t plant_type = 0; plant_type < plant_meshes_.size(); ++ plant_type)
		{
			auto const & aabbox = plant_meshes_[plant_type]->PosBound();
			float3 const dimensions = aabbox.Max() - aabbox.Min();
			float3 imposter_eye_pos = aabbox.Center();
			imposter_eye_pos.z() = aabbox.Min().z();
			imposter_camera->ViewParams(imposter_eye_pos, aabbox.Center());
			imposter_camera->ProjOrthoParams(dimensions.x(), dimensions.y(), 0, dimensions.z());

			imposter_fb->GetViewport()->left = static_cast<int>(plant_type * IMPOSTOR_SIZE);

			for (size_t impostor_index = 0; impostor_index < NUM_IMPOSTOR_ANGLES; ++ impostor_index)
			{
				plant_imposters_center_tc_[plant_type * NUM_IMPOSTOR_ANGLES + impostor_index]
					= float2((plant_type + 0.5f) / plant_meshes_.size(), (impostor_index + 0.5f) / NUM_IMPOSTOR_ANGLES);

				float const angle = impostor_index * angle_step;
				Quaternion rot_quat = MathLib::rotation_axis(float3(0, 1, 0), -angle);

				{
					GraphicsBuffer::Mapper mapper(*imposter_instance_vb, BA_Write_Only);
					auto p = mapper.Pointer<PlantInstanceData>();
					p->pos = float3(0, 0, 0);
					p->scale = 1;
					p->rotation.x() = rot_quat.y();
					p->rotation.y() = rot_quat.w();
				}

				imposter_fb->GetViewport()->top = static_cast<int>(impostor_index * IMPOSTOR_SIZE);
				re.BindFrameBuffer(imposter_fb);

				for (uint32_t i = 0; i < plant_meshes_[plant_type]->NumSubrenderables(); ++ i)
				{
					auto mesh = checked_cast<FoliageMesh*>(plant_meshes_[plant_type]->Subrenderable(i).get());
					while (!mesh->AllHWResourceReady());
					mesh->InstanceBuffer(imposter_instance_vb);
					mesh->Pass(PT_OpaqueGBufferMRT);
					mesh->ForceNumInstances(1);
					mesh->Render();
				}
			}

			for (uint32_t i = 0; i < plant_meshes_[plant_type]->NumSubrenderables(); ++ i)
			{
				auto mesh = checked_cast<FoliageMesh*>(plant_meshes_[plant_type]->Subrenderable(i).get());
				mesh->InstanceBuffer(plant_instance_buffers_[plant_type]);
			}
		}

		plant_impostors_g_buffer_rt0_->BuildMipSubLevels();
		plant_impostors_g_buffer_rt1_->BuildMipSubLevels();

		//SaveTexture(plant_impostors_g_buffer_rt0_, "plant_imposter_rt0.dds");
		//SaveTexture(plant_impostors_g_buffer_rt1_, "plant_imposter_rt1.dds");

		re.BindFrameBuffer(old_fb);

		for (size_t plant_type = 0; plant_type < plant_meshes_.size(); ++ plant_type)
		{
			checked_pointer_cast<FoliageImpostorMesh>(plant_impostor_meshes_[plant_type])->ImpostorTexture(plant_impostors_g_buffer_rt0_,
				plant_impostors_g_buffer_rt1_, float2(0.5f / plant_meshes_.size(), 0.5f / NUM_IMPOSTOR_ANGLES));
		}
	}
}
