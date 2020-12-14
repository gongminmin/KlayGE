// JudaTexture.hpp
// KlayGE Tera-pixel Texture header file
// Ver 3.11.0
// Copyright(C) Minmin Gong, 2010
// Homepage: http://www.klayge.org
//
// 3.11.0
// First release (2010.12.4)
//
// CHANGE LIST
/////////////////////////////////////////////////////////////////////////////////

#ifndef _JUDATEXTURE_HPP
#define _JUDATEXTURE_HPP

#pragma once

#include <KlayGE/PreDeclare.hpp>
#include <KlayGE/Texture.hpp>
#include <KlayGE/RenderStateObject.hpp>
#include <KlayGE/TexCompressionBC.hpp>

#include <vector>
#include <deque>
#include <unordered_map>

#include <KlayGE/LZMACodec.hpp>

namespace KlayGE
{
	KLAYGE_CORE_API JudaTexturePtr LoadJudaTexture(std::string const & file_name);
	KLAYGE_CORE_API void SaveJudaTexture(JudaTexturePtr const & juda_tex, std::string const & file_name);

	class KLAYGE_CORE_API JudaTexture final : boost::noncopyable
	{
		friend KLAYGE_CORE_API JudaTexturePtr LoadJudaTexture(std::string const & file_name);
		friend KLAYGE_CORE_API void SaveJudaTexture(JudaTexturePtr const & juda_tex, std::string const & file_name);

	private:
		static uint32_t const EMPTY_DATA_INDEX = static_cast<uint32_t>(-1);

		struct QuadTreeNode
		{
			std::shared_ptr<QuadTreeNode> children[4];
			uint32_t data_index{EMPTY_DATA_INDEX};
			uint32_t attr{};
		};

		static uint32_t const MAX_TREE_LEVEL = 12;
		static uint32_t const MAX_NUM_TILES = 1UL << MAX_TREE_LEVEL;
		static uint32_t const TILE_MASK = (1UL << MAX_TREE_LEVEL) - 1;
		static uint32_t const MAX_TILE_SIZE = 256;
		static uint32_t const MAX_NUM_CHANNELS = 4;

		static uint32_t const LEVEL_SHIFT = 28;

	public:
		JudaTexture(uint32_t num_tiles, uint32_t tile_size, ElementFormat format);

		uint32_t EncodeTileID(uint32_t level, uint32_t tile_x, uint32_t tile_y) const;
		void DecodeTileID(uint32_t& level, uint32_t& tile_x, uint32_t& tile_y, uint32_t tile_id) const;

		uint32_t NumNonEmptyNodes() const;

		uint32_t NumTiles() const;
		uint32_t TreeLevels() const;
		uint32_t TileSize() const;
		ElementFormat Format() const;

		void AddImageEntry(std::string const & name, uint32_t x, uint32_t y, uint32_t w, uint32_t h, TexAddressingMode addr_u, TexAddressingMode addr_v, Color const & border_clr);

		void CommitTiles(std::vector<std::vector<uint8_t>> const & data, std::vector<uint32_t> const & tile_ids, std::vector<uint32_t> const & tile_attrs);
		void DecodeTiles(std::vector<std::vector<uint8_t>>& data, std::vector<uint32_t> const & tile_ids, uint32_t mipmaps);

		void CacheProperty(uint32_t pages, ElementFormat format, uint32_t border_size, uint32_t cache_tile_size = 0);

		TexturePtr const & CacheTex() const;
		std::vector<TexturePtr> const & CacheTexArray() const;
		TexturePtr const & IndirectTex() const;

		void SetParams(RenderEffect const & effect);

		void UpdateCache(std::vector<uint32_t> const & tile_ids);

	private:
		void DecodeATile(std::vector<uint8_t>* data, uint32_t shuff, uint32_t mipmaps);
		uint32_t DecodeAAttr(uint32_t shuff);
		uint8_t* RetrieveATile(uint32_t data_index);

		uint32_t NumNonEmptySubNodes(QuadTreeNode const& node) const;
		QuadTreeNode& GetNode(uint32_t shuff);
		QuadTreeNode& AddNode(uint32_t shuff);
		void CompactNode(uint32_t shuff);

		uint32_t ShuffLevel(uint32_t shuff) const;
		uint32_t ShuffLevel(uint32_t shuff, uint32_t level) const;
		uint32_t Pos2Shuff(uint32_t level, uint32_t x, uint32_t y) const;
		void Shuff2Pos(uint32_t& x, uint32_t& y, uint32_t level, uint32_t shuff) const;
		uint32_t GetLevelBranch(uint32_t shuff, uint32_t level) const;
		uint32_t SetLevelBranch(uint32_t shuff, uint32_t level, uint32_t bits) const;
		uint32_t GetParentShuff(uint32_t shuff) const;
		uint32_t GetChildShuff(uint32_t shuff, uint32_t branch) const;

		void Upsample(uint8_t* output, uint8_t const * input, uint32_t in_width, uint32_t in_height, uint32_t in_pitch,
			uint32_t rx, uint32_t ry, uint32_t rw, uint32_t rh);
		void Upsample(uint8_t* output, uint8_t const * input, uint32_t in_width, uint32_t in_height, uint32_t in_pitch);
		void Downsample(std::vector<uint8_t>& output, uint8_t const * input, uint32_t in_width, uint32_t in_height, uint32_t in_pitch);

		uint32_t AllocateDataBlock();
		void DeallocateDataBlock(uint32_t index);

	private:
		std::shared_ptr<QuadTreeNode> root_;

		uint32_t num_tiles_;
		uint32_t tree_levels_;
		uint32_t lower_levels_;

		uint32_t tile_size_;
		ElementFormat format_;
		uint32_t texel_size_;

		struct TexelOp
		{
			typedef void (*copy_func)(uint8_t* output, uint8_t const * rhs);
			typedef void (*copy_array_func)(uint8_t* output, uint8_t const * rhs, uint32_t num);
			typedef void (*add_func)(uint8_t* output, uint8_t const * lhs, uint8_t const * rhs);
			typedef void (*sub_func)(uint8_t* output, uint8_t const * lhs, uint8_t const * rhs);
			typedef void (*to_float4_func)(float* output, uint8_t const * rhs);
			typedef void (*from_float4_func)(uint8_t* output, float const * rhs);
			typedef int (*mse_func)(uint8_t const * rhs);
			typedef int (*bias_func)(int bias, uint8_t const * rhs);

			copy_func copy;
			copy_array_func copy_array;
			add_func add;
			sub_func sub;
			to_float4_func to_float4;
			from_float4_func from_float4;
			mse_func mse;
			bias_func bias;
		};
		TexelOp texel_op_;

		struct ImageEntry
		{
			std::string name;
			uint16_t x, y;
			uint16_t w, h;
			uint32_t addr_u_v;
			Color border_clr;
		};
		std::vector<ImageEntry> image_entries_;

	private:
		// Output only
		std::vector<std::vector<uint8_t>> data_blocks_;
		std::deque<uint32_t> data_block_free_list_;

	private:
		// Input only
		ResIdentifierPtr input_file_;
		uint32_t data_blocks_offset_;
		LZMACodec lzma_dec_;
		struct DecodedBlockInfo
		{
			std::unique_ptr<uint8_t[]> data;
			uint64_t tick;

			DecodedBlockInfo(std::unique_ptr<uint8_t[]>&& d, uint64_t t)
				: data(std::move(d)), tick(t)
			{
			}
		};
		std::unordered_map<uint32_t, DecodedBlockInfo> decoded_block_cache_;
		uint64_t decode_tick_;

	private:
		// Cache
		TexturePtr tex_cache_;
		std::vector<TexturePtr> tex_cache_array_;
		TexturePtr tex_indirect_;
		uint32_t cache_tile_border_size_;
		uint32_t cache_tile_size_;
		std::unique_ptr<TexCompression> tex_codec_;

		struct TileInfo
		{
			uint32_t x, y, z;
			uint32_t attr;
			uint64_t tick;
		};
		std::unordered_map<uint32_t, TileInfo> tile_info_map_;
		std::deque<std::pair<uint32_t, uint32_t>> tile_free_list_;
		uint64_t tile_tick_;
	};
}

#endif		// _JUDATEXTURE_HPP
