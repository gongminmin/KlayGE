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

#ifndef KLAYGE_CORE_SOURCE
#define KLAYGE_LIB_NAME KlayGE_Core
#include <KlayGE/config/auto_link.hpp>
#endif

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

		void Update();

	protected:
		float base_level_;
		float strength_;
	};
}

#endif		// _INFTERRAIN_HPP
