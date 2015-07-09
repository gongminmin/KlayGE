// Player.cpp
// KlayGE ��� ʵ���ļ�
// Ver 1.4.8.4
// ��Ȩ����(C) ������, 2003
// Homepage: http://www.klayge.org
//
// 1.4.8.3
// ���ν��� (2003.3.8)
//
// 1.4.8.4
// �����˶��߳̽��յ����� (2003.4.7)
//
// �޸ļ�¼
/////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KlayGE/Lobby.hpp>

#include <algorithm>
#include <ctime>
#include <cstring>

#include <KlayGE/NetMsg.hpp>
#include <KlayGE/Player.hpp>

#ifndef KLAYGE_PLATFORM_WINDOWS_RUNTIME

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
	// ���캯��
	/////////////////////////////////////////////////////////////////////////////////
	Player::Player()
	{
	}

	// ��������
	/////////////////////////////////////////////////////////////////////////////////
	Player::~Player()
	{
		this->Destroy();
	}

	// ��Ϣ���ܺ���
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
				// ���Ͷ��������Ϣ
				typedef decltype(sendQueue_) SendQueneType;
				for (SendQueneType::reference msg : sendQueue_)
				{
					socket_.Send(&msg[0], static_cast<int>(msg.size()));
				}
			}

			char revBuf[Max_Buffer];
			std::fill_n(revBuf, sizeof(revBuf), 0);
			if (socket_.Receive(revBuf, sizeof(revBuf)) != -1)
			{
				uint32_t ID;
				std::memcpy(&ID, &revBuf[1], 4);

				// ɾ���ѷ��͵���Ϣ
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

	// ���������
	/////////////////////////////////////////////////////////////////////////////////
	bool Player::Join(sockaddr_in const & lobbyAddr)
	{
		socket_.Close();
		socket_.Create(SOCK_DGRAM);
		socket_.Connect(lobbyAddr);

		socket_.TimeOut(2000);

		char buf[Max_Buffer];
		std::fill_n(buf, sizeof(buf), 0);

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

	// �˳�������
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

	// �������
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

	// �����������
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

	// ��������
	/////////////////////////////////////////////////////////////////////////////////
	int Player::Receive(void* buf, int maxSize, sockaddr_in& from)
	{
		return socket_.ReceiveFrom(buf, maxSize, from);
	}

	// ��������
	/////////////////////////////////////////////////////////////////////////////////
	int Player::Send(void const * buf, int size)
	{
		return socket_.Send(buf, size);
	}
}

#endif
