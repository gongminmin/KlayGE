// Socket.cpp
// KlayGE 套接字 实现文件
// Ver 1.4.8.4
// 版权所有(C) 龚敏敏, 2003
// Homepage: http://www.enginedev.com
//
// 1.4.8.1
// 初次建立 (2003.1.23)
//
// 1.4.8.4
// TransAddr支持了名字解析 (2003.4.3)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KlayGE/ThrowErr.hpp>

#include <cassert>
#include <cstring>

#include <KlayGE/Socket.hpp>

#pragma comment(lib, "wsock32.lib")

// 初始化Winsock
/////////////////////////////////////////////////////////////////////////////////
class WSAIniter
{
public:
	WSAIniter()
	{
		WSADATA wsaData;

		WSAStartup(MAKEWORD(2, 0), &wsaData);
	}

	~WSAIniter()
	{
		WSACleanup();
	}
} wsaInit;

namespace KlayGE
{
	// 翻译网络地址
	/////////////////////////////////////////////////////////////////////////////////
	SOCKADDR_IN TransAddr(std::string const & address, uint16_t port)
	{
		SOCKADDR_IN sockAddr_in;
		std::memset(&sockAddr_in, 0, sizeof(sockAddr_in));

		if (address.empty())
		{
			sockAddr_in.sin_addr.s_addr = htonl(INADDR_ANY);
		}
		else
		{
			sockAddr_in.sin_addr.s_addr = inet_addr(address.c_str());
		}

		if (INADDR_NONE == sockAddr_in.sin_addr.s_addr)
		{
			LPHOSTENT pHostEnt = gethostbyname(address.c_str());
			if (pHostEnt != NULL)
			{
				std::memcpy(&sockAddr_in.sin_addr.s_addr,
					pHostEnt->h_addr_list[0], pHostEnt->h_length);
			}
			else
			{
				THR(E_FAIL);
			}
		}

		sockAddr_in.sin_family = AF_INET;
		sockAddr_in.sin_port = htons(port);

		return sockAddr_in;
	}

	std::string TransAddr(SOCKADDR_IN const & sockAddr, uint16_t& port)
	{
		port = ntohs(sockAddr.sin_port);
		return std::string(inet_ntoa(sockAddr.sin_addr));
	}

	// 获取主机地址
	/////////////////////////////////////////////////////////////////////////////////
	IN_ADDR Host()
	{
		IN_ADDR addr;
		memset(&addr, 0, sizeof(addr));

		char host[256];
		if (0 == gethostname(host, sizeof(host)))
		{
			HOSTENT* pHostEnt = gethostbyname(host);
			std::memcpy(&addr.S_un.S_addr, pHostEnt->h_addr_list[0], pHostEnt->h_length);
		}

		return addr;
	}


	// 构造函数
	/////////////////////////////////////////////////////////////////////////////////
	Socket::Socket()
			: socket_(INVALID_SOCKET)
	{
	}

	// 析构函数
	/////////////////////////////////////////////////////////////////////////////////
	Socket::~Socket()
	{
		this->Close();
	}

	// 建立套接字
	/////////////////////////////////////////////////////////////////////////////////
	void Socket::Create(int socketType, int protocolType, int addressFormat)
	{
		this->Close();

		this->socket_ = socket(addressFormat, socketType, protocolType);
		assert(this->socket_ != INVALID_SOCKET);
	}

	// 关闭套接字
	/////////////////////////////////////////////////////////////////////////////////
	void Socket::Close()
	{
		if (this->socket_ != INVALID_SOCKET)
		{
	#ifdef _WIN32
			closesocket(this->socket_);
	#else
			close(this->socket_);
	#endif
			this->socket_ = INVALID_SOCKET;
		}
	}

	// 服务端应答
	/////////////////////////////////////////////////////////////////////////////////
	void Socket::Accept(Socket& connectedSocket, SOCKADDR_IN& sockAddr)
	{
		connectedSocket.Close();

		int len(sizeof(sockAddr));
		connectedSocket.socket_ = accept(this->socket_,
			reinterpret_cast<SOCKADDR*>(&sockAddr), &len);
	}

	void Socket::Accept(Socket& connectedSocket)
	{
		connectedSocket.Close();

		connectedSocket.socket_ = accept(this->socket_, NULL, NULL);
	}

	// 绑定端口
	/////////////////////////////////////////////////////////////////////////////////
	void Socket::Bind(SOCKADDR_IN const & sockAddr)
	{
		assert(this->socket_ != INVALID_SOCKET);

		Verify(bind(this->socket_, reinterpret_cast<SOCKADDR const *>(&sockAddr),
			sizeof(sockAddr)) != SOCKET_ERROR);
	}

	// 套接字IO控制
	/////////////////////////////////////////////////////////////////////////////////
	void Socket::IOCtl(long command, uint32_t* argument)
	{
		assert(this->socket_ != INVALID_SOCKET);

	#ifdef _WIN32
		Verify(ioctlsocket(this->socket_, command, argument) != SOCKET_ERROR);
	#else
		Verify(ioctl(this->socket_, command, argument) != SOCKET_ERROR);
	#endif
	}

	// 服务端监听
	/////////////////////////////////////////////////////////////////////////////////
	void Socket::Listen(int connectionBacklog)
	{
		assert(this->socket_ != INVALID_SOCKET);
		
		Verify(listen(this->socket_, connectionBacklog) != SOCKET_ERROR);
	}

	// 有连接的情况下发送数据
	/////////////////////////////////////////////////////////////////////////////////
	int Socket::Send(void const * buf, int len, int flags)
	{
		assert(this->socket_ != INVALID_SOCKET);

		return send(this->socket_, static_cast<char const *>(buf), len, flags);
	}

	// 有连接的情况下接收数据
	/////////////////////////////////////////////////////////////////////////////////
	int Socket::Receive(void* buf, int len, int flags)
	{
		assert(this->socket_ != INVALID_SOCKET);

		return recv(this->socket_, static_cast<char*>(buf), len, flags);
	}

	// 强制关闭
	/////////////////////////////////////////////////////////////////////////////////
	void Socket::ShutDown(ShutDownMode how)
	{
		assert(this->socket_ != INVALID_SOCKET);

		Verify(shutdown(this->socket_, how) != SOCKET_ERROR);
	}

	// 有连接的情况下获取点名称
	/////////////////////////////////////////////////////////////////////////////////
	void Socket::PeerName(SOCKADDR_IN& sockAddr, int& len)
	{
		assert(this->socket_ != INVALID_SOCKET);

		Verify(getpeername(this->socket_,
			reinterpret_cast<SOCKADDR*>(&sockAddr), &len) != SOCKET_ERROR);
	}

	// 获取套接字名称
	/////////////////////////////////////////////////////////////////////////////////
	void Socket::SockName(SOCKADDR_IN& sockAddr, int& len)
	{
		assert(this->socket_ != INVALID_SOCKET);

		Verify(getsockname(this->socket_,
			reinterpret_cast<SOCKADDR*>(&sockAddr), &len) != SOCKET_ERROR);
	}

	// 设置套接字参数
	/////////////////////////////////////////////////////////////////////////////////
	void Socket::SetSockOpt(int optionName, void const * optionValue, int optionLen, int level)
	{
		assert(this->socket_ != INVALID_SOCKET);

		Verify(setsockopt(this->socket_, level, optionName,
			static_cast<char const *>(optionValue), optionLen) != SOCKET_ERROR);
	}

	// 获取套接字参数
	/////////////////////////////////////////////////////////////////////////////////
	void Socket::GetSockOpt(int optionName, void* optionValue, int& optionLen, int level)
	{
		assert(this->socket_ != INVALID_SOCKET);

		Verify(getsockopt(this->socket_, level, optionName,
			static_cast<char*>(optionValue), &optionLen) != SOCKET_ERROR);
	}

	// 无连接情况下接收数据
	/////////////////////////////////////////////////////////////////////////////////
	int Socket::ReceiveFrom(void* buf, int len, SOCKADDR_IN& sockFrom, int flags)
	{
		assert(this->socket_ != INVALID_SOCKET);

		int fromLen(sizeof(sockFrom));
		return recvfrom(this->socket_, static_cast<char*>(buf), len, flags,
			reinterpret_cast<SOCKADDR*>(&sockFrom), &fromLen);
	}

	// 无连接情况下发送数据
	/////////////////////////////////////////////////////////////////////////////////
	int Socket::SendTo(void const * buf, int len, SOCKADDR_IN const & sockTo, int flags)
	{
		assert(this->socket_ != INVALID_SOCKET);

		return sendto(this->socket_, static_cast<char const *>(buf), len, flags,
			reinterpret_cast<SOCKADDR const *>(&sockTo), sizeof(sockTo));
	}

	// 连接服务端
	/////////////////////////////////////////////////////////////////////////////////
	void Socket::Connect(SOCKADDR_IN const & sockAddr)
	{
		assert(this->socket_ != INVALID_SOCKET);

		Verify(connect(this->socket_,
			reinterpret_cast<SOCKADDR const *>(&sockAddr), sizeof(sockAddr)) != SOCKET_ERROR);
	}

	// 设置超时时间
	/////////////////////////////////////////////////////////////////////////////////
	void Socket::TimeOut(uint32_t MicroSecs)
	{
		timeval timeOut;

		timeOut.tv_sec = MicroSecs / 1000;
		timeOut.tv_usec = MicroSecs % 1000;

		SetSockOpt(SO_RCVTIMEO, &timeOut, sizeof(timeOut));
		SetSockOpt(SO_SNDTIMEO, &timeOut, sizeof(timeOut));
	}

	// 获取超时时间
	/////////////////////////////////////////////////////////////////////////////////
	uint32_t Socket::TimeOut()
	{
		timeval timeOut;
		int len(sizeof(timeOut));

		GetSockOpt(SO_RCVTIMEO, &timeOut, len);

		return timeOut.tv_sec * 1000 + timeOut.tv_usec;
	}
}