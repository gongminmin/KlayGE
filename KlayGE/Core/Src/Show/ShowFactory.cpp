// ShowFactory.cpp
// KlayGE ����������󹤳� ʵ���ļ�
// Ver 3.4.0
// ��Ȩ����(C) ������, 2006
// Homepage: http://www.klayge.org
//
// 3.4.0
// ���ν��� (2006.7.15)
//
// �޸ļ�¼
/////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KFL/Util.hpp>
#include <KlayGE/Show.hpp>

#include <string>

#include <KlayGE/ShowFactory.hpp>

namespace KlayGE
{
	ShowEngine& ShowFactory::ShowEngineInstance()
	{
		if (!se_)
		{
			se_ = this->MakeShowEngine();
		}

		return *se_;
	}

	void ShowFactory::Suspend()
	{
		if (se_)
		{
			se_->Suspend();
		}
		this->DoSuspend();
	}

	void ShowFactory::Resume()
	{
		this->DoResume();
		if (se_)
		{
			se_->Resume();
		}
	}
}
