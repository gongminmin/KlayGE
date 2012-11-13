// Lobby.hpp
// KlayGE 游戏大厅 头文件
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

#ifndef _LOBBY_HPP
#define _LOBBY_HPP

#pragma once

#ifndef KLAYGE_CORE_SOURCE
#define KLAYGE_LIB_NAME KlayGE_Core
#include <KlayGE/config/auto_link.hpp>
#endif

#include <vector>
#include <list>
#include <KlayGE/Socket.hpp>

#ifndef KLAYGE_PLATFORM_WINDOWS_METRO

namespace KlayGE
{
	typedef std::list<std::vector<char> > SendQueueType;

	uint32_t const Max_Buffer(64);

	class Processor
	{
	public:
        virtual ~Processor()
        {
        }

		virtual void OnJoin(uint32_t /*ID*/) const
		{
        }
		virtual void OnQuit(uint32_t /*ID*/) const
		{
        }
		virtual void OnDefault(void* /*revBuf*/, int /*maxSize*/,
			void* /*sendBuf*/, int& /*numSend*/, sockaddr_in& /*from*/) const
		{
        }
	};

	// 描述Player
	struct PlayerDes
	{
		std::string		name;
		sockaddr_in		addr;

		uint32_t				time;

		SendQueueType	msgs;
	};

	class KLAYGE_CORE_API Lobby
	{
		typedef std::vector<std::pair<uint32_t, PlayerDes> >	PlayerAddrs;
		typedef PlayerAddrs::iterator		PlayerAddrsIter;

	public:
		Lobby();
		~Lobby();

		void Create(std::string const & Name, char maxPlayers, uint16_t port, Processor const & pro);
		void Close();

		void LobbyName(std::string const & Name);
		std::string const & LobbyName() const;

		char NumPlayer() const;

		void MaxPlayers(char maxPlayers);
		char MaxPlayers() const;

		int Receive(void* buf, int maxSize, sockaddr_in& from);
		int Send(void const * buf, int maxSize, sockaddr_in const & to);

		void TimeOut(uint32_t timeOut)
			{ this->socket_.TimeOut(timeOut); }
		uint32_t TimeOut()
			{ return this->socket_.TimeOut(); }

		sockaddr_in const & SockAddr() const
			{ return this->sockAddr_; }

	private:
		void OnJoin(char* revbuf, char* sendbuf, int& sendnum, sockaddr_in& From, Processor const & pro);
		void OnQuit(PlayerAddrsIter iter, char* sendbuf, int& sendnum, Processor const & pro);

		void OnGetLobbyInfo(char* sendbuf, int& sendnum, Processor const & pro);
		void OnNop(PlayerAddrsIter iter);

		PlayerAddrsIter ID(sockaddr_in const & Addr);

	private:
		Socket			socket_;
		PlayerAddrs		players_;

		sockaddr_in		sockAddr_;

		std::string		name_;
	};
}

#endif

#endif			// _LOBBY_HPP

