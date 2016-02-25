// LensFlare.hpp
// KlayGE Lens Flare header file
// Ver 3.11.0
// Copyright(C) Minmin Gong, 2010
// Homepage: http://www.klayge.org
//
// 3.11.0
// First release (2010.8.21)
//
// CHANGE LIST
/////////////////////////////////////////////////////////////////////////////////

#ifndef _LENSFLARE_HPP
#define _LENSFLARE_HPP

#pragma once

#include <KlayGE/RenderableHelper.hpp>
#include <KlayGE/SceneObjectHelper.hpp>

namespace KlayGE
{
	class KLAYGE_CORE_API LensFlareRenderable : public RenderableHelper
	{
	public:
		LensFlareRenderable();

		void FlareParam(std::vector<float3> const & param, float alpha_fac);

		void OnRenderBegin();
	};
	
	class KLAYGE_CORE_API LensFlareSceneObject : public SceneObjectHelper
	{
	public:
		LensFlareSceneObject();

		void Direction(float3 const & dir);
		float3 const & Direction() const;

		bool LFVisible() const
		{
			return lf_visible_;
		}

		virtual bool MainThreadUpdate(float app_time, float elapsed_time) override;

		virtual void Pass(PassType type);

	private:
		float3 dir_;
		bool lf_visible_;
	};
}

#endif		// _LENSFLARE_HPP
