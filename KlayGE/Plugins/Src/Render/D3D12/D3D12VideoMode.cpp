/**
 * @file D3D12VideoMode.cpp
 * @author Minmin Gong
 *
 * @section DESCRIPTION
 *
 * This source file is part of KlayGE
 * For the latest info, see http://www.klayge.org
 *
 * @section LICENSE
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published
 * by the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 * You may alternatively use this source under the terms of
 * the KlayGE Proprietary License (KPL). You can obtained such a license
 * from http://www.klayge.org/licensing/.
 */

#include <KlayGE/KlayGE.hpp>

#include <KlayGE/D3D12/D3D12VideoMode.hpp>

namespace KlayGE
{
	D3D12VideoMode::D3D12VideoMode()
				: width_(0),
					height_(0),
					format_(DXGI_FORMAT_UNKNOWN)
	{
	}

	D3D12VideoMode::D3D12VideoMode(uint32_t width, uint32_t height, DXGI_FORMAT format)
				: width_(width),
					height_(height),
					format_(format)
	{
	}

	uint32_t D3D12VideoMode::Width() const
	{
		return width_;
	}

	uint32_t D3D12VideoMode::Height() const
	{
		return height_;
	}

	DXGI_FORMAT D3D12VideoMode::Format() const
	{
		return format_;
	}

	uint32_t D3D12VideoMode::ColorDepth() const
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

	bool operator<(D3D12VideoMode const & lhs, D3D12VideoMode const & rhs)
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

	bool operator==(D3D12VideoMode const & lhs, D3D12VideoMode const & rhs)
	{
		return (lhs.Width() == rhs.Width())
			&& (lhs.Height() == rhs.Height())
			&& (lhs.Format() == rhs.Format());
	}
}
