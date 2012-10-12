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

#include <boost/function.hpp>

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

		bool is_tbdr;

		bool hw_instancing_support;
		bool instance_id_support;
		bool stream_output_support;
		bool alpha_to_coverage_support;
		bool primitive_restart_support;
		bool multithread_rendering_support;
		bool multithread_res_creating_support;
		bool mrt_independent_bit_depths_support;
		bool standard_derivatives_support;
		bool logic_op_support;

		bool gs_support;
		bool cs_support;
		bool hs_support;
		bool ds_support;

		TessellationMethod tess_method;

		boost::function<bool(ElementFormat)> vertex_format_support;
		boost::function<bool(ElementFormat)> texture_format_support;
		boost::function<bool(ElementFormat, uint32_t, uint32_t)> rendertarget_format_support;
	};
}

#endif			// _RENDERDEVICECAPS_HPP
