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

#pragma once

#ifdef KLAYGE_COMPILER_MSVC
#pragma warning(push)
#pragma warning(disable: 4251 4275 4512 4702)
#endif
#include <boost/program_options.hpp>
#ifdef KLAYGE_COMPILER_MSVC
#pragma warning(pop)
#endif

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
	KLAYGE_DINPUT_IE_API void MakeInputFactory(KlayGE::InputFactoryPtr& ptr, boost::program_options::variables_map const & vm);
}

#endif			// _DINPUTFACTORY_HPP
