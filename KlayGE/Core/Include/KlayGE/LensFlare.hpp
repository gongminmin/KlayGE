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
#include <KlayGE/SceneNode.hpp>

namespace KlayGE
{
	class KLAYGE_CORE_API LensFlareRenderable : public Renderable
	{
	public:
		LensFlareRenderable();

		void FlareParam(std::vector<float3> const & param, float alpha_fac);

		void OnRenderBegin();
	};

	class KLAYGE_CORE_API LensFlareRenderableComponent : public RenderableComponent
	{
	public:
#if defined(KLAYGE_COMPILER_CLANGCL)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Winconsistent-missing-override"
#endif
		BOOST_TYPE_INDEX_REGISTER_RUNTIME_CLASS((RenderableComponent))
#if defined(KLAYGE_COMPILER_CLANGCL)
#pragma clang diagnostic pop
#endif

		LensFlareRenderableComponent();
	};
}

#endif		// _LENSFLARE_HPP
