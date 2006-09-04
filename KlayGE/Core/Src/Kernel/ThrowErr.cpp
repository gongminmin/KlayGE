// ThrowErr.cpp
// KlayGE 抛出错误 实现文件
// Ver 3.4.0
// 版权所有(C) 龚敏敏, 2006
// Homepage: http://klayge.sourceforge.net
//
// Ver 3.4.0
// 初次建立 (2006.9.5)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>

#include <sstream>

#include <KlayGE/ThrowErr.hpp>

namespace KlayGE
{
	Exception::Exception(std::string const & errFile, uint32_t errLine, int32_t errCode, std::string const & msg) throw()
			: errFile_(errFile), errLine_(errLine),
				errCode_(errCode)
	{
		std::ostringstream ss;
		ss << "File: " << errFile << " Line: " << errLine << " ErrCode: " << errCode << " Msg: " << msg;
		msg_ = ss.str();
	}
}
