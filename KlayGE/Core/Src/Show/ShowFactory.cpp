// ShowFactory.cpp
// KlayGE 播放引擎抽象工厂 实现文件
// Ver 3.4.0
// 版权所有(C) 龚敏敏, 2006
// Homepage: http://www.klayge.org
//
// 3.4.0
// 初次建立 (2006.7.15)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KFL/Util.hpp>
#include <KlayGE/Show.hpp>

#include <string>

#include <KlayGE/ShowFactory.hpp>

namespace KlayGE
{
	ShowFactory::~ShowFactory() noexcept = default;

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
