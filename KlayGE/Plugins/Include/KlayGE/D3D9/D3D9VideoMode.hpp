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
		D3D9VideoMode(U32 width, U32 height, U32 format);

		U32 Width() const;
		U32 Height() const;
		U32 Format() const;
		U32 ColorDepth() const;

	private:
		U32		width_;
		U32		height_;
		U32		format_;
	};

	bool operator<(const D3D9VideoMode& lhs, const D3D9VideoMode& rhs);
	bool operator==(const D3D9VideoMode& lhs, const D3D9VideoMode& rhs);
}

#endif			// _D3D9VIDEOMODE_HPP