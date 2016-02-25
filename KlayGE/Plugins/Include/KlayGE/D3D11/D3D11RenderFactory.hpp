// D3D11RenderFactory.hpp
// KlayGE D3D11渲染引擎抽象工厂 头文件
// Ver 3.8.0
// 版权所有(C) 龚敏敏, 2009
// Homepage: http://www.klayge.org
//
// 3.8.0
// 初次建立 (2009.1.30)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#ifndef _D3D11RENDERFACTORY_HPP
#define _D3D11RENDERFACTORY_HPP

#pragma once

#include <KlayGE/PreDeclare.hpp>

#ifdef KLAYGE_D3D11_RE_SOURCE				// Build dll
	#define KLAYGE_D3D11_RE_API KLAYGE_SYMBOL_EXPORT
#else										// Use dll
	#define KLAYGE_D3D11_RE_API KLAYGE_SYMBOL_IMPORT
#endif

extern "C"
{
	KLAYGE_D3D11_RE_API void MakeRenderFactory(std::unique_ptr<KlayGE::RenderFactory>& ptr);
}

#endif			// _D3D11RENDERFACTORY_HPP
