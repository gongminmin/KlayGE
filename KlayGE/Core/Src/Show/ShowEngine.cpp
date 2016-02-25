// ShowEngine.cpp
// KlayGE �������� ʵ���ļ�
// Ver 1.2.8.11
// ��Ȩ����(C) ������, 2001--2002
// Homepage: http://www.klayge.org
//
// 1.2.8.10
// ��string�����ַ���ָ�� (2002.10.27)
//
// 1.2.8.11
// ����UNICODE���� (2002.11.7)
//
// �޸ļ�¼
/////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KFL/Util.hpp>
#include <KlayGE/Texture.hpp>

#include <KlayGE/Show.hpp>

namespace KlayGE
{
	ShowEngine::ShowEngine()
		: state_(SS_Uninit),
			resume_playing_(false)
	{
	}

	ShowEngine::~ShowEngine()
	{
	}

	void ShowEngine::Suspend()
	{
		if (SS_Playing == state_)
		{
			resume_playing_ = true;
			this->Pause();
		}
		this->DoSuspend();
	}

	void ShowEngine::Resume()
	{
		this->DoResume();
		if (resume_playing_)
		{
			this->Play();
			resume_playing_ = false;
		}
	}

	// ���Բ���
	/////////////////////////////////////////////////////////////////////////////////
	bool ShowEngine::CanPlay() const
	{
		return (SS_Stopped == state_) || (SS_Paused == state_);
	}

	// ����ֹͣ
	/////////////////////////////////////////////////////////////////////////////////
	bool ShowEngine::CanStop() const
	{
		return (SS_Playing == state_) || (SS_Paused == state_);
	}

	// ������ͣ
	/////////////////////////////////////////////////////////////////////////////////
	bool ShowEngine::CanPause() const
	{
		return (SS_Playing == state_) || (SS_Paused == state_);
	}

	// ��ʼ�����
	/////////////////////////////////////////////////////////////////////////////////
	bool ShowEngine::IsInitialized() const
	{
		return state_ != SS_Uninit;
	}

	// ����
	/////////////////////////////////////////////////////////////////////////////////
	void ShowEngine::Play()
	{
		if (this->CanPlay())
		{
			this->DoPlay();

			state_ = SS_Playing;
		}
	}

	// ��ͣ����
	/////////////////////////////////////////////////////////////////////////////////
	void ShowEngine::Pause()
	{
		if (this->CanPause())
		{
			this->DoPause();

			state_ = SS_Paused;
		}
	}

	// ֹͣ����
	/////////////////////////////////////////////////////////////////////////////////
	void ShowEngine::Stop()
	{
		if (this->CanStop())
		{
			this->DoStop();

			state_ = SS_Stopped;
		}
	}
}
