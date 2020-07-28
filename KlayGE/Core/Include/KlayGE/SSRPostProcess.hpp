/**
 * @file SSRPostProcess.hpp
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

#ifndef _SSRPOSTPROCESS_HPP
#define _SSRPOSTPROCESS_HPP

#pragma once

#include <KlayGE/PreDeclare.hpp>
#include <KlayGE/PostProcess.hpp>

namespace KlayGE
{
	class KLAYGE_CORE_API SSRPostProcess final : public PostProcess
	{
	public:
		explicit SSRPostProcess(bool multi_sample);

		void Apply() override;

	private:
		RenderEffectParameter* proj_param_;
		RenderEffectParameter* inv_proj_param_;
		RenderEffectParameter* near_q_far_param_;
		RenderEffectParameter* ray_length_param_;
	};
}

#endif		// _SSRPOSTPROCESS_HPP
