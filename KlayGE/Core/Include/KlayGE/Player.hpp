// Player.cpp
// KlayGE 套接字 头文件
// Ver 1.4.8.3
// 版权所有(C) 龚敏敏, 2003
// Homepage: http://www.enginedev.com
//
// 1.4.8.3
// 初次建立 (2003.3.8)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#ifndef _PLAYER_HPP
#define _PLAYER_HPP

#include <pthread.h>
#include <KlayGE/Socket.hpp>

#pragma comment(lib, "KlayGE_Core.lib")

namespace KlayGE
{
	struct LobbyDes
	{
		char			PlayerNum;
		char			MaxPlayers;
		String			Name;
		SOCKADDR_IN		Addr;
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

		void Name(const String& name);
		const String& Name()
			{ return this->name_; }

		int Receive(void* buf, int maxSize, SOCKADDR_IN& from);
		int Send(const void* buf, int size);

	private:
		Socket		socket_;

		char		playerID_;
		String		name_;

		pthread_t	receiveThread_;
		bool		receiveLoop_;

		static void* ReceiveThread_Func(void* arg);
	};
}

#endif			// _PLAYER_HPP
