// Player.cpp
// KlayGE 游戏者 头文件
// Ver 2.1.2
// 版权所有(C) 龚敏敏, 2003-2004
// Homepage: http://klayge.sourceforge.net
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

#include <list>
#include <vector>
#include <pthread.h>
#include <KlayGE/Socket.hpp>

#pragma comment(lib, "KlayGE_Core.lib")

namespace KlayGE
{
	struct LobbyDes
	{
		char			numPlayer;
		char			maxPlayers;
		std::string		name;
		SOCKADDR_IN		addr;
	};

	class Player
	{
	public:
		Player();
		~Player();

		bool Join(const SOCKADDR_IN& lobbyAddr);
		void Quit();
		void Destroy();
		LobbyDes LobbyInfo();

		void Name(const std::string& name);
		const std::string& Name()
			{ return this->name_; }

		int Receive(void* buf, int maxSize, SOCKADDR_IN& from);
		int Send(const void* buf, int size);

	private:
		Socket		socket_;

		char		playerID_;
		std::string	name_;

		pthread_t	receiveThread_;
		bool		receiveLoop_;

		static void* ReceiveThread_Func(void* arg);

		typedef std::list<std::vector<char> > SendQueueType;
		SendQueueType	sendQueue_;
	};
}

#endif			// _PLAYER_HPP
