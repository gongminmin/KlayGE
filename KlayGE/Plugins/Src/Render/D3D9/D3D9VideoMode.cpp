#include <KlayGE/KlayGE.hpp>

#include <KlayGE/D3D9/D3D9VideoMode.hpp>

namespace KlayGE
{
	D3D9VideoMode::D3D9VideoMode()
				: width_(0),
					height_(0),
					format_(0)
	{
	}

	D3D9VideoMode::D3D9VideoMode(uint32_t width, uint32_t height, uint32_t format)
				: width_(width),
					height_(height),
					format_(format)
	{
	}

	uint32_t D3D9VideoMode::Width() const
	{
		return width_;
	}

	uint32_t D3D9VideoMode::Height() const
	{
		return height_;
	}

	uint32_t D3D9VideoMode::Format() const
	{
		return format_;
	}

	uint32_t D3D9VideoMode::ColorDepth() const
	{
		uint32_t colorDepth;
		if ((Format() == D3DFMT_X8R8G8B8)
			|| (Format() == D3DFMT_A8R8G8B8)
			|| (Format() == D3DFMT_R8G8B8))
		{
			colorDepth = 32;
		}
		else
		{
			colorDepth = 16;
		}

		return colorDepth;
	}

	bool operator<(D3D9VideoMode const & lhs, D3D9VideoMode const & rhs)
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

	bool operator==(D3D9VideoMode const & lhs, D3D9VideoMode const & rhs)
	{
		return (lhs.Width() == rhs.Width())
			&& (lhs.Height() == rhs.Height())
			&& (lhs.ColorDepth() == rhs.ColorDepth());
	}
}
