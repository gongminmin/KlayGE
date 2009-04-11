// Bound.hpp
// KlayGE 边框 头文件
// Ver 2.5.0
// 版权所有(C) 龚敏敏, 2004-2005
// Homepage: http://klayge.sourceforge.net
//
// 2.5.0
// 改为模板 (2005.4.12)
//
// 2.1.1
// 初次建立 (2004.4.30)
//
// 修改记录
///////////////////////////////////////////////////////////////////////////////

#ifndef _BOUND_HPP
#define _BOUND_HPP

#pragma KLAYGE_ONCE

namespace KlayGE
{
	template <typename T>
	class Bound_T
	{
	public:
		virtual ~Bound_T()
		{
		}

		virtual bool IsEmpty() const = 0;

		virtual bool VecInBound(Vector_T<T, 3> const & v) const = 0;
		virtual T MaxRadiusSq() const = 0;
	};

	typedef Bound_T<float> Bound;
}

#endif			// _BOUND_HPP
