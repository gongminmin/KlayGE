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

#include <KlayGE/PreDeclare.hpp>

#include <vector>

#ifdef KLAYGE_DEBUG
	#pragma comment(lib, "KlayGE_Core_d.lib")
#else
	#pragma comment(lib, "KlayGE_Core.lib")
#endif

namespace KlayGE
{
	struct RenderDeviceCaps
	{
		uint32_t max_shader_model;

		uint32_t max_texture_width;
		uint32_t max_texture_height;
		uint32_t max_texture_depth;
		uint32_t max_texture_cube_size;
		uint32_t max_textures_units;
		uint32_t max_texture_anisotropy;
		uint32_t max_vertex_texture_units;

		uint32_t max_user_clip_planes;

		uint32_t max_simultaneous_rts;

		uint32_t max_vertices;
		uint32_t max_indices;

		uint32_t texture_1d_filter_caps;
		uint32_t texture_2d_filter_caps;
		uint32_t texture_3d_filter_caps;
		uint32_t texture_cube_filter_caps;
	};
}

#endif			// _RENDERDEVICECAPS_HPP