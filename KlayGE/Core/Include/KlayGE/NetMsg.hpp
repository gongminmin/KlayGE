// NetMsg.hpp
// KlayGE 游戏消息定义 头文件
// Ver 2.0.0
// 版权所有(C) 龚敏敏, 2003
// Homepage: http://www.enginedev.com
//
// 2.0.0
// 初次建立 (2003.7.24)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#ifndef _NETMSG_HPP
#define _NETMSG_HPP

#pragma KLAYGE_ONCE


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
