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

	D3D9VideoMode::D3D9VideoMode(U32 width, U32 height, U32 format)
				: width_(width),
					height_(height),
					format_(format)
	{
	}

	U32 D3D9VideoMode::Width() const
	{
		return width_;
	}

	U32 D3D9VideoMode::Height() const
	{
		return height_;
	}

	U32 D3D9VideoMode::Format() const
	{
		return format_;
	}

	U32 D3D9VideoMode::ColorDepth() const
	{
		U32 colorDepth;
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

	bool operator<(const D3D9VideoMode& lhs, const D3D9VideoMode& rhs)
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

	bool operator==(const D3D9VideoMode& lhs, const D3D9VideoMode& rhs)
	{
		return (lhs.Width() == rhs.Width())
			&& (lhs.Height() == rhs.Height())
			&& (lhs.ColorDepth() == rhs.ColorDepth());
	}
}
