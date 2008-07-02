// RenderDeviceCaps.hpp
// KlayGE 渲染设备能力类 头文件
// Ver 2.8.0
// 版权所有(C) 龚敏敏, 2005
// Homepage: http://klayge.sourceforge.net
//
// 2.8.0
// 初次建立 (2005.7.17)
//
// 修改记录
//////////////////////////////////////////////////////////////////////////////////

#ifndef _RENDERDEVICECAPS_HPP
#define _RENDERDEVICECAPS_HPP

#define KLAYGE_LIB_NAME KlayGE_Core
#include <KlayGE/config/auto_link.hpp>

#include <KlayGE/PreDeclare.hpp>

#include <vector>

namespace KlayGE
{
	struct RenderDeviceCaps
	{
		uint8_t max_shader_model;

		uint32_t max_texture_width;
		uint32_t max_texture_height;
		uint32_t max_texture_depth;
		uint32_t max_texture_cube_size;
		uint32_t max_texture_array_length;
		uint8_t max_texture_units;
		uint8_t max_vertex_texture_units;

		uint8_t max_simultaneous_rts;

		uint32_t max_vertices;
		uint32_t max_indices;

		uint8_t texture_1d_filter_caps;
		uint8_t texture_2d_filter_caps;
		uint8_t texture_3d_filter_caps;
		uint8_t texture_cube_filter_caps;
		uint8_t max_texture_anisotropy;

		bool hw_instancing_support;
		bool stream_output_support;
		bool alpha_to_coverage_support;
	};
}

#endif			// _RENDERDEVICECAPS_HPP
