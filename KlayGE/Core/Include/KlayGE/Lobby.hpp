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

	uint32_t const Max_Buffer(64);

	class Processer
	{
	public:
		virtual void OnJoin(uint32_t /*ID*/) const
			{ }
		virtual void OnQuit(uint32_t /*ID*/) const
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

		uint32_t				time;

		SendQueueType	msgs;
	};

	class Lobby
	{
		typedef std::vector<std::pair<uint32_t, PlayerDes> >	PlayerAddrs;
		typedef PlayerAddrs::iterator		PlayerAddrsIter;

	public:
		Lobby();
		~Lobby();

		void Create(std::string const & Name, char maxPlayers, uint16_t port, Processer const & pro);
		void Close();

		void LobbyName(std::string const & Name);
		std::string const & LobbyName() const;

		char NumPlayer() const;

		void MaxPlayers(char maxPlayers);
		char MaxPlayers() const;

		int Receive(void* buf, int maxSize, SOCKADDR_IN& from);
		int Send(void const * buf, int maxSize, SOCKADDR_IN const & to);

		void TimeOut(uint32_t timeOut)
			{ this->socket_.TimeOut(timeOut); }
		uint32_t TimeOut()
			{ return this->socket_.TimeOut(); }

		SOCKADDR_IN const & SockAddr() const
			{ return this->sockAddr_; }

	private:
		void OnJoin(char* revbuf, char* sendbuf, int& sendnum, SOCKADDR_IN& From, Processer const & pro);
		void OnQuit(PlayerAddrsIter iter, char* sendbuf, int& sendnum, Processer const & pro);

		void OnGetLobbyInfo(char* sendbuf, int& sendnum, Processer const & pro);
		void OnNop(PlayerAddrsIter iter);

		PlayerAddrsIter ID(SOCKADDR_IN const & Addr);

	private:
		Socket			socket_;
		PlayerAddrs		players_;

		SOCKADDR_IN		sockAddr_;

		std::string		name_;
	};
}

#endif			// _LOBBY_HPP

