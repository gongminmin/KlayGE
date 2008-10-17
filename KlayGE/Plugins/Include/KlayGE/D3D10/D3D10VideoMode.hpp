// D3D10VideoMode.hpp
// KlayGE D3D10显示模式 头文件
// Ver 3.8.0
// 版权所有(C) 龚敏敏, 2008
// Homepage: http://klayge.sourceforge.net
//
// 3.8.0
// 初次建立 (2008.9.21)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#ifndef _D3D10VIDEOMODE_HPP
#define _D3D10VIDEOMODE_HPP

#include <KlayGE/D3D10/D3D10MinGWDefs.hpp>
#include <d3d10.h>

namespace KlayGE
{
	// 保存显示模式信息
	/////////////////////////////////////////////////////////////////////////////////
	class D3D10VideoMode
	{
	public:
		D3D10VideoMode();
		D3D10VideoMode(uint32_t width, uint32_t height, DXGI_FORMAT format);

		uint32_t Width() const;
		uint32_t Height() const;
		DXGI_FORMAT Format() const;
		uint32_t ColorDepth() const;

	private:
		uint32_t		width_;
		uint32_t		height_;
		DXGI_FORMAT		format_;
	};

	bool operator<(D3D10VideoMode const & lhs, D3D10VideoMode const & rhs);
	bool operator==(D3D10VideoMode const & lhs, D3D10VideoMode const & rhs);
}

#endif			// _D3D10VIDEOMODE_HPP
