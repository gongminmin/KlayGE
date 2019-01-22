// Player.cpp
// KlayGE 玩家 实现文件
// Ver 1.4.8.4
// 版权所有(C) 龚敏敏, 2003
// Homepage: http://www.klayge.org
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

#include <algorithm>
#include <ctime>
#include <cstring>

#include <KlayGE/NetMsg.hpp>
#include <KlayGE/Player.hpp>

namespace
{
	class ReceiveThreadFunc
	{
	public:
		explicit ReceiveThreadFunc(KlayGE::Player* player)
			: player_(player)
			{ }

		void operator()()
			{ player_->ReceiveFunc(); }

	private:
		KlayGE::Player* player_;
	};
}

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

	// 消息接受函数
	/////////////////////////////////////////////////////////////////////////////////
	void Player::ReceiveFunc()
	{
		static time_t lastTime = std::time(nullptr);

		for (;;)
		{
			if (std::time(nullptr) - lastTime >= 10 * 1000)
			{
				char msg(MSG_NOP);
				socket_.Send(&msg, sizeof(msg));
				lastTime = std::time(nullptr);
			}

			if (!sendQueue_.empty())
			{
				// 发送队列里的消息
				for (auto const & msg : sendQueue_)
				{
					socket_.Send(&msg[0], static_cast<int>(msg.size()));
				}
			}

			char revBuf[Max_Buffer];
			memset(revBuf, 0, sizeof(revBuf));
			if (socket_.Receive(revBuf, sizeof(revBuf)) != -1)
			{
				uint32_t ID;
				std::memcpy(&ID, &revBuf[1], 4);

				// 删除已发送的信息
				for (auto iter = sendQueue_.begin(); iter != sendQueue_.end();)
				{
					std::vector<char>& msg = *iter;

					uint32_t sendID;
					std::memcpy(&sendID, &msg[1], 4);
					if (sendID == ID)
					{
						iter = sendQueue_.erase(iter);
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
	}

	// 加入服务器
	/////////////////////////////////////////////////////////////////////////////////
	bool Player::Join(sockaddr_in const & lobbyAddr)
	{
		socket_.Close();
		socket_.Create(SOCK_DGRAM);
		socket_.Connect(lobbyAddr);

		socket_.TimeOut(2000);

		char buf[Max_Buffer];
		memset(buf, 0, sizeof(buf));

		buf[0] = MSG_JOIN;
		name_.copy(&buf[1], this->name_.length());

		socket_.Send(buf, sizeof(buf));

		socket_.Receive(&playerID_, sizeof(playerID_));
		if (0 == playerID_)
		{
			return false;
		}

		receiveLoop_ = true;
		receiveThread_ = Context::Instance().ThreadPool()(ReceiveThreadFunc(this));

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
			receiveThread_();
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
	void Player::Name(std::string const & name)
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
	int Player::Receive(void* buf, int maxSize, sockaddr_in& from)
	{
		return socket_.ReceiveFrom(buf, maxSize, from);
	}

	// 发送数据
	/////////////////////////////////////////////////////////////////////////////////
	int Player::Send(void const * buf, int size)
	{
		return socket_.Send(buf, size);
	}
}
