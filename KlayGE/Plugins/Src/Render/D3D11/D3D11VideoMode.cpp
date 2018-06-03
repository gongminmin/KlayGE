// D3D11VideoMode.cpp
// KlayGE D3D11显示模式 实现文件
// Ver 3.8.0
// 版权所有(C) 龚敏敏, 2009
// Homepage: http://www.klayge.org
//
// 3.8.0
// 初次建立 (2009.1.30)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>

#include <KlayGE/D3D11/D3D11VideoMode.hpp>

namespace KlayGE
{
	D3D11VideoMode::D3D11VideoMode()
				: D3D11VideoMode(0, 0, DXGI_FORMAT_UNKNOWN)
	{
	}

	D3D11VideoMode::D3D11VideoMode(uint32_t width, uint32_t height, DXGI_FORMAT format)
				: width_(width),
					height_(height),
					format_(format)
	{
	}

	uint32_t D3D11VideoMode::Width() const
	{
		return width_;
	}

	uint32_t D3D11VideoMode::Height() const
	{
		return height_;
	}

	DXGI_FORMAT D3D11VideoMode::Format() const
	{
		return format_;
	}

	uint32_t D3D11VideoMode::ColorDepth() const
	{
		uint32_t colorDepth;
		if ((format_ == DXGI_FORMAT_R8G8B8A8_UNORM)
			|| (format_ == DXGI_FORMAT_R8G8B8A8_UNORM_SRGB)
			|| (format_ == DXGI_FORMAT_B8G8R8A8_UNORM)
			|| (format_ == DXGI_FORMAT_B8G8R8A8_UNORM_SRGB)
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

	bool operator<(D3D11VideoMode const & lhs, D3D11VideoMode const & rhs)
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
						if (lhs.Format() < rhs.Format())
						{
							return true;
						}
					}
				}
			}
		}

		return false;
	}

	bool operator==(D3D11VideoMode const & lhs, D3D11VideoMode const & rhs)
	{
		return (lhs.Width() == rhs.Width())
			&& (lhs.Height() == rhs.Height())
			&& (lhs.Format() == rhs.Format());
	}
}
