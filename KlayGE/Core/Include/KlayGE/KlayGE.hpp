// KlayGE.hpp
// KlayGE 头文件
// Ver 3.8.0
// 版权所有(C) 龚敏敏, 2003-2010
// Homepage: http://klayge.sourceforge.net
//
// 3.8.0
// 改成DLL的形式 (2008.10.17)
//
// 2.2.0
// 去掉了Safe*函数 (2004.10.31)
//
// 2.1.0
// 去掉了汇编代码 (2004.4.20)
//
// 2.0.0
// 初次建立 (2003.8.10)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#ifndef _KLAYGE_HPP
#define _KLAYGE_HPP

#include <KlayGE/Config.hpp>

#pragma once

#include <KlayGE/Types.hpp>

#define KLAYGE_NAME			KlayGE
#define KLAYGE_VER_MAJOR	3
#define KLAYGE_VER_MINOR	10
#define KLAYGE_VER_RELEASE	0
#define KLAYGE_VER_STR		KLAYGE_STRINGIZE(KLAYGE_NAME)" "KLAYGE_STRINGIZE(KLAYGE_VER_MAJOR)"."KLAYGE_STRINGIZE(KLAYGE_VER_MINOR)"."KLAYGE_STRINGIZE(KLAYGE_VER_RELEASE)

#include <vector>
#include <string>

#include <boost/assert.hpp>

#include <KlayGE/PreDeclare.hpp>
#include <KlayGE/ThrowErr.hpp>
#include <KlayGE/Util.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/Math.hpp>

#include <boost/config/requires_threads.hpp>

#endif		// _KLAYGE_HPP
