// DShowFactory.cpp
// KlayGE DirectShow播放引擎抽象工厂 实现文件
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
#include <KlayGE/ShowFactory.hpp>

#include <KlayGE/DShow/DShow.hpp>

namespace KlayGE
{
	class DShowFactory : public ShowFactory
	{
	public:
		std::wstring const & Name() const
		{
			static std::wstring const name(L"DirectShow Show Factory");
			return name;
		}

	private:
		virtual std::unique_ptr<ShowEngine> MakeShowEngine() override
		{
			return MakeUniquePtr<DShowEngine>();
		}

		virtual void DoSuspend() override
		{
		}
		virtual void DoResume() override
		{
		}
	};
}

extern "C"
{
	KLAYGE_SYMBOL_EXPORT void MakeShowFactory(std::unique_ptr<KlayGE::ShowFactory>& ptr)
	{
		ptr = KlayGE::MakeUniquePtr<KlayGE::DShowFactory>();
	}
}
