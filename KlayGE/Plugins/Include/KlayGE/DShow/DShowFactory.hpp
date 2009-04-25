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

#pragma once

#ifdef KLAYGE_HAS_DECLSPEC
	#ifdef KLAYGE_DSHOW_SE_SOURCE				// Build dll
		#define KLAYGE_DSHOW_SE_API __declspec(dllexport)
	#else										// Use dll
		#define KLAYGE_DSHOW_SE_API __declspec(dllimport)
	#endif
#else
	#define KLAYGE_DSHOW_SE_API
#endif // KLAYGE_HAS_DECLSPEC

extern "C"
{
	KLAYGE_DSHOW_SE_API void MakeShowFactory(KlayGE::ShowFactoryPtr& ptr, void* extra_param);
}

#endif			// _DSHOWFACTORY_HPP
