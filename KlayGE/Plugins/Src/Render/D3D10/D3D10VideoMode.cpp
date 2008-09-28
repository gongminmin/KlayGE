// D3D10VideoMode.cpp
// KlayGE D3D10显示模式 实现文件
// Ver 3.8.0
// 版权所有(C) 龚敏敏, 2008
// Homepage: http://klayge.sourceforge.net
//
// 3.8.0
// 初次建立 (2008.9.21)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>

#include <KlayGE/D3D10/D3D10VideoMode.hpp>

namespace KlayGE
{
	D3D10VideoMode::D3D10VideoMode()
				: width_(0),
					height_(0),
					format_(DXGI_FORMAT_UNKNOWN)
	{
	}

	D3D10VideoMode::D3D10VideoMode(uint32_t width, uint32_t height, DXGI_FORMAT format)
				: width_(width),
					height_(height),
					format_(format)
	{
	}

	uint32_t D3D10VideoMode::Width() const
	{
		return width_;
	}

	uint32_t D3D10VideoMode::Height() const
	{
		return height_;
	}

	DXGI_FORMAT D3D10VideoMode::Format() const
	{
		return format_;
	}

	uint32_t D3D10VideoMode::ColorDepth() const
	{
		uint32_t colorDepth;
		if ((format_ == DXGI_FORMAT_R8G8B8A8_UNORM)
			|| (format_ == DXGI_FORMAT_R10G10B10A2_UNORM))
		{
			colorDepth = 32;
		}
		else
		{
			colorDepth = 16;
		}

		return colorDepth;
	}

	bool operator<(D3D10VideoMode const & lhs, D3D10VideoMode const & rhs)
	{
		if (lhs.Width() < rhs.Width())
		{
			return true;
		}
		else
		{
			if (lhs.Width() == rhs.Width())
			{
				if (lhs.Height() < rhs.Height())
				{
					return true;
				}
				else
				{
					if (lhs.Height() == rhs.Height())
					{
						if (lhs.ColorDepth() < rhs.ColorDepth())
						{
							return true;
						}
					}
				}
			}
		}

		return false;
	}

	bool operator==(D3D10VideoMode const & lhs, D3D10VideoMode const & rhs)
	{
		return (lhs.Width() == rhs.Width())
			&& (lhs.Height() == rhs.Height())
			&& (lhs.ColorDepth() == rhs.ColorDepth());
	}
}
