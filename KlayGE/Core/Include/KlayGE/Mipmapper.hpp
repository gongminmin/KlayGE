/**
 * @file Mipmapper.hpp
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

#ifndef KLAYGE_CORE_MIPMAPPER_HPP
#define KLAYGE_CORE_MIPMAPPER_HPP

#pragma once

#include <KlayGE/PreDeclare.hpp>
#include <KlayGE/Blitter.hpp>

namespace KlayGE
{
	class KLAYGE_CORE_API Mipmapper final : boost::noncopyable
	{
	public:
		Mipmapper();

		void BuildSubLevels(TexturePtr const& texture, TextureFilter filter) const;
		void BuildSubLevels(
			TexturePtr const& texture, uint32_t array_index, uint32_t start_level, uint32_t num_levels, TextureFilter filter) const;

	private:
		static uint32_t constexpr MAX_LEVELS = 6;

		RenderEffectPtr effect_;
		RenderTechnique* mipmap_tech_[2][2];
		RenderEffectParameter* src_level_width_height_param_;
		RenderEffectParameter* num_levels_param_;
		RenderEffectParameter* src_2d_tex_param_;
		RenderEffectParameter* output_2d_tex_param_[MAX_LEVELS - 1];
		RenderEffectParameter* output_2d_tex_array_param_[MAX_LEVELS - 1];

		std::unique_ptr<Blitter> blitter_;
	};
} // namespace KlayGE

#endif // KLAYGE_CORE_BLITTER_HPP
