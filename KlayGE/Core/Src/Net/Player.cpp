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
#include <KlayGE/Memory.hpp>

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

			if (!player->sendQueue_.empty())
			{
				// 发送队列里的消息
				for (SendQueueType::iterator iter = player->sendQueue_.begin();
					iter != player->sendQueue_.end(); ++ iter)
				{
					std::vector<char>& msg = *iter;
					player->socket_.Send(&msg[0], msg.size());
				}
			}

			char revBuf[Max_Buffer];
			MemoryLib::Zero(revBuf, sizeof(revBuf));
			if (player->socket_.Receive(revBuf, sizeof(revBuf)) != -1)
			{
				U32 ID;
				memcpy(&ID, &revBuf[1], 4);

				// 删除已发送的信息
				for (SendQueueType::iterator iter = player->sendQueue_.begin();
					iter != player->sendQueue_.end();)
				{
					std::vector<char>& msg = *iter;

					U32 sendID;
					memcpy(&sendID, &msg[1], 4);
					if (sendID == ID)
					{
						iter = player->sendQueue_.erase(iter);
					}
					else
					{
						++ iter;
					}
				}

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
		socket_.Close();
		socket_.Create(SOCK_DGRAM);
		socket_.Connect(lobbyAddr);

		socket_.TimeOut(2000);

		char buf[Max_Buffer];
		MemoryLib::Zero(buf, sizeof(buf));

		buf[0] = MSG_JOIN;
		name_.copy(&buf[1], this->name_.length());

		socket_.Send(buf, sizeof(buf));

		socket_.Receive(&playerID_, sizeof(playerID_));
		if (0 == playerID_)
		{
			return false;
		}

		receiveLoop_ = true;
		pthread_create(&receiveThread_, NULL, ReceiveThread_Func, this);

		return true;
	}

	// 退出服务器
	/////////////////////////////////////////////////////////////////////////////////
	void Player::Quit()
	{
		if (receiveLoop_)
		{
			char msg(MSG_QUIT);
			socket_.Send(&msg, sizeof(msg));

			receiveLoop_ = false;
			pthread_join(receiveThread_, NULL);
		}
	}

	// 销毁玩家
	/////////////////////////////////////////////////////////////////////////////////
	void Player::Destroy()
	{
		this->Quit();
		socket_.Close();
	}

	LobbyDes Player::LobbyInfo()
	{
		LobbyDes lobbydes;
		lobbydes.numPlayer = 0;
		lobbydes.maxPlayers = 0;

		char msg(MSG_GETLOBBYINFO);
		socket_.Send(&msg, sizeof(msg));

		char buf[18];
		socket_.Receive(buf, sizeof(buf));
		if (MSG_GETLOBBYINFO == buf[0])
		{
			lobbydes.numPlayer = buf[1];
			lobbydes.maxPlayers = buf[2];
			size_t i(0);
			while (buf[3 + i] != 0)
			{
				++ i;
			}
			lobbydes.name = std::string(&buf[3], i);
		}

		return lobbydes;
	}

	// 设置玩家名字
	/////////////////////////////////////////////////////////////////////////////////
	void Player::Name(const std::string& name)
	{
		if (name.length() > 16)
		{
			name_ = name.substr(0, 16);
		}
		else
		{
			name_ = name;
		}
	}

	// 接收数据
	/////////////////////////////////////////////////////////////////////////////////
	int Player::Receive(void* buf, int maxSize, SOCKADDR_IN& from)
	{
		return socket_.ReceiveFrom(buf, maxSize, from);
	}

	// 发送数据
	/////////////////////////////////////////////////////////////////////////////////
	int Player::Send(const void* buf, int size)
	{
		return socket_.Send(buf, size);
	}
}
