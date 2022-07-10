/**
* @file TexCompressionETC.hpp
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

#ifndef _TEXCOMPRESSIONETC_HPP
#define _TEXCOMPRESSIONETC_HPP

#pragma once

#include <KlayGE/TexCompression.hpp>

namespace KlayGE
{
#ifdef KLAYGE_HAS_STRUCT_PACK
	#pragma pack(push, 1)
#endif
	struct ETC1Block
	{
		uint8_t r;
		uint8_t g;
		uint8_t b;
		uint8_t cw_diff_flip;
		uint16_t msb;
		uint16_t lsb;
	};
	static_assert(sizeof(ETC1Block) == 8);

	struct ETC2TModeBlock
	{
		uint8_t r1;
		uint8_t g1_b1;
		uint8_t r2_g2;
		uint8_t b2_d;
		uint16_t msb;
		uint16_t lsb;
	};
	static_assert(sizeof(ETC2TModeBlock) == 8);

	struct ETC2HModeBlock
	{
		uint8_t r1_g1;
		uint8_t g1_b1;
		uint8_t b1_r2_g2;
		uint8_t g2_b2_d;
		uint16_t msb;
		uint16_t lsb;
	};
	static_assert(sizeof(ETC2HModeBlock) == 8);

	struct ETC2PlanarModeBlock
	{
		uint8_t ro_go;
		uint8_t go_bo;
		uint8_t bo;
		uint8_t bo_rh;
		uint8_t gh_bh;
		uint8_t bh_rv;
		uint8_t rv_gv;
		uint8_t gv_bv;
	};
	static_assert(sizeof(ETC2PlanarModeBlock) == 8);

	union ETC2Block
	{
		ETC1Block etc1;
		ETC2TModeBlock etc2_t_mode;
		ETC2HModeBlock etc2_h_mode;
		ETC2PlanarModeBlock etc2_planar_mode;
	};
	static_assert(sizeof(ETC2Block) == 8);
#ifdef KLAYGE_HAS_STRUCT_PACK
	#pragma pack(pop)
#endif

	class KLAYGE_CORE_API TexCompressionETC1 final : public TexCompression
	{
	public:
		struct Params
		{
			Params();

			TexCompressionMethod quality_;

			uint32_t num_src_pixels_;
			ARGBColor32 const * src_pixels_;

			bool use_color4_;
			int const * scan_deltas_;
			uint32_t scan_delta_size_;

			ARGBColor32 base_color5_;
			bool constrain_against_base_color5_;
		};

		struct Results
		{
			Results();

			Results& operator=(Results const & rhs);

			uint64_t error_;
			ARGBColor32 block_color_unscaled_;
			uint32_t block_inten_table_;
			std::vector<uint8_t> selectors_;
			bool block_color4_;
		};

	public:
		TexCompressionETC1();

		virtual void EncodeBlock(void* output, void const * input, TexCompressionMethod method) override;
		virtual void DecodeBlock(void* output, void const * input) override;

		uint64_t EncodeETC1BlockInternal(ETC1Block& output, ARGBColor32 const * argb, TexCompressionMethod method);
		void DecodeETCIndividualModeInternal(ARGBColor32* argb, ETC1Block const & etc1) const;
		void DecodeETCDifferentialModeInternal(ARGBColor32* argb, ETC1Block const & etc1, bool alpha) const;

		static int GetModifier(int cw, int selector);

	private:
		struct ETC1SolutionCoordinates
		{
			ETC1SolutionCoordinates();
			ETC1SolutionCoordinates(int r, int g, int b, uint32_t inten_table, bool color4);
			ETC1SolutionCoordinates(ARGBColor32 const & c, uint32_t inten_table, bool color4);
			ETC1SolutionCoordinates(ETC1SolutionCoordinates const & rhs);

			ETC1SolutionCoordinates& operator=(ETC1SolutionCoordinates const & rhs);

			void Clear();

			void ScaledColor(uint8_t& br, uint8_t& bg, uint8_t& bb) const;
			ARGBColor32 ScaledColor() const;
			void BlockColors(ARGBColor32* block_colors) const;

			ARGBColor32 unscaled_color_;
			uint32_t inten_table_;
			bool color4_;
		};

		struct PotentialSolution
		{
			PotentialSolution();

			void Clear();

			ETC1SolutionCoordinates coords_;
			uint8_t selectors_[8];
			uint64_t error_;
			bool valid_;
		};

	private:
		uint32_t ETC1DecodeValue(uint32_t diff, uint32_t inten, uint32_t selector, uint32_t packed_c) const;

		uint64_t PackETC1UniformBlock(ETC1Block& block, ARGBColor32 const * argb) const;
		uint32_t PackETC1UniformPartition(Results& results, uint32_t num_colors, ARGBColor32 const * argb,
			bool use_diff, ARGBColor32 const * base_color5_unscaled) const;

		void InitSolver(Params const & params, Results& result);
		bool Solve();
		bool EvaluateSolution(ETC1SolutionCoordinates const & coords, PotentialSolution& trial_solution, PotentialSolution& best_solution);
		bool EvaluateSolutionFast(ETC1SolutionCoordinates const & coords, PotentialSolution& trial_solution, PotentialSolution& best_solution);

	private:
		Params const * params_;
		Results* result_;

		int limit_;

		float3 avg_color_;
		int br_, bg_, bb_;
		uint16_t luma_[8];
		uint32_t sorted_luma_[2][8];
		uint32_t const * sorted_luma_indices_;
		uint32_t* sorted_luma_ptr_;

		PotentialSolution best_solution_;
		PotentialSolution trial_solution_;
		uint8_t temp_selectors_[8];
	};

	class KLAYGE_CORE_API TexCompressionETC2RGB8 final : public TexCompression
	{
	public:
		TexCompressionETC2RGB8();

		virtual void EncodeBlock(void* output, void const * input, TexCompressionMethod method) override;
		virtual void DecodeBlock(void* output, void const * input) override;

		void DecodeETCTModeInternal(ARGBColor32* argb, ETC2TModeBlock const & etc2, bool alpha);
		void DecodeETCHModeInternal(ARGBColor32* argb, ETC2HModeBlock const & etc2, bool alpha);
		void DecodeETCPlanarModeInternal(ARGBColor32* argb, ETC2PlanarModeBlock const & etc2);

	private:
		std::unique_ptr<TexCompressionETC1> etc1_codec_;
	};

	class KLAYGE_CORE_API TexCompressionETC2RGB8A1 final : public TexCompression
	{
	public:
		TexCompressionETC2RGB8A1();

		virtual void EncodeBlock(void* output, void const * input, TexCompressionMethod method) override;
		virtual void DecodeBlock(void* output, void const * input) override;

	private:
		std::unique_ptr<TexCompressionETC1> etc1_codec_;
		std::unique_ptr<TexCompressionETC2RGB8> etc2_rgb8_codec_;
	};
}

#endif		// _TEXCOMPRESSIONETC_HPP
