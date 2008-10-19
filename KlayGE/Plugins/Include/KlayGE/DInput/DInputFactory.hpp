// DInputFactory.hpp
// KlayGE DirectInput输入引擎抽象工厂 头文件
// Ver 2.0.3
// 版权所有(C) 龚敏敏, 2003-2004
// Homepage: http://klayge.sourceforge.net
//
// 2.0.3
// 改为template实现 (2004.3.4)
//
// 2.0.0
// 初次建立 (2003.8.30)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#ifndef _DINPUTFACTORY_HPP
#define _DINPUTFACTORY_HPP

#ifdef KLAYGE_HAS_DECLSPEC
	#ifdef KLAYGE_DINPUT_IE_SOURCE			// Build dll
		#define KLAYGE_DINPUT_IE_API __declspec(dllexport)
	#else									// Use dll
		#define KLAYGE_DINPUT_IE_API __declspec(dllimport)
	#endif
#else
	#define KLAYGE_DINPUT_IE_API
#endif // KLAYGE_HAS_DECLSPEC

extern "C"
{
	KLAYGE_DINPUT_IE_API void InputFactoryInstance(KlayGE::InputFactoryPtr& ptr);
	KLAYGE_DINPUT_IE_API std::string const & Name();
}

#endif			// _DINPUTFACTORY_HPP
