// DShowFactory.cpp
// KlayGE DirectShow播放引擎抽象工厂 实现文件
// Ver 3.4.0
// 版权所有(C) 龚敏敏, 2006
// Homepage: http://klayge.sourceforge.net
//
// 3.4.0
// 初次建立 (2006.7.15)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KlayGE/ShowFactory.hpp>

#include <KlayGE/DShow/DShow.hpp>
#include <KlayGE/DShow/DShowFactory.hpp>

namespace KlayGE
{
	ShowFactoryPtr const & DShowFactoryInstance()
	{
		static ShowFactoryPtr sf(new ConcreteShowFactory<DShowEngine>(L"DirectShow Show Factory"));
		return sf;
	}
}
