// DInputFactory.hpp
// KlayGE DirectInput输入引擎抽象工厂 头文件
// Ver 2.0.0
// 版权所有(C) 龚敏敏, 2003
// Homepage: http://www.enginedev.com
//
// 2.0.0
// 初次建立 (2003.8.30)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#ifndef _DINPUTFACTORY_HPP
#define _DINPUTFACTORY_HPP

#include <KlayGE/PreDeclare.hpp>
#include <KlayGE/InputFactory.hpp>

#pragma comment(lib, "KlayGE_InputEngine_DInput.lib")

namespace KlayGE
{
	class DInputFactory : public InputFactory
	{
	public:
		const WString& Name() const;

		InputEngine& InputEngineInstance();
	};
}

#endif			// _DINPUTFACTORY_HPP