// Crc32.hpp
// KlayGE CRC32 头文件
// Ver 1.2.8.10
// 版权所有(C) 龚敏敏, 2002
// Homepage: http://www.enginedev.com
//
// 1.2.8.10
// 用string代替字符串指针 (2002.10.27)
//
// 1.2.8.11
// 改用UNICODE内核 (2002.11.21)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#ifndef _CRC32_HPP
#define _CRC32_HPP

#include <KlayGE/PreDeclare.hpp>
#include <string>

#pragma comment(lib, "KlayGE_Core.lib")

namespace KlayGE
{
	namespace Crc32
	{
		U32 CrcMem(const U8* data, size_t len);
		U32 CrcString(const std::string& str);
		U32 CrcFile(VFile& file);
	};
}

#endif		// _CRC32_HPP
