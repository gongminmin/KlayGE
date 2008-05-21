// ThrowErr.cpp
// KlayGE 抛出错误 实现文件
// Ver 3.7.0
// 版权所有(C) 龚敏敏, 2006-2008
// Homepage: http://klayge.sourceforge.net
//
// 3.7.0
// 改用Boost.System的异常 (2008.5.21)
//
// 3.4.0
// 初次建立 (2006.9.5)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>

#include <sstream>

#include <KlayGE/ThrowErr.hpp>

namespace KlayGE
{
	std::string CombineFileLine(std::string const & file, int line)
	{
		std::stringstream ss;
		ss << file << ": " << line;
		return ss.str();
	}
}
