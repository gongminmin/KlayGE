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

#include <KlayGE/PreDeclare.hpp>

#include <array>

namespace KlayGE
{
	enum TexCompressionMethod
	{
		TCM_Speed,
		TCM_Balanced,
		TCM_Quality
	};

	enum TexCompressionErrorMetric
	{
		TCEM_Uniform,     // Treats r, g, and b channels equally
		TCEM_Nonuniform,  // { 0.3, 0.59, 0.11 }
	};

	KLAYGE_CORE_API uint32_t BlockWidth(ElementFormat format);
	KLAYGE_CORE_API uint32_t BlockHeight(ElementFormat format);
	KLAYGE_CORE_API uint32_t BlockDepth(ElementFormat format);
	KLAYGE_CORE_API uint32_t BlockBytes(ElementFormat format);
	KLAYGE_CORE_API ElementFormat DecodedFormat(ElementFormat format);

	class KLAYGE_CORE_API TexCompression : boost::noncopyable
	{
	public:
		virtual ~TexCompression() noexcept;

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
		ElementFormat compression_format_;
	};

	class ARGBColor32 final : boost::equality_comparable<ARGBColor32>
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

	class RGBACluster final
	{
		static int const MAX_NUM_DATA_POINTS = 16;

	public:
		RGBACluster(ARGBColor32 const * pixels, uint32_t num,
			std::function<uint32_t(uint32_t, uint32_t, uint32_t)> const & get_partition);

		float4& Point(uint32_t index)
		{
			return data_points_[point_map_[index]];
		}
		float4 const & Point(uint32_t index) const
		{
			return data_points_[point_map_[index]];
		}

		ARGBColor32 const & Pixel(uint32_t index) const
		{
			return data_pixels_[point_map_[index]];
		}

		uint32_t NumValidPoints() const
		{
			return num_valid_points_;
		}

		float4 const & Avg() const
		{
			return avg_;
		}

		void BoundingBox(float4& min_clr, float4& max_clr) const
		{
			min_clr = min_clr_;
			max_clr = max_clr_;
		}

		bool AllSamePoint() const
		{
			return min_clr_ == max_clr_;
		}

		uint32_t PrincipalAxis(float4& axis, float* eig_one, float* eig_two) const;

		void ShapeIndex(uint32_t shape_index, uint32_t num_partitions)
		{
			shape_index_ = shape_index;
			num_partitions_ = num_partitions;
		}

		void ShapeIndex(uint32_t shape_index)
		{
			this->ShapeIndex(shape_index, num_partitions_);
		}

		void Partition(uint32_t part);

		bool IsPointValid(uint32_t index) const
		{
			return selected_partition_ == get_partition_(num_partitions_, shape_index_, index);
		}

	private:
		void Recalculate(bool consider_valid);
		int PowerMethod(float4x4 const & mat, float4& eig_vec, float* eig_val = nullptr) const;

	private:
		uint32_t num_valid_points_;
		uint32_t num_partitions_;
		uint32_t selected_partition_;
		uint32_t shape_index_;

		float4 avg_;

		std::array<float4, MAX_NUM_DATA_POINTS> data_points_;
		std::array<ARGBColor32, MAX_NUM_DATA_POINTS> data_pixels_;
		std::array<uint8_t, MAX_NUM_DATA_POINTS> point_map_;
		float4 min_clr_;
		float4 max_clr_;

		std::function<uint32_t(uint32_t, uint32_t, uint32_t)> get_partition_;
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

	inline float4 FromARGBColor32(ARGBColor32 const & pixel)
	{
		return float4(pixel.r(), pixel.g(), pixel.b(), pixel.a());
	}

	template <int N>
	inline uint8_t ExtendNTo8Bits(int input)
	{
		return static_cast<uint8_t>((input >> (N - (8 - N))) | (input << (8 - N)));
	}

	inline uint8_t Extend4To8Bits(int input)
	{
		return ExtendNTo8Bits<4>(input);
	}

	inline uint8_t Extend5To8Bits(int input)
	{
		return ExtendNTo8Bits<5>(input);
	}

	inline uint8_t Extend6To8Bits(int input)
	{
		return ExtendNTo8Bits<6>(input);
	}

	inline uint8_t Extend7To8Bits(int input)
	{
		return ExtendNTo8Bits<7>(input);
	}
}

#endif		// _TEXCOMPRESSIONBC_HPP
