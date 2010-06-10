// NetMsg.hpp
// KlayGE 游戏消息定义 头文件
// Ver 2.0.0
// 版权所有(C) 龚敏敏, 2003
// Homepage: http://www.klayge.org
//
// 2.0.0
// 初次建立 (2003.7.24)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#ifndef _NETMSG_HPP
#define _NETMSG_HPP

#pragma once


namespace KlayGE
{
	enum
	{
		MSG_JOIN,
		MSG_QUIT,

		MSG_GETLOBBYINFO,

		MSG_NOP,
	};
}

#endif			// _NETMSG_HPP
