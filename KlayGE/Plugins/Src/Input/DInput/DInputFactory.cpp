// DInputFactory.cpp
// KlayGE DirectInput输入引擎抽象工厂 实现文件
// Ver 2.0.0
// 版权所有(C) 龚敏敏, 2003
// Homepage: http://www.enginedev.com
//
// 2.0.0
// 初次建立 (2003.8.30)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KlayGE/DInput/DInput.hpp>

#include <KlayGE/DInput/DInputFactory.hpp>

namespace KlayGE
{
	// 输入工厂的名字
	/////////////////////////////////////////////////////////////////////////////////
	const WString& DInputFactory::Name() const
	{
		static WString name(L"DirectInput Input Factory");
		return name;
	}

	// 获取输入引擎实例
	//////////////////////////////////////////////////////////////////////////////////
	InputEngine& DInputFactory::InputEngineInstance()
	{
		static DInputEngine inputEngine;
		return inputEngine;
	}
}