// RenderDeviceCaps.hpp
// KlayGE 渲染设备能力类 头文件
// Ver 2.8.0
// 版权所有(C) 龚敏敏, 2005
// Homepage: http://www.klayge.org
//
// 2.8.0
// 初次建立 (2005.7.17)
//
// 修改记录
//////////////////////////////////////////////////////////////////////////////////

#ifndef _RENDERDEVICECAPS_HPP
#define _RENDERDEVICECAPS_HPP

#pragma once

#include <KlayGE/PreDeclare.hpp>
#include <KlayGE/ElementFormat.hpp>

namespace KlayGE
{
	enum TessellationMethod
	{
		TM_Hardware,
		TM_Instanced,
		TM_No
	};

	struct RenderDeviceCaps
	{
		uint8_t max_shader_model;

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
		bool mrt_independent_bit_depths_support : 1;
		bool standard_derivatives_support : 1;
		bool shader_texture_lod_support : 1;
		bool logic_op_support : 1;
		bool independent_blend_support : 1;
		bool depth_texture_support : 1;
		bool fp_color_support : 1;
		bool pack_to_rgba_required : 1;
		bool draw_indirect_support : 1;
		bool no_overwrite_support : 1;

		bool gs_support : 1;
		bool cs_support : 1;
		bool hs_support : 1;
		bool ds_support : 1;

		TessellationMethod tess_method;

		function<bool(ElementFormat)> vertex_format_support;
		function<bool(ElementFormat)> texture_format_support;
		function<bool(ElementFormat, uint32_t, uint32_t)> rendertarget_format_support;
	};
}

#endif			// _RENDERDEVICECAPS_HPP
