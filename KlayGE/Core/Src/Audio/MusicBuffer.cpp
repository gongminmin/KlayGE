// MusicBuffer.cpp
// KlayGE ���ֻ������� ʵ���ļ�
// Ver 2.0.0
// ��Ȩ����(C) ������, 2003
// Homepage: http://www.klayge.org
//
// 2.0.0
// ���ν��� (2003.7.7)
//
// �޸ļ�¼
/////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KlayGE/AudioDataSource.hpp>

#include <boost/assert.hpp>

#include <KlayGE/Audio.hpp>

namespace KlayGE
{
	uint32_t MusicBuffer::PreSecond = 2;

	// ���캯��
	/////////////////////////////////////////////////////////////////////////////////
	MusicBuffer::MusicBuffer(AudioDataSourcePtr const & dataSource)
					: AudioBuffer(dataSource)
	{
	}

	// ��������
	/////////////////////////////////////////////////////////////////////////////////
	MusicBuffer::~MusicBuffer()
	{
	}

	// �Ƿ�������
	/////////////////////////////////////////////////////////////////////////////////
	bool MusicBuffer::IsSound() const
	{
		return false;
	}

	// ��������λ�Ա��ڴ�ͷ����
	/////////////////////////////////////////////////////////////////////////////////
	void MusicBuffer::Reset()
	{
		this->Stop();

		this->DoReset();
	}

	// ������Ƶ��
	/////////////////////////////////////////////////////////////////////////////////
	void MusicBuffer::Play(bool loop)
	{
		this->DoStop();
		this->DoPlay(loop);
	}

	// ֹͣ������Ƶ��
	////////////////////////////////////////////////////////////////////////////////
	void MusicBuffer::Stop()
	{
		if (this->IsPlaying())
		{
			this->DoStop();
			dataSource_->Reset();
		}
	}
}
