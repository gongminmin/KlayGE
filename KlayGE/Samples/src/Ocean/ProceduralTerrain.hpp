#ifndef _PROCEDURALTERRAIN_HPP
#define _PROCEDURALTERRAIN_HPP

#pragma once

#include <KlayGE/PreDeclare.hpp>

#include <KlayGE/SceneNode.hpp>
#include <KlayGE/InfTerrain.hpp>

#include <vector>

namespace KlayGE
{
	class ProceduralTerrain : public HQTerrainRenderable
	{
	public:
		ProceduralTerrain();

		void ReflectionPlane(Plane const & plane);

		void OnRenderBegin() override;

	protected:
		virtual void FlushTerrainData() override;

	private:
		PostProcessPtr height_pp_;
		PostProcessPtr gradient_pp_;
		PostProcessPtr mask_pp_;
		Plane reflection_plane_;

		RenderEffectParameter* mvp_wo_oblique_param_;
	};
}

#endif		// _PROCEDURALTERRAIN_HPP
