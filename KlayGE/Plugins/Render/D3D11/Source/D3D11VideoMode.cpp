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

#include "D3D11VideoMode.hpp"

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

	uint32_t D3D11VideoMode::Width() const noexcept
	{
		return width_;
	}

	uint32_t D3D11VideoMode::Height() const noexcept
	{
		return height_;
	}

	DXGI_FORMAT D3D11VideoMode::Format() const noexcept
	{
		return format_;
	}

	bool D3D11VideoMode::operator<(D3D11VideoMode const & rhs) const noexcept
	{
		if (width_ < rhs.width_)
		{
			return true;
		}
		else if (width_ == rhs.width_)
		{
			if (height_ < rhs.height_)
			{
				return true;
			}
			else if (height_ == rhs.height_)
			{
				if (format_ < rhs.format_)
				{
					return true;
				}
			}
		}

		return false;
	}

	bool D3D11VideoMode::operator==(D3D11VideoMode const & rhs) const noexcept
	{
		return (width_ == rhs.width_) && (height_ == rhs.height_) && (format_ == rhs.format_);
	}
}
