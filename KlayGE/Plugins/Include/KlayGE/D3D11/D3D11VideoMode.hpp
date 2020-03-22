// D3D11VideoMode.hpp
// KlayGE D3D11显示模式 头文件
// Ver 3.8.0
// 版权所有(C) 龚敏敏, 2009
// Homepage: http://www.klayge.org
//
// 3.8.0
// 初次建立 (2008.9.21)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#ifndef _D3D11VIDEOMODE_HPP
#define _D3D11VIDEOMODE_HPP

#pragma once

#include <dxgiformat.h>

namespace KlayGE
{
	// 保存显示模式信息
	/////////////////////////////////////////////////////////////////////////////////
	class D3D11VideoMode final
	{
	public:
		D3D11VideoMode();
		D3D11VideoMode(uint32_t width, uint32_t height, DXGI_FORMAT format);

		uint32_t Width() const noexcept;
		uint32_t Height() const noexcept;
		DXGI_FORMAT Format() const noexcept;

	private:
		uint32_t		width_;
		uint32_t		height_;
		DXGI_FORMAT		format_;
	};

	bool operator<(D3D11VideoMode const & lhs, D3D11VideoMode const & rhs) noexcept;
	bool operator==(D3D11VideoMode const & lhs, D3D11VideoMode const & rhs) noexcept;
}

#endif			// _D3D11VIDEOMODE_HPP
