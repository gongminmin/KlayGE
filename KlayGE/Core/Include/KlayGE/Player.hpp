// Player.cpp
// KlayGE 游戏者 头文件
// Ver 2.1.2
// 版权所有(C) 龚敏敏, 2003-2004
// Homepage: http://www.klayge.org
//
// 2.1.2
// 增加了发送队列 (2004.5.28)
//
// 1.4.8.3
// 初次建立 (2003.3.8)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#ifndef _PLAYER_HPP
#define _PLAYER_HPP

#pragma once

#include <list>
#include <vector>

#include <KFL/Thread.hpp>
#include <KlayGE/Socket.hpp>

namespace KlayGE
{
	struct LobbyDes
	{
		char			numPlayer;
		char			maxPlayers;
		std::string		name;
		sockaddr_in		addr;
	};

	class KLAYGE_CORE_API Player final : boost::noncopyable
	{
	public:
		Player();
		~Player();

		bool Join(sockaddr_in const & lobbyAddr);
		void Quit();
		void Destroy();
		LobbyDes LobbyInfo();

		void Name(std::string const & name);
		std::string const & Name()
			{ return this->name_; }

		int Receive(void* buf, int maxSize, sockaddr_in& from);
		int Send(void const * buf, int size);

		void ReceiveFunc();

	private:
		Socket		socket_;

		char		playerID_;
		std::string	name_;

		joiner<void>	receiveThread_;
		bool			receiveLoop_;

		std::list<std::vector<char>> sendQueue_;
	};
}

#endif			// _PLAYER_HPP
