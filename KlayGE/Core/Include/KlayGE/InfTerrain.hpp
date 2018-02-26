// InfTerrain.hpp
// KlayGE Infinite Terrain header file
// Ver 3.11.0
// Copyright(C) Minmin Gong, 2010
// Homepage: http://www.klayge.org
//
// 3.11.0
// First release (2010.8.21)
//
// CHANGE LIST
/////////////////////////////////////////////////////////////////////////////////

#ifndef _INFTERRAIN_HPP
#define _INFTERRAIN_HPP

#pragma once

#include <KlayGE/PreDeclare.hpp>

#include <array>

#include <KlayGE/RenderableHelper.hpp>
#include <KlayGE/SceneObjectHelper.hpp>

namespace KlayGE
{
	class KLAYGE_CORE_API InfTerrainRenderable : public RenderableHelper
	{
	public:
		InfTerrainRenderable(std::wstring const & name, uint32_t num_grids = 256, float stride = 1, float increate_rate = 1.012f);
		virtual ~InfTerrainRenderable();

		float2 const & XDir() const
		{
			return x_dir_;
		}

		float2 const & YDir() const
		{
			return y_dir_;
		}

		void SetStretch(float stretch);
		void SetBaseLevel(float base_level);
		void OffsetY(float y);

		void OnRenderBegin();

	protected:
		float2 x_dir_, y_dir_;
	};

	class KLAYGE_CORE_API InfTerrainSceneObject : public SceneObjectHelper
	{
	public:
		InfTerrainSceneObject();
		virtual ~InfTerrainSceneObject();

		virtual bool MainThreadUpdate(float app_time, float elapsed_time) override;

	protected:
		float base_level_;
		float strength_;
	};


	class KLAYGE_CORE_API HQTerrainRenderable : public RenderableHelper
	{
		friend class HQTerrainSceneObject;

	private:
#pragma pack(push, 1)
		struct Adjacency
		{
			float neighbor_minus_x;
			float neighbor_minus_y;
			float neighbor_plus_x;
			float neighbor_plus_y;
		};

		struct InstanceData
		{
			float x, y;
			Adjacency adjacency;
		};
#pragma pack(pop)

		//----------------------------------------------------------------------------------
		// Defines and draws one ring of tiles in a concentric, nested set of rings. Each ring
		// is a different LOD and uses a different tile/patch size. These are actually square
		// rings. (Is there a term for that?) But they could conceivably change to circular.
		// The inner-most LOD is a square, represented by a degenerate ring.
		//

		// Int dimensions specified to the ctor are in numbers of tiles. It's symmetrical in
		// each direction. (Don't read much into the exact numbers of #s in this diagram.)
		//
		//    <-   outer_width  ->
		//    ####################
		//    ####################
		//    ###              ###
		//    ###<-hole_width->###
		//    ###              ###
		//    ###    (0, 0)    ###
		//    ###              ###
		//    ###              ###
		//    ###              ###
		//    ####################
		//    ####################
		//
		class TileRing
		{
		public:
			// hole_width & outer_width are num of tiles
			// tile_size is a world-space length
			TileRing(int hole_width, int outer_width, float tile_size,
				GraphicsBufferPtr const & tile_non_tess_ib,
				GraphicsBufferPtr const & tile_non_tess_vid_vb,
				GraphicsBufferPtr const & tile_tess_ib);

			int OuterWidth() const
			{
				return outer_width_;
			}
			int NumTiles() const
			{
				return num_tiles_;
			}
			float TileSize() const
			{
				return tile_size_;
			}

			RenderLayoutPtr const & GetNonTessRL() const
			{
				return tile_non_tess_rl_;
			}
			RenderLayoutPtr const & GetTessRL() const
			{
				return tile_tess_rl_;
			}

		private:
			void CreateInstanceDataVB();
			bool InRing(int x, int y) const;
			void AssignNeighbourSizes(int x, int y, Adjacency& adj) const;

			GraphicsBufferPtr vb_;

			RenderLayoutPtr tile_non_tess_rl_;
			RenderLayoutPtr tile_tess_rl_;
			GraphicsBufferPtr tile_non_tess_ib_;
			GraphicsBufferPtr tile_non_tess_vid_vb_;
			GraphicsBufferPtr tile_tess_ib_;

			int const hole_width_, outer_width_, ring_width_;
			int const num_tiles_;
			float const tile_size_;

			TileRing(TileRing const & rhs);
			TileRing& operator=(TileRing const & rhs);
		};

	public:
		explicit HQTerrainRenderable(RenderEffectPtr const & effect,
			float world_scale = 800, float vertical_scale = 2.5f, int world_uv_repeats = 8);
		virtual ~HQTerrainRenderable()
		{
		}

		virtual void Render() override;

		virtual void ModelMatrix(float4x4 const & mat) override;

		void Tessellation(bool tess);
		void ShowPatches(bool sp);
		void ShowTiles(bool st);
		void Wireframe(bool wf);
		void DetailNoiseScale(float scale);
		void TessellatedTriSize(int size);

		void TextureLayer(uint32_t layer, TexturePtr const & tex);
		void TextureScale(uint32_t layer, float2 const & scale);

		float GetHeight(float x, float z);

	protected:
		void CreateNonTessIB();
		void CreateNonTessVIDVB();
		void CreateTessIB();
		float3 CalcUVOffset(Camera const & camera) const;
		void SetMatrices(Camera const & camera);
		void RenderTerrain();
		void UpdateTechniques() override;

		virtual void FlushTerrainData() = 0;

	protected:
		float world_scale_;
		float vertical_scale_;
		int world_uv_repeats_;

		int ridge_octaves_;
		int fBm_octaves_;
		int tex_twist_octaves_;
		float detail_noise_scale_;
		bool hw_tessellation_;
		int tessellated_tri_size_;
		bool wireframe_;
		bool show_patches_;
		bool show_tiles_;

		float snap_grid_size_;
		float snapped_x_, snapped_z_;
		std::vector<std::shared_ptr<TileRing>> tile_rings_;

		float3 texture_world_offset_;

		RenderTechnique* terrain_gbuffer_mrt_techs_[4];
		RenderEffectParameter* height_map_param_;
		RenderEffectParameter* gradient_map_param_;
		RenderEffectParameter* mask_map_param_;
		RenderEffectParameter* eye_pos_param_;
		RenderEffectParameter* view_dir_param_;
		RenderEffectParameter* proj_mat_param_;
		RenderEffectParameter* texture_world_offset_param_;
		RenderEffectParameter* tri_size_param_;
		RenderEffectParameter* tile_size_param_;
		RenderEffectParameter* debug_show_patches_param_;
		RenderEffectParameter* debug_show_tiles_param_;
		RenderEffectParameter* detail_noise_param_;
		RenderEffectParameter* detail_uv_param_;
		RenderEffectParameter* sample_spacing_param_;
		RenderEffectParameter* frame_size_param_;
		std::array<RenderEffectParameter*, 4> terrain_tex_layer_params_;
		std::array<RenderEffectParameter*, 4> terrain_tex_layer_scale_params_;

		GraphicsBufferPtr tile_non_tess_ib_;
		GraphicsBufferPtr tile_non_tess_vid_vb_;
		GraphicsBufferPtr tile_tess_ib_;
		TexturePtr height_map_tex_;
		TexturePtr gradient_map_tex_;
		TexturePtr mask_map_tex_;
		TexturePtr height_map_cpu_tex_;
		TexturePtr gradient_map_cpu_tex_;
		TexturePtr mask_map_cpu_tex_;
	};

	class KLAYGE_CORE_API HQTerrainSceneObject : public SceneObjectHelper
	{
	public:
		explicit HQTerrainSceneObject(RenderablePtr const & renderable);
		virtual ~HQTerrainSceneObject();

		virtual bool MainThreadUpdate(float app_time, float elapsed_time) override;

		void Tessellation(bool tess);
		void ShowPatches(bool sp);
		void ShowTiles(bool st);
		void Wireframe(bool wf);
		void DetailNoiseScale(float scale);
		void TessellatedTriSize(int size);

		void TextureLayer(uint32_t layer, TexturePtr const & tex);
		void TextureScale(uint32_t layer, float2 const & scale);

		float GetHeight(float x, float z);

	private:
		bool reset_terrain_;
		float3 last_eye_pos_;
	};
}

#endif		// _INFTERRAIN_HPP
