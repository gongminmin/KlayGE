// Socket.hpp
// KlayGE 套接字 头文件
// Ver 1.4.8.4
// 版权所有(C) 龚敏敏, 2003
// Homepage: http://www.klayge.org
//
// 1.4.8.3
// 增加了同步Socket (2003.3.8)
//
// 1.4.8.1
// 初次建立 (2003.1.23)
//
// 修改记录
//////////////////////////////////////////////////////////////////////////////////

#ifndef	_SOCKET_HPP
#define	_SOCKET_HPP

#pragma once

#include <string>

#if defined KLAYGE_PLATFORM_WINDOWS
	#ifndef _WINSOCK_DEPRECATED_NO_WARNINGS
		#define _WINSOCK_DEPRECATED_NO_WARNINGS
	#endif
	#include <winsock2.h>
	typedef int socklen_t;
#else
	#include <sys/types.h>
	#include <sys/socket.h>
	#include <netinet/in.h>
	#include <arpa/inet.h>
	#include <sys/ioctl.h>
	#include <netdb.h>
	typedef int SOCKET;
	#define INVALID_SOCKET (~0)
	#define SOCKET_ERROR (-1)
#endif

namespace KlayGE
{
	KLAYGE_CORE_API sockaddr_in TransAddr(std::string const & address, uint16_t port);
	KLAYGE_CORE_API std::string TransAddr(sockaddr_in const & sockAddr, uint16_t& port);
	KLAYGE_CORE_API in_addr Host();

	// 同步套接字
	///////////////////////////////////////////////////////////////////////////////
	class KLAYGE_CORE_API Socket final : boost::noncopyable
	{
	public:
		Socket();
		~Socket();

		void Create(int socketType = SOCK_STREAM, int protocolType = 0, int addressFormat = PF_INET);
		void Close();

		void Accept(Socket& connectedSocket);
		void Accept(Socket& connectedSocket, sockaddr_in& sockAddr);
		void Bind(sockaddr_in const & sockAddr);

		void Connect(sockaddr_in const & sockAddr);

		void IOCtl(long command, uint32_t* argument);
		void Listen(int connectionBacklog = 5);

		int Receive(void* buf, int len, int flags = 0);
		int Send(void const * buf, int len, int flags = 0);

		int ReceiveFrom(void* buf, int len, sockaddr_in& sockFrom, int flags = 0);
		int SendTo(void const * buf, int len, sockaddr_in const & sockTo, int flags = 0);

		enum ShutDownMode
		{
			SDM_Receives = 0,
			SDM_Sends = 1,
			SDM_Both = 2,
		};
		void ShutDown(ShutDownMode how = SDM_Sends);

		void PeerName(sockaddr_in& sockAddr, socklen_t& len);
		void SockName(sockaddr_in& sockAddr, socklen_t& len);

		void SetSockOpt(int optionName, void const * optionValue,
			socklen_t optionLen, int level = SOL_SOCKET);
		void GetSockOpt(int optionName, void* optionValue,
			socklen_t& optionLen, int level = SOL_SOCKET);

		void NonBlock(bool nonBlock)
		{
			uint32_t on(nonBlock);
			this->IOCtl(FIONBIO, &on);
		}

		void TimeOut(uint32_t microSecs);
		uint32_t TimeOut();

	private:
		SOCKET		socket_;
	};
}

#endif			// _SOCKET_HPP
