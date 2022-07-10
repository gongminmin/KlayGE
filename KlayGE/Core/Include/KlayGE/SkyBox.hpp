/**
 * @file SkyBox.cpp
 * @author Minmin Gong
 *
 * @section DESCRIPTION
 *
 * This source file is part of KlayGE
 * For the latest info, see http://www.klayge.org
 *
 * @section LICENSE
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published
 * by the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 * You may alternatively use this source under the terms of
 * the KlayGE Proprietary License (KPL). You can obtained such a license
 * from http://www.klayge.org/licensing/.
 */

#ifndef _SKYBOX_HPP
#define _SKYBOX_HPP

#pragma once

#include <KlayGE/PreDeclare.hpp>
#include <KlayGE/RenderableHelper.hpp>

namespace KlayGE
{
	class KLAYGE_CORE_API RenderableSkyBox : public Renderable
	{
	public:
		RenderableSkyBox();

		virtual void Technique(RenderEffectPtr const & effect, RenderTechnique* tech);
		void CubeMap(TexturePtr const & cube);
		void CompressedCubeMap(TexturePtr const & y_cube, TexturePtr const & c_cube);

		void OnRenderBegin();

		// For deferred only

		virtual void Pass(PassType type);

	protected:
		RenderEffectParameter* depth_far_ep_;
		RenderEffectParameter* inv_mvp_ep_;
		RenderEffectParameter* skybox_cube_tex_ep_;
		RenderEffectParameter* skybox_Ccube_tex_ep_;
		RenderEffectParameter* skybox_compressed_ep_;
	};
}

#endif		// _SKYBOX_HPP
