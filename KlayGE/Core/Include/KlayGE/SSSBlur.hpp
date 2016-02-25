/**
* @file SSSBlur.hpp
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

#ifndef _SSSBLUR_HPP
#define _SSSBLUR_HPP

#pragma once

#include <KlayGE/PreDeclare.hpp>

#include <string>

#include <KlayGE/PostProcess.hpp>

namespace KlayGE
{
	class KLAYGE_CORE_API SSSBlurPP : public PostProcess
	{
	public:
		SSSBlurPP();

		virtual void InputPin(uint32_t index, TexturePtr const & tex) override;
		using PostProcess::InputPin;

		virtual void Apply() override;

	private:
		bool mrt_blend_support_;

		FrameBufferPtr blur_x_fb_;
		FrameBufferPtr blur_y_fb_;
		TexturePtr blur_x_tex_;
		TexturePtr blur_y_tex_;
		RenderTechniquePtr copy_tech_;
		RenderTechniquePtr blur_x_tech_;
		RenderTechniquePtr blur_y_techs_[3];
		RenderTechniquePtr accum_techs_[3];
		RenderEffectParameterPtr src_tex_param_;
		RenderEffectParameterPtr step_param_;
		RenderEffectParameterPtr far_plane_param_;
	};
}

#endif		// _SSSBLUR_HPP
