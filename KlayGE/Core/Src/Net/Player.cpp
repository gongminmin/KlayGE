// Player.cpp
// KlayGE 玩家 实现文件
// Ver 1.4.8.4
// 版权所有(C) 龚敏敏, 2003
// Homepage: http://www.enginedev.com
//
// 1.4.8.3
// 初次建立 (2003.3.8)
//
// 1.4.8.4
// 增加了多线程接收的能力 (2003.4.7)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KlayGE/Lobby.hpp>
#include <KlayGE/SharePtr.hpp>
#include <KlayGE/Memory.hpp>
#include <KlayGE/Engine.hpp>

#include <ctime>

#include <KlayGE/NetMsg.hpp>
#include <KlayGE/Player.hpp>

namespace KlayGE
{
	// 构造函数
	/////////////////////////////////////////////////////////////////////////////////
	Player::Player()
	{
	}

	// 析构函数
	/////////////////////////////////////////////////////////////////////////////////
	Player::~Player()
	{
		this->Destroy();
	}

	void* Player::ReceiveThread_Func(void* arg)
	{
		Player* player(reinterpret_cast<Player*>(arg));
		static time_t lastTime = std::time(NULL);

		for (;;)
		{
			if (std::time(NULL) - lastTime >= 10 * 1000)
			{
				char msg(MSG_NOP);
				player->socket_.Send(&msg, sizeof(msg));
				lastTime = std::time(NULL);
			}

			char revBuf[Max_Buffer];
			Engine::MemoryInstance().Zero(revBuf, sizeof(revBuf));
			if (player->socket_.Receive(revBuf, sizeof(revBuf)) != -1)
			{
				if (MSG_QUIT == revBuf[0])
				{
					break;
				}
			}
		}

		return NULL;
	}

	// 加入服务器
	/////////////////////////////////////////////////////////////////////////////////
	bool Player::Join(const SOCKADDR_IN& lobbyAddr)
	{
		this->socket_.Close();
		this->socket_.Create(SOCK_DGRAM);
		this->socket_.Connect(lobbyAddr);

		this->socket_.TimeOut(2000);

		char buf[Max_Buffer];
		Engine::MemoryInstance().Zero(buf, sizeof(buf));

		buf[0] = MSG_JOIN;
		this->name_.copy(&buf[1], this->name_.length());

		this->socket_.Send(buf, sizeof(buf));

		this->socket_.Receive(&this->playerID_, sizeof(this->playerID_));
		if (0 == this->playerID_)
		{
			return false;
		}

		this->receiveLoop_ = true;
		pthread_create(&receiveThread_, NULL, ReceiveThread_Func, this);

		return true;
	}

	// 退出服务器
	/////////////////////////////////////////////////////////////////////////////////
	void Player::Quit()
	{
		if (this->receiveLoop_)
		{
			char msg(MSG_QUIT);
			this->socket_.Send(&msg, sizeof(msg));

			this->receiveLoop_ = false;
			pthread_join(this->receiveThread_, NULL);
		}
	}

	// 销毁玩家
	/////////////////////////////////////////////////////////////////////////////////
	void Player::Destroy()
	{
		this->Quit();
		this->socket_.Close();
	}

	LobbyDes Player::LobbyInfo()
	{
		LobbyDes lobbydes;
		lobbydes.PlayerNum = 0;
		lobbydes.MaxPlayers = 0;

		char msg(MSG_GETLOBBYINFO);
		this->socket_.Send(&msg, sizeof(msg));

		char buf[18];
		this->socket_.Receive(buf, sizeof(buf));
		if (MSG_GETLOBBYINFO == buf[0])
		{
			lobbydes.PlayerNum = buf[1];
			lobbydes.MaxPlayers = buf[2];
			size_t i(0);
			while (buf[3 + i] != 0)
			{
				++ i;
			}
			lobbydes.Name = String(&buf[3], i);
		}

		return lobbydes;
	}

	// 设置玩家名字
	/////////////////////////////////////////////////////////////////////////////////
	void Player::Name(const String& name)
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

	// 接收数据
	/////////////////////////////////////////////////////////////////////////////////
	int Player::Receive(void* buf, int maxSize, SOCKADDR_IN& from)
	{
		return this->socket_.ReceiveFrom(buf, maxSize, from);
	}

	// 发送数据
	/////////////////////////////////////////////////////////////////////////////////
	int Player::Send(const void* buf, int size)
	{
		return this->socket_.Send(buf, size);
	}
}