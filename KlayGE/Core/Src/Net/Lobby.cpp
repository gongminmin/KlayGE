// Lobby.cpp
// KlayGE 游戏大厅 实现文件
// Ver 1.4.8.3
// 版权所有(C) 龚敏敏, 2003
// Homepage: http://www.enginedev.com
//
// 1.4.8.3
// 初次建立 (2003.3.8)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KlayGE/Player.hpp>
#include <KlayGE/SharedPtr.hpp>
#include <KlayGE/Memory.hpp>

#include <algorithm>
#include <ctime>

#include <KlayGE/NetMsg.hpp>
#include <KlayGE/Lobby.hpp>

namespace KlayGE
{
	// 构造函数
	/////////////////////////////////////////////////////////////////////////////////
	Lobby::Lobby()
	{
		this->socket_.Create(SOCK_DGRAM);
	}

	// 析构函数
	/////////////////////////////////////////////////////////////////////////////////
	Lobby::~Lobby()
	{
		Close();
	}

	Lobby::PlayerAddrsIter Lobby::ID(const SOCKADDR_IN& addr)
	{
		for (PlayerAddrsIter iter = this->players_.begin(); iter != this->players_.end(); ++ iter)
		{
			if (0 == MemoryLib::Compare(&addr, &(iter->Addr), sizeof(addr)))
			{
				return iter;
			}
		}

		return this->players_.end();
	}

	// 建立游戏大厅
	/////////////////////////////////////////////////////////////////////////////////
	void Lobby::Create(const std::string& Name, char maxPlayers, U16 port, const Processer& pro)
	{
		this->LobbyName(Name);

		this->MaxPlayers(maxPlayers);

		this->socket_.Bind(TransAddr("", port));

		SOCKADDR_IN from;
		char revBuf[Max_Buffer];
		char sendBuf[Max_Buffer];
		int numSend = 0;
		for (;;)
		{
			this->Receive(revBuf, sizeof(revBuf), from);

			// 每个消息前面都包含1字节的消息类型
			char* revPtr(&revBuf[1]);
			char* sendPtr(&sendBuf[1]);
			sendBuf[0] = revBuf[0];

			switch (revBuf[0])
			{
			case MSG_JOIN:
				this->OnJoin(revPtr, sendPtr, numSend, from, pro);
				break;

			case MSG_QUIT:
				this->OnQuit(this->ID(from), sendPtr, numSend, pro);
				break;

			case MSG_GETLOBBYINFO:
				this->OnGetLobbyInfo(sendPtr, numSend, pro);
				break;

			case MSG_NOP:
				this->OnNop(this->ID(from));
				break;

			default:
				pro.OnDefault(revBuf, sizeof(revBuf), sendBuf, numSend, from);
				break;
			}

			if (numSend != 0)
			{
				this->Send(sendBuf, numSend + 1, from);
			}
		}
	}

	// 设置最大人数
	/////////////////////////////////////////////////////////////////////////////////
	char Lobby::NumPlayer() const
	{
		char n = 0;
		for (PlayerAddrs::const_iterator iter = this->players_.begin();
			iter != this->players_.end(); ++ iter)
		{
			if (iter->ID != 0)
			{
				++ n;
			}
		}

		return n;
	}

	// 设置大厅名称
	/////////////////////////////////////////////////////////////////////////////////
	void Lobby::LobbyName(const std::string& name)
	{
		if (name.length() > 16)
		{
			this->name_ = name.substr(0, 16);
		}
		else
		{
			this->name_ = name;
		}
	}

	// 获取大厅名称
	/////////////////////////////////////////////////////////////////////////////////
	const std::string& Lobby::LobbyName() const
	{
		return this->name_;
	}

	// 设置最大人数
	/////////////////////////////////////////////////////////////////////////////////
	void Lobby::MaxPlayers(char maxPlayers)
	{
		this->players_.resize(maxPlayers);
		PlayerAddrs(this->players_).swap(this->players_);

		for (PlayerAddrsIter iter = this->players_.begin();
						iter != this->players_.end(); ++ iter)
		{
			iter->ID = 0;
		}
	}

	// 获取最大人数
	/////////////////////////////////////////////////////////////////////////////////
	char Lobby::MaxPlayers() const
	{
		return static_cast<char>(this->players_.size());
	}

	// 关闭游戏大厅
	/////////////////////////////////////////////////////////////////////////////////
	void Lobby::Close()
	{
		this->socket_.Close();
	}

	// 发送数据
	/////////////////////////////////////////////////////////////////////////////////
	int Lobby::Receive(void* buf, int maxSize, SOCKADDR_IN& from)
	{
		return this->socket_.ReceiveFrom(buf, maxSize, from);
	}

	// 接收数据
	/////////////////////////////////////////////////////////////////////////////////
	int Lobby::Send(const void* buf, int maxSize, const SOCKADDR_IN& to)
	{
		return this->socket_.SendTo(buf, maxSize, to);
	}


	void Lobby::OnJoin(char* revBuf, char* sendBuf, int& numSend,
							SOCKADDR_IN& from, const Processer& pro)
	{
		// 命令格式:
		//			Player名字		16 字节

		char id(1);
		PlayerAddrsIter iter(this->players_.begin());
		for (; iter != this->players_.end(); ++ iter, ++ id)
		{
			if (0 == iter->ID)
			{
				size_t i(0);
				while (revBuf[i] != 0)
				{
					++ i;
				}
				std::string name(&revBuf[0], i);
				iter->ID	= id;
				iter->Name	= name;
				iter->Addr	= from;

				pro.OnJoin(iter->ID);
				break;
			}
		}

		// 返回格式:
		//			Player ID		1 字节

		// 已经满了
		if (iter == this->players_.end())
		{
			sendBuf[0] = 1;
		}
		else
		{
			sendBuf[0] = 0;
		}

		numSend = 1;
	}

	void Lobby::OnQuit(PlayerAddrsIter iter, char* sendBuf,
							int& numSend, const Processer& pro)
	{
		if (iter != this->players_.end())
		{
			this->players_[iter->ID].ID = 0;
			sendBuf[0] = 0;
			pro.OnQuit(iter->ID);
		}
		else
		{
			sendBuf[0] = 1;
		}

		numSend = 1;
	}

	void Lobby::OnGetLobbyInfo(char* sendBuf, int& numSend, const Processer& /*pro*/)
	{
		// 返回格式:
		//			当前Players数	1 字节
		//			最大Players数	1 字节
		//			Lobby名字		16 字节

		MemoryLib::Zero(sendBuf, 18);
		sendBuf[0] = this->NumPlayer();
		sendBuf[1] = this->MaxPlayers();
		MemoryLib::Copy(&sendBuf[2], this->LobbyName().c_str(), this->LobbyName().length());
		numSend = 18;
	}

	void Lobby::OnNop(PlayerAddrsIter iter)
	{
		if (iter != this->players_.end())
		{
			iter->Time = static_cast<U32>(std::time(NULL));
		}
	}
}
