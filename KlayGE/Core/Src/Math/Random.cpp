// Random.hpp
// KlayGE 随机数 实现文件
// Ver 2.1.1
// 版权所有(C) 龚敏敏, 2004
// Homepage: http://klayge.sourceforge.net
//
// 2.1.1
// 初次建立 (2004.4.22)
//
// 修改记录
///////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>

#include <ctime>

#include <KlayGE/Math.hpp>

namespace KlayGE
{
	Random& Random::Instance()
	{
		static Random random;
		return random;
	}

	Random::Random()
	{
		std::srand(static_cast<unsigned int>(time(NULL)));
	}

	int Random::Next() const
	{
		return std::rand();
	}
}
