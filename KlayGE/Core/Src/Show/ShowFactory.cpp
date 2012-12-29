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
	class NullShowFactory : public ShowFactory
	{
	public:
		std::wstring const & Name() const
		{
			static std::wstring const name(L"Null Show Factory");
			return name;
		}

		ShowEnginePtr MakeShowEngine()
		{
			return ShowEngine::NullObject();
		}
	};

	ShowFactoryPtr ShowFactory::NullObject()
	{
		static ShowFactoryPtr obj = MakeSharedPtr<NullShowFactory>();
		return obj;
	}

	ShowEngine& ShowFactory::ShowEngineInstance()
	{
		if (!se_)
		{
			se_ = this->MakeShowEngine();
		}

		return *se_;
	}
}
