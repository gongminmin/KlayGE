// Frustum.hpp
// KlayGE 八叉树视锥类 头文件
// Ver 2.5.0
// 版权所有(C) 龚敏敏, 2004-2005
// Homepage: http://klayge.sourceforge.net
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
#include <boost/array.hpp>

namespace KlayGE
{
	class Frustum
	{
	public:
		enum VIS
		{
			VIS_YES,
			VIS_NO,
			VIS_PART,
		};

	public:
		explicit Frustum(float4x4 const & clip);

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
