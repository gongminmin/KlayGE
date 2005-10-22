// Query.hpp
// KlayGE 查询抽象类 实现文件
// Ver 3.0.0
// 版权所有(C) 龚敏敏, 2005
// Homepage: http://klayge.sourceforge.net
//
// 3.0.0
// 初次建立 (2005.10.18)
//
// 修改记录
//////////////////////////////////////////////////////////////////////////////////

#ifndef _QUERY_HPP
#define _QUERY_HPP

#ifdef KLAYGE_DEBUG
	#pragma comment(lib, "KlayGE_Core_d.lib")
#else
	#pragma comment(lib, "KlayGE_Core.lib")
#endif

namespace KlayGE
{
	class Query
	{
	public:
		virtual ~Query()
		{
		}

		virtual void Begin() = 0;
		virtual void End() = 0;
	};
}

#endif		// _QUERY_HPP
