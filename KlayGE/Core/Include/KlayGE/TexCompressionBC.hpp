/**
* @file TexCompressionBC.hpp
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

#ifndef _TEXCOMPRESSIONBC_HPP
#define _TEXCOMPRESSIONBC_HPP

#pragma once

#include <cstring>

#include <KlayGE/TexCompression.hpp>

namespace KlayGE
{
#ifdef KLAYGE_HAS_STRUCT_PACK
#pragma pack(push, 1)
#endif
	struct BC1Block
	{
		uint16_t clr_0, clr_1;
		uint16_t bitmap[2];
	};
	KLAYGE_STATIC_ASSERT(sizeof(BC1Block) == 8);

	struct BC2Block
	{
		uint16_t alpha[4];
		BC1Block bc1;
	};
	KLAYGE_STATIC_ASSERT(sizeof(BC2Block) == 16);

	struct BC4Block
	{
		uint8_t alpha_0, alpha_1;
		uint8_t bitmap[6];
	};
	KLAYGE_STATIC_ASSERT(sizeof(BC4Block) == 8);

	struct BC3Block
	{
		BC4Block alpha;
		BC1Block bc1;
	};
	KLAYGE_STATIC_ASSERT(sizeof(BC3Block) == 16);

	struct BC5Block
	{
		BC4Block red;
		BC4Block green;
	};
	KLAYGE_STATIC_ASSERT(sizeof(BC5Block) == 16);
#ifdef KLAYGE_HAS_STRUCT_PACK
#pragma pack(pop)
#endif

	class KLAYGE_CORE_API TexCompressionBC1 final : public TexCompression
	{
	public:
		TexCompressionBC1();

		virtual void EncodeBlock(void* output, void const * input, TexCompressionMethod method) override;
		virtual void DecodeBlock(void* output, void const * input) override;

		void EncodeBC1Internal(BC1Block& bc1, ARGBColor32 const * argb, bool alpha, TexCompressionMethod method) const;

	private:
		ARGBColor32 RGB565To888(uint16_t rgb) const;
		uint16_t RGB888To565(ARGBColor32 const & rgb) const;
		uint32_t MatchColorsBlock(ARGBColor32 const * argb, ARGBColor32 const & min_clr, ARGBColor32 const & max_clr, bool alpha) const;
		void OptimizeColorsBlock(ARGBColor32 const * argb, ARGBColor32& min_clr, ARGBColor32& max_clr, TexCompressionMethod method) const;
		bool RefineBlock(ARGBColor32 const * argb, ARGBColor32& min_clr, ARGBColor32& max_clr, uint32_t mask) const;
	};

	class KLAYGE_CORE_API TexCompressionBC2 final : public TexCompression
	{
	public:
		TexCompressionBC2();

		virtual void EncodeBlock(void* output, void const * input, TexCompressionMethod method) override;
		virtual void DecodeBlock(void* output, void const * input) override;

	private:
		TexCompressionBC1 bc1_codec_;
	};

	class KLAYGE_CORE_API TexCompressionBC4 final : public TexCompression
	{
	public:
		TexCompressionBC4();

		virtual void EncodeBlock(void* output, void const * input, TexCompressionMethod method) override;
		virtual void DecodeBlock(void* output, void const * input) override;
	};

	class KLAYGE_CORE_API TexCompressionBC3 final : public TexCompression
	{
	public:
		TexCompressionBC3();

		virtual void EncodeBlock(void* output, void const * input, TexCompressionMethod method) override;
		virtual void DecodeBlock(void* output, void const * input) override;

	private:
		TexCompressionBC1 bc1_codec_;
		TexCompressionBC4 bc4_codec_;
	};

	class KLAYGE_CORE_API TexCompressionBC5 final : public TexCompression
	{
	public:
		TexCompressionBC5();

		virtual void EncodeBlock(void* output, void const * input, TexCompressionMethod method) override;
		virtual void DecodeBlock(void* output, void const * input) override;

	private:
		TexCompressionBC4 bc4_codec_;
	};

	class KLAYGE_CORE_API TexCompressionBC6U final : public TexCompression
	{
	public:
		TexCompressionBC6U();

		virtual void EncodeBlock(void* output, void const * input, TexCompressionMethod method) override;
		virtual void DecodeBlock(void* output, void const * input) override;

		void DecodeBC6Internal(void* output, void const * input, bool signed_fmt);

	private:
		int Unquantize(int comp, uint8_t bits_per_comp, bool signed_fmt);
		int FinishUnquantize(int comp, bool signed_fmt);

	private:
		static uint32_t const BC6_MAX_REGIONS = 2;
		static uint32_t const BC6_MAX_INDICES = 16;

		static int32_t const BC6_WEIGHT_MAX = 64;
		static uint32_t const BC6_WEIGHT_SHIFT = 6;
		static int32_t const BC6_WEIGHT_ROUND = 32;

		enum ModeField : uint8_t
		{
			NA, // N/A
			M,  // Mode
			D,  // Shape
			RW,
			RX,
			RY,
			RZ,
			GW,
			GX,
			GY,
			GZ,
			BW,
			BX,
			BY,
			BZ,
		};

		struct ModeDescriptor
		{
			ModeField field;
			uint8_t bit;
		};

		struct ModeInfo
		{
			uint8_t mode;
			uint8_t partitions;
			bool transformed;
			uint8_t index_prec;
			ARGBColor32 rgba_prec[BC6_MAX_REGIONS][2];
		};

		static ModeDescriptor const mode_desc_[][82];
		static ModeInfo const mode_info_[];
		static int const mode_to_info_[];
	};

	class KLAYGE_CORE_API TexCompressionBC6S final : public TexCompression
	{
	public:
		TexCompressionBC6S();

		virtual void EncodeBlock(void* output, void const * input, TexCompressionMethod method) override;
		virtual void DecodeBlock(void* output, void const * input) override;

	private:
		TexCompressionBC6U bc6u_codec_;
	};

	class KLAYGE_CORE_API TexCompressionBC7 final : public TexCompression
	{
		static uint32_t const BC7_MAX_REGIONS = 3;
		static uint32_t const BC7_NUM_CHANNELS = 4;
		static uint32_t const BC7_MAX_SHAPES = 64;

		static uint32_t const BC7_WEIGHT_MAX = 64;
		static uint32_t const BC7_WEIGHT_SHIFT = 6;
		static uint32_t const BC7_WEIGHT_ROUND = 32;

		enum PBitType
		{
			PBT_None,
			PBT_Shared,
			PBT_Unique
		};

		struct ModeInfo
		{
			uint8_t partitions;
			uint8_t partition_bits;
			uint8_t p_bits;
			uint8_t rotation_bits;
			uint8_t index_mode_bits;
			uint8_t index_prec_1;
			uint8_t index_prec_2;
			ARGBColor32 rgba_prec;
			ARGBColor32 rgba_prec_with_p;
			PBitType p_bit_type;
		};

		struct CompressParams
		{
			float4 p1[BC7_MAX_REGIONS], p2[BC7_MAX_REGIONS];
			uint8_t indices[BC7_MAX_REGIONS][16];
			uint8_t alpha_indices[16];
			uint8_t pbit_combo[BC7_MAX_REGIONS];
			int8_t rotation_mode;
			int8_t index_mode;
			uint32_t shape_index;

			CompressParams()
			{
			}
			explicit CompressParams(uint32_t shape)
				: rotation_mode(-1), index_mode(-1), shape_index(shape)
			{
				std::fill(std::begin(p1), std::end(p1), float4(0, 0, 0, 0));
				std::fill(std::begin(p2), std::end(p2), float4(0, 0, 0, 0));
				memset(indices, 0xFF, sizeof(indices));
				memset(alpha_indices, 0xFF, sizeof(alpha_indices));
				memset(pbit_combo, 0xFF, sizeof(pbit_combo));
			}
		};

	public:
		TexCompressionBC7();

		virtual void EncodeBlock(void* output, void const * input, TexCompressionMethod method) override;
		virtual void DecodeBlock(void* output, void const * input) override;

	private:
		void PackBC7UniformBlock(void* output, ARGBColor32 const & pixel);
		void PackBC7Block(int mode, CompressParams& params, void* output);
		int RotationMode(ModeInfo const & mode_info) const;
		int NumBitsPerIndex(ModeInfo const & mode_info, int8_t index_mode = -1) const;
		int NumBitsPerAlpha(ModeInfo const & mode_info, int8_t index_mode = -1) const;
		uint4 ErrorMetric(ModeInfo const & mode_info) const;
		ARGBColor32 QuantizationMask(ModeInfo const & mode_info) const;
		int NumPbitCombos(ModeInfo const & mode_info) const;
		int const * PBitCombo(ModeInfo const & mode_info, int idx) const;
		uint64_t OptimizeEndpointsForCluster(int mode, RGBACluster const & cluster,
			float4& p1, float4& p2, uint8_t* best_indices, uint8_t& best_pbit_combo) const;
		void PickBestNeighboringEndpoints(int mode, float4 const & p1, float4 const & p2,
			int cur_pbit_combo, float4& np1, float4& np2, int& pbit_combo, float step_sz = 1) const;
		bool AcceptNewEndpointError(uint64_t new_err, uint64_t old_err, float temp) const;
		uint64_t CompressSingleColor(ModeInfo const & mode_info,
			ARGBColor32 const & pixel, float4& p1, float4& p2, uint8_t& best_pbit_combo) const;
		uint64_t CompressCluster(int mode, RGBACluster const & cluster,
			float4& p1, float4& p2, uint8_t* best_indices, uint8_t& best_pbit_combo) const;
		uint64_t CompressCluster(int mode, RGBACluster const & cluster,
			float4& p1, float4& p2, uint8_t *best_indices, uint8_t* alpha_indices) const;
		void ClampEndpoints(float4& p1, float4& p2) const;
		void ClampEndpointsToGrid(ModeInfo const & mode_info,
			float4& p1, float4& p2, uint8_t& best_pbit_combo) const;
		uint64_t TryCompress(int mode, int simulated_annealing_steps, TexCompressionErrorMetric metric,
			CompressParams& params, uint32_t shape_index, RGBACluster& cluster);

		uint8_t Unquantize(uint8_t comp, size_t prec) const;
		ARGBColor32 Unquantize(ARGBColor32 const & c, ARGBColor32 const & rgba_prec) const;
		ARGBColor32 Interpolate(ARGBColor32 const & c0, ARGBColor32 const & c1,
			size_t wc, size_t wa, size_t wc_prec, size_t wa_prec) const;

	private:
		int sa_steps_;
		TexCompressionErrorMetric error_metric_;
		int rotate_mode_;
		int index_mode_;

		static ModeInfo const mode_info_[];
	};

	KLAYGE_CORE_API void BC4ToBC1G(BC1Block& bc1, BC4Block const & bc4);
}

#endif		// _TEXCOMPRESSIONBC_HPP
