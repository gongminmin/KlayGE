#ifndef _PROCEDURALTERRAIN_HPP
#define _PROCEDURALTERRAIN_HPP

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
	public:
		ProceduralTerrain();

	protected:
		virtual void FlushTerrainData() override;

	private:
		PostProcessPtr height_pp_;
		PostProcessPtr gradient_pp_;
		PostProcessPtr mask_pp_;
	};
}

#endif		// _PROCEDURALTERRAIN_HPP
