/**
 * @file RenderDeviceCaps.hpp
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

#ifndef KLAYGE_CORE_RENDER_DEVICE_CAPS_HPP
#define KLAYGE_CORE_RENDER_DEVICE_CAPS_HPP

#pragma once

#include <KlayGE/PreDeclare.hpp>
#include <KFL/CXX2a/span.hpp>
#include <KlayGE/ElementFormat.hpp>

#include <map>
#include <vector>

#include <boost/operators.hpp>
#if defined(KLAYGE_COMPILER_GCC)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wstrict-aliasing" // Ignore aliasing in flat_tree.hpp
#endif
#include <boost/container/flat_map.hpp>
#if defined(KLAYGE_COMPILER_GCC)
#pragma GCC diagnostic pop
#endif

namespace KlayGE
{
	enum TessellationMethod
	{
		TM_Hardware,
		TM_No
	};

	struct ShaderModel : boost::less_than_comparable<ShaderModel,
							boost::equality_comparable<ShaderModel>>
	{
		uint8_t major_ver : 6;
		uint8_t minor_ver : 2;

		constexpr ShaderModel() noexcept
			: major_ver(0), minor_ver(0)
		{
		}
		constexpr ShaderModel(uint8_t major, uint8_t minor) noexcept
			: major_ver(major), minor_ver(minor)
		{
		}

		uint32_t FullVersion() const noexcept
		{
			return (major_ver << 2) | minor_ver;
		}

		bool operator<(ShaderModel const & rhs) const noexcept
		{
			return this->FullVersion() < rhs.FullVersion();
		}
		bool operator==(ShaderModel const & rhs) const noexcept
		{
			return this->FullVersion() == rhs.FullVersion();
		}
	};

	struct KLAYGE_CORE_API RenderDeviceCaps
	{
		ShaderModel max_shader_model;

		uint32_t max_texture_width;
		uint32_t max_texture_height;
		uint32_t max_texture_depth;
		uint32_t max_texture_cube_size;
		uint32_t max_texture_array_length;
		uint8_t max_vertex_texture_units;
		uint8_t max_pixel_texture_units;
		uint8_t max_geometry_texture_units;
		uint8_t max_simultaneous_rts;
		uint8_t max_simultaneous_uavs;
		uint8_t max_vertex_streams;
		uint8_t max_texture_anisotropy;

		bool is_tbdr : 1;

		bool hw_instancing_support : 1;
		bool instance_id_support : 1;
		bool stream_output_support : 1;
		bool alpha_to_coverage_support : 1;
		bool primitive_restart_support : 1;
		bool multithread_rendering_support : 1;
		bool multithread_res_creating_support : 1;
		bool arbitrary_multithread_rendering_support : 1;
		bool mrt_independent_bit_depths_support : 1;
		bool logic_op_support : 1;
		bool independent_blend_support : 1;
		bool depth_texture_support : 1;
		bool fp_color_support : 1;
		bool pack_to_rgba_required : 1;
		bool draw_indirect_support : 1;
		bool no_overwrite_support : 1;
		bool full_npot_texture_support : 1;
		bool render_to_texture_array_support : 1;
		bool explicit_multi_sample_support : 1;
		bool load_from_buffer_support : 1;
		bool uavs_at_every_stage_support : 1;
		bool rovs_support : 1;
		bool flexible_srvs_support : 1;

		bool gs_support : 1;
		bool cs_support : 1;
		bool hs_support : 1;
		bool ds_support : 1;

		TessellationMethod tess_method;

		bool VertexFormatSupport(ElementFormat format) const;
		bool TextureFormatSupport(ElementFormat format) const;
		bool RenderTargetFormatSupport(ElementFormat format, uint32_t sample_count, uint32_t sample_quality) const;
		bool TextureRenderTargetFormatSupport(ElementFormat format, uint32_t sample_count, uint32_t sample_quality) const;
		bool UavFormatSupport(ElementFormat format) const;

		ElementFormat BestMatchVertexFormat(std::span<ElementFormat const> formats) const;
		ElementFormat BestMatchTextureFormat(std::span<ElementFormat const> formats) const;
		ElementFormat BestMatchRenderTargetFormat(std::span<ElementFormat const> formats,
			uint32_t sample_count, uint32_t sample_quality) const;
		ElementFormat BestMatchTextureRenderTargetFormat(std::span<ElementFormat const> formats,
			uint32_t sample_count, uint32_t sample_quality) const;
		ElementFormat BestMatchUavFormat(std::span<ElementFormat const> formats) const;

		static constexpr uint32_t EncodeSampleCountQuality(uint32_t sample_count, uint32_t sample_quality) noexcept
		{
			return sample_count | (sample_quality << 16);
		}
		static constexpr uint32_t DecodeSampleCount(uint32_t encoded) noexcept
		{
			return encoded & 0xFFFF;
		}
		static constexpr uint32_t DecodeSampleQuality(uint32_t encoded) noexcept
		{
			return encoded >> 16;
		}

		void AssignVertexFormats(std::vector<ElementFormat> vertex_formats);
		void AssignTextureFormats(std::vector<ElementFormat> texture_formats);
		void AssignRenderTargetFormats(std::map<ElementFormat, std::vector<uint32_t>> render_target_formats);
		void AssignUavFormats(std::vector<ElementFormat> uav_formats);

	private:
		void UpdateSupportBits();

	private:
		std::vector<ElementFormat> vertex_formats_;
		std::vector<ElementFormat> texture_formats_;
		boost::container::flat_map<ElementFormat, std::vector<uint32_t>> render_target_formats_;
		std::vector<ElementFormat> uav_formats_;
	};
}

#endif			// KLAYGE_CORE_RENDER_DEVICE_CAPS_HPP
