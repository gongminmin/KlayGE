/**
* @file TexCompression.hpp
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

#ifndef _TEXCOMPRESSION_HPP
#define _TEXCOMPRESSION_HPP

#pragma once

namespace KlayGE
{
	enum TexCompressionMethod
	{
		TCM_Speed,
		TCM_Balanced,
		TCM_Quality
	};

	class KLAYGE_CORE_API TexCompression
	{
	public:
		virtual ~TexCompression()
		{
		}

		uint32_t BlockWidth() const
		{
			return block_width_;
		}
		uint32_t BlockHeight() const
		{
			return block_height_;
		}
		uint32_t BlockDepth() const
		{
			return block_depth_;
		}
		uint32_t BlockBytes() const
		{
			return block_bytes_;
		}
		ElementFormat DecodedFormat() const
		{
			return decoded_fmt_;
		}

		virtual void EncodeBlock(void* output, void const * input, TexCompressionMethod method) = 0;
		virtual void DecodeBlock(void* output, void const * input) = 0;

		virtual void EncodeMem(uint32_t width, uint32_t height, 
			void* output, uint32_t out_row_pitch, uint32_t out_slice_pitch,
			void const * input, uint32_t in_row_pitch, uint32_t in_slice_pitch,
			TexCompressionMethod method);
		virtual void DecodeMem(uint32_t width, uint32_t height,
			void* output, uint32_t out_row_pitch, uint32_t out_slice_pitch,
			void const * input, uint32_t in_row_pitch, uint32_t in_slice_pitch);

		virtual void EncodeTex(TexturePtr const & out_tex, TexturePtr const & in_tex, TexCompressionMethod method);
		virtual void DecodeTex(TexturePtr const & out_tex, TexturePtr const & in_tex);

	protected:
		uint32_t block_width_;
		uint32_t block_height_;
		uint32_t block_depth_;
		uint32_t block_bytes_;
		ElementFormat decoded_fmt_;
	};

	class ARGBColor32 : boost::equality_comparable<ARGBColor32>
	{
	public:
		enum
		{
			BChannel = 0,
			GChannel = 1,
			RChannel = 2,
			AChannel = 3
		};

	public:
		ARGBColor32()
		{
		}
		ARGBColor32(uint8_t a, uint8_t r, uint8_t g, uint8_t b)
		{
			this->a() = a;
			this->r() = r;
			this->g() = g;
			this->b() = b;
		}
		explicit ARGBColor32(uint32_t dw)
		{
			clr32_.dw = dw;
		}

		uint32_t& ARGB()
		{
			return clr32_.dw;
		}
		uint32_t ARGB() const
		{
			return clr32_.dw;
		}

		uint8_t& operator[](uint32_t ch)
		{
			BOOST_ASSERT(ch < 4);
			return clr32_.argb[ch];
		}
		uint8_t operator[](uint32_t ch) const
		{
			BOOST_ASSERT(ch < 4);
			return clr32_.argb[ch];
		}

		uint8_t& a()
		{
			return (*this)[AChannel];
		}
		uint8_t a() const
		{
			return (*this)[AChannel];
		}

		uint8_t& r()
		{
			return (*this)[RChannel];
		}
		uint8_t r() const
		{
			return (*this)[RChannel];
		}

		uint8_t& g()
		{
			return (*this)[GChannel];
		}
		uint8_t g() const
		{
			return (*this)[GChannel];
		}

		uint8_t& b()
		{
			return (*this)[BChannel];
		}
		uint8_t b() const
		{
			return (*this)[BChannel];
		}

		bool operator==(ARGBColor32 const & rhs) const
		{
			return clr32_.dw == rhs.clr32_.dw;
		}

	private:
		union Clr32
		{
			uint32_t dw;
			uint8_t argb[4];
		} clr32_;
	};

	// Helpers

	inline int Mul8Bit(int a, int b)
	{
		int t = a * b + 128;
		return (t + (t >> 8)) >> 8;
	}

	inline ARGBColor32 From4Ints(int a, int r, int g, int b)
	{
		return ARGBColor32(static_cast<uint8_t>(MathLib::clamp(a, 0, 255)),
			static_cast<uint8_t>(MathLib::clamp(r, 0, 255)),
			static_cast<uint8_t>(MathLib::clamp(g, 0, 255)),
			static_cast<uint8_t>(MathLib::clamp(b, 0, 255)));
	}

	inline uint8_t Extend4To8Bits(int input)
	{
		return static_cast<uint8_t>(input | (input << 4));
	}

	inline uint8_t Extend5To8Bits(int input)
	{
		return static_cast<uint8_t>((input >> 2) | (input << 3));
	}

	inline uint8_t Extend6To8Bits(int input)
	{
		return static_cast<uint8_t>((input >> 4) | (input << 2));
	}

	inline uint8_t Extend7To8Bits(int input)
	{
		return static_cast<uint8_t>((input >> 6) | (input << 1));
	}
}

#endif		// _TEXCOMPRESSIONBC_HPP
