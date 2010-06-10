// Frustum.hpp
// KlayGE 视锥类 头文件
// Ver 3.10.0
// 版权所有(C) 龚敏敏, 2004-2010
// Homepage: http://www.klayge.org
//
// 3.10.0
// 移致Core中 (2010.5.23)
//
// 2.5.0
// 改为LUT实现 (2005.3.30)
//
// 2.1.2
// 初次建立 (2004.6.15)
//
// 修改记录
//////////////////////////////////////////////////////////////////////////////////

#ifndef _FRUSTUM_HPP
#define _FRUSTUM_HPP

#pragma once

#include <KlayGE/Math.hpp>
#ifdef KLAYGE_COMPILER_MSVC
#pragma warning(push)
#pragma warning(disable: 6385)
#endif
#include <boost/array.hpp>
#ifdef KLAYGE_COMPILER_MSVC
#pragma warning(pop)
#endif

namespace KlayGE
{
	class KLAYGE_CORE_API Frustum
	{
	public:
		enum VIS
		{
			VIS_YES,
			VIS_NO,
			VIS_PART,
		};

	public:
		void ClipMatrix(float4x4 const & clip);
		VIS Visiable(Box const & box) const;

	private:
		typedef boost::array<Plane, 6> planes_t;
		planes_t planes_;

		// Look-Up Table
		typedef boost::array<int, 6> vertex_lut_t;
		vertex_lut_t vertex_lut_;
	};
}

#endif			// _FRUSTUM_HPP
