// Bound.hpp
// KlayGE 边框 头文件
// Ver 2.1.1
// 版权所有(C) 龚敏敏, 2004
// Homepage: http://klayge.sourceforge.net
//
// 2.1.1
// 初次建立 (2004.4.30)
//
// 修改记录
///////////////////////////////////////////////////////////////////////////////

#ifndef _BOUND_HPP
#define _BOUND_HPP

#include <boost/operators.hpp>

namespace KlayGE
{
	class Bound
	{
	public:
		virtual ~Bound()
		{
		}

		virtual bool IsEmpty() const = 0;

		virtual bool VecInBound(Vector3 const & v) const = 0;
		virtual float MaxRadiusSq() const = 0;
	};
}

#endif			// _BOUND_HPP
