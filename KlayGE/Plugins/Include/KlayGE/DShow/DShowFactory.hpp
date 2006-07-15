// DShowFactory.hpp
// KlayGE DirectShow播放引擎抽象工厂 头文件
// Ver 3.4.0
// 版权所有(C) 龚敏敏, 2006
// Homepage: http://klayge.sourceforge.net
//
// 3.4.0
// 初次建立 (2006.7.15)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#ifndef _DSHOWFACTORY_HPP
#define _DSHOWFACTORY_HPP

#include <KlayGE/PreDeclare.hpp>
#include <KlayGE/ShowFactory.hpp>

#ifdef KLAYGE_DEBUG
	#pragma comment(lib, "KlayGE_ShowEngine_DShow_d.lib")
#else
	#pragma comment(lib, "KlayGE_ShowEngine_DShow.lib")
#endif

namespace KlayGE
{
	ShowFactory& DShowFactoryInstance();
}

#endif			// _DSHOWFACTORY_HPP
