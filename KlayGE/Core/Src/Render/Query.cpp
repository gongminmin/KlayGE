// Query.hpp
// KlayGE 查询抽象类 头文件
// Ver 3.1.0
// 版权所有(C) 龚敏敏, 2005
// Homepage: http://klayge.sourceforge.net
//
// 3.1.0
// 初次建立 (2005.10.29)
//
// 修改记录
//////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KlayGE/Query.hpp>

namespace KlayGE
{
	class NullQuery : public Query
	{
	public:
		void Begin()
		{
		}
		void End()
		{
		}
	};

	QueryPtr Query::NullObject()
	{
		static QueryPtr obj(new NullQuery);
		return obj;
	}
}
