// Lobby.hpp
// KlayGE 游戏大厅 头文件
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

#ifndef _LOBBY_HPP
#define _LOBBY_HPP

#include <vector>
#include <list>
#include <KlayGE/Socket.hpp>

#pragma comment(lib, "KlayGE_Core.lib")

namespace KlayGE
{
	typedef std::list<std::vector<char> > SendQueueType;

	const U32 Max_Buffer(64);

	class Processer
	{
	public:
		virtual void OnJoin(U32 /*ID*/) const
			{ }
		virtual void OnQuit(U32 /*ID*/) const
			{ }
		virtual void OnDefault(PVOID /*revBuf*/, int /*maxSize*/,
			PVOID /*sendBuf*/, int& /*numSend*/, SOCKADDR_IN& /*from*/) const
			{ }
	};

	// 描述Player
	struct PlayerDes
	{
		std::string		name;
		SOCKADDR_IN		addr;

		U32				time;

		SendQueueType	msgs;
	};

	class Lobby
	{
		typedef std::vector<std::pair<U32, PlayerDes> >	PlayerAddrs;
		typedef PlayerAddrs::iterator		PlayerAddrsIter;

	public:
		Lobby();
		~Lobby();

		void Create(const std::string& Name, char maxPlayers, U16 port, const Processer& pro);
		void Close();

		void LobbyName(const std::string& Name);
		const std::string& LobbyName() const;

		char NumPlayer() const;

		void MaxPlayers(char maxPlayers);
		char MaxPlayers() const;

		int Receive(void* buf, int maxSize, SOCKADDR_IN& from);
		int Send(const void* buf, int maxSize, const SOCKADDR_IN& to);

		void TimeOut(U32 timeOut)
			{ this->socket_.TimeOut(timeOut); }
		U32 TimeOut()
			{ return this->socket_.TimeOut(); }

		const SOCKADDR_IN& SockAddr() const
			{ return this->sockAddr_; }

	private:
		void OnJoin(char* revbuf, char* sendbuf, int& sendnum, SOCKADDR_IN& From, const Processer& pro);
		void OnQuit(PlayerAddrsIter iter, char* sendbuf, int& sendnum, const Processer& pro);

		void OnGetLobbyInfo(char* sendbuf, int& sendnum, const Processer& pro);
		void OnNop(PlayerAddrsIter iter);

		PlayerAddrsIter ID(const SOCKADDR_IN& Addr);

	private:
		Socket			socket_;
		PlayerAddrs		players_;

		SOCKADDR_IN		sockAddr_;

		std::string		name_;
	};
}

#endif			// _LOBBY_HPP

