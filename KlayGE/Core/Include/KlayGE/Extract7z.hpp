// Extract7z.hpp
// KlayGE 打包系统7z提取器 头文件 来自7zip
// Ver 3.6.0
// 版权所有(C) 龚敏敏, 2007
// Homepage: http://klayge.sourceforge.net
//
// 3.6.0
// 初次建立 (2007.5.24)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#ifndef _EXTRACT7Z_HPP
#define _EXTRACT7Z_HPP

#pragma KLAYGE_ONCE

#ifndef KLAYGE_CORE_SOURCE
#define KLAYGE_LIB_NAME KlayGE_Core
#include <KlayGE/config/auto_link.hpp>
#endif

#include <KlayGE/PreDeclare.hpp>

#include <string>

namespace KlayGE
{
	KLAYGE_CORE_API uint32_t Find7z(ResIdentifierPtr const & archive_is,
		std::string const & password,
		std::string const & extract_file_path);
	KLAYGE_CORE_API void Extract7z(ResIdentifierPtr const & archive_is,
		std::string const & password,
		std::string const & extract_file_path,
		boost::shared_ptr<std::ostream> const & os);
}

#endif		// _EXTRACT7Z_HPP
