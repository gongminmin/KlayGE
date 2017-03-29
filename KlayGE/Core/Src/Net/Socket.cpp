// Socket.cpp
// KlayGE �׽��� ʵ���ļ�
// Ver 1.4.8.4
// ��Ȩ����(C) ������, 2003
// Homepage: http://www.klayge.org
//
// 1.4.8.1
// ���ν��� (2003.1.23)
//
// 1.4.8.4
// TransAddr֧�������ֽ��� (2003.4.3)
//
// �޸ļ�¼
/////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KFL/ErrorHandling.hpp>

#include <cstring>
#include <system_error>
#include <boost/assert.hpp>

#include <KlayGE/Socket.hpp>

#ifndef KLAYGE_PLATFORM_WINDOWS_STORE

#ifdef KLAYGE_COMPILER_MSVC
#ifndef KLAYGE_CPU_ARM
#pragma comment(lib, "wsock32.lib")
#endif
#endif

#if defined KLAYGE_PLATFORM_WINDOWS
	// ��ʼ��Winsock
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
#endif

namespace KlayGE
{
	// ���������ַ
	/////////////////////////////////////////////////////////////////////////////////
	sockaddr_in TransAddr(std::string const & address, uint16_t port)
	{
		sockaddr_in sockAddr_in;
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
			hostent* pHostEnt = gethostbyname(address.c_str());
			if (pHostEnt != nullptr)
			{
				std::memcpy(&sockAddr_in.sin_addr.s_addr,
					pHostEnt->h_addr_list[0], pHostEnt->h_length);
			}
			else
			{
				TERRC(std::errc::bad_address);
			}
		}

		sockAddr_in.sin_family = AF_INET;
		sockAddr_in.sin_port = htons(port);

		return sockAddr_in;
	}

	std::string TransAddr(sockaddr_in const & sockAddr, uint16_t& port)
	{
		port = ntohs(sockAddr.sin_port);
		return std::string(inet_ntoa(sockAddr.sin_addr));
	}

	// ��ȡ������ַ
	/////////////////////////////////////////////////////////////////////////////////
	in_addr Host()
	{
		in_addr addr;
		memset(&addr, 0, sizeof(addr));

		char host[256];
		if (0 == gethostname(host, sizeof(host)))
		{
			hostent* pHostEnt = gethostbyname(host);
#if defined KLAYGE_PLATFORM_WINDOWS
			std::memcpy(&addr.S_un.S_addr, pHostEnt->h_addr_list[0], pHostEnt->h_length);
#else
			std::memcpy(&addr.s_addr, pHostEnt->h_addr_list[0], pHostEnt->h_length);
#endif
		}

		return addr;
	}


	// ���캯��
	/////////////////////////////////////////////////////////////////////////////////
	Socket::Socket()
			: socket_(INVALID_SOCKET)
	{
	}

	// ��������
	/////////////////////////////////////////////////////////////////////////////////
	Socket::~Socket()
	{
		this->Close();
	}

	// �����׽���
	/////////////////////////////////////////////////////////////////////////////////
	void Socket::Create(int socketType, int protocolType, int addressFormat)
	{
		this->Close();

		this->socket_ = socket(addressFormat, socketType, protocolType);
		BOOST_ASSERT(this->socket_ != INVALID_SOCKET);
	}

	// �ر��׽���
	/////////////////////////////////////////////////////////////////////////////////
	void Socket::Close()
	{
		if (this->socket_ != INVALID_SOCKET)
		{
#ifdef KLAYGE_PLATFORM_WINDOWS
			closesocket(this->socket_);
#else
			close(this->socket_);
#endif
			this->socket_ = INVALID_SOCKET;
		}
	}

	// �����Ӧ��
	/////////////////////////////////////////////////////////////////////////////////
	void Socket::Accept(Socket& connectedSocket, sockaddr_in& sockAddr)
	{
		connectedSocket.Close();

		socklen_t len(sizeof(sockAddr));
		connectedSocket.socket_ = accept(this->socket_,
			reinterpret_cast<sockaddr*>(&sockAddr), &len);
	}

	void Socket::Accept(Socket& connectedSocket)
	{
		connectedSocket.Close();

		connectedSocket.socket_ = accept(this->socket_, nullptr, nullptr);
	}

	// �󶨶˿�
	/////////////////////////////////////////////////////////////////////////////////
	void Socket::Bind(sockaddr_in const & sockAddr)
	{
		BOOST_ASSERT(this->socket_ != INVALID_SOCKET);

		Verify(::bind(this->socket_, reinterpret_cast<sockaddr const *>(&sockAddr),
			sizeof(sockAddr)) != SOCKET_ERROR);
	}

	// �׽���IO����
	/////////////////////////////////////////////////////////////////////////////////
	void Socket::IOCtl(long command, uint32_t* argument)
	{
		BOOST_ASSERT(this->socket_ != INVALID_SOCKET);

#ifdef KLAYGE_PLATFORM_WINDOWS
		Verify(ioctlsocket(this->socket_, command, reinterpret_cast<u_long*>(argument)) != SOCKET_ERROR);
#else
		Verify(ioctl(this->socket_, command, argument) != SOCKET_ERROR);
#endif
	}

	// ����˼���
	/////////////////////////////////////////////////////////////////////////////////
	void Socket::Listen(int connectionBacklog)
	{
		BOOST_ASSERT(this->socket_ != INVALID_SOCKET);

		Verify(listen(this->socket_, connectionBacklog) != SOCKET_ERROR);
	}

	// �����ӵ�����·�������
	/////////////////////////////////////////////////////////////////////////////////
	int Socket::Send(void const * buf, int len, int flags)
	{
		BOOST_ASSERT(this->socket_ != INVALID_SOCKET);

		return send(this->socket_, static_cast<char const *>(buf), len, flags);
	}

	// �����ӵ�����½�������
	/////////////////////////////////////////////////////////////////////////////////
	int Socket::Receive(void* buf, int len, int flags)
	{
		BOOST_ASSERT(this->socket_ != INVALID_SOCKET);

		return recv(this->socket_, static_cast<char*>(buf), len, flags);
	}

	// ǿ�ƹر�
	/////////////////////////////////////////////////////////////////////////////////
	void Socket::ShutDown(ShutDownMode how)
	{
		BOOST_ASSERT(this->socket_ != INVALID_SOCKET);

		Verify(shutdown(this->socket_, how) != SOCKET_ERROR);
	}

	// �����ӵ�����»�ȡ������
	/////////////////////////////////////////////////////////////////////////////////
	void Socket::PeerName(sockaddr_in& sockAddr, socklen_t& len)
	{
		BOOST_ASSERT(this->socket_ != INVALID_SOCKET);

		Verify(getpeername(this->socket_,
			reinterpret_cast<sockaddr*>(&sockAddr), &len) != SOCKET_ERROR);
	}

	// ��ȡ�׽�������
	/////////////////////////////////////////////////////////////////////////////////
	void Socket::SockName(sockaddr_in& sockAddr, socklen_t& len)
	{
		BOOST_ASSERT(this->socket_ != INVALID_SOCKET);

		Verify(getsockname(this->socket_,
			reinterpret_cast<sockaddr*>(&sockAddr), &len) != SOCKET_ERROR);
	}

	// �����׽��ֲ���
	/////////////////////////////////////////////////////////////////////////////////
	void Socket::SetSockOpt(int optionName, void const * optionValue, socklen_t optionLen, int level)
	{
		BOOST_ASSERT(this->socket_ != INVALID_SOCKET);

		Verify(setsockopt(this->socket_, level, optionName,
			static_cast<char const *>(optionValue), optionLen) != SOCKET_ERROR);
	}

	// ��ȡ�׽��ֲ���
	/////////////////////////////////////////////////////////////////////////////////
	void Socket::GetSockOpt(int optionName, void* optionValue, socklen_t& optionLen, int level)
	{
		BOOST_ASSERT(this->socket_ != INVALID_SOCKET);

		Verify(getsockopt(this->socket_, level, optionName,
			static_cast<char*>(optionValue), &optionLen) != SOCKET_ERROR);
	}

	// ����������½�������
	/////////////////////////////////////////////////////////////////////////////////
	int Socket::ReceiveFrom(void* buf, int len, sockaddr_in& sockFrom, int flags)
	{
		BOOST_ASSERT(this->socket_ != INVALID_SOCKET);

		socklen_t fromLen(sizeof(sockFrom));
		return recvfrom(this->socket_, static_cast<char*>(buf), len, flags,
			reinterpret_cast<sockaddr*>(&sockFrom), &fromLen);
	}

	// ����������·�������
	/////////////////////////////////////////////////////////////////////////////////
	int Socket::SendTo(void const * buf, int len, sockaddr_in const & sockTo, int flags)
	{
		BOOST_ASSERT(this->socket_ != INVALID_SOCKET);

		return sendto(this->socket_, static_cast<char const *>(buf), len, flags,
			reinterpret_cast<sockaddr const *>(&sockTo), sizeof(sockTo));
	}

	// ���ӷ����
	/////////////////////////////////////////////////////////////////////////////////
	void Socket::Connect(sockaddr_in const & sockAddr)
	{
		BOOST_ASSERT(this->socket_ != INVALID_SOCKET);

		Verify(connect(this->socket_,
			reinterpret_cast<sockaddr const *>(&sockAddr), sizeof(sockAddr)) != SOCKET_ERROR);
	}

	// ���ó�ʱʱ��
	/////////////////////////////////////////////////////////////////////////////////
	void Socket::TimeOut(uint32_t MicroSecs)
	{
		timeval timeOut;

		timeOut.tv_sec = MicroSecs / 1000;
		timeOut.tv_usec = MicroSecs % 1000;

		SetSockOpt(SO_RCVTIMEO, &timeOut, sizeof(timeOut));
		SetSockOpt(SO_SNDTIMEO, &timeOut, sizeof(timeOut));
	}

	// ��ȡ��ʱʱ��
	/////////////////////////////////////////////////////////////////////////////////
	uint32_t Socket::TimeOut()
	{
		timeval timeOut;
		socklen_t len(sizeof(timeOut));

		this->GetSockOpt(SO_RCVTIMEO, &timeOut, len);

		return timeOut.tv_sec * 1000 + timeOut.tv_usec;
	}
}

#endif
