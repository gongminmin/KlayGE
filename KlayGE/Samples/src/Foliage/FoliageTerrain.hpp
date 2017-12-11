#ifndef _FOLIAGETERRAIN_HPP
#define _FOLIAGETERRAIN_HPP

#pragma once

#include <KlayGE/PreDeclare.hpp>

#include <KlayGE/RenderableHelper.hpp>
#include <KlayGE/SceneObjectHelper.hpp>
#include <KlayGE/InfTerrain.hpp>

#include <vector>

namespace KlayGE
{
	class ProceduralTerrain : public HQTerrainRenderable
	{
#pragma pack(push, 1)
		struct PlantInstanceData
		{
			float3 pos;
			float scale;
			float2 rotation;
		};
#pragma pack(pop)

	public:
		ProceduralTerrain();

		void Render() override;

		uint32_t Num3DPlants() const
		{
			return num_3d_plants_;
		}
		uint32_t NumImpostorPlants() const
		{
			return num_impostor_plants_;
		}

	protected:
		void FlushTerrainData() override;

	private:
		PostProcessPtr height_pp_;
		PostProcessPtr gradient_pp_;
		PostProcessPtr mask_pp_;

		std::vector<RenderablePtr> plant_meshes_;
		std::vector<RenderablePtr> plant_impostor_meshes_;
		std::vector<ImposterPtr> plant_imposters_;

		std::vector<std::vector<GraphicsBufferPtr>> plant_lod_instance_buffers_;
		std::vector<std::vector<RenderLayoutPtr>> plant_lod_instance_rls_;
		std::vector<GraphicsBufferPtr> plant_impostor_instance_buffers_;
		std::vector<RenderLayoutPtr> plant_impostor_instance_rls_;
		std::vector<uint4> num_tile_plants_;
		uint32_t num_tiles_edge_;
		std::vector<uint4> tile_addr_offset_width_;

		RenderEffectPtr foliage_dist_effect_;
		RenderTechnique* foliage_dist_tech_;
		RenderTechnique* foliage_impostor_dist_tech_;
		RenderLayoutPtr foliage_dist_rl_;

		uint32_t num_3d_plants_;
		uint32_t num_impostor_plants_;

		std::vector<std::vector<QueryPtr>> plant_lod_primitive_written_query_;
		std::vector<QueryPtr> plant_impostor_primitive_written_query_;
	};
}

#endif		// _FOLIAGETERRAIN_HPP
