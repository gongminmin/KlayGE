#ifndef _D3D9VIDEOMODE_HPP
#define _D3D9VIDEOMODE_HPP

#include <d3d9.h>
#include <vector>

#pragma comment(lib, "KlayGE_RenderEngine_D3D9.lib")

namespace KlayGE
{
	// 保存显示模式信息
	/////////////////////////////////////////////////////////////////////////////////
	class D3D9VideoMode
	{
	public:
		D3D9VideoMode();
		D3D9VideoMode(uint32 width, uint32 height, uint32 format);

		uint32 Width() const;
		uint32 Height() const;
		uint32 Format() const;
		uint32 ColorDepth() const;

	private:
		uint32		width_;
		uint32		height_;
		uint32		format_;
	};

	bool operator<(D3D9VideoMode const & lhs, D3D9VideoMode const & rhs);
	bool operator==(D3D9VideoMode const & lhs, D3D9VideoMode const & rhs);
}

#endif			// _D3D9VIDEOMODE_HPP