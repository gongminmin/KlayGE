// Socket.hpp
// KlayGE 套接字 头文件
// Ver 1.4.8.4
// 版权所有(C) 龚敏敏, 2003
// Homepage: http://www.enginedev.com
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

#include <string>

#ifdef _WIN32
	#define NOMINMAX
	#ifndef _WINSOCKAPI_
	#include <winsock.h>
	#endif
#else
	#include <sys/socket.h>
#endif

#ifdef _MSC_VER
#pragma comment(lib, "KlayGE_Core.lib")
#endif

namespace KlayGE
{
	SOCKADDR_IN TransAddr(std::string const & address, U16 port);
	std::string TransAddr(SOCKADDR_IN const & sockAddr, U16& port);
	IN_ADDR Host();

	// 同步套接字
	///////////////////////////////////////////////////////////////////////////////
	class Socket
	{
	public:
		Socket();
		~Socket();

		void Create(int socketType = SOCK_STREAM, int protocolType = 0, int addressFormat = PF_INET);
		void Close();

		void Accept(Socket& connectedSocket);
		void Accept(Socket& connectedSocket, SOCKADDR_IN& sockAddr);
		void Bind(SOCKADDR_IN const & sockAddr);

		void Connect(SOCKADDR_IN const & sockAddr);

		void IOCtl(long command, U32* argument);
		void Listen(int connectionBacklog = 5);

		int Receive(void* buf, int len, int flags = 0);
		int Send(void const * buf, int len, int flags = 0);

		int ReceiveFrom(void* buf, int len, SOCKADDR_IN& sockFrom, int flags = 0);
		int SendTo(void const * buf, int len, SOCKADDR_IN const & sockTo, int flags = 0);

		enum ShutDownMode
		{
			SDM_Receives = 0,
			SDM_Sends = 1,
			SDM_Both = 2,
		};
		void ShutDown(ShutDownMode how = SDM_Sends);

		void PeerName(SOCKADDR_IN& sockAddr, int& len);
		void SockName(SOCKADDR_IN& sockAddr, int& len);

		void SetSockOpt(int optionName, void const * optionValue,
			int optionLen, int level = SOL_SOCKET);
		void GetSockOpt(int optionName, void* optionValue,
			int& optionLen, int level = SOL_SOCKET);

		void NonBlock(bool nonBlock)
		{
			U32 on(nonBlock);
			this->IOCtl(FIONBIO, &on);
		}

		void TimeOut(U32 microSecs);
		U32 TimeOut();

	private:
		SOCKET		socket_;
	};
}

#endif			// _SOCKET_HPP
