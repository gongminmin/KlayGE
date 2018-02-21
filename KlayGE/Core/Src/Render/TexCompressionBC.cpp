/**
* @file TexCompressionBC.cpp
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
#include <KFL/CXX17/iterator.hpp>
#include <KFL/ErrorHandling.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KFL/Color.hpp>
#include <KlayGE/Texture.hpp>
#include <KFL/Half.hpp>

#include <vector>
#include <cstring>
#include <boost/assert.hpp>
#ifdef KLAYGE_COMPILER_MSVC
	#include <intrin.h>		// For _BitScanForward
#endif

#include <KlayGE/TexCompressionBC.hpp>
#include "../Base/TableGen/Tables.hpp"

namespace
{
	using namespace KlayGE;

	static int const BC67_PREC_WEIGHTS[][16] =
	{
		{ 0, 21, 43, 64 },
		{ 0, 9, 18, 27, 37, 46, 55, 64 },
		{ 0, 4, 9, 13, 17, 21, 26, 30, 34, 38, 43, 47, 51, 55, 60, 64 }
	};

	// Partition, Shape, Pixel (index into 4x4 block)
	static uint32_t const BC67_PARTITION_TABLE[2][64] =
	{
		{
			0x50505050, 0x40404040, 0x54545454, 0x54505040,
			0x50404000, 0x55545450, 0x55545040, 0x54504000,
			0x50400000, 0x55555450, 0x55544000, 0x54400000,
			0x55555440, 0x55550000, 0x55555500, 0x55000000,
			0x55150100, 0x00004054, 0x15010000, 0x00405054,
			0x00004050, 0x15050100, 0x05010000, 0x40505054,
			0x00404050, 0x05010100, 0x14141414, 0x05141450,
			0x01155440, 0x00555500, 0x15014054, 0x05414150,
			0x44444444, 0x55005500, 0x11441144, 0x05055050,
			0x05500550, 0x11114444, 0x41144114, 0x44111144,
			0x15055054, 0x01055040, 0x05041050, 0x05455150,
			0x14414114, 0x50050550, 0x41411414, 0x00141400,
			0x00041504, 0x00105410, 0x10541000, 0x04150400,
			0x50410514, 0x41051450, 0x05415014, 0x14054150,
			0x41050514, 0x41505014, 0x40011554, 0x54150140,
			0x50505500, 0x00555050, 0x15151010, 0x54540404
		},
		{
			0xAA685050, 0x6A5A5040, 0x5A5A4200, 0x5450A0A8,
			0xA5A50000, 0xA0A05050, 0x5555A0A0, 0x5A5A5050,
			0xAA550000, 0xAA555500, 0xAAAA5500, 0x90909090,
			0x94949494, 0xA4A4A4A4, 0xA9A59450, 0x2A0A4250,
			0xA5945040, 0x0A425054, 0xA5A5A500, 0x55A0A0A0,
			0xA8A85454, 0x6A6A4040, 0xA4A45000, 0x1A1A0500,
			0x0050A4A4, 0xAAA59090, 0x14696914, 0x69691400,
			0xA08585A0, 0xAA821414, 0x50A4A450, 0x6A5A0200,
			0xA9A58000, 0x5090A0A8, 0xA8A09050, 0x24242424,
			0x00AA5500, 0x24924924, 0x24499224, 0x50A50A50,
			0x500AA550, 0xAAAA4444, 0x66660000, 0xA5A0A5A0,
			0x50A050A0, 0x69286928, 0x44AAAA44, 0x66666600,
			0xAA444444, 0x54A854A8, 0x95809580, 0x96969600,
			0xA85454A8, 0x80959580, 0xAA141414, 0x96960000,
			0xAAAA1414, 0xA05050A0, 0xA0A5A5A0, 0x96000000,
			0x40804080, 0xA9A8A9A8, 0xAAAAAA44, 0x2A4A5254
		}
	};

	// Partition, Shape, Fixup
	static uint16_t const FIX_UP_TABLE[2][64] =
	{
		{
			0x00F0, 0x00F0, 0x00F0, 0x00F0, 0x00F0, 0x00F0, 0x00F0, 0x00F0,
			0x00F0, 0x00F0, 0x00F0, 0x00F0, 0x00F0, 0x00F0, 0x00F0, 0x00F0,
			0x00F0, 0x0020, 0x0080, 0x0020, 0x0020, 0x0080, 0x0080, 0x00F0,
			0x0020, 0x0080, 0x0020, 0x0020, 0x0080, 0x0080, 0x0020, 0x0020,
			0x00F0, 0x00F0, 0x0060, 0x0080, 0x0020, 0x0080, 0x00F0, 0x00F0,
			0x0020, 0x0080, 0x0020, 0x0020, 0x0020, 0x00F0, 0x00F0, 0x0060,
			0x0060, 0x0020, 0x0060, 0x0080, 0x00F0, 0x00F0, 0x0020, 0x0020,
			0x00F0, 0x00F0, 0x00F0, 0x00F0, 0x00F0, 0x0020, 0x0020, 0x00F0
		},
		{
			0x0F30, 0x0830, 0x08F0, 0x03F0, 0x0F80, 0x0F30, 0x03F0, 0x08F0,
			0x0F80, 0x0F80, 0x0F60, 0x0F60, 0x0F60, 0x0F50, 0x0F30, 0x0830,
			0x0F30, 0x0830, 0x0F80, 0x03F0, 0x0F30, 0x0830, 0x0F60, 0x08A0,
			0x0350, 0x0F80, 0x0680, 0x0A60, 0x0F80, 0x0F50, 0x0AF0, 0x08F0,
			0x0F80, 0x03F0, 0x0F30, 0x0A50, 0x0A60, 0x08A0, 0x0980, 0x0AF0,
			0x06F0, 0x0F30, 0x08F0, 0x0F50, 0x03F0, 0x06F0, 0x06F0, 0x08F0,
			0x0F30, 0x03F0, 0x0F50, 0x0F50, 0x0F50, 0x0F80, 0x0F50, 0x0FA0,
			0x0F50, 0x0FA0, 0x0F80, 0x0FD0, 0x03F0, 0x0FC0, 0x0F30, 0x0830
		}
	};

	static uint32_t const BC67_MAX_NUM_DATA_POINTS = 16;

	static std::pair<uint32_t, uint32_t> const BC67_INTERPOLATION_VALUES[4][16] =
	{
		{
			std::make_pair(64, 0), std::make_pair(33, 31), std::make_pair(0, 64), std::make_pair(0, 0),
			std::make_pair(0, 0), std::make_pair(0, 0), std::make_pair(0, 0), std::make_pair(0, 0),
			std::make_pair(0, 0), std::make_pair(0, 0), std::make_pair(0, 0), std::make_pair(0, 0),
			std::make_pair(0, 0), std::make_pair(0, 0), std::make_pair(0, 0), std::make_pair(0, 0)
		},
		{
			std::make_pair(64, 0), std::make_pair(43, 21), std::make_pair(21, 43), std::make_pair(0, 64),
			std::make_pair(0, 0), std::make_pair(0, 0), std::make_pair(0, 0), std::make_pair(0, 0),
			std::make_pair(0, 0), std::make_pair(0, 0), std::make_pair(0, 0), std::make_pair(0, 0),
			std::make_pair(0, 0), std::make_pair(0, 0), std::make_pair(0, 0), std::make_pair(0, 0)
		},
		{
			std::make_pair(64, 0), std::make_pair(55, 9), std::make_pair(46, 18), std::make_pair(37, 27),
			std::make_pair(27, 37), std::make_pair(18, 46), std::make_pair(9, 55), std::make_pair(0, 64),
			std::make_pair(0, 0), std::make_pair(0, 0), std::make_pair(0, 0), std::make_pair(0, 0),
			std::make_pair(0, 0), std::make_pair(0, 0), std::make_pair(0, 0), std::make_pair(0, 0)
		},
		{
			std::make_pair(64, 0), std::make_pair(60, 4), std::make_pair(55, 9), std::make_pair(51, 13),
			std::make_pair(47, 17), std::make_pair(43, 21), std::make_pair(38, 26), std::make_pair(34, 30),
			std::make_pair(30, 34), std::make_pair(26, 38), std::make_pair(21, 43), std::make_pair(17, 47),
			std::make_pair(13, 51), std::make_pair(9, 55), std::make_pair(4, 60), std::make_pair(0, 64)
		}
	};

	uint8_t ReadBit(void const * input, size_t& start_bit)
	{
		BOOST_ASSERT(start_bit < 128);

		uint8_t const * bits = static_cast<uint8_t const *>(input);

		size_t index = start_bit >> 3;
		uint8_t ret = (bits[index] >> (start_bit - (index << 3))) & 0x01;
		++ start_bit;

		return ret;
	}

	uint8_t ReadBits(void const * input, size_t& start_bit, size_t num_bits)
	{
		if (0 == num_bits)
		{
			return 0;
		}

		uint8_t const * bits = static_cast<uint8_t const *>(input);

		BOOST_ASSERT((start_bit + num_bits <= 128) && (num_bits <= 8));

		uint8_t ret;
		size_t index = start_bit / 8;
		size_t base = start_bit - index * 8;
		if (base + num_bits > 8)
		{
			size_t first_index_bits = 8 - base;
			size_t next_index_bits = num_bits - first_index_bits;
			ret = (bits[index] >> base) | ((bits[index + 1] & ((1 << next_index_bits) - 1)) << first_index_bits);
		}
		else
		{
			ret = (bits[index] >> base) & ((1 << num_bits) - 1);
		}
		BOOST_ASSERT(ret < (1 << num_bits));
		start_bit += num_bits;

		return ret;
	}

	void WriteBit(void* output, size_t& start_bit, uint8_t val)
	{
		BOOST_ASSERT((start_bit < 128) && (val < 2));

		uint8_t* bits = static_cast<uint8_t*>(output);

		size_t index = start_bit >> 3;
		size_t base = start_bit - (index << 3);
		bits[index] &= ~(1 << base);
		bits[index] |= val << base;
		++ start_bit;
	}

	void WriteBits(void* output, size_t& start_bit, size_t num_bits, uint8_t val)
	{
		if (0 == num_bits)
		{
			return;
		}

		uint8_t* bits = static_cast<uint8_t*>(output);

		BOOST_ASSERT((start_bit + num_bits <= 128) && (num_bits <= 8));
		BOOST_ASSERT(val < (1 << num_bits));

		size_t index = start_bit >> 3;
		size_t base = start_bit - (index << 3);
		if (base + num_bits > 8)
		{
			size_t first_index_bits = 8 - base;
			size_t next_index_bits = num_bits - first_index_bits;
			bits[index + 0] &= ~(((1 << first_index_bits) - 1) << base);
			bits[index + 0] |= val << base;
			bits[index + 1] &= ~((1 << next_index_bits) - 1);
			bits[index + 1] |= val >> first_index_bits;
		}
		else
		{
			bits[index] &= ~(((1 << num_bits) - 1) << base);
			bits[index] |= val << base;
		}
		start_bit += num_bits;
	}

	uint32_t GetPartition(uint32_t partitions, uint32_t shape, uint32_t offset)
	{
		BOOST_ASSERT((partitions <= 3) && (shape < 64) && (offset < 16));
		return (partitions > 1) ? (BC67_PARTITION_TABLE[partitions - 2][shape] >> (offset * 2)) & 0x3 : 0;
	}

	bool IsFixUpOffset(size_t partitions, size_t shape, size_t offset)
	{
		BOOST_ASSERT((partitions <= 3) && (shape < 64) && (offset < 16));
		size_t const fix_up = (partitions > 1) ? FIX_UP_TABLE[partitions - 2][shape] : 0;
		for (size_t p = 0; p < partitions; ++ p)
		{
			if (offset == ((fix_up >> (p * 4)) & 0xF))
			{
				return true;
			}
		}
		return false;
	}

	void SignExtend(int3& clr, ARGBColor32 const & prec)
	{
		if (clr.x() & (1UL << (prec.r() - 1)))
		{
			clr.x() |= (0xFFFFFFFF << prec.r());
		}
		if (clr.y() & (1UL << (prec.g() - 1)))
		{
			clr.y() |= (0xFFFFFFFF << prec.g());
		}
		if (clr.z() & (1UL << (prec.b() - 1)))
		{
			clr.z() |= (0xFFFFFFFF << prec.b());
		}
	}

	half Int2F16(int input, bool signed_fmt)
	{
		half h;
		uint16_t out;
		if (signed_fmt)
		{
			int s = 0;
			if (input < 0)
			{
				s = 0x8000;
				input = -input;
			}
			out = static_cast<uint16_t>(s | input);
		}
		else
		{
			BOOST_ASSERT((input >= 0) && (input <= 0x7BFF));
			out = static_cast<uint16_t>(input);
		}

		*(reinterpret_cast<uint16_t*>(&h)) = out;
		return h;
	}

	void ToF16(Vector_T<half, 4>& f16, int3 const & clr, bool signed_fmt)
	{
		f16.x() = Int2F16(clr.x(), signed_fmt);
		f16.y() = Int2F16(clr.y(), signed_fmt);
		f16.z() = Int2F16(clr.z(), signed_fmt);
	}

	void TransformInverse(std::pair<int3, int3>* end_pts, ARGBColor32 const & prec, bool signed_fmt)
	{
		int3 wrap_mask((1 << prec.r()) - 1, (1 << prec.g()) - 1, (1 << prec.b()) - 1);
		end_pts[0].second += end_pts[0].first;
		end_pts[0].second.x() &= wrap_mask.x();
		end_pts[0].second.y() &= wrap_mask.y();
		end_pts[0].second.z() &= wrap_mask.z();
		end_pts[1].first += end_pts[0].first;
		end_pts[1].first.x() &= wrap_mask.x();
		end_pts[1].first.y() &= wrap_mask.y();
		end_pts[1].first.z() &= wrap_mask.z();
		end_pts[1].second += end_pts[0].first;
		end_pts[1].second.x() &= wrap_mask.x();
		end_pts[1].second.y() &= wrap_mask.y();
		end_pts[1].second.z() &= wrap_mask.z();
		if (signed_fmt)
		{
			SignExtend(end_pts[0].second, prec);
			SignExtend(end_pts[1].first, prec);
			SignExtend(end_pts[1].second, prec);
		}
	}

	bool Bsf32(uint32_t& index, uint32_t v)
	{
#ifdef KLAYGE_COMPILER_MSVC
		return _BitScanForward(reinterpret_cast<unsigned long*>(&index), v) != 0;
#else
		if (0 == v)
		{
			index = 0;
			return 0;
		}
		else
		{
			v &= ~v + 1;
			union FNU
			{
				float f;
				uint32_t u;
			} fnu;
			fnu.f = static_cast<float>(v);
			index = (fnu.u >> 23) - 127;
			return 1;
		}
#endif
	}

	uint32_t CountBitsInMask(uint8_t n)
	{
		if (!n)
		{
			return 0;
		}

		uint32_t c;
		for (c = 0; n; ++ c)
		{
			n &= n - 1;
		}
		return c;
	}

	uint8_t QuantizeChannel(uint8_t val, uint8_t mask, int bit = -1)
	{
		// If the mask is all the bits, then we can just return the value.
		if (0xFF == mask)
		{
			return val;
		}

		// Otherwise if the mask is no bits then we'll assume that they want
		// all the bits ... this is only really relevant for alpha...
		if (0 == mask)
		{
			return 0xFF;
		}

		uint32_t prec = CountBitsInMask(mask);
		const uint32_t step = 1 << (8 - prec);

		BOOST_ASSERT(step - 1 == static_cast<uint8_t>(~mask));

		uint32_t lval = val & mask;
		uint32_t hval = lval + step;

		if (bit >= 0)
		{
			++ prec;
			lval |= !!bit << (8 - prec);
			hval |= !!bit << (8 - prec);
		}

		if (lval > val)
		{
			lval -= step;
			hval -= step;
		}

		lval |= lval >> prec;
		hval |= hval >> prec;

		if (abs(val - static_cast<uint8_t>(lval)) < abs(val - static_cast<uint8_t>(hval)))
		{
			return static_cast<uint8_t>(lval);
		}
		else
		{
			return static_cast<uint8_t>(hval);
		}
	}

	ARGBColor32 Quantize(float4 const & p, ARGBColor32 const & channelMask = ARGBColor32(255, 255, 255, 255), int bit = -1)
	{
		uint8_t const r = QuantizeChannel(static_cast<uint32_t>(p.x() + 0.5) & 0xFF, channelMask.r(), bit);
		uint8_t const g = QuantizeChannel(static_cast<uint32_t>(p.y() + 0.5) & 0xFF, channelMask.g(), bit);
		uint8_t const b = QuantizeChannel(static_cast<uint32_t>(p.z() + 0.5) & 0xFF, channelMask.b(), bit);
		uint8_t const a = QuantizeChannel(static_cast<uint32_t>(p.w() + 0.5) & 0xFF, channelMask.a(), bit);
		return ARGBColor32(a, r, g, b);
	}

	// Returns the error if we were to quantize the colors right now with the
	// given number of buckets and bit mask.
	uint64_t QuantizedError(RGBACluster const & cluster, float4 const & p1, float4 const & p2,
		uint32_t buckets, ARGBColor32 const & bit_mask, uint4 const & error_metric,
		int const pbits[2] = nullptr, uint8_t* indices = nullptr)
	{
		BOOST_ASSERT((4 == buckets) || (8 == buckets) || (16 == buckets));

		uint32_t index_prec;
		Bsf32(index_prec, buckets);
		BOOST_ASSERT((index_prec >= 2) && (index_prec <= 4));

		std::pair<uint32_t, uint32_t> const * interp_vals = BC67_INTERPOLATION_VALUES[index_prec - 1];

		ARGBColor32 qp1, qp2;
		if (pbits)
		{
			qp1 = Quantize(p1, bit_mask, pbits[0]);
			qp2 = Quantize(p2, bit_mask, pbits[1]);
		}
		else
		{
			qp1 = Quantize(p1, bit_mask);
			qp2 = Quantize(p2, bit_mask);
		}

		static uint32_t const rgba_channels[] = { ARGBColor32::RChannel, ARGBColor32::GChannel,
			ARGBColor32::BChannel, ARGBColor32::AChannel };

		float4 const uqp1 = FromARGBColor32(qp1);
		float4 const uqp2 = FromARGBColor32(qp2);
		float const uqpl_sq = MathLib::length_sq(uqp1 - uqp2);
		float4 const uqp_dir = uqp2 - uqp1;

		uint64_t total_err = 0;
		if (0 == uqpl_sq)
		{
			// If both endpoints are the same then the indices don't matter...
			for (uint32_t i = 0; i < cluster.NumValidPoints(); ++ i)
			{
				ARGBColor32 const & pixel = cluster.Pixel(i);

				uint32_t interp_0 = interp_vals[0].first;
				uint32_t interp_1 = interp_vals[0].second;

				uint4 error_vec(0, 0, 0, 0);
				for (uint32_t k = 0; k < 4; ++ k)
				{
					int const ch = rgba_channels[k];
					int ip = (((qp1[ch] * interp_0) + (qp2[ch] * interp_1) + 32) >> 6) & 0xFF;
					int dist = abs(pixel[ch] - ip);
					error_vec[k] = dist * error_metric[k];
				}

				total_err += MathLib::dot(error_vec, error_vec);

				if (indices != nullptr)
				{
					indices[i] = 0;
				}
			}

			return total_err;
		}

		for (uint32_t i = 0; i < cluster.NumValidPoints(); ++ i)
		{
			// Project this point unto the direction denoted by uqp_dir...
			float4 const & pt = cluster.Point(i);
#if 0
			float const pct = MathLib::clamp(MathLib::dot(pt - uqp1, uqp_dir) / uqpl_sq, 0.0f, 1.0f) * (BUCKETS - 1);
			int32_t const j1 = static_cast<int32>(pct);
			int32_t const j2 = static_cast<int32>(pct + 0.7f);
#else
			float const pct = MathLib::dot(pt - uqp1, uqp_dir) / uqpl_sq * (buckets - 1);
			int32_t const j1 = MathLib::clamp(static_cast<int32_t>(floor(pct)), 0, static_cast<int32_t>(buckets - 1));
			int32_t const j2 = std::min(static_cast<int32_t>(ceil(pct)), static_cast<int32_t>(buckets - 1));
#endif

			BOOST_ASSERT((j1 >= 0) && (j2 <= static_cast<int32_t>(buckets - 1)));

			ARGBColor32 const & pixel = cluster.Pixel(i);

			uint64_t min_err = std::numeric_limits<uint64_t>::max();
			uint32_t best_bucket = 0;
			int32_t j = j1;
			do
			{
				uint32_t interp_0 = interp_vals[j].first;
				uint32_t interp_1 = interp_vals[j].second;

				uint4 error_vec(0, 0, 0, 0);
				for (uint32_t k = 0; k < 4; ++ k)
				{
					int const ch = rgba_channels[k];
					int ip = (((qp1[ch] * interp_0) + (qp2[ch] * interp_1) + 32) >> 6) & 0xFF;
					int dist = abs(pixel[ch] - ip);
					error_vec[k] = dist * error_metric[k];
				}

				uint64_t error = MathLib::dot(error_vec, error_vec);
				if (error < min_err)
				{
					min_err = error;
					best_bucket = j;
				}
				else if (error > min_err + 1)
				{
					break;
				}

				++ j;
			} while (j <= j2);

			total_err += min_err;

			if (indices != nullptr)
			{
				indices[i] = static_cast<uint8_t>(best_bucket);
			}
		}

		return total_err;
	}

	void ChangePointForDirWithPbitChange(float4& v, uint32_t dir, uint32_t old_pbit, float4 const & step)
	{
		if ((dir & 1UL) && (0 == old_pbit))
		{
			v.x() -= step.x();
		}
		else if (!(dir & 1UL) && (1 == old_pbit))
		{
			v.x() += step.x();
		}

		if ((dir & 2UL) && (0 == old_pbit))
		{
			v.y() -= step.y();
		}
		else if (!(dir & 2UL) && (1 == old_pbit))
		{
			v.y() += step.y();
		}

		if ((dir & 4UL) && (0 == old_pbit))
		{
			v.z() -= step.z();
		}
		else if (!(dir & 4UL) && (1 == old_pbit))
		{
			v.z() += step.z();
		}

		if ((dir & 8UL) && (0 == old_pbit))
		{
			v.w() -= step.w();
		}
		else if (!(dir & 8UL) && (1 == old_pbit))
		{
			v.w() += step.w();
		}
	}

	void ChangePointForDirWithoutPbitChange(float4& v, uint32_t dir, float4 const & step)
	{
		if (dir & 1UL)
		{
			v.x() -= step.x();
		}
		else
		{
			v.x() += step.x();
		}

		if (dir & 2UL)
		{
			v.y() -= step.y();
		}
		else
		{
			v.y() += step.y();
		}

		if (dir & 4UL)
		{
			v.z() -= step.z();
		}
		else
		{
			v.z() += step.z();
		}

		if (dir & 8UL)
		{
			v.w() -= step.w();
		}
		else
		{
			v.w() += step.w();
		}
	}

	// The various available block modes that a BC7 compressor can choose from.
	// The enum is specialized to be power-of-two values so that an BC7BlockMode
	// variable can be used as a bit mask.
	enum BC7BlockMode
	{
		BC7BM_Zero = 1UL << 0,
		BC7BM_One = 1UL << 1,
		BC7BM_Two = 1UL << 2,
		BC7BM_Three = 1UL << 3,
		BC7BM_Four = 1UL << 4,
		BC7BM_Five = 1UL << 5,
		BC7BM_Six = 1UL << 6,
		BC7BM_Seven = 1UL << 7
	};

	// A shape consists of an index into the table of shapes and the number
	// of partitions that the index corresponds to. Different BPTC modes
	// interpret the shape differently and some are even illegal (such as
	// having an index >= 16 on mode 0). Hence, each shape corresponds to
	// these two variables.
	struct Shape
	{
		uint32_t num_partitions;
		uint32_t index;
	};

	// A shape selection can influence the results of the compressor by choosing
	// different modes to compress or not compress. The shape index is a value
	// between zero and sixty-four that corresponds to one of the available
	// partitioning schemes defined by the BPTC format.
	struct ShapeSelection
	{
		// These are the shape indices to use when evaluating two-partition shapes.
		std::vector<Shape> shapes;

		// This is the additional mask to prevent modes once shape selection
		// is done. This value is &-ed with m_BlockModes from CompressionSettings
		// to determine what the final considered blocks are.
		uint32_t selected_modes;

		// Defaults
		ShapeSelection()
			: selected_modes(static_cast<BC7BlockMode>(0xFF))
		{
		}
	};

	static uint32_t const TWO_PARTITION_MODES = BC7BM_One | BC7BM_Three | BC7BM_Seven;
	static uint32_t const THREE_PARTITION_MODES = BC7BM_Zero | BC7BM_Two;
	static uint32_t const ALPHA_MODES = BC7BM_Four | BC7BM_Five | BC7BM_Six | BC7BM_Seven;

	static uint4 const ERROR_METRICS[] =
	{
		uint4(1, 1, 1, 1),
		uint4(55, 75, 33, 100) // sqrt(0.3f, 0.56f, 0.11f) * 100
	};

	template <int BUCKETS>
	uint64_t EstimateNClusterError(TexCompressionErrorMetric metric, RGBACluster& c)
	{
		float4 min_clr, max_clr;
		c.BoundingBox(min_clr, max_clr);
		if (min_clr == max_clr)
		{
			return 0;
		}

		uint4 const & w = ERROR_METRICS[metric];
		return QuantizedError(c, min_clr, max_clr, BUCKETS,
			ARGBColor32(255, 255, 255, 255), w) * 2 + 1;
	}

	uint64_t EstimateTwoClusterError(TexCompressionErrorMetric metric, RGBACluster& c)
	{
		return EstimateNClusterError<8>(metric, c);
	}

	uint64_t EstimateThreeClusterError(TexCompressionErrorMetric metric, RGBACluster& c)
	{
		return EstimateNClusterError<4>(metric, c);
	}

	ShapeSelection BoxSelection(RGBACluster& cluster, TexCompressionErrorMetric metric)
	{
		ShapeSelection result;

		bool opaque = true;
		for (uint32_t i = 0; i < 16; ++ i)
		{
			uint8_t a = cluster.Pixel(i).a();
			opaque = opaque && (a >= 250); // For all intents and purposes...
		}

		// First we must figure out which shape to use. To do this, simply
		// see which shape has the smallest sum of minimum bounding spheres.
		uint64_t best_err = std::numeric_limits<uint64_t>::max();

		result.shapes.resize(1);
		result.shapes[0].num_partitions = 2;
		for (uint32_t i = 0; i < 64; ++ i)
		{
			cluster.ShapeIndex(i, 2);

			uint64_t err = 0;
			for (int ci = 0; ci < 2; ++ ci)
			{
				cluster.Partition(ci);
				err += EstimateTwoClusterError(metric, cluster);
			}

			if (err < best_err)
			{
				best_err = err;
				result.shapes[0].index = i;
			}

			// If it's small, we'll take it!
			if (err < 1)
			{
				result.selected_modes = TWO_PARTITION_MODES;
				return result;
			}
		}

		// There are not 3 subset blocks that support alpha, so only check these
		// if the entire block is opaque.
		if (!opaque)
		{
			result.selected_modes &= ALPHA_MODES;
			return result;
		}

		// If it's opaque, we get more value out of mode 6 than modes
		// 4 and 5, so just ignore those.
		result.selected_modes &= ~(BC7BM_Four | BC7BM_Five);

		best_err = std::numeric_limits<uint64_t>::max();

		result.shapes.resize(2);
		result.shapes[1].num_partitions = 3;
		for (uint32_t i = 0; i < 64; ++ i)
		{
			cluster.ShapeIndex(i, 3);

			uint64_t err = 0;
			for (int ci = 0; ci < 3; ++ ci)
			{
				cluster.Partition(ci);
				err += EstimateThreeClusterError(metric, cluster);
			}

			if (err < best_err)
			{
				best_err = err;
				result.shapes[1].index = i;
			}

			// If it's small, we'll take it!
			if (err < 1)
			{
				result.selected_modes = THREE_PARTITION_MODES;
				return result;
			}
		}

		return result;
	}

	uint32_t AnchorIndexForSubset(uint32_t partition, uint32_t shape_index, uint32_t num_partitions)
	{
		static int const anchor_idx_2[64] =
		{
			15, 15, 15, 15, 15, 15, 15, 15,
			15, 15, 15, 15, 15, 15, 15, 15,
			15, 2, 8, 2, 2, 8, 8, 15,
			2, 8, 2, 2, 8, 8, 2, 2,
			15, 15, 6, 8, 2, 8, 15, 15,
			2, 8, 2, 2, 2, 15, 15, 6,
			6, 2, 6, 8, 15, 15, 2, 2,
			15, 15, 15, 15, 15, 2, 2, 15
		};

		static int const anchor_idx_3[2][64] =
		{
			{
				3, 3, 15, 15, 8, 3, 15, 15,
				8, 8, 6, 6, 6, 5, 3, 3,
				3, 3, 8, 15, 3, 3, 6, 10,
				5, 8, 8, 6, 8, 5, 15, 15,
				8, 15, 3, 5, 6, 10, 8, 15,
				15, 3, 15, 5, 15, 15, 15, 15,
				3, 15, 5, 5, 5, 8, 5, 10,
				5, 10, 8, 13, 15, 12, 3, 3
			},
			{
				15, 8, 8, 3, 15, 15, 3, 8,
				15, 15, 15, 15, 15, 15, 15, 8,
				15, 8, 15, 3, 15, 8, 15, 8,
				3, 15, 6, 10, 15, 15, 10, 8,
				15, 3, 15, 10, 10, 8, 9, 10,
				6, 15, 8, 15, 3, 6, 6, 8,
				15, 3, 15, 15, 15, 15, 15, 15,
				15, 15, 15, 15, 3, 15, 15, 8
			}
		};

		int anchor_idx = 0;
		switch (partition)
		{
		case 1:
			anchor_idx = (2 == num_partitions) ? anchor_idx_2[shape_index] : anchor_idx_3[0][shape_index];
			break;

		case 2:
			BOOST_ASSERT(3 == num_partitions);
			anchor_idx = anchor_idx_3[1][shape_index];
			break;

		default:
			break;
		}

		return anchor_idx;
	}

	template <typename T>
	void Rotation(Vector_T<T, 4>& v, int mode)
	{
		switch (mode)
		{
		case 0:
			break;
		case 1:
			std::swap(v.x(), v.w());
			break;
		case 2:
			std::swap(v.y(), v.w());
			break;
		case 3:
			std::swap(v.z(), v.w());
			break;

		default:
			KFL_UNREACHABLE("Invalid rotation mode");
		}
	}
}

namespace KlayGE
{
	using namespace TexCompressionLUT;

	TexCompressionBC1::TexCompressionBC1()
	{
		block_width_ = block_height_ = 4;
		block_depth_ = 1;
		block_bytes_ = NumFormatBytes(EF_BC1) * 4;
		decoded_fmt_ = EF_ARGB8;
	}

	void TexCompressionBC1::EncodeBlock(void* output, void const * input, TexCompressionMethod method)
	{
		BOOST_ASSERT(output);
		BOOST_ASSERT(input);

		BC1Block& bc1 = *static_cast<BC1Block*>(output);
		ARGBColor32 const * argb = static_cast<ARGBColor32 const *>(input);

		std::array<ARGBColor32, 16> tmp_argb;
		bool alpha = false;
		for (size_t i = 0; i < tmp_argb.size(); ++ i)
		{
			if (argb[i].a() < 0x80)
			{
				tmp_argb[i] = ARGBColor32(0, 0, 0, 0);
				alpha = true;
			}
			else
			{
				tmp_argb[i] = argb[i];
			}
		}

		this->EncodeBC1Internal(bc1, &tmp_argb[0], alpha, method);
	}

	void TexCompressionBC1::DecodeBlock(void* output, void const * input)
	{
		BOOST_ASSERT(output);
		BOOST_ASSERT(input);

		ARGBColor32* argb = static_cast<ARGBColor32*>(output);
		BC1Block const & bc1 = *static_cast<BC1Block const *>(input);

		ARGBColor32 max_clr = this->RGB565To888(bc1.clr_0);
		ARGBColor32 min_clr = this->RGB565To888(bc1.clr_1);

		std::array<ARGBColor32, 4> clr;
		clr[0] = max_clr;
		clr[1] = min_clr;
		if (bc1.clr_0 > bc1.clr_1)
		{
			clr[2].r() = (max_clr.r() * 2 + min_clr.r()) / 3;
			clr[2].g() = (max_clr.g() * 2 + min_clr.g()) / 3;
			clr[2].b() = (max_clr.b() * 2 + min_clr.b()) / 3;
			clr[2].a() = 255;
			clr[3].r() = (max_clr.r() + min_clr.r() * 2) / 3;
			clr[3].g() = (max_clr.g() + min_clr.g() * 2) / 3;
			clr[3].b() = (max_clr.b() + min_clr.b() * 2) / 3;
			clr[3].a() = 255;
		}
		else
		{
			clr[2].r() = (max_clr.r() + min_clr.r()) / 2;
			clr[2].g() = (max_clr.g() + min_clr.g()) / 2;
			clr[2].b() = (max_clr.b() + min_clr.b()) / 2;
			clr[2].a() = 255;
			clr[3] = ARGBColor32(0, 0, 0, 0);
		}

		for (int i = 0; i < 2; ++ i)
		{
			for (int j = 0; j < 8; ++ j)
			{
				argb[i * 8 + j] = clr[(bc1.bitmap[i] >> (j * 2)) & 0x3];
			}
		}
	}

	ARGBColor32 TexCompressionBC1::RGB565To888(uint16_t rgb) const
	{
		return ARGBColor32(255, EXPAND5[(rgb >> 11) & 0x1F], EXPAND6[(rgb >> 5) & 0x3F],
			EXPAND5[(rgb >> 0) & 0x1F]);
	}

	uint16_t TexCompressionBC1::RGB888To565(ARGBColor32 const & rgb) const
	{
		return ((rgb.r() >> 3) << 11) | ((rgb.g() >> 2) << 5) | ((rgb.b() >> 3) << 0);
	}

	// The color matching function
	uint32_t TexCompressionBC1::MatchColorsBlock(ARGBColor32 const * argb,
			ARGBColor32 const & min_clr, ARGBColor32 const & max_clr, bool alpha) const
	{
		std::array<ARGBColor32, 4> color;
		color[0] = max_clr;
		color[1] = min_clr;
		if (!alpha)
		{
			color[2].r() = (max_clr.r() * 2 + min_clr.r()) / 3;
			color[2].g() = (max_clr.g() * 2 + min_clr.g()) / 3;
			color[2].b() = (max_clr.b() * 2 + min_clr.b()) / 3;
			color[2].a() = 255;
			color[3].r() = (max_clr.r() + min_clr.r() * 2) / 3;
			color[3].g() = (max_clr.g() + min_clr.g() * 2) / 3;
			color[3].b() = (max_clr.b() + min_clr.b() * 2) / 3;
			color[3].a() = 255;
		}

		uint32_t mask = 0;
		int dirr = color[0].r() - color[1].r();
		int dirg = color[0].g() - color[1].g();
		int dirb = color[0].b() - color[1].b();

		int dots[16];
		for (int i = 0; i < 16; ++ i)
		{
			dots[i] = argb[i].r() * dirr + argb[i].g() * dirg + argb[i].b() * dirb;
		}

		if (alpha)
		{
			std::array<int, 2> stops;
			for (int i = 0; i < 2; ++ i)
			{
				stops[i] = color[i].r() * dirr + color[i].g() * dirg + color[i].b() * dirb;
			}

			int c0_point = (stops[0] + stops[1] * 2) / 3;
			int c3_point = (stops[0] * 2 + stops[1]) / 3;

			for (int i = 15; i >= 0; -- i)
			{
				mask <<= 2;
				int dot = dots[i];
				if (0 == argb[i].a())
				{
					mask |= 3;
				}
				else
				{
					if (dot >= c0_point)
					{
						mask |= (dot < c3_point) ? 2 : 1;
					}
				}
			}
		}
		else
		{
			std::array<int, 4> stops;
			for (int i = 0; i < 4; ++ i)
			{
				stops[i] = color[i].r() * dirr + color[i].g() * dirg + color[i].b() * dirb;
			}

			int c0_point = (stops[1] + stops[3]) >> 1;
			int half_point = (stops[3] + stops[2]) >> 1;
			int c3_point = (stops[2] + stops[0]) >> 1;

			for (int i = 15; i >= 0; -- i)
			{
				mask <<= 2;
				int dot = dots[i];

				if (dot < half_point)
				{
					mask |= (dot < c0_point) ? 1 : 3;
				}
				else
				{
					mask |= (dot < c3_point) ? 2 : 0;
				}
			}
		}

		return mask;
	}

	// The color optimization function. (Clever code, part 1)
	void TexCompressionBC1::OptimizeColorsBlock(ARGBColor32 const * argb,
			ARGBColor32& min_clr, ARGBColor32& max_clr, TexCompressionMethod method) const
	{
		if (method != TCM_Quality)
		{
			Color const LUM_WEIGHT(0.2126f, 0.7152f, 0.0722f, 0);

			max_clr = min_clr = argb[0];
			float min_lum = MathLib::dot(Color(min_clr.ARGB()), LUM_WEIGHT);
			float max_lum = min_lum;
			for (size_t i = 1; i < 16; ++ i)
			{
				float lum = MathLib::dot(Color(argb[i].ARGB()), LUM_WEIGHT);
				if (lum < min_lum)
				{
					min_lum = lum;
					min_clr = argb[i];
				}
				if (lum > max_lum)
				{
					max_lum = lum;
					max_clr = argb[i];
				}
			}
		}
		else
		{
			static int const ITER_POWER = 4;

			// determine color distribution
			int mu[3], min[3], max[3];

			for (int ch = 0; ch < 3; ++ ch)
			{
				int muv, minv, maxv;

				muv = minv = maxv = argb[0][ch];
				for (int i = 1; i < 16; ++ i)
				{
					muv += argb[i][ch];
					minv = std::min<int>(minv, argb[i][ch]);
					maxv = std::max<int>(maxv, argb[i][ch]);
				}

				mu[ch] = (muv + 8) >> 4;
				min[ch] = minv;
				max[ch] = maxv;
			}

			// determine covariance matrix
			int cov[6];
			for (int i = 0; i < 6; ++ i)
			{
				cov[i] = 0;
			}

			for (int i = 0; i < 16; ++ i)
			{
				int r = argb[i].r() - mu[ARGBColor32::RChannel];
				int g = argb[i].g() - mu[ARGBColor32::GChannel];
				int b = argb[i].b() - mu[ARGBColor32::BChannel];

				cov[0] += r * r;
				cov[1] += r * g;
				cov[2] += r * b;
				cov[3] += g * g;
				cov[4] += g * b;
				cov[5] += b * b;
			}

			// convert covariance matrix to float, find principal axis via power iter
			float covf[6], vfr, vfg, vfb;
			for (int i = 0; i < 6; ++ i)
			{
				covf[i] = cov[i] / 255.0f;
			}

			vfr = static_cast<float>(max[ARGBColor32::RChannel] - min[ARGBColor32::RChannel]);
			vfg = static_cast<float>(max[ARGBColor32::GChannel] - min[ARGBColor32::GChannel]);
			vfb = static_cast<float>(max[ARGBColor32::BChannel] - min[ARGBColor32::BChannel]);

			for (int iter = 0; iter < ITER_POWER; ++ iter)
			{
				float r = vfr * covf[0] + vfg * covf[1] + vfb * covf[2];
				float g = vfr * covf[1] + vfg * covf[3] + vfb * covf[4];
				float b = vfr * covf[2] + vfg * covf[4] + vfb * covf[5];

				vfr = r;
				vfg = g;
				vfb = b;
			}

			float magn = std::max(std::max(std::abs(vfr), std::abs(vfg)), std::abs(vfb));
			int v_r, v_g, v_b;

			if (magn < 4.0f) // too small, default to luminance
			{
				v_r = 148;
				v_g = 300;
				v_b = 58;
			}
			else
			{
				magn = 512.0f / magn;
				v_r = static_cast<int>(vfr * magn);
				v_g = static_cast<int>(vfg * magn);
				v_b = static_cast<int>(vfb * magn);
			}

			// Pick colors at extreme points
			int min_d = 0x7FFFFFFF, max_d = -min_d;
			min_clr = max_clr = ARGBColor32(0, 0, 0, 0);
			for (int i = 0; i < 16; ++ i)
			{
				int dot = argb[i].r() * v_r + argb[i].g() * v_g + argb[i].b() * v_b;
				if (dot < min_d)
				{
					min_d = dot;
					min_clr = argb[i];
				}
				if (dot > max_d)
				{
					max_d = dot;
					max_clr = argb[i];
				}
			}
		}
	}

	// The refinement function. (Clever code, part 2)
	// Tries to optimize colors to suit block contents better.
	// (By solving a least squares system via normal equations+Cramer's rule)
	bool TexCompressionBC1::RefineBlock(ARGBColor32 const * argb,
			ARGBColor32& min_clr, ARGBColor32& max_clr, uint32_t mask) const
	{
		static int const w1Tab[4] = { 3, 0, 2, 1 };
		static int const prods[4] = { 0x090000, 0x000900, 0x040102, 0x010402 };
		// ^some magic to save a lot of multiplies in the accumulating loop...

		int akku = 0;
		int At1_r, At1_g, At1_b;
		int At2_r, At2_g, At2_b;

		At1_r = At1_g = At1_b = 0;
		At2_r = At2_g = At2_b = 0;
		for (int i = 0; i < 16; ++ i, mask >>= 2)
		{
			int step = mask & 3;
			int w1 = w1Tab[step];
			int r = argb[i].r();
			int g = argb[i].g();
			int b = argb[i].b();

			akku += prods[step];
			At1_r += w1 * r;
			At1_g += w1 * g;
			At1_b += w1 * b;
			At2_r += r;
			At2_g += g;
			At2_b += b;
		}

		At2_r = 3 * At2_r - At1_r;
		At2_g = 3 * At2_g - At1_g;
		At2_b = 3 * At2_b - At1_b;

		// extract solutions and decide solvability
		int xx = akku >> 16;
		int yy = (akku >> 8) & 0xFF;
		int xy = (akku >> 0) & 0xFF;

		if (!yy || !xx || (xx * yy == xy * xy))
		{
			return false;
		}

		float const f = 3.0f / 255.0f / (xx * yy - xy * xy);
		float const frb = f * 31.0f;
		float const fg = f * 63.0f;

		uint16_t old_min = this->RGB888To565(min_clr);
		uint16_t old_max = this->RGB888To565(max_clr);

		// solve.
		int max_r = MathLib::clamp<int>(static_cast<int>((At1_r * yy - At2_r * xy) * frb + 0.5f), 0, 31);
		int max_g = MathLib::clamp<int>(static_cast<int>((At1_g * yy - At2_g * xy) * fg + 0.5f), 0, 63);
		int max_b = MathLib::clamp<int>(static_cast<int>((At1_b * yy - At2_b * xy) * frb + 0.5f), 0, 31);

		int min_r = MathLib::clamp<int>(static_cast<int>((At2_r * xx - At1_r * xy) * frb + 0.5f), 0, 31);
		int min_g = MathLib::clamp<int>(static_cast<int>((At2_g * xx - At1_g * xy) * fg + 0.5f), 0, 63);
		int min_b = MathLib::clamp<int>(static_cast<int>((At2_b * xx - At1_b * xy) * frb + 0.5f), 0, 31);

		uint16_t max16 = static_cast<uint16_t>((max_r << 11) | (max_g << 5) | (max_b << 0));
		uint16_t min16 = static_cast<uint16_t>((min_r << 11) | (min_g << 5) | (min_b << 0));

		if ((old_min != min16) || (old_max != max16))
		{
			min_clr = this->RGB565To888(min16);
			max_clr = this->RGB565To888(max16);

			return true;
		}
		else
		{
			return false;
		}
	}

	void TexCompressionBC1::EncodeBC1Internal(BC1Block& bc1, ARGBColor32 const * argb,
			bool alpha, TexCompressionMethod method) const
	{
		BOOST_ASSERT(argb);

		// check if block is constant
		uint32_t min32, max32;
		min32 = max32 = argb[0].ARGB();
		for (int i = 1; i < 16; ++ i)
		{
			min32 = std::min(min32, argb[i].ARGB());
			max32 = std::max(max32, argb[i].ARGB());
		}

		uint32_t mask;
		uint16_t max16, min16;
		if (min32 != max32) // no constant color
		{
			ARGBColor32 max_clr, min_clr;
			this->OptimizeColorsBlock(argb, min_clr, max_clr, method);
			max16 = this->RGB888To565(max_clr);
			min16 = this->RGB888To565(min_clr);
			if (max16 != min16)
			{
				mask = this->MatchColorsBlock(argb, min_clr, max_clr, alpha);
			}
			else
			{
				mask = 0;
			}
			if (!alpha && (method != TCM_Speed))
			{
				if (this->RefineBlock(argb, min_clr, max_clr, mask))
				{
					max16 = this->RGB888To565(max_clr);
					min16 = this->RGB888To565(min_clr);
					if (max16 != min16)
					{
						mask = this->MatchColorsBlock(argb, min_clr, max_clr, alpha);
					}
					else
					{
						mask = 0;
					}
				}
			}
		}
		else // constant color
		{
			if (alpha && (0 == argb[0].ARGB()))
			{
				mask = 0xFFFFFFFF;
				max16 = min16 = 0;
			}
			else
			{
				int const r = argb[0].r();
				int const g = argb[0].g();
				int const b = argb[0].b();

				mask = 0xAAAAAAAA;
				max16 = (O_MATCH5[r][0] << 11) | (O_MATCH6[g][0] << 5) | O_MATCH5[b][0];
				min16 = (O_MATCH5[r][1] << 11) | (O_MATCH6[g][1] << 5) | O_MATCH5[b][1];
			}
		}

		if (alpha)
		{
			if (max16 < min16)
			{
				std::swap(max16, min16);

				uint32_t xor_mask = 0;
				for (int i = 15; i >= 0; -- i)
				{
					xor_mask <<= 2;

					uint32_t pixel_mask = (mask >> (i * 2)) & 0x3;
					if (pixel_mask <= 1)
					{
						xor_mask |= 0x1;
					}
				}
				mask ^= xor_mask;
			}

			bc1.clr_0 = min16;
			bc1.clr_1 = max16;
		}
		else
		{
			if (max16 < min16)
			{
				std::swap(max16, min16);
				mask ^= 0x55555555;
			}

			bc1.clr_0 = max16;
			bc1.clr_1 = min16;
		}
		std::memcpy(bc1.bitmap, &mask, sizeof(mask));
	}


	TexCompressionBC2::TexCompressionBC2()
	{
		block_width_ = block_height_ = 4;
		block_depth_ = 1;
		block_bytes_ = NumFormatBytes(EF_BC2) * 4;
		decoded_fmt_ = EF_ARGB8;
	}

	void TexCompressionBC2::EncodeBlock(void* output, void const * input, TexCompressionMethod method)
	{
		BOOST_ASSERT(output);
		BOOST_ASSERT(input);

		BC2Block& bc2 = *static_cast<BC2Block*>(output);
		ARGBColor32 const * argb = static_cast<ARGBColor32 const *>(input);

		std::array<uint8_t, 16> alpha;
		std::array<ARGBColor32, 16> xrgb;
		for (size_t i = 0; i < xrgb.size(); ++ i)
		{
			xrgb[i] = argb[i];
			xrgb[i].a() = 255;
			alpha[i] = static_cast<uint8_t>(argb[i].a() >> 4);
		}

		bc1_codec_.EncodeBC1Internal(bc2.bc1, &xrgb[0], false, method);
		
		for (int i = 0; i < 4; ++ i)
		{
			bc2.alpha[i] = (alpha[i * 4 + 0] << 0) | (alpha[i * 4 + 1] << 4)
				| (alpha[i * 4 + 2] << 8) | (alpha[i * 4 + 3] << 12);
		}
	}

	void TexCompressionBC2::DecodeBlock(void* output, void const * input)
	{
		BOOST_ASSERT(output);
		BOOST_ASSERT(input);

		ARGBColor32* argb = static_cast<ARGBColor32*>(output);
		BC2Block const * bc2_block = static_cast<BC2Block const *>(input);

		bc1_codec_.DecodeBlock(argb, &bc2_block->bc1);

		for (int i = 0; i < 4; ++ i)
		{
			for (int j = 0; j < 4; ++ j)
			{
				argb[i * 4 + j].a() = ((bc2_block->alpha[i] >> (4 * j)) & 0xF) << 4;
			}
		}
	}


	TexCompressionBC3::TexCompressionBC3()
	{
		block_width_ = block_height_ = 4;
		block_depth_ = 1;
		block_bytes_ = NumFormatBytes(EF_BC3) * 4;
		decoded_fmt_ = EF_ARGB8;
	}

	void TexCompressionBC3::EncodeBlock(void* output, void const * input, TexCompressionMethod method)
	{
		BOOST_ASSERT(output);
		BOOST_ASSERT(input);

		BC3Block& bc3 = *static_cast<BC3Block*>(output);
		ARGBColor32 const * argb = static_cast<ARGBColor32 const *>(input);

		std::array<uint8_t, 16> alpha;
		std::array<ARGBColor32, 16> xrgb;
		for (size_t i = 0; i < xrgb.size(); ++ i)
		{
			xrgb[i] = argb[i];
			xrgb[i].a() = 255;
			alpha[i] = static_cast<uint8_t>(argb[i].a());
		}

		bc1_codec_.EncodeBC1Internal(bc3.bc1, &xrgb[0], false, method);
		bc4_codec_.EncodeBlock(&bc3.alpha, &alpha[0], method);
	}

	void TexCompressionBC3::DecodeBlock(void* output, void const * input)
	{
		BOOST_ASSERT(output);
		BOOST_ASSERT(input);

		ARGBColor32* argb = static_cast<ARGBColor32*>(output);
		BC3Block const * bc3_block = static_cast<BC3Block const *>(input);

		bc1_codec_.DecodeBlock(argb, &bc3_block->bc1);

		std::array<uint8_t, 16> alpha_block;
		bc4_codec_.DecodeBlock(&alpha_block[0], &bc3_block->alpha);

		for (size_t i = 0; i < alpha_block.size(); ++ i)
		{
			argb[i].a() = alpha_block[i];
		}
	}


	TexCompressionBC4::TexCompressionBC4()
	{
		block_width_ = block_height_ = 4;
		block_depth_ = 1;
		block_bytes_ = NumFormatBytes(EF_BC4) * 4;
		decoded_fmt_ = EF_R8;
	}

	// Alpha block compression (this is easy for a change)
	void TexCompressionBC4::EncodeBlock(void* output, void const * input, TexCompressionMethod method)
	{
		BOOST_ASSERT(output);
		BOOST_ASSERT(input);

		KFL_UNUSED(method);

		BC4Block& bc4 = *static_cast<BC4Block*>(output);
		uint8_t const * r = static_cast<uint8_t const *>(input);

		// find min/max color
		int min, max;
		min = max = r[0];

		for (int i = 1; i < 16; ++ i)
		{
			min = std::min<int>(min, r[i]);
			max = std::max<int>(max, r[i]);
		}

		// encode them
		bc4.alpha_0 = static_cast<uint8_t>(max);
		bc4.alpha_1 = static_cast<uint8_t>(min);

		// determine bias and emit color indices
		int dist = max - min;
		int bias = min * 7 - (dist >> 1);
		int dist4 = dist * 4;
		int dist2 = dist * 2;
		int bits = 0, mask = 0;

		int dest = 0;
		for (int i = 0; i < 16; ++ i)
		{
			int a = r[i] * 7 - bias;
			int ind, t;

			// select index (hooray for bit magic)
			t = (dist4 - a) >> 31;  ind = t & 4; a -= dist4 & t;
			t = (dist2 - a) >> 31;  ind += t & 2; a -= dist2 & t;
			t = (dist - a) >> 31;   ind += t & 1;

			ind = -ind & 7;
			ind ^= (2 > ind);

			// write index
			mask |= ind << bits;
			if ((bits += 3) >= 8)
			{
				bc4.bitmap[dest] = static_cast<uint8_t>(mask);
				++ dest;
				mask >>= 8;
				bits -= 8;
			}
		}
	}

	void TexCompressionBC4::DecodeBlock(void* output, void const * input)
	{
		BOOST_ASSERT(output);
		BOOST_ASSERT(input);

		uint8_t* alpha_block = static_cast<uint8_t*>(output);
		BC4Block const & bc4 = *static_cast<BC4Block const *>(input);

		std::array<uint8_t, 8> alpha;
		float falpha0 = bc4.alpha_0 / 255.0f;
		float falpha1 = bc4.alpha_1 / 255.0f;
		alpha[0] = bc4.alpha_0;
		alpha[1] = bc4.alpha_1;
		if (alpha[0] > alpha[1])
		{
			alpha[2] = static_cast<uint8_t>(MathLib::clamp(static_cast<int>(MathLib::lerp(falpha0, falpha1, 1 / 7.0f) * 255 + 0.5f), 0, 255));
			alpha[3] = static_cast<uint8_t>(MathLib::clamp(static_cast<int>(MathLib::lerp(falpha0, falpha1, 2 / 7.0f) * 255 + 0.5f), 0, 255));
			alpha[4] = static_cast<uint8_t>(MathLib::clamp(static_cast<int>(MathLib::lerp(falpha0, falpha1, 3 / 7.0f) * 255 + 0.5f), 0, 255));
			alpha[5] = static_cast<uint8_t>(MathLib::clamp(static_cast<int>(MathLib::lerp(falpha0, falpha1, 4 / 7.0f) * 255 + 0.5f), 0, 255));
			alpha[6] = static_cast<uint8_t>(MathLib::clamp(static_cast<int>(MathLib::lerp(falpha0, falpha1, 5 / 7.0f) * 255 + 0.5f), 0, 255));
			alpha[7] = static_cast<uint8_t>(MathLib::clamp(static_cast<int>(MathLib::lerp(falpha0, falpha1, 6 / 7.0f) * 255 + 0.5f), 0, 255));
		}
		else
		{
			alpha[2] = static_cast<uint8_t>(MathLib::clamp(static_cast<int>(MathLib::lerp(falpha0, falpha1, 1 / 5.0f) * 255 + 0.5f), 0, 255));
			alpha[3] = static_cast<uint8_t>(MathLib::clamp(static_cast<int>(MathLib::lerp(falpha0, falpha1, 2 / 5.0f) * 255 + 0.5f), 0, 255));
			alpha[4] = static_cast<uint8_t>(MathLib::clamp(static_cast<int>(MathLib::lerp(falpha0, falpha1, 3 / 5.0f) * 255 + 0.5f), 0, 255));
			alpha[5] = static_cast<uint8_t>(MathLib::clamp(static_cast<int>(MathLib::lerp(falpha0, falpha1, 4 / 5.0f) * 255 + 0.5f), 0, 255));
			alpha[6] = 0;
			alpha[7] = 255;
		}

		for (int i = 0; i < 2; ++ i)
		{
			uint32_t alpha32 = (bc4.bitmap[i * 3 + 2] << 16) | (bc4.bitmap[i * 3 + 1] << 8) | (bc4.bitmap[i * 3 + 0] << 0);
			for (int j = 0; j < 8; ++ j)
			{
				alpha_block[i * 8 + j] = alpha[(alpha32 >> (j * 3)) & 0x7];
			}
		}
	}


	TexCompressionBC5::TexCompressionBC5()
	{
		block_width_ = block_height_ = 4;
		block_depth_ = 1;
		block_bytes_ = NumFormatBytes(EF_BC5) * 4;
		decoded_fmt_ = EF_GR8;
	}

	void TexCompressionBC5::EncodeBlock(void* output, void const * input, TexCompressionMethod method)
	{
		BOOST_ASSERT(output);
		BOOST_ASSERT(input);

		BC5Block& bc5 = *static_cast<BC5Block*>(output);
		uint16_t const * gr = static_cast<uint16_t const *>(input);

		std::array<uint8_t, 16> r = { { 0 } };
		std::array<uint8_t, 16> g = { { 0 } };
		for (size_t i = 0; i < r.size(); ++ i)
		{
			r[i] = gr[i] & 0xFF;
			g[i] = gr[i] >> 8;
		}

		bc4_codec_.EncodeBlock(&bc5.red, &r[0], method);
		bc4_codec_.EncodeBlock(&bc5.green, &g[0], method);
	}

	void TexCompressionBC5::DecodeBlock(void* output, void const * input)
	{
		BOOST_ASSERT(output);
		BOOST_ASSERT(input);

		uint16_t* gr = static_cast<uint16_t*>(output);
		BC5Block const * bc5_block = static_cast<BC5Block const *>(input);

		std::array<uint8_t, 16> r;
		bc4_codec_.DecodeBlock(&r[0], &bc5_block->red);
		std::array<uint8_t, 16> g;
		bc4_codec_.DecodeBlock(&g[0], &bc5_block->green);

		for (size_t i = 0; i < r.size(); ++ i)
		{
			gr[i] = r[i] | (g[i] << 8);
		}
	}


	// BC6H Compression
	TexCompressionBC6U::ModeDescriptor const TexCompressionBC6U::mode_desc_[14][82] =
	{
		{   // Mode 1 (0x00) - 10 5 5 5
			{ M, 0 }, { M, 1 }, { GY, 4 }, { BY, 4 }, { BZ, 4 }, { RW, 0 }, { RW, 1 }, { RW, 2 }, { RW, 3 },
			{ RW, 4 }, { RW, 5 }, { RW, 6 }, { RW, 7 }, { RW, 8 }, { RW, 9 }, { GW, 0 }, { GW, 1 }, { GW, 2 },
			{ GW, 3 }, { GW, 4 }, { GW, 5 }, { GW, 6 }, { GW, 7 }, { GW, 8 }, { GW, 9 }, { BW, 0 }, { BW, 1 },
			{ BW, 2 }, { BW, 3 }, { BW, 4 }, { BW, 5 }, { BW, 6 }, { BW, 7 }, { BW, 8 }, { BW, 9 }, { RX, 0 },
			{ RX, 1 }, { RX, 2 }, { RX, 3 }, { RX, 4 }, { GZ, 4 }, { GY, 0 }, { GY, 1 }, { GY, 2 }, { GY, 3 },
			{ GX, 0 }, { GX, 1 }, { GX, 2 }, { GX, 3 }, { GX, 4 }, { BZ, 0 }, { GZ, 0 }, { GZ, 1 }, { GZ, 2 },
			{ GZ, 3 }, { BX, 0 }, { BX, 1 }, { BX, 2 }, { BX, 3 }, { BX, 4 }, { BZ, 1 }, { BY, 0 }, { BY, 1 },
			{ BY, 2 }, { BY, 3 }, { RY, 0 }, { RY, 1 }, { RY, 2 }, { RY, 3 }, { RY, 4 }, { BZ, 2 }, { RZ, 0 },
			{ RZ, 1 }, { RZ, 2 }, { RZ, 3 }, { RZ, 4 }, { BZ, 3 }, { D, 0 }, { D, 1 }, { D, 2 }, { D, 3 },
			{ D, 4 }
		},

		{   // Mode 2 (0x01) - 7 6 6 6
			{ M, 0 }, { M, 1 }, { GY, 5 }, { GZ, 4 }, { GZ, 5 }, { RW, 0 }, { RW, 1 }, { RW, 2 }, { RW, 3 },
			{ RW, 4 }, { RW, 5 }, { RW, 6 }, { BZ, 0 }, { BZ, 1 }, { BY, 4 }, { GW, 0 }, { GW, 1 }, { GW, 2 },
			{ GW, 3 }, { GW, 4 }, { GW, 5 }, { GW, 6 }, { BY, 5 }, { BZ, 2 }, { GY, 4 }, { BW, 0 }, { BW, 1 },
			{ BW, 2 }, { BW, 3 }, { BW, 4 }, { BW, 5 }, { BW, 6 }, { BZ, 3 }, { BZ, 5 }, { BZ, 4 }, { RX, 0 },
			{ RX, 1 }, { RX, 2 }, { RX, 3 }, { RX, 4 }, { RX, 5 }, { GY, 0 }, { GY, 1 }, { GY, 2 }, { GY, 3 },
			{ GX, 0 }, { GX, 1 }, { GX, 2 }, { GX, 3 }, { GX, 4 }, { GX, 5 }, { GZ, 0 }, { GZ, 1 }, { GZ, 2 },
			{ GZ, 3 }, { BX, 0 }, { BX, 1 }, { BX, 2 }, { BX, 3 }, { BX, 4 }, { BX, 5 }, { BY, 0 }, { BY, 1 },
			{ BY, 2 }, { BY, 3 }, { RY, 0 }, { RY, 1 }, { RY, 2 }, { RY, 3 }, { RY, 4 }, { RY, 5 }, { RZ, 0 },
			{ RZ, 1 }, { RZ, 2 }, { RZ, 3 }, { RZ, 4 }, { RZ, 5 }, { D, 0 }, { D, 1 }, { D, 2 }, { D, 3 },
			{ D, 4 }
		},

		{   // Mode 3 (0x02) - 11 5 4 4
			{ M, 0 }, { M, 1 }, { M, 2 }, { M, 3 }, { M, 4 }, { RW, 0 }, { RW, 1 }, { RW, 2 }, { RW, 3 },
			{ RW, 4 }, { RW, 5 }, { RW, 6 }, { RW, 7 }, { RW, 8 }, { RW, 9 }, { GW, 0 }, { GW, 1 }, { GW, 2 },
			{ GW, 3 }, { GW, 4 }, { GW, 5 }, { GW, 6 }, { GW, 7 }, { GW, 8 }, { GW, 9 }, { BW, 0 }, { BW, 1 },
			{ BW, 2 }, { BW, 3 }, { BW, 4 }, { BW, 5 }, { BW, 6 }, { BW, 7 }, { BW, 8 }, { BW, 9 }, { RX, 0 },
			{ RX, 1 }, { RX, 2 }, { RX, 3 }, { RX, 4 }, { RW, 10 }, { GY, 0 }, { GY, 1 }, { GY, 2 }, { GY, 3 },
			{ GX, 0 }, { GX, 1 }, { GX, 2 }, { GX, 3 }, { GW, 10 }, { BZ, 0 }, { GZ, 0 }, { GZ, 1 }, { GZ, 2 },
			{ GZ, 3 }, { BX, 0 }, { BX, 1 }, { BX, 2 }, { BX, 3 }, { BW, 10 }, { BZ, 1 }, { BY, 0 }, { BY, 1 },
			{ BY, 2 }, { BY, 3 }, { RY, 0 }, { RY, 1 }, { RY, 2 }, { RY, 3 }, { RY, 4 }, { BZ, 2 }, { RZ, 0 },
			{ RZ, 1 }, { RZ, 2 }, { RZ, 3 }, { RZ, 4 }, { BZ, 3 }, { D, 0 }, { D, 1 }, { D, 2 }, { D, 3 },
			{ D, 4 }
		},

		{   // Mode 4 (0x06) - 11 4 5 4
			{ M, 0 }, { M, 1 }, { M, 2 }, { M, 3 }, { M, 4 }, { RW, 0 }, { RW, 1 }, { RW, 2 }, { RW, 3 },
			{ RW, 4 }, { RW, 5 }, { RW, 6 }, { RW, 7 }, { RW, 8 }, { RW, 9 }, { GW, 0 }, { GW, 1 }, { GW, 2 },
			{ GW, 3 }, { GW, 4 }, { GW, 5 }, { GW, 6 }, { GW, 7 }, { GW, 8 }, { GW, 9 }, { BW, 0 }, { BW, 1 },
			{ BW, 2 }, { BW, 3 }, { BW, 4 }, { BW, 5 }, { BW, 6 }, { BW, 7 }, { BW, 8 }, { BW, 9 }, { RX, 0 },
			{ RX, 1 }, { RX, 2 }, { RX, 3 }, { RW, 10 }, { GZ, 4 }, { GY, 0 }, { GY, 1 }, { GY, 2 }, { GY, 3 },
			{ GX, 0 }, { GX, 1 }, { GX, 2 }, { GX, 3 }, { GX, 4 }, { GW, 10 }, { GZ, 0 }, { GZ, 1 }, { GZ, 2 },
			{ GZ, 3 }, { BX, 0 }, { BX, 1 }, { BX, 2 }, { BX, 3 }, { BW, 10 }, { BZ, 1 }, { BY, 0 }, { BY, 1 },
			{ BY, 2 }, { BY, 3 }, { RY, 0 }, { RY, 1 }, { RY, 2 }, { RY, 3 }, { BZ, 0 }, { BZ, 2 }, { RZ, 0 },
			{ RZ, 1 }, { RZ, 2 }, { RZ, 3 }, { GY, 4 }, { BZ, 3 }, { D, 0 }, { D, 1 }, { D, 2 }, { D, 3 },
			{ D, 4 }
		},

		{   // Mode 5 (0x0A) - 11 4 4 5
			{ M, 0 }, { M, 1 }, { M, 2 }, { M, 3 }, { M, 4 }, { RW, 0 }, { RW, 1 }, { RW, 2 }, { RW, 3 },
			{ RW, 4 }, { RW, 5 }, { RW, 6 }, { RW, 7 }, { RW, 8 }, { RW, 9 }, { GW, 0 }, { GW, 1 }, { GW, 2 },
			{ GW, 3 }, { GW, 4 }, { GW, 5 }, { GW, 6 }, { GW, 7 }, { GW, 8 }, { GW, 9 }, { BW, 0 }, { BW, 1 },
			{ BW, 2 }, { BW, 3 }, { BW, 4 }, { BW, 5 }, { BW, 6 }, { BW, 7 }, { BW, 8 }, { BW, 9 }, { RX, 0 },
			{ RX, 1 }, { RX, 2 }, { RX, 3 }, { RW, 10 }, { BY, 4 }, { GY, 0 }, { GY, 1 }, { GY, 2 }, { GY, 3 },
			{ GX, 0 }, { GX, 1 }, { GX, 2 }, { GX, 3 }, { GW, 10 }, { BZ, 0 }, { GZ, 0 }, { GZ, 1 }, { GZ, 2 },
			{ GZ, 3 }, { BX, 0 }, { BX, 1 }, { BX, 2 }, { BX, 3 }, { BX, 4 }, { BW, 10 }, { BY, 0 }, { BY, 1 },
			{ BY, 2 }, { BY, 3 }, { RY, 0 }, { RY, 1 }, { RY, 2 }, { RY, 3 }, { BZ, 1 }, { BZ, 2 }, { RZ, 0 },
			{ RZ, 1 }, { RZ, 2 }, { RZ, 3 }, { BZ, 4 }, { BZ, 3 }, { D, 0 }, { D, 1 }, { D, 2 }, { D, 3 },
			{ D, 4 }
		},

		{   // Mode 6 (0x0E) - 9 5 5 5
			{ M, 0 }, { M, 1 }, { M, 2 }, { M, 3 }, { M, 4 }, { RW, 0 }, { RW, 1 }, { RW, 2 }, { RW, 3 },
			{ RW, 4 }, { RW, 5 }, { RW, 6 }, { RW, 7 }, { RW, 8 }, { BY, 4 }, { GW, 0 }, { GW, 1 }, { GW, 2 },
			{ GW, 3 }, { GW, 4 }, { GW, 5 }, { GW, 6 }, { GW, 7 }, { GW, 8 }, { GY, 4 }, { BW, 0 }, { BW, 1 },
			{ BW, 2 }, { BW, 3 }, { BW, 4 }, { BW, 5 }, { BW, 6 }, { BW, 7 }, { BW, 8 }, { BZ, 4 }, { RX, 0 },
			{ RX, 1 }, { RX, 2 }, { RX, 3 }, { RX, 4 }, { GZ, 4 }, { GY, 0 }, { GY, 1 }, { GY, 2 }, { GY, 3 },
			{ GX, 0 }, { GX, 1 }, { GX, 2 }, { GX, 3 }, { GX, 4 }, { BZ, 0 }, { GZ, 0 }, { GZ, 1 }, { GZ, 2 },
			{ GZ, 3 }, { BX, 0 }, { BX, 1 }, { BX, 2 }, { BX, 3 }, { BX, 4 }, { BZ, 1 }, { BY, 0 }, { BY, 1 },
			{ BY, 2 }, { BY, 3 }, { RY, 0 }, { RY, 1 }, { RY, 2 }, { RY, 3 }, { RY, 4 }, { BZ, 2 }, { RZ, 0 },
			{ RZ, 1 }, { RZ, 2 }, { RZ, 3 }, { RZ, 4 }, { BZ, 3 }, { D, 0 }, { D, 1 }, { D, 2 }, { D, 3 },
			{ D, 4 }
		},

		{   // Mode 7 (0x12) - 8 6 5 5
			{ M, 0 }, { M, 1 }, { M, 2 }, { M, 3 }, { M, 4 }, { RW, 0 }, { RW, 1 }, { RW, 2 }, { RW, 3 },
			{ RW, 4 }, { RW, 5 }, { RW, 6 }, { RW, 7 }, { GZ, 4 }, { BY, 4 }, { GW, 0 }, { GW, 1 }, { GW, 2 },
			{ GW, 3 }, { GW, 4 }, { GW, 5 }, { GW, 6 }, { GW, 7 }, { BZ, 2 }, { GY, 4 }, { BW, 0 }, { BW, 1 },
			{ BW, 2 }, { BW, 3 }, { BW, 4 }, { BW, 5 }, { BW, 6 }, { BW, 7 }, { BZ, 3 }, { BZ, 4 }, { RX, 0 },
			{ RX, 1 }, { RX, 2 }, { RX, 3 }, { RX, 4 }, { RX, 5 }, { GY, 0 }, { GY, 1 }, { GY, 2 }, { GY, 3 },
			{ GX, 0 }, { GX, 1 }, { GX, 2 }, { GX, 3 }, { GX, 4 }, { BZ, 0 }, { GZ, 0 }, { GZ, 1 }, { GZ, 2 },
			{ GZ, 3 }, { BX, 0 }, { BX, 1 }, { BX, 2 }, { BX, 3 }, { BX, 4 }, { BZ, 1 }, { BY, 0 }, { BY, 1 },
			{ BY, 2 }, { BY, 3 }, { RY, 0 }, { RY, 1 }, { RY, 2 }, { RY, 3 }, { RY, 4 }, { RY, 5 }, { RZ, 0 },
			{ RZ, 1 }, { RZ, 2 }, { RZ, 3 }, { RZ, 4 }, { RZ, 5 }, { D, 0 }, { D, 1 }, { D, 2 }, { D, 3 },
			{ D, 4 }
		},

		{   // Mode 8 (0x16) - 8 5 6 5
			{ M, 0 }, { M, 1 }, { M, 2 }, { M, 3 }, { M, 4 }, { RW, 0 }, { RW, 1 }, { RW, 2 }, { RW, 3 },
			{ RW, 4 }, { RW, 5 }, { RW, 6 }, { RW, 7 }, { BZ, 0 }, { BY, 4 }, { GW, 0 }, { GW, 1 }, { GW, 2 },
			{ GW, 3 }, { GW, 4 }, { GW, 5 }, { GW, 6 }, { GW, 7 }, { GY, 5 }, { GY, 4 }, { BW, 0 }, { BW, 1 },
			{ BW, 2 }, { BW, 3 }, { BW, 4 }, { BW, 5 }, { BW, 6 }, { BW, 7 }, { GZ, 5 }, { BZ, 4 }, { RX, 0 },
			{ RX, 1 }, { RX, 2 }, { RX, 3 }, { RX, 4 }, { GZ, 4 }, { GY, 0 }, { GY, 1 }, { GY, 2 }, { GY, 3 },
			{ GX, 0 }, { GX, 1 }, { GX, 2 }, { GX, 3 }, { GX, 4 }, { GX, 5 }, { GZ, 0 }, { GZ, 1 }, { GZ, 2 },
			{ GZ, 3 }, { BX, 0 }, { BX, 1 }, { BX, 2 }, { BX, 3 }, { BX, 4 }, { BZ, 1 }, { BY, 0 }, { BY, 1 },
			{ BY, 2 }, { BY, 3 }, { RY, 0 }, { RY, 1 }, { RY, 2 }, { RY, 3 }, { RY, 4 }, { BZ, 2 }, { RZ, 0 },
			{ RZ, 1 }, { RZ, 2 }, { RZ, 3 }, { RZ, 4 }, { BZ, 3 }, { D, 0 }, { D, 1 }, { D, 2 }, { D, 3 },
			{ D, 4 }
		},

		{   // Mode 9 (0x1A) - 8 5 5 6
			{ M, 0 }, { M, 1 }, { M, 2 }, { M, 3 }, { M, 4 }, { RW, 0 }, { RW, 1 }, { RW, 2 }, { RW, 3 },
			{ RW, 4 }, { RW, 5 }, { RW, 6 }, { RW, 7 }, { BZ, 1 }, { BY, 4 }, { GW, 0 }, { GW, 1 }, { GW, 2 },
			{ GW, 3 }, { GW, 4 }, { GW, 5 }, { GW, 6 }, { GW, 7 }, { BY, 5 }, { GY, 4 }, { BW, 0 }, { BW, 1 },
			{ BW, 2 }, { BW, 3 }, { BW, 4 }, { BW, 5 }, { BW, 6 }, { BW, 7 }, { BZ, 5 }, { BZ, 4 }, { RX, 0 },
			{ RX, 1 }, { RX, 2 }, { RX, 3 }, { RX, 4 }, { GZ, 4 }, { GY, 0 }, { GY, 1 }, { GY, 2 }, { GY, 3 },
			{ GX, 0 }, { GX, 1 }, { GX, 2 }, { GX, 3 }, { GX, 4 }, { BZ, 0 }, { GZ, 0 }, { GZ, 1 }, { GZ, 2 },
			{ GZ, 3 }, { BX, 0 }, { BX, 1 }, { BX, 2 }, { BX, 3 }, { BX, 4 }, { BX, 5 }, { BY, 0 }, { BY, 1 },
			{ BY, 2 }, { BY, 3 }, { RY, 0 }, { RY, 1 }, { RY, 2 }, { RY, 3 }, { RY, 4 }, { BZ, 2 }, { RZ, 0 },
			{ RZ, 1 }, { RZ, 2 }, { RZ, 3 }, { RZ, 4 }, { BZ, 3 }, { D, 0 }, { D, 1 }, { D, 2 }, { D, 3 },
			{ D, 4 }
		},

		{   // Mode 10 (0x1E) - 6 6 6 6
			{ M, 0 }, { M, 1 }, { M, 2 }, { M, 3 }, { M, 4 }, { RW, 0 }, { RW, 1 }, { RW, 2 }, { RW, 3 },
			{ RW, 4 }, { RW, 5 }, { GZ, 4 }, { BZ, 0 }, { BZ, 1 }, { BY, 4 }, { GW, 0 }, { GW, 1 }, { GW, 2 },
			{ GW, 3 }, { GW, 4 }, { GW, 5 }, { GY, 5 }, { BY, 5 }, { BZ, 2 }, { GY, 4 }, { BW, 0 }, { BW, 1 },
			{ BW, 2 }, { BW, 3 }, { BW, 4 }, { BW, 5 }, { GZ, 5 }, { BZ, 3 }, { BZ, 5 }, { BZ, 4 }, { RX, 0 },
			{ RX, 1 }, { RX, 2 }, { RX, 3 }, { RX, 4 }, { RX, 5 }, { GY, 0 }, { GY, 1 }, { GY, 2 }, { GY, 3 },
			{ GX, 0 }, { GX, 1 }, { GX, 2 }, { GX, 3 }, { GX, 4 }, { GX, 5 }, { GZ, 0 }, { GZ, 1 }, { GZ, 2 },
			{ GZ, 3 }, { BX, 0 }, { BX, 1 }, { BX, 2 }, { BX, 3 }, { BX, 4 }, { BX, 5 }, { BY, 0 }, { BY, 1 },
			{ BY, 2 }, { BY, 3 }, { RY, 0 }, { RY, 1 }, { RY, 2 }, { RY, 3 }, { RY, 4 }, { RY, 5 }, { RZ, 0 },
			{ RZ, 1 }, { RZ, 2 }, { RZ, 3 }, { RZ, 4 }, { RZ, 5 }, { D, 0 }, { D, 1 }, { D, 2 }, { D, 3 },
			{ D, 4 }
		},

		{   // Mode 11 (0x03) - 10 10
			{ M, 0 }, { M, 1 }, { M, 2 }, { M, 3 }, { M, 4 }, { RW, 0 }, { RW, 1 }, { RW, 2 }, { RW, 3 },
			{ RW, 4 }, { RW, 5 }, { RW, 6 }, { RW, 7 }, { RW, 8 }, { RW, 9 }, { GW, 0 }, { GW, 1 }, { GW, 2 },
			{ GW, 3 }, { GW, 4 }, { GW, 5 }, { GW, 6 }, { GW, 7 }, { GW, 8 }, { GW, 9 }, { BW, 0 }, { BW, 1 },
			{ BW, 2 }, { BW, 3 }, { BW, 4 }, { BW, 5 }, { BW, 6 }, { BW, 7 }, { BW, 8 }, { BW, 9 }, { RX, 0 },
			{ RX, 1 }, { RX, 2 }, { RX, 3 }, { RX, 4 }, { RX, 5 }, { RX, 6 }, { RX, 7 }, { RX, 8 }, { RX, 9 },
			{ GX, 0 }, { GX, 1 }, { GX, 2 }, { GX, 3 }, { GX, 4 }, { GX, 5 }, { GX, 6 }, { GX, 7 }, { GX, 8 },
			{ GX, 9 }, { BX, 0 }, { BX, 1 }, { BX, 2 }, { BX, 3 }, { BX, 4 }, { BX, 5 }, { BX, 6 }, { BX, 7 },
			{ BX, 8 }, { BX, 9 }, { NA, 0 }, { NA, 0 }, { NA, 0 }, { NA, 0 }, { NA, 0 }, { NA, 0 }, { NA, 0 },
			{ NA, 0 }, { NA, 0 }, { NA, 0 }, { NA, 0 }, { NA, 0 }, { NA, 0 }, { NA, 0 }, { NA, 0 }, { NA, 0 },
			{ NA, 0 }
		},

		{   // Mode 12 (0x07) - 11 9
			{ M, 0 }, { M, 1 }, { M, 2 }, { M, 3 }, { M, 4 }, { RW, 0 }, { RW, 1 }, { RW, 2 }, { RW, 3 },
			{ RW, 4 }, { RW, 5 }, { RW, 6 }, { RW, 7 }, { RW, 8 }, { RW, 9 }, { GW, 0 }, { GW, 1 }, { GW, 2 },
			{ GW, 3 }, { GW, 4 }, { GW, 5 }, { GW, 6 }, { GW, 7 }, { GW, 8 }, { GW, 9 }, { BW, 0 }, { BW, 1 },
			{ BW, 2 }, { BW, 3 }, { BW, 4 }, { BW, 5 }, { BW, 6 }, { BW, 7 }, { BW, 8 }, { BW, 9 }, { RX, 0 },
			{ RX, 1 }, { RX, 2 }, { RX, 3 }, { RX, 4 }, { RX, 5 }, { RX, 6 }, { RX, 7 }, { RX, 8 }, { RW, 10 },
			{ GX, 0 }, { GX, 1 }, { GX, 2 }, { GX, 3 }, { GX, 4 }, { GX, 5 }, { GX, 6 }, { GX, 7 }, { GX, 8 },
			{ GW, 10 }, { BX, 0 }, { BX, 1 }, { BX, 2 }, { BX, 3 }, { BX, 4 }, { BX, 5 }, { BX, 6 }, { BX, 7 },
			{ BX, 8 }, { BW, 10 }, { NA, 0 }, { NA, 0 }, { NA, 0 }, { NA, 0 }, { NA, 0 }, { NA, 0 }, { NA, 0 },
			{ NA, 0 }, { NA, 0 }, { NA, 0 }, { NA, 0 }, { NA, 0 }, { NA, 0 }, { NA, 0 }, { NA, 0 }, { NA, 0 },
			{ NA, 0 }
		},

		{   // Mode 13 (0x0B) - 12 8
			{ M, 0 }, { M, 1 }, { M, 2 }, { M, 3 }, { M, 4 }, { RW, 0 }, { RW, 1 }, { RW, 2 }, { RW, 3 },
			{ RW, 4 }, { RW, 5 }, { RW, 6 }, { RW, 7 }, { RW, 8 }, { RW, 9 }, { GW, 0 }, { GW, 1 }, { GW, 2 },
			{ GW, 3 }, { GW, 4 }, { GW, 5 }, { GW, 6 }, { GW, 7 }, { GW, 8 }, { GW, 9 }, { BW, 0 }, { BW, 1 },
			{ BW, 2 }, { BW, 3 }, { BW, 4 }, { BW, 5 }, { BW, 6 }, { BW, 7 }, { BW, 8 }, { BW, 9 }, { RX, 0 },
			{ RX, 1 }, { RX, 2 }, { RX, 3 }, { RX, 4 }, { RX, 5 }, { RX, 6 }, { RX, 7 }, { RW, 11 }, { RW, 10 },
			{ GX, 0 }, { GX, 1 }, { GX, 2 }, { GX, 3 }, { GX, 4 }, { GX, 5 }, { GX, 6 }, { GX, 7 }, { GW, 11 },
			{ GW, 10 }, { BX, 0 }, { BX, 1 }, { BX, 2 }, { BX, 3 }, { BX, 4 }, { BX, 5 }, { BX, 6 }, { BX, 7 },
			{ BW, 11 }, { BW, 10 }, { NA, 0 }, { NA, 0 }, { NA, 0 }, { NA, 0 }, { NA, 0 }, { NA, 0 }, { NA, 0 },
			{ NA, 0 }, { NA, 0 }, { NA, 0 }, { NA, 0 }, { NA, 0 }, { NA, 0 }, { NA, 0 }, { NA, 0 }, { NA, 0 },
			{ NA, 0 }
		},

		{   // Mode 14 (0x0F) - 16 4
			{ M, 0 }, { M, 1 }, { M, 2 }, { M, 3 }, { M, 4 }, { RW, 0 }, { RW, 1 }, { RW, 2 }, { RW, 3 },
			{ RW, 4 }, { RW, 5 }, { RW, 6 }, { RW, 7 }, { RW, 8 }, { RW, 9 }, { GW, 0 }, { GW, 1 }, { GW, 2 },
			{ GW, 3 }, { GW, 4 }, { GW, 5 }, { GW, 6 }, { GW, 7 }, { GW, 8 }, { GW, 9 }, { BW, 0 }, { BW, 1 },
			{ BW, 2 }, { BW, 3 }, { BW, 4 }, { BW, 5 }, { BW, 6 }, { BW, 7 }, { BW, 8 }, { BW, 9 }, { RX, 0 },
			{ RX, 1 }, { RX, 2 }, { RX, 3 }, { RW, 15 }, { RW, 14 }, { RW, 13 }, { RW, 12 }, { RW, 11 }, { RW, 10 },
			{ GX, 0 }, { GX, 1 }, { GX, 2 }, { GX, 3 }, { GW, 15 }, { GW, 14 }, { GW, 13 }, { GW, 12 }, { GW, 11 },
			{ GW, 10 }, { BX, 0 }, { BX, 1 }, { BX, 2 }, { BX, 3 }, { BW, 15 }, { BW, 14 }, { BW, 13 }, { BW, 12 },
			{ BW, 11 }, { BW, 10 }, { NA, 0 }, { NA, 0 }, { NA, 0 }, { NA, 0 }, { NA, 0 }, { NA, 0 }, { NA, 0 },
			{ NA, 0 }, { NA, 0 }, { NA, 0 }, { NA, 0 }, { NA, 0 }, { NA, 0 }, { NA, 0 }, { NA, 0 }, { NA, 0 },
			{ NA, 0 }
		}
	};

	// Mode, Partitions, Transformed, IndexPrec, RGBAPrec
	TexCompressionBC6U::ModeInfo const TexCompressionBC6U::mode_info_[] =
	{
		{ 0x00, 2, true, 3, { { ARGBColor32(0, 10, 10, 10), ARGBColor32(0, 5, 5, 5) },
			{ ARGBColor32(0, 5, 5, 5), ARGBColor32(0, 5, 5, 5) } } },
		{ 0x01, 2, true, 3, { { ARGBColor32(0, 7, 7, 7), ARGBColor32(0, 6, 6, 6) },
			{ ARGBColor32(0, 6, 6, 6), ARGBColor32(0, 6, 6, 6) } } },
		{ 0x02, 2, true, 3, { { ARGBColor32(0, 11, 11, 11), ARGBColor32(0, 5, 4, 4) },
			{ ARGBColor32(0, 5, 4, 4), ARGBColor32(0, 5, 4, 4) } } },
		{ 0x06, 2, true, 3, { { ARGBColor32(0, 11, 11, 11), ARGBColor32(0, 4, 5, 4) },
			{ ARGBColor32(0, 4, 5, 4), ARGBColor32(0, 4, 5, 4) } } },
		{ 0x0A, 2, true, 3, { { ARGBColor32(0, 11, 11, 11), ARGBColor32(0, 4, 4, 5) },
			{ ARGBColor32(0, 4, 4, 5), ARGBColor32(0, 4, 4, 5) } } },
		{ 0x0E, 2, true, 3, { { ARGBColor32(0, 9, 9, 9), ARGBColor32(0, 5, 5, 5) },
			{ ARGBColor32(0, 5, 5, 5), ARGBColor32(0, 5, 5, 5) } } },
		{ 0x12, 2, true, 3, { { ARGBColor32(0, 8, 8, 8), ARGBColor32(0, 6, 5, 5) },
			{ ARGBColor32(0, 6, 5, 5), ARGBColor32(0, 6, 5, 5) } } },
		{ 0x16, 2, true, 3, { { ARGBColor32(0, 8, 8, 8), ARGBColor32(0, 5, 6, 5) },
			{ ARGBColor32(0, 5, 6, 5), ARGBColor32(0, 5, 6, 5) } } },
		{ 0x1A, 2, true, 3, { { ARGBColor32(0, 8, 8, 8), ARGBColor32(0, 5, 5, 6) },
			{ ARGBColor32(0, 5, 5, 6), ARGBColor32(0, 5, 5, 6) } } },
		{ 0x1E, 2, false, 3, { { ARGBColor32(0, 6, 6, 6), ARGBColor32(0, 6, 6, 6) },
			{ ARGBColor32(0, 6, 6, 6), ARGBColor32(0, 6, 6, 6) } } },
		{ 0x03, 1, false, 4, { { ARGBColor32(0, 10, 10, 10), ARGBColor32(0, 10, 10, 10) },
			{ ARGBColor32(0, 0, 0, 0), ARGBColor32(0, 0, 0, 0) } } },
		{ 0x07, 1, true, 4, { { ARGBColor32(0, 11, 11, 11), ARGBColor32(0, 9, 9, 9) },
			{ ARGBColor32(0, 0, 0, 0), ARGBColor32(0, 0, 0, 0) } } },
		{ 0x0B, 1, true, 4, { { ARGBColor32(0, 12, 12, 12), ARGBColor32(0, 8, 8, 8) },
			{ ARGBColor32(0, 0, 0, 0), ARGBColor32(0, 0, 0, 0) } } },
		{ 0x0F, 1, true, 4, { { ARGBColor32(0, 16, 16, 16), ARGBColor32(0, 4, 4, 4) },
			{ ARGBColor32(0, 0, 0, 0), ARGBColor32(0, 0, 0, 0) } } }
	};

	int const TexCompressionBC6U::mode_to_info_[] =
	{
		0, // Mode 1   - 0x00
		1, // Mode 2   - 0x01
		2, // Mode 3   - 0x02
		10, // Mode 11  - 0x03
		-1, // Invalid  - 0x04
		-1, // Invalid  - 0x05
		3, // Mode 4   - 0x06
		11, // Mode 12  - 0x07
		-1, // Invalid  - 0x08
		-1, // Invalid  - 0x09
		4, // Mode 5   - 0x0a
		12, // Mode 13  - 0x0b
		-1, // Invalid  - 0x0c
		-1, // Invalid  - 0x0d
		5, // Mode 6   - 0x0e
		13, // Mode 14  - 0x0f
		-1, // Invalid  - 0x10
		-1, // Invalid  - 0x11
		6, // Mode 7   - 0x12
		-1, // Reserved - 0x13
		-1, // Invalid  - 0x14
		-1, // Invalid  - 0x15
		7, // Mode 8   - 0x16
		-1, // Reserved - 0x17
		-1, // Invalid  - 0x18
		-1, // Invalid  - 0x19
		8, // Mode 9   - 0x1a
		-1, // Reserved - 0x1b
		-1, // Invalid  - 0x1c
		-1, // Invalid  - 0x1d
		9, // Mode 10  - 0x1e
		-1, // Resreved - 0x1f
	};

	TexCompressionBC6U::TexCompressionBC6U()
	{
		block_width_ = block_height_ = 4;
		block_depth_ = 1;
		block_bytes_ = NumFormatBytes(EF_BC6) * 4;
		decoded_fmt_ = EF_ABGR16F;
	}

	void TexCompressionBC6U::EncodeBlock(void* output, void const * input, TexCompressionMethod method)
	{
		KFL_UNUSED(output);
		KFL_UNUSED(input);
		KFL_UNUSED(method);

		// TODO
	}

	void TexCompressionBC6U::DecodeBlock(void* output, void const * input)
	{
		this->DecodeBC6Internal(output, input, false);
	}

	void TexCompressionBC6U::DecodeBC6Internal(void* output, void const * input, bool signed_fmt)
	{
		BOOST_ASSERT(output);
		BOOST_ASSERT(input);

		Vector_T<half, 4>* abgr = static_cast<Vector_T<half, 4>*>(output);

		size_t start_bit = 0;
		uint8_t mode = ReadBits(input, start_bit, 2);
		if (mode > 1)
		{
			mode = (ReadBits(input, start_bit, 3) << 2) | mode;
		}
		BOOST_ASSERT(mode < 32);

		if (mode_to_info_[mode] >= 0)
		{
			BOOST_ASSERT(mode_to_info_[mode] < static_cast<int>(std::size(mode_info_)));
			ModeDescriptor const * desc = mode_desc_[mode_to_info_[mode]];

			BOOST_ASSERT(mode_to_info_[mode] < static_cast<int>(std::size(mode_desc_)));
			ModeInfo const & info = mode_info_[mode_to_info_[mode]];

			std::array<std::pair<int3, int3>, BC6_MAX_REGIONS> end_pts;
			memset(&end_pts[0], 0, BC6_MAX_REGIONS * sizeof(end_pts[0]));
			uint32_t shape = 0;

			// Read header
			size_t const header_bits = info.partitions > 1 ? 82 : 65;
			while (start_bit < header_bits)
			{
				size_t curr_bit = start_bit;
				if (ReadBit(input, start_bit))
				{
					uint32_t val = 1UL << desc[curr_bit].bit;
					switch (desc[curr_bit].field)
					{
					case D:
						shape |= val;
						break;
					case RW:
						end_pts[0].first.x() |= val;
						break;
					case RX:
						end_pts[0].second.x() |= val;
						break;
					case RY:
						end_pts[1].first.x() |= val;
						break;
					case RZ:
						end_pts[1].second.x() |= val;
						break;
					case GW:
						end_pts[0].first.y() |= val;
						break;
					case GX:
						end_pts[0].second.y() |= val;
						break;
					case GY:
						end_pts[1].first.y() |= val;
						break;
					case GZ:
						end_pts[1].second.y() |= val;
						break;
					case BW:
						end_pts[0].first.z() |= val;
						break;
					case BX:
						end_pts[0].second.z() |= val;
						break;
					case BY:
						end_pts[1].first.z() |= val;
						break;
					case BZ:
						end_pts[1].second.z() |= val;
						break;

					default:
						memset(abgr, 0, 16 * sizeof(abgr[0]));
						return;
					}
				}
			}

			BOOST_ASSERT(shape < 64);

			// Sign extend necessary end points
			if (signed_fmt)
			{
				SignExtend(end_pts[0].first, info.rgba_prec[0][0]);
			}
			if (signed_fmt || info.transformed)
			{
				BOOST_ASSERT(info.partitions <= BC6_MAX_REGIONS);
				for (size_t p = 0; p < info.partitions; ++ p)
				{
					if (p > 0)
					{
						SignExtend(end_pts[p].first, info.rgba_prec[p][0]);
					}
					SignExtend(end_pts[p].second, info.rgba_prec[p][1]);
				}
			}

			// Inverse transform the end points
			if (info.transformed)
			{
				TransformInverse(&end_pts[0], info.rgba_prec[0][0], signed_fmt);
			}

			// Read indices
			for (uint32_t i = 0; i < 16; ++ i)
			{
				size_t num_bits = IsFixUpOffset(info.partitions, shape, i) ? info.index_prec - 1 : info.index_prec;
				if (start_bit + num_bits > 128)
				{
					memset(abgr, 0, 16 * sizeof(abgr[0]));
					return;
				}

				uint8_t index = ReadBits(input, start_bit, num_bits);
				if (index >= ((info.partitions > 1) ? 8 : 16))
				{
					memset(abgr, 0, 16 * sizeof(abgr[0]));
					return;
				}

				size_t region = GetPartition(info.partitions, shape, i);
				BOOST_ASSERT(region < BC6_MAX_REGIONS);

				// Unquantize endpoints and interpolate
				int r1 = this->Unquantize(end_pts[region].first.x(), info.rgba_prec[0][0].r(), signed_fmt);
				int g1 = this->Unquantize(end_pts[region].first.y(), info.rgba_prec[0][0].g(), signed_fmt);
				int b1 = this->Unquantize(end_pts[region].first.z(), info.rgba_prec[0][0].b(), signed_fmt);
				int r2 = this->Unquantize(end_pts[region].second.x(), info.rgba_prec[0][0].r(), signed_fmt);
				int g2 = this->Unquantize(end_pts[region].second.y(), info.rgba_prec[0][0].g(), signed_fmt);
				int b2 = this->Unquantize(end_pts[region].second.z(), info.rgba_prec[0][0].b(), signed_fmt);
				int const * weights = BC67_PREC_WEIGHTS[1 + (1 == info.partitions)];
				int3 fc;
				fc.x() = this->FinishUnquantize((r1 * (BC6_WEIGHT_MAX - weights[index])
					+ r2 * weights[index] + BC6_WEIGHT_ROUND) >> BC6_WEIGHT_SHIFT, signed_fmt);
				fc.y() = this->FinishUnquantize((g1 * (BC6_WEIGHT_MAX - weights[index])
					+ g2 * weights[index] + BC6_WEIGHT_ROUND) >> BC6_WEIGHT_SHIFT, signed_fmt);
				fc.z() = this->FinishUnquantize((b1 * (BC6_WEIGHT_MAX - weights[index])
					+ b2 * weights[index] + BC6_WEIGHT_ROUND) >> BC6_WEIGHT_SHIFT, signed_fmt);
				ToF16(abgr[i], fc, signed_fmt);
				abgr[i].w() = half(1.0f);
			}
		}
		else
		{
			// Per the BC6H format spec, we must return opaque black
			for (size_t i = 0; i < 16; ++ i)
			{
				abgr[i] = Vector_T<half, 4>(half(0.0f), half(0.0f), half(0.0f), half(1.0f));
			}
		}
	}

	int TexCompressionBC6U::Unquantize(int comp, uint8_t bits_per_comp, bool signed_fmt)
	{
		int unq = 0;
		if (signed_fmt)
		{
			if (bits_per_comp >= 16)
			{
				unq = comp;
			}
			else
			{
				int s = 0;
				if (comp < 0)
				{
					s = 1;
					comp = -comp;
				}

				if (0 == comp)
				{
					unq = 0;
				}
				else if (comp >= ((1 << (bits_per_comp - 1)) - 1))
				{
					unq = 0x7FFF;
				}
				else
				{
					unq = ((comp << 15) + 0x4000) >> (bits_per_comp - 1);
				}

				if (s)
				{
					unq = -unq;
				}
			}
		}
		else
		{
			if (bits_per_comp >= 15)
			{
				unq = comp;
			}
			else if (0 == comp)
			{
				unq = 0;
			}
			else if (comp == ((1 << bits_per_comp) - 1))
			{
				unq = 0xFFFF;
			}
			else
			{
				unq = ((comp << 16) + 0x8000) >> bits_per_comp;
			}
		}

		return unq;
	}

	int TexCompressionBC6U::FinishUnquantize(int comp, bool signed_fmt)
	{
		if (signed_fmt)
		{
			return (comp < 0) ? -(((-comp) * 31) >> 5) : (comp * 31) >> 5;  // scale the magnitude by 31/32
		}
		else
		{
			return (comp * 31) >> 6;                                        // scale the magnitude by 31/64
		}
	}


	TexCompressionBC6S::TexCompressionBC6S()
	{
		block_width_ = block_height_ = 4;
		block_depth_ = 1;
		block_bytes_ = NumFormatBytes(EF_SIGNED_BC6) * 4;
		decoded_fmt_ = EF_ABGR16F;
	}

	void TexCompressionBC6S::EncodeBlock(void* output, void const * input, TexCompressionMethod method)
	{
		KFL_UNUSED(output);
		KFL_UNUSED(input);
		KFL_UNUSED(method);

		// TODO
	}

	void TexCompressionBC6S::DecodeBlock(void* output, void const * input)
	{
		bc6u_codec_.DecodeBC6Internal(output, input, true);
	}


	// BC7 compression: mode partitions, partition_bits, p_bits, rotation_bits, index_mode_bits, index_prec, index_prec_2, rgba_prec, rgba_prec_with_p, p_bit_type
	TexCompressionBC7::ModeInfo const TexCompressionBC7::mode_info_[] =
	{
		// Mode 0: Color only, 3 Subsets, RGBP 4441 (unique P-bit), 3-bit indecies, 16 partitions
		{ 3, 4, 6, 0, 0, 3, 0, ARGBColor32(0, 4, 4, 4), ARGBColor32(0, 5, 5, 5), PBT_Unique },
		// Mode 1: Color only, 2 Subsets, RGBP 6661 (shared P-bit), 3-bit indecies, 64 partitions
		{ 2, 6, 2, 0, 0, 3, 0, ARGBColor32(0, 6, 6, 6), ARGBColor32(0, 7, 7, 7), PBT_Shared },
		// Mode 2: Color only, 3 Subsets, RGB 555, 2-bit indecies, 64 partitions
		{ 3, 6, 0, 0, 0, 2, 0, ARGBColor32(0, 5, 5, 5), ARGBColor32(0, 5, 5, 5), PBT_None },
		// Mode 3: Color only, 2 Subsets, RGBP 7771 (unique P-bit), 2-bits indecies, 64 partitions
		{ 2, 6, 4, 0, 0, 2, 0, ARGBColor32(0, 7, 7, 7), ARGBColor32(0, 8, 8, 8), PBT_Unique },
		// Mode 4: Color w/ Separate Alpha, 1 Subset, RGB 555, A6, 16x2/16x3-bit indices, 2-bit rotation, 1-bit index selector
		{ 1, 0, 0, 2, 1, 2, 3, ARGBColor32(6, 5, 5, 5), ARGBColor32(6, 5, 5, 5), PBT_None },
		// Mode 5: Color w/ Separate Alpha, 1 Subset, RGB 777, A8, 16x2/16x2-bit indices, 2-bit rotation
		{ 1, 0, 0, 2, 0, 2, 2, ARGBColor32(8, 7, 7, 7), ARGBColor32(8, 7, 7, 7), PBT_None },
		// Mode 6: Color+Alpha, 1 Subset, RGBAP 77771 (unique P-bit), 16x4-bit indecies
		{ 1, 0, 2, 0, 0, 4, 0, ARGBColor32(7, 7, 7, 7), ARGBColor32(8, 8, 8, 8), PBT_Unique },
		// Mode 7: Color+Alpha, 2 Subsets, RGBAP 55551 (unique P-bit), 2-bit indices, 64 partitions
		{ 2, 6, 4, 0, 0, 2, 0, ARGBColor32(5, 5, 5, 5), ARGBColor32(6, 6, 6, 6), PBT_Unique }
	};

	TexCompressionBC7::TexCompressionBC7()
		: index_mode_(0)
	{
		block_width_ = block_height_ = 4;
		block_depth_ = 1;
		block_bytes_ = NumFormatBytes(EF_BC7) * 4;
		decoded_fmt_ = EF_ARGB8;
	}

	void TexCompressionBC7::EncodeBlock(void* output, void const * input, TexCompressionMethod method)
	{
		BOOST_ASSERT(output);
		BOOST_ASSERT(input);
		
		// Based on FasTC: Accelerated Texture Encoding (http://gamma.cs.unc.edu/FasTC/)

		ARGBColor32 const * argb = static_cast<ARGBColor32 const *>(input);

		bool uniform_block = true;
		for (int i = 1; i < 16; ++ i)
		{
			if (argb[i] != argb[0])
			{
				uniform_block = false;
				break;
			}
		}
		if (uniform_block)
		{
			this->PackBC7UniformBlock(output, argb[0]);
			return;
		}

		TexCompressionErrorMetric metric = TCEM_Uniform;
		int sa_steps;
		switch (method)
		{
		case TCM_Quality:
			sa_steps = 50;
			break;
		case TCM_Balanced:
			sa_steps = 10;
			break;
		case TCM_Speed:
			sa_steps = 0;
			break;

		default:
			KFL_UNREACHABLE("Invalid compression method");
		}

		RGBACluster block_cluster(argb, block_width_ * block_height_, GetPartition);
		ShapeSelection selection = BoxSelection(block_cluster, metric);
		BOOST_ASSERT(selection.selected_modes > 0);

		uint64_t best_err = std::numeric_limits<uint64_t>::max();
		uint32_t best_mode = 8;
		CompressParams best_params;

		uint32_t selected_modes = selection.selected_modes;
		size_t num_shape_indices = selection.shapes.size();

		// If we don't have any indices, turn off two and three partition modes,
		// since the compressor will simply ignore the shapeIndex variable afterwards...
		if (0 == num_shape_indices)
		{
			num_shape_indices = 1;
			selected_modes &= ~(TWO_PARTITION_MODES | THREE_PARTITION_MODES);
		}

		for (uint32_t mode = 0; mode < 8; ++ mode)
		{
			if ((selected_modes & (1 << mode)) != 0)
			{
				for (uint32_t shape_index = 0; shape_index < num_shape_indices; ++ shape_index)
				{
					Shape const & shape = selection.shapes[shape_index];

					uint32_t partitions = mode_info_[mode].partitions;
					if ((1 == partitions) || (partitions == shape.num_partitions))
					{
						// Block mode zero only has four bits for the partition index,
						// so if the chosen three-partition shape is not within this range,
						// then we shouldn't consider using this block mode...
						if ((shape.index < 16) || (mode != 0))
						{
							block_cluster.ShapeIndex(shape.index, partitions);

							CompressParams params;
							uint64_t error = this->TryCompress(mode, sa_steps, metric, params, shape.index, block_cluster);
							if (error < best_err)
							{
								best_err = error;
								best_mode = mode;
								best_params = params;
							}
						}
					}
				}
			}
		}
		BOOST_ASSERT(best_mode < 8);

		index_mode_ = 0;
		this->PackBC7Block(best_mode, best_params, output);
	}

	void TexCompressionBC7::DecodeBlock(void* output, void const * input)
	{
		BOOST_ASSERT(output);
		BOOST_ASSERT(input);

		ARGBColor32* argb = static_cast<ARGBColor32*>(output);

		size_t first = 0;
		while ((first < 128) && !ReadBit(input, first));
		size_t mode = first - 1;
		if (mode < 8)
		{
			uint8_t const partitions = mode_info_[mode].partitions;
			BOOST_ASSERT(partitions <= BC7_MAX_REGIONS);

			uint8_t const num_end_pts = partitions << 1;
			uint8_t const index_prec_1 = mode_info_[mode].index_prec_1;
			uint8_t const index_prec_2 = mode_info_[mode].index_prec_2;
			size_t start_bit = mode + 1;
			std::array<uint8_t, 6> p;
			uint8_t shape = ReadBits(input, start_bit, mode_info_[mode].partition_bits);
			BOOST_ASSERT(shape < BC7_MAX_SHAPES);

			uint8_t rotation = ReadBits(input, start_bit, mode_info_[mode].rotation_bits);
			BOOST_ASSERT(rotation < 4);

			uint8_t index_mode = ReadBits(input, start_bit, mode_info_[mode].index_mode_bits);
			BOOST_ASSERT(index_mode < 2);

			std::array<ARGBColor32, BC7_MAX_REGIONS << 1> c;
			ARGBColor32 const & rgba_prec = mode_info_[mode].rgba_prec;
			ARGBColor32 const & rgba_prec_with_p = mode_info_[mode].rgba_prec_with_p;

			BOOST_ASSERT(num_end_pts <= (BC7_MAX_REGIONS << 1));

			static uint32_t const rgba_channels[] = { ARGBColor32::RChannel, ARGBColor32::GChannel,
				ARGBColor32::BChannel, ARGBColor32::AChannel };

			for (int i = 0; i < 4; ++ i)
			{
				int const ch = rgba_channels[i];
				for (uint32_t j = 0; j < num_end_pts; ++ j)
				{
					if (start_bit + rgba_prec[ch] > 128)
					{
						memset(argb, 0, 16 * sizeof(argb[0]));
						return;
					}

					c[j][ch] = ((ch != ARGBColor32::AChannel) || rgba_prec.a())
						? ReadBits(input, start_bit, rgba_prec[ch]) : 255;
				}
			}

			BOOST_ASSERT(mode_info_[mode].p_bits <= 6);
			for (uint32_t i = 0; i < mode_info_[mode].p_bits; ++ i)
			{
				if (start_bit > 127)
				{
					memset(argb, 0, 16 * sizeof(argb[0]));
					return;
				}

				p[i] = ReadBit(input, start_bit);
			}

			if (mode_info_[mode].p_bits)
			{
				for (uint32_t i = 0; i < num_end_pts; ++ i)
				{
					size_t pi = i * mode_info_[mode].p_bits / num_end_pts;
					for (uint8_t ch = 0; ch < BC7_NUM_CHANNELS; ++ ch)
					{
						if (rgba_prec[ch] != rgba_prec_with_p[ch])
						{
							c[i][ch] = (c[i][ch] << 1) | p[pi];
						}
					}
				}
			}

			for (uint32_t i = 0; i < num_end_pts; ++ i)
			{
				c[i] = this->Unquantize(c[i], rgba_prec_with_p);
			}

			std::array<uint8_t, 16> w1;
			std::array<uint8_t, 16> w2;

			for (uint32_t i = 0; i < 16; ++ i)
			{
				size_t num_bits = IsFixUpOffset(mode_info_[mode].partitions, shape, i)
					? index_prec_1 - 1 : index_prec_1;
				if (start_bit + num_bits > 128)
				{
					memset(argb, 0, 16 * sizeof(argb[0]));
					return;
				}
				w1[i] = ReadBits(input, start_bit, num_bits);
			}

			if (index_prec_2)
			{
				for (uint32_t i = 0; i < 16; ++ i)
				{
					size_t num_bits = i ? index_prec_2 : index_prec_2 - 1;
					if (start_bit + num_bits > 128)
					{
						memset(argb, 0, 16 * sizeof(argb[0]));
						return;
					}
					w2[i] = ReadBits(input, start_bit, num_bits);
				}
			}

			for (uint32_t i = 0; i < 16; ++ i)
			{
				size_t region = GetPartition(partitions, shape, i);
				ARGBColor32 out_pixel;
				if (0 == index_prec_2)
				{
					out_pixel = this->Interpolate(c[region << 1], c[(region << 1) + 1],
						w1[i], w1[i], index_prec_1, index_prec_1);
				}
				else if (0 == index_mode)
				{
					out_pixel = this->Interpolate(c[region << 1], c[(region << 1) + 1],
						w1[i], w2[i], index_prec_1, index_prec_2);
				}
				else
				{
					out_pixel = this->Interpolate(c[region << 1], c[(region << 1) + 1],
						w2[i], w1[i], index_prec_2, index_prec_1);
				}

				switch (rotation)
				{
				case 1:
					std::swap(out_pixel.r(), out_pixel.a());
					break;
				case 2:
					std::swap(out_pixel.g(), out_pixel.a());
					break;
				case 3:
					std::swap(out_pixel.b(), out_pixel.a());
					break;
				}

				argb[i] = out_pixel;
			}
		}
		else
		{
			memset(argb, 0, 16 * sizeof(argb[0]));
		}
	}

	void TexCompressionBC7::PackBC7UniformBlock(void* output, ARGBColor32 const & pixel)
	{
		size_t start_bit = 0;
		WriteBits(output, start_bit, 6, 1 << 5);
		WriteBits(output, start_bit, 2, 0);

		uint8_t const r = pixel.r();
		uint8_t const g = pixel.g();
		uint8_t const b = pixel.b();
		uint8_t const a = pixel.a();

		WriteBits(output, start_bit, 7, O_MATCH7[r][0]);
		WriteBits(output, start_bit, 7, O_MATCH7[r][1]);

		WriteBits(output, start_bit, 7, O_MATCH7[g][0]);
		WriteBits(output, start_bit, 7, O_MATCH7[g][1]);

		WriteBits(output, start_bit, 7, O_MATCH7[b][0]);
		WriteBits(output, start_bit, 7, O_MATCH7[b][1]);

		WriteBits(output, start_bit, 8, a);
		WriteBits(output, start_bit, 8, a);

		// Color indices are 1 for each pixel...
		// Anchor index is 0, so 1 bit for the first pixel, then
		// 01 for each following pixel giving the sequence of 31 bits:
		// ...010101011
		WriteBits(output, start_bit, 8, 0xAB);
		WriteBits(output, start_bit, 8, 0xAA);
		WriteBits(output, start_bit, 8, 0xAA);
		WriteBits(output, start_bit, 7, 0x2A);

		// Alpha indices...
		WriteBits(output, start_bit, 8, 0xAB);
		WriteBits(output, start_bit, 8, 0xAA);
		WriteBits(output, start_bit, 8, 0xAA);
		WriteBits(output, start_bit, 7, 0x2A);
	}

	void TexCompressionBC7::PackBC7Block(int mode, CompressParams& params, void* output)
	{
		ModeInfo const & mode_info = mode_info_[mode];
		int const partition_bits = mode_info.partition_bits;
		int const partitions = mode_info.partitions;

		size_t start_bit = 0;

		WriteBits(output, start_bit, mode + 1, 1 << mode);

		BOOST_ASSERT(!partition_bits || ((((1 << partition_bits) - 1) & params.shape_index) == params.shape_index));
		WriteBits(output, start_bit, partition_bits, static_cast<uint8_t>(params.shape_index));

		WriteBits(output, start_bit, mode_info.rotation_bits, params.rotation_mode);
		WriteBits(output, start_bit, mode_info.index_mode_bits, params.index_mode);

#ifdef KLAYGE_DEBUG
		for (int i = 0; i < 16; ++ i)
		{
			int set = 0;
			for (int j = 0; j < partitions; ++ j)
			{
				if (params.indices[j][i] < 255)
				{
					++ set;
				}
			}

			BOOST_ASSERT(1 == set);
		}
#endif

		ARGBColor32 const qmask = this->QuantizationMask(mode_info);

		ARGBColor32 pixel1[BC7_MAX_REGIONS];
		ARGBColor32 pixel2[BC7_MAX_REGIONS];
		for (int i = 0; i < partitions; ++ i)
		{
			switch (mode_info.p_bit_type)
			{
			case PBT_None:
				pixel1[i] = Quantize(params.p1[i], qmask);
				pixel2[i] = Quantize(params.p2[i], qmask);
				break;

			case PBT_Shared:
			case PBT_Unique:
				pixel1[i] = Quantize(params.p1[i], qmask, this->PBitCombo(mode_info, params.pbit_combo[i])[0]);
				pixel2[i] = Quantize(params.p2[i], qmask, this->PBitCombo(mode_info, params.pbit_combo[i])[1]);
				break;

			default:
				KFL_UNREACHABLE("Invalid p-bit type");
			}
		}

		// If the anchor index does not have 0 in the leading bit, then
		// we need to swap EVERYTHING.
		for (int par_index = 0; par_index < partitions; ++ par_index)
		{
			int anchor_index = AnchorIndexForSubset(par_index, params.shape_index, partitions);
			BOOST_ASSERT(params.indices[par_index][anchor_index] != 255);

			int const alpha_index_bits = this->NumBitsPerAlpha(mode_info, params.index_mode);
			int const index_bits = this->NumBitsPerIndex(mode_info, params.index_mode);
			if (params.indices[par_index][anchor_index] >> (index_bits - 1))
			{
				std::swap(pixel1[par_index], pixel2[par_index]);

				int index_vals = 1 << index_bits;
				for (int i = 0; i < 16; ++ i)
				{
					params.indices[par_index][i]
						= static_cast<uint8_t>((index_vals - 1) - params.indices[par_index][i]);
				}

				int alpha_index_vals = 1 << alpha_index_bits;
				if (mode_info.rotation_bits != 0)
				{
					for (int i = 0; i < 16; ++ i)
					{
						params.alpha_indices[i]
							= static_cast<uint8_t>((alpha_index_vals - 1) - params.alpha_indices[i]);
					}
				}
			}

			bool const rotated = (params.alpha_indices[anchor_index] >> (alpha_index_bits - 1)) > 0;
			if ((mode_info.rotation_bits != 0) && rotated)
			{
				std::swap(pixel1[par_index].a(), pixel2[par_index].a());

				int alpha_index_vals = 1 << alpha_index_bits;
				for (int i = 0; i < 16; ++ i)
				{
					params.alpha_indices[i] = static_cast<uint8_t>((alpha_index_vals - 1) - params.alpha_indices[i]);
				}
			}

			BOOST_ASSERT(!(params.indices[par_index][anchor_index] >> (index_bits - 1)));
			BOOST_ASSERT((0 == mode_info.rotation_bits)
				|| !(params.alpha_indices[anchor_index] >> (alpha_index_bits - 1)));
		}

		static uint32_t const rgba_channels[] = { ARGBColor32::RChannel, ARGBColor32::GChannel,
			ARGBColor32::BChannel, ARGBColor32::AChannel };
		for (int i = 0; i < 4; ++ i)
		{
			int const ch = rgba_channels[i];
			int const bits = mode_info.rgba_prec[ch];
			for (int j = 0; j < partitions; ++ j)
			{
				WriteBits(output, start_bit, bits, pixel1[j][ch] >> (8 - bits));
				WriteBits(output, start_bit, bits, pixel2[j][ch] >> (8 - bits));
			}
		}

		// Write out the best pbits..
		if (mode_info.p_bit_type != PBT_None)
		{
			for (int s = 0; s < partitions; ++ s)
			{
				int const * pbits = this->PBitCombo(mode_info, params.pbit_combo[s]);
				WriteBit(output, start_bit, static_cast<uint8_t>(pbits[0]));
				if (mode_info.p_bit_type != PBT_Shared)
				{
					WriteBit(output, start_bit,  static_cast<uint8_t>(pbits[1]));
				}
			}
		}

		// If our index mode has changed, then we need to write the alpha indices first.
		if ((mode_info.index_mode_bits != 0) && (1 == params.index_mode))
		{
			BOOST_ASSERT(mode_info.rotation_bits != 0);

			for (int i = 0; i < 16; ++ i)
			{
				int const idx = params.alpha_indices[i];
				BOOST_ASSERT(0 == AnchorIndexForSubset(0, params.shape_index, partitions));
				BOOST_ASSERT(2 == this->NumBitsPerAlpha(mode_info, params.index_mode));
				BOOST_ASSERT((idx >= 0) && (idx < (1 << 2)));
				BOOST_ASSERT_MSG((i != 0) || !(idx >> 1), "Leading bit of anchor index is not zero!");
				WriteBits(output, start_bit, (0 == i) ? 1 : 2, static_cast<uint8_t>(idx));
			}

			for (int i = 0; i < 16; ++ i)
			{
				int const idx = params.indices[0][i];
				BOOST_ASSERT(0 == GetPartition(partitions, params.shape_index, i));
				BOOST_ASSERT(0 == AnchorIndexForSubset(0, params.shape_index, partitions));
				BOOST_ASSERT(this->NumBitsPerIndex(mode_info, params.index_mode) == 3);
				BOOST_ASSERT((idx >= 0) && (idx < (1 << 3)));
				BOOST_ASSERT_MSG((i != 0) || !(idx >> 2), "Leading bit of anchor index is not zero!");
				WriteBits(output, start_bit, (0 == i) ? 2 : 3, static_cast<uint8_t>(idx));
			}
		}
		else
		{
			for (int i = 0; i < 16; ++ i)
			{
				int const subs = GetPartition(partitions, params.shape_index, i);
				int const idx = params.indices[subs][i];
				int const anchor_idx = AnchorIndexForSubset(subs, params.shape_index, partitions);
				int const bits_for_idx = this->NumBitsPerIndex(mode_info, params.index_mode);
				BOOST_ASSERT((idx >= 0) && (idx < (1 << bits_for_idx)));
				BOOST_ASSERT_MSG((i != anchor_idx) || !(idx >> (bits_for_idx - 1)),
					"Leading bit of anchor index is not zero!");
				WriteBits(output, start_bit, (i == anchor_idx) ? bits_for_idx - 1 : bits_for_idx,
					static_cast<uint8_t>(idx));
			}

			if (mode_info.rotation_bits != 0)
			{
				for (int i = 0; i < 16; ++ i)
				{
					int const idx = params.alpha_indices[i];
					int const anchor_idx = 0;
					int const bits_for_idx = this->NumBitsPerAlpha(mode_info, params.index_mode);
					BOOST_ASSERT((idx >= 0) && (idx < (1 << bits_for_idx)));
					BOOST_ASSERT_MSG((i != anchor_idx) || (!(idx >> (bits_for_idx - 1))),
						"Leading bit of anchor index is not zero!");
					WriteBits(output, start_bit, (i == anchor_idx) ? bits_for_idx - 1 : bits_for_idx,
						static_cast<uint8_t>(idx));
				}
			}
		}
	}

	int TexCompressionBC7::RotationMode(ModeInfo const & mode_info) const
	{
		return mode_info.rotation_bits ? rotate_mode_ : 0;
	}
	
	int TexCompressionBC7::NumBitsPerIndex(ModeInfo const & mode_info, int8_t index_mode) const
	{
		if (index_mode < 0)
		{
			index_mode = static_cast<uint8_t>(index_mode_);
		}
		if (0 == index_mode)
		{
			return mode_info.index_prec_1;
		}
		else
		{
			return mode_info.index_prec_2;
		}
	}

	int TexCompressionBC7::NumBitsPerAlpha(ModeInfo const & mode_info, int8_t index_mode) const
	{
		if (index_mode < 0)
		{
			index_mode = static_cast<uint8_t>(index_mode_);
		}
		if (0 == index_mode)
		{
			return mode_info.index_prec_2;
		}
		else
		{
			return mode_info.index_prec_1;
		}
	}

	// This returns the proper error metric even if we have rotation bits set
	uint4 TexCompressionBC7::ErrorMetric(ModeInfo const & mode_info) const
	{
		uint4 w = ERROR_METRICS[error_metric_];
		Rotation(w, this->RotationMode(mode_info));
		return w;
	}

	// This function creates an integer that represents the maximum values in each
	// channel. We can use this to figure out the proper endpoint values for a
	// given mode.
	ARGBColor32 TexCompressionBC7::QuantizationMask(ModeInfo const & mode_info) const
	{
		int const mask_seed = 0xFFFFFF80;
		uint32_t const alpha_prec = mode_info.rgba_prec.a();
		uint32_t const cbits = mode_info.rgba_prec.r() - 1;
		uint32_t const abits = mode_info.rgba_prec.a() - 1;
		return ARGBColor32(alpha_prec > 0 ? (mask_seed >> abits) & 0xFF : 0, (mask_seed >> cbits) & 0xFF,
			(mask_seed >> cbits) & 0xFF, (mask_seed >> cbits) & 0xFF);
	}

	int TexCompressionBC7::NumPbitCombos(ModeInfo const & mode_info) const
	{
		switch (mode_info.p_bit_type)
		{
		case PBT_None:
			return 1;
		case PBT_Shared:
			return 2;
		case PBT_Unique:
			return 4;

		default:
			KFL_UNREACHABLE("Invalid p-bit type");
		}
	}

	int const * TexCompressionBC7::PBitCombo(ModeInfo const & mode_info, int idx) const
	{
		static int const pbits[4][2] =
		{
			{ 0, 0 },
			{ 0, 1 },
			{ 1, 0 },
			{ 1, 1 }
		};

		switch (mode_info.p_bit_type)
		{
		case PBT_None:
			return pbits[0];
		case PBT_Shared:
			return idx ? pbits[3] : pbits[0];
		case PBT_Unique:
			return pbits[idx % 4];

		default:
			KFL_UNREACHABLE("Invalid p-bit type");
		}
	}

	// This performs simulated annealing on the endpoints p1 and p2 based on the
	// current MaxAnnealingIterations. This is set by calling the function
	// SetQualityLevel
	uint64_t TexCompressionBC7::OptimizeEndpointsForCluster(int mode, RGBACluster const & cluster,
			float4& p1, float4& p2, uint8_t* best_indices, uint8_t& best_pbit_combo) const
	{
		ModeInfo const & mode_info = mode_info_[mode];
		const uint32_t buckets = (1 << this->NumBitsPerIndex(mode_info));
		ARGBColor32 const qmask = this->QuantizationMask(mode_info);

		// Here we use simulated annealing to traverse the space of clusters to find
		// the best possible endpoints.
		uint64_t cur_err = QuantizedError(cluster, p1, p2, buckets, qmask, this->ErrorMetric(mode_info),
			this->PBitCombo(mode_info, best_pbit_combo), best_indices);

		int cur_pbit_combo = best_pbit_combo;
		uint64_t best_err = cur_err;

		// Clamp endpoints to the grid...
		ARGBColor32 qp1, qp2;
		if (mode_info.p_bit_type != PBT_None)
		{
			qp1 = Quantize(p1, qmask, this->PBitCombo(mode_info, best_pbit_combo)[0]);
			qp2 = Quantize(p2, qmask, this->PBitCombo(mode_info, best_pbit_combo)[1]);
		}
		else
		{
			qp1 = Quantize(p1, qmask);
			qp2 = Quantize(p2, qmask);
		}

		p1 = FromARGBColor32(qp1);
		p2 = FromARGBColor32(qp2);

		float4 bp1 = p1, bp2 = p2;

		int const max_energy = sa_steps_;
		for (int energy = 0; (best_err > 0) && (energy < max_energy); ++ energy)
		{
			float temp = (energy + 0.5f) / static_cast<float>(max_energy - 0.5f);

			uint8_t indices[BC67_MAX_NUM_DATA_POINTS];
			float4 np1, np2;
			int pbit_combo = 0;

			this->PickBestNeighboringEndpoints(mode, p1, p2, cur_pbit_combo,
				np1, np2, pbit_combo);

			uint64_t error = QuantizedError(cluster, np1, np2, buckets, qmask,
				this->ErrorMetric(mode_info), this->PBitCombo(mode_info, pbit_combo), indices);

			if (this->AcceptNewEndpointError(error, cur_err, temp))
			{
				cur_err = error;
				p1 = np1;
				p2 = np2;
				cur_pbit_combo = pbit_combo;
			}

			if (error < best_err)
			{
				memcpy(best_indices, indices, sizeof(indices));
				bp1 = np1;
				bp2 = np2;
				best_pbit_combo = static_cast<uint8_t>(pbit_combo);
				best_err = error;

				// Restart...
				energy = 0;
			}
		}

		p1 = bp1;
		p2 = bp2;

		return best_err;
	}

	// This function performs the heuristic to choose the "best" neighboring
	// endpoints to p1 and p2 based on the compression mode (index precision,
	// endpoint precision etc)
	void TexCompressionBC7::PickBestNeighboringEndpoints(int mode, float4 const & p1, float4 const & p2,
			int cur_pbit_combo, float4& np1, float4& np2, int& pbit_combo, float step_sz) const
	{
		ModeInfo const & mode_info = mode_info_[mode];

		// !SPEED! There might be a way to make this faster since we're working
		// with floating point values that are powers of two. We should be able
		// to just set the proper bits in the exponent and leave the mantissa to 0.
		float4 step = step_sz * float4(static_cast<float>(1 << (8 - mode_info.rgba_prec[ARGBColor32::RChannel])),
			static_cast<float>(1 << (8 - mode_info.rgba_prec[ARGBColor32::GChannel])),
			static_cast<float>(1 << (8 - mode_info.rgba_prec[ARGBColor32::BChannel])),
			static_cast<float>(1 << (8 - mode_info.rgba_prec[ARGBColor32::AChannel])));
		if (mode < 4)
		{
			step[(this->RotationMode(mode_info) + 3) & 0x3] = 0;
		}

		// First, let's figure out the new pbit combo... if there's no pbit then we
		// don't need to worry about it.
		bool const has_pbits = (mode_info.p_bit_type != PBT_None);
		if (has_pbits)
		{
			// If there is a pbit, then we must change it, because those will provide
			// the closest values to the current point.
			if (PBT_Shared == mode_info.p_bit_type)
			{
				pbit_combo = (cur_pbit_combo + 1) % 2;
			}
			else
			{
				// Not shared... p1 needs to change and p2 needs to change... which means
				// that combo 0 gets rotated to combo 3, combo 1 gets rotated to combo 2
				// and vice versa...
				pbit_combo = 3 - cur_pbit_combo;
			}

			BOOST_ASSERT(1 == this->PBitCombo(mode_info, cur_pbit_combo)[0]
				+ this->PBitCombo(mode_info, pbit_combo)[0]);
			BOOST_ASSERT(1 == this->PBitCombo(mode_info, cur_pbit_combo)[1]
				+ this->PBitCombo(mode_info, pbit_combo)[1]);
		}

		for (int pt = 0; pt < 2; ++ pt)
		{
			float4 const & p = pt ? p1 : p2;
			float4& np = pt ? np1 : np2;
			uint32_t const rdir = rand() & 0xF;

			np = p;
			if (has_pbits)
			{
				uint32_t const pbit = this->PBitCombo(mode_info, cur_pbit_combo)[pt];
				ChangePointForDirWithPbitChange(np, rdir, pbit, step);
			}
			else
			{
				ChangePointForDirWithoutPbitChange(np, rdir, step);
			}

			for (uint32_t i = 0; i < np.size(); ++ i)
			{
				np[i] = MathLib::clamp(np[i], 0.0f, 255.0f);
			}
		}
	}

	// This is used by simulated annealing to determine whether or not the
	// newError (from the neighboring endpoints) is sufficient to continue the
	// annealing process from these new endpoints based on how good the oldError
	// was, and how long we've been annealing (t)
	bool TexCompressionBC7::AcceptNewEndpointError(uint64_t new_err, uint64_t old_err, float temp) const
	{
		// Always accept better endpoints.
		if (new_err < old_err)
		{
			return true;
		}

		size_t const p = static_cast<size_t>(exp(0.1f * static_cast<int64_t>(old_err - new_err) / temp) * RAND_MAX);
		size_t const r = rand();

		return r < p;
	}

	// This function figures out the best compression for the single color p, and
	// places the endpoints in p1 and p2. If the compression mode supports p-bits,
	// then we choose the best p-bit combo and return it as well.
	uint64_t TexCompressionBC7::CompressSingleColor(ModeInfo const & mode_info, ARGBColor32 const & pixel,
			float4& p1, float4& p2, uint8_t& best_pbit_combo) const
	{
		uint64_t best_err = std::numeric_limits<uint64_t>::max();

		for (int pbi = 0; pbi < this->NumPbitCombos(mode_info); ++ pbi)
		{
			int const * pbit_combo = this->PBitCombo(mode_info, pbi);

			uint4 dist(0, 0, 0, 0);
			uint4 best_val_i(0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF);
			uint4 best_val_j = best_val_i;
			static uint32_t const rgba_channels[] = { ARGBColor32::RChannel, ARGBColor32::GChannel,
				ARGBColor32::BChannel, ARGBColor32::AChannel };

			for (uint32_t ci = 0; ci < dist.size(); ++ ci)
			{
				int const ch = rgba_channels[ci];
				uint8_t const val = pixel[ch];
				int bits = mode_info.rgba_prec[ch];

				// If we don't handle this channel, then it must be the full value (alpha)
				if (0 == bits)
				{
					best_val_i[ci] = best_val_j[ci] = 0xFF;
					dist[ci] = std::max(dist[ci], static_cast<uint32_t>(0xFF - val));
					continue;
				}

				int const poss_vals = (1 << bits);
				int poss_vals_h[256];
				int poss_vals_l[256];

				bool const have_pbit = (mode_info.p_bit_type != PBT_None);
				if (have_pbit)
				{
					++ bits;
				}

				for (int i = 0; i < poss_vals; ++ i)
				{
					int vh = i, vl = i;
					if (have_pbit)
					{
						vh <<= 1;
						vl <<= 1;

						vh |= pbit_combo[1];
						vl |= pbit_combo[0];
					}

					poss_vals_h[i] = (vh << (8 - bits));
					poss_vals_h[i] |= (poss_vals_h[i] >> bits);

					poss_vals_l[i] = (vl << (8 - bits));
					poss_vals_l[i] |= (poss_vals_l[i] >> bits);
				}

				uint8_t const bpi = static_cast<uint8_t>(this->NumBitsPerIndex(mode_info) - 1);
				uint32_t const interp_val_0 = BC67_INTERPOLATION_VALUES[bpi][1].first;
				uint32_t const interp_val_1 = BC67_INTERPOLATION_VALUES[bpi][1].second;

				// Find the closest interpolated val that to the given val...
				uint32_t best_channel_dist = 0xFF;
				for (int i = 0; (best_channel_dist > 0) && (i < poss_vals); ++ i)
				{
					for (int j = 0; (best_channel_dist > 0) && (j < poss_vals); ++ j)
					{
						uint32_t const v1 = poss_vals_l[i];
						uint32_t const v2 = poss_vals_h[j];

						uint32_t const combo = (interp_val_0 * v1 + interp_val_1 * v2 + 32) >> 6;
						uint32_t const err = (combo > val) ? combo - val : val - combo;

						if (err < best_channel_dist)
						{
							best_channel_dist = err;
							best_val_i[ci] = v1;
							best_val_j[ci] = v2;
						}
					}
				}

				dist[ci] = std::max(best_channel_dist, dist[ci]);
			}

			uint4 const & error_weights = ERROR_METRICS[error_metric_];
			uint64_t error = 0;
			for (uint32_t i = 0; i < dist.size(); ++ i)
			{
				uint32_t e = dist[i] * error_weights[i];
				error += e * e;
			}

			if (error < best_err)
			{
				best_err = error;
				best_pbit_combo = static_cast<uint8_t>(pbi);

				for (uint32_t ci = 0; ci < p1.size(); ++ci)
				{
					p1[ci] = static_cast<float>(best_val_i[ci]);
					p2[ci] = static_cast<float>(best_val_j[ci]);
				}
			}
		}

		return best_err;
	}

	// Compress the cluster using a generalized cluster fit. This figures out the
	// proper endpoints assuming that we have no alpha.
	uint64_t TexCompressionBC7::CompressCluster(int mode, RGBACluster const & cluster,
		float4& p1, float4& p2, uint8_t* best_indices, uint8_t& best_pbit_combo) const
	{
		ModeInfo const & mode_info = mode_info_[mode];

		// If all the points are the same in the cluster, then we need to figure out
		// what the best approximation to this point is....
		if (cluster.AllSamePoint())
		{
			ARGBColor32 const & p = cluster.Pixel(0);
			uint64_t best_err = this->CompressSingleColor(mode_info, p, p1, p2, best_pbit_combo);

			// We're assuming all indices will be index 1...
			for (uint32_t i = 0; i < cluster.NumValidPoints(); ++ i)
			{
				best_indices[i] = 1;
			}
			return cluster.NumValidPoints() * best_err;
		}

		uint32_t const buckets = (1 << this->NumBitsPerIndex(mode_info));

#if 1
		float4 axis;
		cluster.PrincipalAxis(axis, nullptr, nullptr);

		float min_dp = std::numeric_limits<float>::max();
		float max_dp = -std::numeric_limits<float>::max();
		for (uint32_t i = 0; i < cluster.NumValidPoints(); ++ i)
		{
			float dp = MathLib::dot(cluster.Point(i) - cluster.Avg(), axis);
			if (dp < min_dp)
			{
				min_dp = dp;
			}
			if (dp > max_dp)
			{
				max_dp = dp;
			}
		}

		p1 = cluster.Avg() + min_dp * axis;
		p2 = cluster.Avg() + max_dp * axis;
#else
		cluster.BoundingBox(p1, p2);
#endif

		this->ClampEndpoints(p1, p2);

		float4 pts[1 << 4];  // At most 4 bits per index.
		uint32_t num_pts[1 << 4];
		BOOST_ASSERT(buckets <= 1 << 4);

		for (uint32_t i = 0; i < buckets; ++ i)
		{
			float s = i / static_cast<float>(buckets - 1);
			pts[i] = MathLib::lerp(p1, p2, s);
		}

		// Do k-means clustering...
		uint32_t bucket_idx[BC67_MAX_NUM_DATA_POINTS] = { 0 };

		bool fixed = false;
		while (!fixed)
		{
			float4 new_pts[1 << 4];

			// Assign each of the existing points to one of the buckets...
			for (uint32_t i = 0; i < cluster.NumValidPoints(); ++ i)
			{
				int min_bucket = -1;
				float min_dist = std::numeric_limits<float>::max();

				for (uint32_t j = 0; j < buckets; ++ j)
				{
					float4 v = cluster.Point(i) - pts[j];
					float dist_sq = MathLib::dot(v, v);
					if (dist_sq < min_dist)
					{
						min_dist = dist_sq;
						min_bucket = j;
					}
				}

				BOOST_ASSERT(min_bucket >= 0);
				bucket_idx[i] = min_bucket;
			}

			// Calculate new buckets based on centroids of clusters...
			for (uint32_t i = 0; i < buckets; ++ i)
			{
				num_pts[i] = 0;
				new_pts[i] = float4(0, 0, 0, 0);
				for (uint32_t j = 0; j < cluster.NumValidPoints(); ++ j)
				{
					if (bucket_idx[j] == i)
					{
						++ num_pts[i];
						new_pts[i] = new_pts[i] + cluster.Point(j);
					}
				}

				// If there are no points in this cluster, then it should
				// remain the same as last time and avoid a divide by zero.
				if (0 != num_pts[i])
				{
					new_pts[i] /= static_cast<float>(num_pts[i]);
				}
			}

			// If we haven't changed, then we're done.
			fixed = true;
			for (uint32_t i = 0; i < buckets; ++ i)
			{
				if (pts[i] != new_pts[i])
				{
					fixed = false;
					break;
				}
			}

			// Assign the new points to be the old points.
			for (uint32_t i = 0; i < buckets; ++ i)
			{
				pts[i] = new_pts[i];
			}
		}

		// If there's only one bucket filled, then just compress for that single color
		int num_buckets_filled = 0;
		int last_filled_bucket = -1;
		for (uint32_t i = 0; i < buckets; ++ i)
		{
			if (num_pts[i] > 0)
			{
				num_buckets_filled++;
				last_filled_bucket = i;
			}
		}

		BOOST_ASSERT(num_buckets_filled > 0);
		if (1 == num_buckets_filled)
		{
			ARGBColor32 const p = Quantize(pts[last_filled_bucket]);
			uint64_t best_err = this->CompressSingleColor(mode_info, p, p1, p2, best_pbit_combo);

			// We're assuming all indices will be index 1...
			for (uint32_t i = 0; i < cluster.NumValidPoints(); ++ i)
			{
				best_indices[i] = 1;
			}
			return cluster.NumValidPoints() * best_err;
		}

		// Now that we know the index of each pixel, we can assign the endpoints based
		// on a least squares fit of the clusters. For more information, take a look
		// at this article by NVidia: http://developer.download.nvidia.com/compute/
		// cuda/1.1-Beta/x86_website/projects/dxtc/doc/cuda_dxtc.pdf
		float asq = 0, bsq = 0, ab = 0;
		float4 ax(0, 0, 0, 0), bx(0, 0, 0, 0);
		for (uint32_t i = 0; i < buckets; ++ i)
		{
			float4 const x = pts[i];
			int const n = num_pts[i];

			float const fbi = static_cast<float>(buckets - 1 - i);
			float const fb = static_cast<float>(buckets - 1);
			float const fi = static_cast<float>(i);
			float const fn = static_cast<float>(n);

			float const a = fbi / fb;
			float const b = fi / fb;

			asq += fn * a * a;
			bsq += fn * b * b;
			ab += fn * a * b;

			ax += x * a * fn;
			bx += x * b * fn;
		}

		float f = 1 / (asq * bsq - ab * ab);
		p1 = f * (ax * bsq - bx * ab);
		p2 = f * (bx * asq - ax * ab);

		this->ClampEndpointsToGrid(mode_info, p1, p2, best_pbit_combo);

#ifdef KLAYGE_DEBUG
		uint8_t pbit_combo = best_pbit_combo;
		float4 tp1 = p1;
		float4 tp2 = p2;
		this->ClampEndpointsToGrid(mode_info, tp1, tp2, pbit_combo);

		BOOST_ASSERT(p1 == tp1);
		BOOST_ASSERT(p2 == tp2);
		BOOST_ASSERT(pbit_combo == best_pbit_combo);
#endif

		return this->OptimizeEndpointsForCluster(mode, cluster,
			p1, p2, best_indices, best_pbit_combo);
	}

	// Compress the non-opaque cluster using a generalized cluster fit, and place
	// the endpoints within p1 and p2. The color indices and alpha indices are
	// computed as well.
	uint64_t TexCompressionBC7::CompressCluster(int mode, RGBACluster const & cluster,
			float4& p1, float4& p2, uint8_t* best_indices, uint8_t* alpha_indices) const
	{
		BOOST_ASSERT((4 == mode) || (5 == mode));
		BOOST_ASSERT(1 == mode_info_[mode].partitions);
		BOOST_ASSERT(BC67_MAX_NUM_DATA_POINTS == cluster.NumValidPoints());
		BOOST_ASSERT(mode_info_[mode].rgba_prec.a() > 0);

		// If all the points are the same in the cluster, then we need to figure out
		// what the best approximation to this point is....
		BOOST_ASSERT_MSG(!cluster.AllSamePoint(), "We should only be using this function in modes 4 & 5 that have a"
			"single subset, in which case single colors should have been"
			"detected much earlier.");

		ModeInfo const & mode_info = mode_info_[mode];

		RGBACluster rgb_cluster(cluster);
		float alpha_vals[BC67_MAX_NUM_DATA_POINTS] = { 0 };

		float alpha_min = std::numeric_limits<float>::max();
		float alpha_max = -std::numeric_limits<float>::max();
		for (uint32_t i = 0; i < rgb_cluster.NumValidPoints(); ++ i)
		{
			float4& v = rgb_cluster.Point(i);
			Rotation(v, this->RotationMode(mode_info));

			alpha_vals[i] = v.w();
			v.w() = 255.0f;

			alpha_min = std::min(alpha_min, alpha_vals[i]);
			alpha_max = std::max(alpha_max, alpha_vals[i]);
		}

		uint8_t dummy_pbit = 0;
		float4 rgb_p1, rgb_p2;
		uint64_t rgb_err = this->CompressCluster(mode, rgb_cluster,
			rgb_p1, rgb_p2, best_indices, dummy_pbit);

		float a1 = alpha_min;
		float a2 = alpha_max;
		uint64_t alpha_err = std::numeric_limits<uint64_t>::max();

		std::pair<uint32_t, uint32_t> const * interp_vals
			= BC67_INTERPOLATION_VALUES[this->NumBitsPerAlpha(mode_info) - 1];

		uint32_t const weight = this->ErrorMetric(mode_info).w();

		uint32_t const num_buckets = (1 << this->NumBitsPerAlpha(mode_info));

		// If they're the same, then we can get them exactly.
		if (a1 == a2)
		{
			uint8_t const a_be = static_cast<uint8_t>(a1);

			// Mode 5 has 8 bits of precision for alpha.
			if (5 == mode)
			{
				for (uint32_t i = 0; i < BC67_MAX_NUM_DATA_POINTS; ++ i)
				{
					alpha_indices[i] = 0;
				}

				alpha_err = 0;
			}
			else
			{
				BOOST_ASSERT(4 == mode);

				// Mode 4 can be treated like the 6 channel of DXT1 compression.
				uint32_t a1i = Extend6To8Bits(O_MATCH6[a_be][1]);
				uint32_t a2i = Extend6To8Bits(O_MATCH6[a_be][0]);

				if (1 == index_mode_)
				{
					for (uint32_t i = 0; i < BC67_MAX_NUM_DATA_POINTS; ++ i)
					{
						alpha_indices[i] = 1;
					}
				}
				else
				{
					for (uint32_t i = 0; i < BC67_MAX_NUM_DATA_POINTS; ++ i)
					{
						alpha_indices[i] = 2;
					}
				}

				uint32_t interp_0 = interp_vals[alpha_indices[0] & 0xFF].first;
				uint32_t interp_1 = interp_vals[alpha_indices[0] & 0xFF].second;

				uint8_t const ip = (((a1i * interp_0) + (a2i * interp_1) + 32) >> 6) & 0xFF;
				uint64_t px_err = weight * abs(a_be - ip);
				px_err *= px_err;
				alpha_err = 16 * px_err;

				a1 = static_cast<float>(a1i);
				a2 = static_cast<float>(a2i);
			}
		}
		else
		{
			// (a1 != a2)
			float vals[1 << 3];
			memset(vals, 0, sizeof(vals));

			uint32_t buckets[BC67_MAX_NUM_DATA_POINTS];

			// Figure out initial positioning.
			for (uint32_t i = 0; i < num_buckets; ++ i)
			{
				float const fi = static_cast<float>(i);
				float const fb = static_cast<float>(num_buckets - 1);
				vals[i] = alpha_min + (fi / fb) * (alpha_max - alpha_min);
			}

			// Assign each value to a bucket
			for (uint32_t i = 0; i < BC67_MAX_NUM_DATA_POINTS; ++ i)
			{
				float min_dist = 255;
				buckets[i] = num_buckets;
				for (uint32_t j = 0; j < num_buckets; ++ j)
				{
					float dist = std::abs(alpha_vals[i] - vals[j]);
					if (dist < min_dist)
					{
						min_dist = dist;
						buckets[i] = j;
					}
				}
			}

			float npts[1 << 3];

			// Do k-means
			bool fixed = false;
			while (!fixed)
			{
				memset(npts, 0, sizeof(npts));

				float avg[1 << 3];
				memset(avg, 0, sizeof(avg));

				// Calculate average of each cluster
				for (uint32_t i = 0; i < num_buckets; ++ i)
				{
					for (uint32_t j = 0; j < BC67_MAX_NUM_DATA_POINTS; ++ j)
					{
						if (buckets[j] == i)
						{
							avg[i] += alpha_vals[j];
							npts[i] += 1;
						}
					}

					if (npts[i] > 0)
					{
						avg[i] /= npts[i];
					}
				}

				// Did we change anything?
				fixed = true;
				for (uint32_t i = 0; i < num_buckets; ++ i)
				{
					fixed = fixed && (avg[i] == vals[i]);
					if (!fixed)
					{
						break;
					}
				}

				// Reassign indices...
				memcpy(vals, avg, sizeof(vals));

				// Reassign each value to a bucket
				for (uint32_t i = 0; i < BC67_MAX_NUM_DATA_POINTS; ++ i)
				{
					float min_dist = 255.0f;
					for (uint32_t j = 0; j < num_buckets; ++ j)
					{
						float dist = std::abs(alpha_vals[i] - vals[j]);
						if (dist < min_dist)
						{
							min_dist = dist;
							buckets[i] = j;
						}
					}
				}
			}

			// Do least squares fit of vals.
			float asq = 0, bsq = 0, ab = 0;
			float ax = 0, bx = 0;
			for (uint32_t i = 0; i < num_buckets; ++ i)
			{
				float const fbi = static_cast<float>(num_buckets - 1 - i);
				float const fb = static_cast<float>(num_buckets - 1);
				float const fi = static_cast<float>(i);

				float a = fbi / fb;
				float b = fi / fb;

				float n = npts[i];
				float x = vals[i];

				asq += n * a * a;
				bsq += n * b * b;
				ab += n * a * b;

				ax += x * a * n;
				bx += x * b * n;
			}

			float f = 1.0f / (asq * bsq - ab * ab);
			a1 = f * (ax * bsq - bx * ab);
			a2 = f * (bx * asq - ax * ab);

			a1 = MathLib::clamp(a1, 0.0f, 255.0f);
			a2 = MathLib::clamp(a2, 0.0f, 255.0f);

			int8_t const maskSeed = -0x7F;
			uint8_t const a1b = QuantizeChannel(static_cast<uint8_t>(a1), (maskSeed >> (mode_info.rgba_prec.a() - 1)));
			uint8_t const a2b = QuantizeChannel(static_cast<uint8_t>(a2), (maskSeed >> (mode_info.rgba_prec.a() - 1)));

			// Compute error
			alpha_err = 0;
			for (uint32_t i = 0; i < BC67_MAX_NUM_DATA_POINTS; ++ i)
			{
				uint8_t val = static_cast<uint8_t>(alpha_vals[i]);

				uint64_t min_err = std::numeric_limits<uint64_t>::max();
				int best_bucket = -1;

				for (uint32_t j = 0; j < num_buckets; ++ j)
				{
					uint32_t interp_0 = interp_vals[j].first;
					uint32_t interp_1 = interp_vals[j].second;

					uint8_t const ip = (((a1b * interp_0) + (a2b * interp_1) + 32) >> 6) & 0xFF;
					uint64_t px_err = weight * abs(val - ip);
					px_err *= px_err;

					if (px_err < min_err)
					{
						min_err = px_err;
						best_bucket = j;
					}
				}

				alpha_err += min_err;
				alpha_indices[i] = static_cast<uint8_t>(best_bucket);
			}
		}

		for (uint32_t i = 0; i < p1.size(); ++ i)
		{
			p1[i] = (i == (p1.size() - 1)) ? a1 : rgb_p1[i];
			p2[i] = (i == (p2.size() - 1)) ? a2 : rgb_p2[i];
		}

		return rgb_err + alpha_err;
	}

	void TexCompressionBC7::ClampEndpoints(float4& p1, float4& p2) const
	{
		for (uint32_t i = 0; i < 4; ++ i)
		{
			MathLib::clamp(p1[i], 0.0f, 255.0f);
			MathLib::clamp(p2[i], 0.0f, 255.0f);
		}
	}

	// This function takes two endpoints in the continuous domain (as floats) and
	// clamps them to the nearest grid points based on the compression mode (and
	// possible pbit values)
	void TexCompressionBC7::ClampEndpointsToGrid(ModeInfo const & mode_info,
			float4& p1, float4& p2, uint8_t& best_pbit_combo) const
	{
		int const pbit_combos = this->NumPbitCombos(mode_info);
		bool const has_pbits = pbit_combos > 1;
		ARGBColor32 const qmask = this->QuantizationMask(mode_info);

		this->ClampEndpoints(p1, p2);

		// !SPEED! This can be faster.
		float min_dist = std::numeric_limits<float>::max();
		float4 bp1, bp2;
		for (int i = 0; i < pbit_combos; ++ i)
		{
			ARGBColor32 qp1, qp2;
			if (has_pbits)
			{
				qp1 = Quantize(p1, qmask, this->PBitCombo(mode_info, i)[0]);
				qp2 = Quantize(p2, qmask, this->PBitCombo(mode_info, i)[1]);
			}
			else
			{
				qp1 = Quantize(p1, qmask);
				qp2 = Quantize(p2, qmask);
			}

			float4 np1 = FromARGBColor32(qp1);
			float4 np2 = FromARGBColor32(qp2);

			float4 d1 = np1 - p1;
			float4 d2 = np2 - p2;
			float dist = MathLib::dot(d1, d1) + MathLib::dot(d2, d2);
			if (dist < min_dist)
			{
				min_dist = dist;
				bp1 = np1;
				bp2 = np2;
				best_pbit_combo = static_cast<uint8_t>(i);
			}
		}

		p1 = bp1;
		p2 = bp2;
	}

	uint64_t TexCompressionBC7::TryCompress(int mode, int simulated_annealing_steps, TexCompressionErrorMetric metric,
			CompressParams& params, uint32_t shape_index, RGBACluster& cluster)
	{
		sa_steps_ = simulated_annealing_steps;
		error_metric_ = metric;
		rotate_mode_ = 0;
		index_mode_ = 0;

		ModeInfo const & mode_info = mode_info_[mode];
		int const partitions = mode_info.partitions;

		params = CompressParams(shape_index);

		uint64_t total_err = 0;
		for (int cidx = 0; cidx < partitions; ++ cidx)
		{
			uint8_t indices[BC67_MAX_NUM_DATA_POINTS] = { 0 };
			cluster.Partition(cidx);

			if (mode_info.rotation_bits != 0)
			{
				BOOST_ASSERT(1 == partitions);

				uint8_t alpha_indices[BC67_MAX_NUM_DATA_POINTS];

				uint64_t best_err = std::numeric_limits<uint64_t>::max();
				for (int rot_mode = 0; rot_mode < 4; ++ rot_mode)
				{
					rotate_mode_ = rot_mode;

					int const idx_modes = (4 == mode) ? 2 : 1;
					for (int idx_mode = 0; idx_mode < idx_modes; ++ idx_mode)
					{
						index_mode_ = idx_mode;

						float4 v1, v2;
						uint64_t error = this->CompressCluster(mode, cluster, v1, v2, indices, alpha_indices);

						if (error < best_err)
						{
							best_err = error;

							memcpy(params.indices[cidx], indices, sizeof(indices));
							memcpy(params.alpha_indices, alpha_indices, sizeof(alpha_indices));

							params.rotation_mode = static_cast<int8_t>(rot_mode);
							params.index_mode = static_cast<int8_t>(idx_mode);

							params.p1[cidx] = v1;
							params.p2[cidx] = v2;
						}
					}
				}

				total_err += best_err;
			}
			else
			{
				total_err += this->CompressCluster(mode, cluster, params.p1[cidx], params.p2[cidx],
					indices, params.pbit_combo[cidx]);

				// Map the indices to their proper position.
				int idx = 0;
				for (int i = 0; i < 16; ++ i)
				{
					int subs = GetPartition(mode_info.partitions, shape_index, i);
					if (subs == cidx)
					{
						params.indices[cidx][i] = indices[idx];
						++ idx;
					}
				}
			}
		}

		return total_err;
	}

	uint8_t TexCompressionBC7::Unquantize(uint8_t comp, size_t prec) const
	{
		BOOST_ASSERT((0 < prec) && (prec <= 8));
		comp = comp << (8 - prec);
		return comp | (comp >> prec);
	}

	ARGBColor32 TexCompressionBC7::Unquantize(ARGBColor32 const & c, ARGBColor32 const & rgba_prec) const
	{
		ARGBColor32 q;
		q.r() = this->Unquantize(c.r(), rgba_prec.r());
		q.g() = this->Unquantize(c.g(), rgba_prec.g());
		q.b() = this->Unquantize(c.b(), rgba_prec.b());
		q.a() = rgba_prec.a() ? this->Unquantize(c.a(), rgba_prec.a()) : 255;
		return q;
	}

	ARGBColor32 TexCompressionBC7::Interpolate(ARGBColor32 const & c0, ARGBColor32 const & c1, size_t wc, size_t wa,
			size_t wc_prec, size_t wa_prec) const
	{
		BOOST_ASSERT((wc_prec >= 2) && (wc_prec <= 4));
		BOOST_ASSERT((wa_prec >= 2) && (wa_prec <= 4));
		BOOST_ASSERT(wc < (static_cast<size_t>(1) << wc_prec));
		BOOST_ASSERT(wa < (static_cast<size_t>(1) << wa_prec));

		ARGBColor32 out;

		int const * weights = BC67_PREC_WEIGHTS[wc_prec - 2];
		out.r() = static_cast<uint8_t>((c0.r() * (BC7_WEIGHT_MAX - weights[wc])
			+ c1.r() * weights[wc] + BC7_WEIGHT_ROUND) >> BC7_WEIGHT_SHIFT);
		out.g() = static_cast<uint8_t>((c0.g() * (BC7_WEIGHT_MAX - weights[wc])
			+ c1.g() * weights[wc] + BC7_WEIGHT_ROUND) >> BC7_WEIGHT_SHIFT);
		out.b() = static_cast<uint8_t>((c0.b() * (BC7_WEIGHT_MAX - weights[wc])
			+ c1.b() * weights[wc] + BC7_WEIGHT_ROUND) >> BC7_WEIGHT_SHIFT);

		weights = BC67_PREC_WEIGHTS[wa_prec - 2];
		out.a() = static_cast<uint8_t>((c0.a() * (BC7_WEIGHT_MAX - weights[wa])
			+ c1.a() * weights[wa] + BC7_WEIGHT_ROUND) >> BC7_WEIGHT_SHIFT);

		return out;
	}


	void BC4ToBC1G(BC1Block& bc1, BC4Block const & bc4)
	{
		bc1.clr_0 = (bc4.alpha_0 >> 2) << 5;
		bc1.clr_1 = (bc4.alpha_1 >> 2) << 5;
		bool swap_clr = false;
		if (bc4.alpha_0 < bc4.alpha_1)
		{
			swap_clr = true;
		}
		for (int i = 0; i < 2; ++ i)
		{
			uint32_t alpha32 = (bc4.bitmap[i * 3 + 2] << 16) | (bc4.bitmap[i * 3 + 1] << 8) | (bc4.bitmap[i * 3 + 0] << 0);
			uint16_t mask = 0;
			for (int j = 0; j < 8; ++ j)
			{
				uint16_t bit = (alpha32 >> (j * 3)) & 0x7;
				if (swap_clr)
				{
					switch (bit)
					{
					case 0:
					case 6:
						bit = 0;
						break;

					case 1:
					case 7:
						bit = 1;
						break;

					case 2:
					case 3:
						bit = 2;
						break;

					case 4:
					case 5:
						bit = 3;
						break;
					}
				}
				else
				{
					switch (bit)
					{
					case 0:
					case 2:
						bit = 0;
						break;

					case 1:
					case 5:
					case 7:
						bit = 1;
						break;

					case 3:
					case 4:
					case 6:
						bit = 2;
						break;
					}
				}

				mask |= bit << (j * 2);
			}

			bc1.bitmap[i] = mask;
		}
	}
}
