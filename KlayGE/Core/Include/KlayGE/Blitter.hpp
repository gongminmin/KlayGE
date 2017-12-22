/**
 * @file Blitter.hpp
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

#ifndef _KLAYGE_BLITTER_HPP
#define _KLAYGE_BLITTER_HPP

#pragma once

#include <KlayGE/PreDeclare.hpp>

namespace KlayGE
{
	class KLAYGE_CORE_API Blitter : boost::noncopyable
	{
	public:
		Blitter();

		void Blit(TexturePtr const & dst, uint32_t dst_array_index, uint32_t dst_level,
			uint32_t dst_x_offset, uint32_t dst_y_offset, uint32_t dst_width, uint32_t dst_height,
			TexturePtr const & src, uint32_t src_array_index, uint32_t src_level,
			uint32_t src_x_offset, uint32_t src_y_offset, uint32_t src_width, uint32_t src_height,
			bool linear);
		void Blit(TexturePtr const & dst, uint32_t dst_array_index, uint32_t dst_level,
			uint32_t dst_x_offset, uint32_t dst_y_offset, uint32_t dst_z_offset, uint32_t dst_width, uint32_t dst_height, uint32_t dst_depth,
			TexturePtr const & src, uint32_t src_array_index, uint32_t src_level,
			uint32_t src_x_offset, uint32_t src_y_offset, uint32_t src_z_offset, uint32_t src_width, uint32_t src_height, uint32_t src_depth,
			bool linear);
		void Blit(TexturePtr const & dst, uint32_t dst_array_index, Texture::CubeFaces dst_face, uint32_t dst_level,
			uint32_t dst_x_offset, uint32_t dst_y_offset, uint32_t dst_width, uint32_t dst_height,
			TexturePtr const & src, uint32_t src_array_index, Texture::CubeFaces src_face, uint32_t src_level,
			uint32_t src_x_offset, uint32_t src_y_offset, uint32_t src_width, uint32_t src_height,
			bool linear);

		void Blit(GraphicsBufferPtr const & dst, uint32_t dst_x_offset,
			TexturePtr const & src, uint32_t src_array_index, uint32_t src_level,
			uint32_t src_x_offset, uint32_t src_y_offset, uint32_t src_width, uint32_t src_height);
		void Blit(TexturePtr const & dst, uint32_t dst_array_index, uint32_t dst_level,
			uint32_t dst_x_offset, uint32_t dst_y_offset, uint32_t dst_width, uint32_t dst_height,
			GraphicsBufferPtr const & src, uint32_t src_x_offset, ElementFormat src_fmt);

	private:
		RenderLayoutPtr quad_rl_;
		RenderLayoutPtr tex_to_buff_rl_;
		RenderLayoutPtr tex_to_buff_so_rl_;
		RenderLayoutPtr buff_to_tex_rl_;
		FrameBufferPtr frame_buffer_;
		RenderEffectPtr effect_;
		RenderTechnique* blit_point_2d_tech_;
		RenderTechnique* blit_point_2d_array_tech_;
		RenderTechnique* blit_linear_2d_tech_;
		RenderTechnique* blit_linear_2d_array_tech_;
		RenderTechnique* blit_point_3d_tech_;
		RenderTechnique* blit_linear_3d_tech_;
		RenderTechnique* blit_2d_to_buff_tech_;
		RenderTechnique* blit_2d_array_to_buff_tech_;
		RenderTechnique* blit_buff_to_2d_tech_;
		RenderEffectParameter* src_2d_tex_param_;
		RenderEffectParameter* src_2d_tex_array_param_;
		RenderEffectParameter* src_3d_tex_param_;
		RenderEffectParameter* src_array_index_param_;
		RenderEffectParameter* src_level_param_;
		RenderEffectParameter* src_offset_param_;
		RenderEffectParameter* src_scale_param_;
		RenderEffectParameter* dst_offset_param_;
		RenderEffectParameter* dst_scale_param_;
		RenderEffectParameter* dst_width_height_param_;
	};
}

#endif		// _KLAYGE_BLITTER_HPP
