#ifndef _D3D9VIDEOMODE_HPP
#define _D3D9VIDEOMODE_HPP

#include <d3d9.h>
#include <vector>

#ifdef KLAYGE_DEBUG
	#pragma comment(lib, "KlayGE_RenderEngine_D3D9_d.lib")
#else
	#pragma comment(lib, "KlayGE_RenderEngine_D3D9.lib")
#endif

namespace KlayGE
{
	// 保存显示模式信息
	/////////////////////////////////////////////////////////////////////////////////
	class D3D9VideoMode
	{
	public:
		D3D9VideoMode();
		D3D9VideoMode(uint32_t width, uint32_t height, uint32_t format);

		uint32_t Width() const;
		uint32_t Height() const;
		uint32_t Format() const;
		uint32_t ColorDepth() const;

	private:
		uint32_t		width_;
		uint32_t		height_;
		uint32_t		format_;
	};

	bool operator<(D3D9VideoMode const & lhs, D3D9VideoMode const & rhs);
	bool operator==(D3D9VideoMode const & lhs, D3D9VideoMode const & rhs);
}

#endif			// _D3D9VIDEOMODE_HPP