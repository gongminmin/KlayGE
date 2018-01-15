// JudaTexture.cpp
// KlayGE Tera-pixel Texture implement file
// Ver 3.11.0
// Copyright(C) Minmin Gong, 2010
// Homepage: http://www.klayge.org
//
// 3.11.0
// First release (2010.12.4)
//
// CHANGE LIST
/////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KFL/ErrorHandling.hpp>
#include <KlayGE/ResLoader.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/RenderEffect.hpp>

#include <fstream>
#include <cstring>
#include <boost/assert.hpp>
#if defined(KLAYGE_COMPILER_CLANGC2)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-variable" // Ignore unused variable (mpl_assertion_in_line_xxx) in boost
#endif
#include <boost/lexical_cast.hpp>
#if defined(KLAYGE_COMPILER_CLANGC2)
#pragma clang diagnostic pop
#endif

#include <KlayGE/JudaTexture.hpp>

namespace
{
	using namespace KlayGE;

	uint32_t const JUDA_TEX_VERSION = 2;

	void u8_copy_1(uint8_t* output, uint8_t const * rhs)
	{
		*output = *rhs;
	}
	void u8_copy_2(uint8_t* output, uint8_t const * rhs)
	{
		*reinterpret_cast<uint16_t*>(output) = *reinterpret_cast<uint16_t const *>(rhs);
	}
	void u8_copy_4(uint8_t* output, uint8_t const * rhs)
	{
		*reinterpret_cast<uint32_t*>(output) = *reinterpret_cast<uint32_t const *>(rhs);
	}

	template <int N>
	void u8_copy_array(uint8_t* output, uint8_t const * rhs, uint32_t num)
	{
		std::memcpy(output, rhs, num * N * sizeof(uint8_t));
	}

	template <int N>
	void u8_add(uint8_t* output, uint8_t const * lhs, uint8_t const * rhs)
	{
		for (int i = 0; i < N; ++ i)
		{
			output[i] = lhs[i] + rhs[i];
		}
	}

	template <int N>
	void u8_sub(uint8_t* output, uint8_t const * lhs, uint8_t const * rhs)
	{
		for (int i = 0; i < N; ++ i)
		{
			output[i] = lhs[i] - rhs[i];
		}
	}

	template <int N>
	void u8_to_float4(float* output, uint8_t const * rhs)
	{
		for (int i = 0; i < N; ++ i)
		{
			output[i] = rhs[i] / 255.0f;
		}
	}

	template <int N>
	void u8_from_float4(uint8_t* output, float const * rhs)
	{
		for (int i = 0; i < N; ++ i)
		{
			output[i] = static_cast<uint8_t>(MathLib::clamp(static_cast<int>(rhs[i] * 255 + 0.5f), 0, 255));
		}
	}

	template <int N>
	int u8_mse(uint8_t const * rhs)
	{
		int ret = 0;
		for (int i = 0; i < N; ++ i)
		{
			int r = static_cast<int>(rhs[i]);
			ret += r * r;
		}
		return ret;
	}

	template <int N>
	int u8_bias(int bias, uint8_t const * rhs)
	{
		for (int i = 0; i < N; ++ i)
		{
			bias = std::max(bias, abs(static_cast<int>(rhs[i])));
		}
		return bias;
	}
}

namespace KlayGE
{
	float const THRESHOLD_MSE = 0.001f;
	int const THRESHOLD_BIAS = 10;

	JudaTexture::JudaTexture(uint32_t num_tiles, uint32_t tile_size, ElementFormat format)
		: root_(MakeSharedPtr<quadtree_node>()),
			num_tiles_(num_tiles), tile_size_(tile_size), format_(format),
			texel_size_(NumFormatBytes(format)),
			decode_tick_(0), tile_tick_(0)
	{
		BOOST_ASSERT(num_tiles_ <= MAX_NUM_TILES);
		BOOST_ASSERT(tile_size_ <= MAX_TILE_SIZE);
		BOOST_ASSERT(0 == (tile_size_ & (tile_size_ - 1)));
		BOOST_ASSERT(NumComponents(format_) <= MAX_NUM_CHANNELS);

		switch (format_)
		{
		case EF_R8:
			texel_op_.copy = u8_copy_1;
			texel_op_.copy_array = u8_copy_array<1>;
			texel_op_.add = u8_add<1>;
			texel_op_.sub = u8_sub<1>;
			texel_op_.to_float4 = u8_to_float4<1>;
			texel_op_.from_float4 = u8_from_float4<1>;
			texel_op_.mse = u8_mse<1>;
			texel_op_.bias = u8_bias<1>;
			break;

		case EF_GR8:
			texel_op_.copy = u8_copy_2;
			texel_op_.copy_array = u8_copy_array<2>;
			texel_op_.add = u8_add<2>;
			texel_op_.sub = u8_sub<2>;
			texel_op_.to_float4 = u8_to_float4<2>;
			texel_op_.from_float4 = u8_from_float4<2>;
			texel_op_.mse = u8_mse<2>;
			texel_op_.bias = u8_bias<2>;
			break;

		case EF_ABGR8:
		case EF_ARGB8:
			texel_op_.copy = u8_copy_4;
			texel_op_.copy_array = u8_copy_array<4>;
			texel_op_.add = u8_add<4>;
			texel_op_.sub = u8_sub<4>;
			texel_op_.to_float4 = u8_to_float4<4>;
			texel_op_.from_float4 = u8_from_float4<4>;
			texel_op_.mse = u8_mse<4>;
			texel_op_.bias = u8_bias<4>;
			break;

		default:
			KFL_UNREACHABLE("Not supported element format");
		}

		tree_levels_ = 0;
		while (num_tiles > (1UL << tree_levels_))
		{
			++ tree_levels_;
		}
		++ tree_levels_;

		lower_levels_ = 0;
		while ((tile_size_ > (1UL << lower_levels_)) && (lower_levels_ < tree_levels_ - 1))
		{
			++ lower_levels_;
		}
	}

	uint32_t JudaTexture::EncodeTileID(uint32_t level, uint32_t tile_x, uint32_t tile_y) const
	{
		BOOST_ASSERT(level <= MAX_TREE_LEVEL);
		BOOST_ASSERT(tile_x < num_tiles_);
		BOOST_ASSERT(tile_y < num_tiles_);

		return (level << LEVEL_SHIFT) | (tile_y << MAX_TREE_LEVEL) | tile_x;
	}

	void JudaTexture::DecodeTileID(uint32_t& level, uint32_t& tile_x, uint32_t& tile_y, uint32_t tile_id) const
	{
		level = tile_id >> LEVEL_SHIFT;
		tile_x = tile_id & TILE_MASK;
		tile_y = (tile_id >> MAX_TREE_LEVEL) & TILE_MASK;
	}

	uint32_t JudaTexture::NumNonEmptyNodes() const
	{
		return this->NumNonEmptySubNodes(root_);
	}

	uint32_t JudaTexture::NumTiles() const
	{
		return num_tiles_;
	}

	uint32_t JudaTexture::TreeLevels() const
	{
		return tree_levels_;
	}

	uint32_t JudaTexture::TileSize() const
	{
		return tile_size_;
	}

	ElementFormat JudaTexture::Format() const
	{
		return format_;
	}

	void JudaTexture::CompactNode(uint32_t shuff)
	{
		if (this->ShuffLevel(shuff) == tree_levels_ - 1)
		{
			return;
		}

		quadtree_node_ptr const & node = this->GetNode(shuff);
		for (int i = 0; i < 4; ++ i)
		{
			if (node->children[i])
			{
				this->CompactNode(this->GetChildShuff(shuff, i));

				if ((EMPTY_DATA_INDEX == node->children[i]->data_index)
					&& !node->children[i]->children[0]
					&& !node->children[i]->children[1]
					&& !node->children[i]->children[2]
					&& !node->children[i]->children[3])
				{
					node->children[i].reset();
				}
			}
		}
	}
	
	void JudaTexture::AddImageEntry(std::string const & name, uint32_t x, uint32_t y, uint32_t w, uint32_t h, TexAddressingMode addr_u, TexAddressingMode addr_v, Color const & border_clr)
	{
		ImageEntry entry;
		entry.name = name;
		entry.x = static_cast<uint16_t>(x);
		entry.y = static_cast<uint16_t>(y);
		entry.w = static_cast<uint16_t>(w);
		entry.h = static_cast<uint16_t>(h);
		entry.addr_u_v = static_cast<uint8_t>((addr_v << 4) | addr_u);
		entry.border_clr = border_clr;
		image_entries_.push_back(entry);
	}

	void JudaTexture::CommitTiles(std::vector<std::vector<uint8_t>> const & data, std::vector<uint32_t> const & tile_ids, std::vector<uint32_t> const & tile_attrs)
	{
		uint32_t const full_tile_bytes = tile_size_ * tile_size_ * texel_size_;

		if (EMPTY_DATA_INDEX == root_->data_index)
		{
			root_->data_index = this->AllocateDataBlock();
			root_->attr = 0xFFFFFFFF;
			data_blocks_[root_->data_index].resize(full_tile_bytes, 0);
		}

		std::vector<uint32_t> shuffs(tile_ids.size());
		for (size_t i = 0; i < tile_ids.size(); ++ i)
		{
			uint32_t level, tile_x, tile_y;
			this->DecodeTileID(level, tile_x, tile_y, tile_ids[i]);
			shuffs[i] = this->Pos2Shuff(level, tile_x, tile_y);

			quadtree_node_ptr node = this->AddNode(shuffs[i]);
			if (EMPTY_DATA_INDEX == node->data_index)
			{
				node->data_index = this->AllocateDataBlock();
			}
			node->attr = tile_attrs[i];
			data_blocks_[node->data_index] = data[i];
		}
		
		for (uint32_t ll = 0; ll < tree_levels_ - 1; ll += lower_levels_)
		{
			std::vector<uint32_t> upper_shuffs(shuffs.size());
			for (size_t i = 0; i < shuffs.size(); ++ i)
			{
				uint32_t shuff = shuffs[i];
				for (uint32_t level = 0; level < lower_levels_; ++ level)
				{
					shuff = this->GetParentShuff(shuff);
				}
				upper_shuffs[i] = shuff;
			}
			std::sort(upper_shuffs.begin(), upper_shuffs.end());
			upper_shuffs.erase(std::unique(upper_shuffs.begin(), upper_shuffs.end()), upper_shuffs.end());

			for (size_t i = 0; i < upper_shuffs.size(); ++ i)
			{
				uint32_t shuff = upper_shuffs[i];
				quadtree_node_ptr node = this->GetNode(shuff);
				if (EMPTY_DATA_INDEX == node->data_index)
				{
					node->data_index = this->AllocateDataBlock();
				}
				node->attr = 0xFFFFFFFF;
				data_blocks_[node->data_index].resize(full_tile_bytes);
				this->DecodeATile(&data_blocks_[node->data_index], shuff, 1);
			}

			for (size_t i = 0; i < shuffs.size(); ++ i)
			{
				uint32_t shuff = shuffs[i];
				quadtree_node_ptr node = this->GetNode(shuff);

				int offset_x = 0;
				int offset_y = 0;
				std::vector<uint8_t> up_data = data_blocks_[node->data_index];
				for (int level = tree_levels_ - 1 - ll; level > std::max(0, static_cast<int>(tree_levels_ - 1 - ll - lower_levels_)); -- level)
				{
					node = this->GetNode(shuff);

					uint32_t level_tile_size = tile_size_ >> (tree_levels_ - 1 - ll - level);

					std::vector<uint8_t> temp_up;
					this->Downsample(temp_up, &up_data[0], level_tile_size, level_tile_size, level_tile_size * texel_size_);

					std::vector<uint8_t> temp_down(level_tile_size * level_tile_size * texel_size_);
					this->Upsample(&temp_down[0], &temp_up[0], level_tile_size / 2, level_tile_size / 2, level_tile_size / 2 * texel_size_);

					if (EMPTY_DATA_INDEX == node->data_index)
					{
						node->data_index = this->AllocateDataBlock();
					}
					if (level != static_cast<int>(tree_levels_) - 1)
					{
						node->attr = 0xFFFFFFFF;
					}
					data_blocks_[node->data_index].resize(full_tile_bytes);
					for (size_t y = 0; y < level_tile_size; ++ y)
					{
						for (size_t x = 0; x < level_tile_size; ++ x)
						{
							texel_op_.sub(&data_blocks_[node->data_index][((offset_y + y) * tile_size_ + (offset_x + x)) * texel_size_],
								&up_data[(y * level_tile_size + x) * texel_size_], &temp_down[(y * level_tile_size + x) * texel_size_]);
						}
					}
					float mse = 0;
					int bias = 0;
					for (uint32_t y = 0; y < tile_size_; ++ y)
					{
						for (uint32_t x = 0; x < tile_size_; ++ x)
						{
							mse += texel_op_.mse(&data_blocks_[node->data_index][(y * tile_size_ + x) * texel_size_]);
							bias = texel_op_.bias(bias, &data_blocks_[node->data_index][(y * tile_size_ + x) * texel_size_]);
						}
					}
					mse /= MathLib::sqr(tile_size_ * 255);
					if ((mse < THRESHOLD_MSE) && (bias < THRESHOLD_BIAS))
					{
						this->DeallocateDataBlock(node->data_index);
						node->data_index = EMPTY_DATA_INDEX;
						node->attr = 0xFFFFFFFF;
					}

					uint32_t branch = this->GetLevelBranch(shuff, level);
					uint32_t bx = (branch >> 0) & 1UL;
					uint32_t by = (branch >> 1) & 1UL;
					shuff = this->GetParentShuff(shuff);
					up_data.swap(temp_up);
					offset_x = offset_x / 2 + bx * tile_size_ / 2;
					offset_y = offset_y / 2 + by * tile_size_ / 2;
				}

				{
					int level = std::max(0, static_cast<int>(tree_levels_ - 1 - ll - lower_levels_));

					uint32_t data_index = this->GetNode(shuff)->data_index;
					uint32_t level_tile_size = tile_size_ >> (tree_levels_ - 1 - ll - level);

					for (uint32_t y = 0; y < level_tile_size; ++ y)
					{
						texel_op_.copy_array(&data_blocks_[data_index][((offset_y + y) * tile_size_ + offset_x) * texel_size_],
							&up_data[(y * level_tile_size) * texel_size_], level_tile_size);
					}
				}
			}

			for (size_t i = 0; i < upper_shuffs.size(); ++ i)
			{
				this->CompactNode(upper_shuffs[i]);
			}

			shuffs.swap(upper_shuffs);
		}
	}

	void JudaTexture::DecodeTiles(std::vector<std::vector<uint8_t>>& data, std::vector<uint32_t> const & tile_ids, uint32_t mipmaps)
	{
		BOOST_ASSERT(mipmaps - 1 <= lower_levels_);

		data.resize(tile_ids.size() * mipmaps);
		std::vector<std::pair<uint32_t, uint32_t>> shuffs(tile_ids.size());
		for (size_t i = 0; i < tile_ids.size(); ++ i)
		{
			uint32_t level, tile_x, tile_y;
			this->DecodeTileID(level, tile_x, tile_y, tile_ids[i]);
			shuffs[i] = std::make_pair(this->Pos2Shuff(level, tile_x, tile_y), static_cast<uint32_t>(i));
		}
		std::sort(shuffs.begin(), shuffs.end());

		uint32_t const full_tile_bytes = cache_tile_size_ * cache_tile_size_ * texel_size_;

		for (size_t i = 0; i < shuffs.size(); ++ i)
		{
			uint32_t shuff = shuffs[i].first;
			uint32_t const index = shuffs[i].second;

			uint32_t scale = tile_size_ / cache_tile_size_;
			if (scale != 1)
			{
				uint32_t level = this->ShuffLevel(shuff);
				while (scale > 1)
				{
					scale /= 2;
					-- level;
				}

				shuff = this->ShuffLevel(shuff, level);
			}

			uint32_t s = full_tile_bytes;
			for (size_t j = 0; j < mipmaps; ++ j)
			{
				data[index * mipmaps + j].resize(s);
				s /= 4;
			}

			this->DecodeATile(&data[index * mipmaps], shuff, mipmaps);
		}
	}

	void JudaTexture::DecodeATile(std::vector<uint8_t>* data, uint32_t shuff, uint32_t mipmaps)
	{
		++ decode_tick_;

		uint32_t const full_tile_bytes = cache_tile_size_ * cache_tile_size_ * texel_size_;
		uint32_t target_level = this->ShuffLevel(shuff);

		quadtree_node_ptr node = root_;
		if (0 == target_level)
		{
			std::memcpy(&data[0][0], this->RetriveATile(root_->data_index), full_tile_bytes);
		}
		else
		{
			int step = (tree_levels_ - 1) % lower_levels_;
			if (0 == step)
			{
				step = lower_levels_;
			}

			std::vector<uint32_t> branches(tree_levels_);
			for (uint32_t i = 0; i <= tree_levels_ - 1; ++ i)
			{
				branches[i] = this->GetLevelBranch(shuff, i);
			}

			uint32_t start_sub_tile_x = 0;
			uint32_t start_sub_tile_y = 0;
			for (uint32_t i = 1; i <= tree_levels_ - 1; ++ i)
			{
				uint32_t branch = branches[i];
				uint32_t by = (branch >> 1) & 1UL;
				uint32_t bx = (branch >> 0) & 1UL;
				start_sub_tile_x = (start_sub_tile_x << 1) + bx;
				start_sub_tile_y = (start_sub_tile_y << 1) + by;
			}

			std::vector<uint8_t> tile_data;
			std::vector<uint8_t> temp;

			for (uint32_t ll = 1; ll <= target_level; ll += step)
			{
				uint32_t const ll_b = ll;
				if (ll > 1)
				{
					step = lower_levels_;
				}
				uint32_t const ll_e = ll + step;
				uint32_t const shift = tree_levels_ - ll_e;

				for (uint32_t i = ll_b, i_end = std::min(target_level + 1, ll_e); i < i_end; ++ i)
				{
					uint32_t const used_w = tile_size_ >> (ll_e - i);
					uint32_t const used_h = used_w;

					temp.resize(used_w * 2 * used_h * 2 * texel_size_);
					if (i == ll_b)
					{
						uint32_t start_x = (start_sub_tile_x >> shift) * used_w;
						uint32_t start_y = (start_sub_tile_y >> shift) * used_h;
						uint8_t const * src;
						if (1 == ll_b)
						{
							src = this->RetriveATile(root_->data_index);
						}
						else
						{
							src = &tile_data[0];
						}
						this->Upsample(&temp[0], src + (start_y * tile_size_ + start_x) * texel_size_, used_w, used_h, tile_size_ * texel_size_);
					}
					else
					{
						this->Upsample(&temp[0], &tile_data[0], used_w, used_h, used_w * texel_size_);
					}

					start_sub_tile_x &= ~(1UL << (tree_levels_ - 1 - i));
					start_sub_tile_y &= ~(1UL << (tree_levels_ - 1 - i));

					if (node)
					{
						node = node->children[branches[i]];
				
						if (node && (node->data_index != EMPTY_DATA_INDEX))
						{
							uint32_t start_x = (start_sub_tile_x >> shift) * used_w * 2;
							uint32_t start_y = (start_sub_tile_y >> shift) * used_h * 2;
							uint8_t const * start_src = this->RetriveATile(node->data_index) + (start_y * tile_size_ + start_x) * texel_size_;
							uint8_t* dst = &temp[0];
							for (size_t y = 0; y < used_h * 2; ++ y)
							{
								uint8_t const * src = start_src + y * tile_size_ * texel_size_;
								for (size_t x = 0; x < used_w * 2; ++ x)
								{
									texel_op_.add(dst, src, dst);
									src += texel_size_;
									dst += texel_size_;
								}
							}
						}
					}

					tile_data.swap(temp);

					if (i > target_level - mipmaps)
					{
						std::memcpy(&data[target_level - i][0], &tile_data[0], tile_data.size() * sizeof(tile_data[0]));
					}
				}
			}
		}
	}

	uint32_t JudaTexture::DecodeAAttr(uint32_t shuff)
	{
		uint32_t target_level = this->ShuffLevel(shuff);

		uint32_t ret_attr = 0xFFFFFFFF;
		quadtree_node_ptr node = root_;
		if (0 == target_level)
		{
			ret_attr = root_->attr;
		}
		else
		{
			int step = (tree_levels_ - 1) % lower_levels_;
			if (0 == step)
			{
				step = lower_levels_;
			}

			std::vector<uint32_t> branches(tree_levels_);
			for (uint32_t i = 0; i <= tree_levels_ - 1; ++ i)
			{
				branches[i] = this->GetLevelBranch(shuff, i);
			}

			for (uint32_t ll = 1; ll <= target_level; ll += step)
			{
				uint32_t const ll_b = ll;
				if (ll > 1)
				{
					step = lower_levels_;
				}
				uint32_t const ll_e = ll + step;

				for (uint32_t i = ll_b, i_end = std::min(target_level + 1, ll_e); i < i_end; ++ i)
				{
					if (node)
					{
						node = node->children[branches[i]];
				
						if (node)
						{
							ret_attr = node->attr;
						}
					}
				}
			}
		}

		return ret_attr;
	}

	uint8_t* JudaTexture::RetriveATile(uint32_t data_index)
	{
		if (data_blocks_.empty())
		{
			auto iter = decoded_block_cache_.find(data_index);
			if (iter != decoded_block_cache_.end())
			{
				iter->second.tick = decode_tick_;
			}
			else
			{
				if (decoded_block_cache_.size() >= 64)
				{
					auto min_iter = decoded_block_cache_.begin();
					uint64_t min_tick = min_iter->second.tick;
					for (auto dbiter = decoded_block_cache_.begin();
						dbiter != decoded_block_cache_.end(); ++ dbiter)
					{
						if (dbiter->second.tick < min_tick)
						{
							min_tick = dbiter->second.tick;
							min_iter = dbiter;
						}
					}

					for (auto dbiter = decoded_block_cache_.begin();
						dbiter != decoded_block_cache_.end();)
					{
						if (dbiter->second.tick == min_tick)
						{
							dbiter = decoded_block_cache_.erase(dbiter);
						}
						else
						{
							++ dbiter;
						}
					}
				}

				uint32_t const full_tile_bytes = tile_size_ * tile_size_ * texel_size_;
				std::shared_ptr<std::vector<uint8_t>> data = MakeSharedPtr<std::vector<uint8_t>>(full_tile_bytes);
				if (data_index != EMPTY_DATA_INDEX)
				{
					uint64_t offsets[2];
					input_file_->seekg(data_blocks_offset_ + data_index * sizeof(uint64_t), std::ios_base::beg);
					input_file_->read(offsets, sizeof(offsets));
					uint32_t const comed_len = static_cast<uint32_t>(offsets[1] - offsets[0]);
					std::vector<uint8_t> comed_data(comed_len);
					input_file_->seekg(offsets[0], std::ios_base::beg);
					input_file_->read(&comed_data[0], comed_len);
					lzma_dec_.Decode(&(*data)[0], &comed_data[0], comed_len, full_tile_bytes);
				}
				else
				{
					memset(&(*data)[0], 0, full_tile_bytes);
				}

				iter = decoded_block_cache_.emplace(data_index, DecodedBlockInfo(data, decode_tick_)).first;
			}

			return &(*iter->second.data)[0];
		}
		else
		{
			return &data_blocks_[data_index][0];
		}
	}

	uint32_t JudaTexture::NumNonEmptySubNodes(quadtree_node_ptr const & node) const
	{
		uint32_t n = 0;
		if (node->data_index != EMPTY_DATA_INDEX)
		{
			++ n;
		}
		for (size_t i = 0; i < 4; ++ i)
		{
			if (node->children[i])
			{
				n += this->NumNonEmptySubNodes(node->children[i]);
			}
		}
		return n;
	}

	JudaTexture::quadtree_node_ptr const & JudaTexture::GetNode(uint32_t shuff)
	{
		uint32_t target_level = this->ShuffLevel(shuff);
		quadtree_node_ptr* node = &root_;
		for (uint32_t level = 1; level <= target_level; ++ level)
		{
			uint32_t branch = this->GetLevelBranch(shuff, level);
			if (*node)
			{
				node = &((*node)->children[branch]);
			}
			else
			{
				break;
			}
		}

		return *node;
	}

	JudaTexture::quadtree_node_ptr const & JudaTexture::AddNode(uint32_t shuff)
	{
		uint32_t target_level = this->ShuffLevel(shuff);
		quadtree_node_ptr* node = &root_;
		for (uint32_t level = 1; level <= target_level; ++ level)
		{
			uint32_t branch = this->GetLevelBranch(shuff, level);
			if (!(*node)->children[branch])
			{
				(*node)->children[branch] = MakeSharedPtr<quadtree_node>();
			}
			
			node = &((*node)->children[branch]);
		}

		return *node;
	}

	uint32_t JudaTexture::ShuffLevel(uint32_t shuff) const
	{
		return shuff >> LEVEL_SHIFT;
	}

	uint32_t JudaTexture::ShuffLevel(uint32_t shuff, uint32_t level) const
	{
		shuff &= ~(0xFU << LEVEL_SHIFT);
		shuff |= (level << LEVEL_SHIFT);
		return shuff;
	}

	uint32_t JudaTexture::Pos2Shuff(uint32_t level, uint32_t x, uint32_t y) const
	{
		uint32_t shuff = level << LEVEL_SHIFT;
		for (int l = level; l >= 0; -- l)
		{
			shuff |= (((y >> l) & 1UL) << (2 * ((MAX_TREE_LEVEL + 1) - tree_levels_ + l) + 1))
				| (((x >> l) & 1UL) << (2 * ((MAX_TREE_LEVEL + 1) - tree_levels_ + l) + 0));
		}
		return shuff;
	}

	void JudaTexture::Shuff2Pos(uint32_t& x, uint32_t& y, uint32_t level, uint32_t shuff) const
	{
		x = 0;
		y = 0;
		for (int l = level; l >= 0; -- l)
		{
			x |= (shuff >> (2 * ((MAX_TREE_LEVEL + 1) - tree_levels_ + l) + 0)) & (1UL << l);
			y |= (shuff >> (2 * ((MAX_TREE_LEVEL + 1) - tree_levels_ + l) + 1)) & (1UL << l);
		}
	}

	uint32_t JudaTexture::GetLevelBranch(uint32_t shuff, uint32_t level) const
	{
		return (shuff >> (2 * (MAX_TREE_LEVEL - level))) & 3UL;
	}

	uint32_t JudaTexture::SetLevelBranch(uint32_t shuff, uint32_t level, uint32_t bits) const
	{
		return (shuff & ~(3UL << (2 * (MAX_TREE_LEVEL - level)))) | (bits << (2 * (MAX_TREE_LEVEL - level)));
	}

	uint32_t JudaTexture::GetParentShuff(uint32_t shuff) const
	{
		return (0 == shuff) ? 0 : (shuff - ((1UL << LEVEL_SHIFT) | (shuff & (3UL << (2 * (MAX_TREE_LEVEL - this->ShuffLevel(shuff)))))));
	}

	uint32_t JudaTexture::GetChildShuff(uint32_t shuff, uint32_t branch) const
	{
		uint32_t level = this->ShuffLevel(shuff);
		return (shuff + (1UL << LEVEL_SHIFT)) | (branch << (2 * (MAX_TREE_LEVEL - level - 1)));
	}

	void JudaTexture::Upsample(uint8_t* output, uint8_t const * input, uint32_t in_width, uint32_t in_height, uint32_t in_pitch,
			uint32_t rx, uint32_t ry, uint32_t rw, uint32_t rh)
	{
		uint32_t out_width = rw * 2;
		uint32_t out_height = rh * 2;

		for (uint32_t y = 0; y < out_height; ++ y)
		{
			int iy = ry + y / 2;
			
			int lower_bound_y = 0;
			int upper_bound_y = in_height - 1;
			int iy0 = MathLib::clamp(iy + 0, lower_bound_y, upper_bound_y);

			for (uint32_t x = 0; x < out_width; ++ x)
			{
				int ix = rx + x / 2;

				int lower_bound_x = 0;
				int upper_bound_x = in_width - 1;
				int ix0 = MathLib::clamp(ix + 0, lower_bound_x, upper_bound_x);

				texel_op_.copy(&output[(y * out_width + x) * texel_size_], &input[iy0 * in_pitch + ix0 * texel_size_]);
			}
		}
	}

	void JudaTexture::Upsample(uint8_t* output, uint8_t const * input, uint32_t in_width, uint32_t in_height, uint32_t in_pitch)
	{
		uint32_t const out_width = in_width * 2;
		for (size_t y = 0; y < in_height; ++ y)
		{
			for (size_t x = 0; x < in_width; ++ x)
			{
				texel_op_.copy(&output[((y * 2 + 0) * out_width + (x * 2 + 0)) * texel_size_], &input[y * in_pitch + x * texel_size_]);
				texel_op_.copy(&output[((y * 2 + 0) * out_width + (x * 2 + 1)) * texel_size_], &input[y * in_pitch + x * texel_size_]);
				texel_op_.copy(&output[((y * 2 + 1) * out_width + (x * 2 + 0)) * texel_size_], &input[y * in_pitch + x * texel_size_]);
				texel_op_.copy(&output[((y * 2 + 1) * out_width + (x * 2 + 1)) * texel_size_], &input[y * in_pitch + x * texel_size_]);
			}
		}
	}

	void JudaTexture::Downsample(std::vector<uint8_t>& output, uint8_t const * input, uint32_t in_width, uint32_t in_height, uint32_t in_pitch)
	{
		uint32_t const out_width = in_width / 2;
		uint32_t const out_height = in_height / 2;

		float4 temp[4];
		output.resize(out_width * out_height * texel_size_);
		for (uint32_t y = 0; y < out_height; ++ y)
		{
			for (uint32_t x = 0; x < out_width; ++ x)
			{
				texel_op_.to_float4(&temp[0].x(), &input[(y * 2 + 0) * in_pitch + (x * 2 + 0) * texel_size_]);
				texel_op_.to_float4(&temp[1].x(), &input[(y * 2 + 0) * in_pitch + (x * 2 + 1) * texel_size_]);
				texel_op_.to_float4(&temp[2].x(), &input[(y * 2 + 1) * in_pitch + (x * 2 + 0) * texel_size_]);
				texel_op_.to_float4(&temp[3].x(), &input[(y * 2 + 1) * in_pitch + (x * 2 + 1) * texel_size_]);

				temp[0] = (temp[0] + temp[1] + temp[2] + temp[3]) * 0.25f;

				texel_op_.from_float4(&output[(y * out_width + x) * texel_size_], &temp[0].x());
			}
		}
	}

	uint32_t JudaTexture::AllocateDataBlock()
	{
		if (data_block_free_list_.empty())
		{
			data_block_free_list_.push_back(static_cast<uint32_t>(data_blocks_.size()));
			data_blocks_.push_back(std::vector<uint8_t>());
		}

		uint32_t ret = data_block_free_list_.front();
		data_block_free_list_.pop_front();
		return ret;
	}

	void JudaTexture::DeallocateDataBlock(uint32_t index)
	{
		auto iter = std::lower_bound(data_block_free_list_.begin(), data_block_free_list_.end(), index);
		data_block_free_list_.insert(iter, index);
		data_blocks_[index].clear();
	}


	JudaTexturePtr LoadJudaTexture(std::string const & file_name)
	{
		uint32_t num_tiles;
		uint32_t tile_size;
		ElementFormat format;

		ResIdentifierPtr file = ResLoader::Instance().Open(ResLoader::Instance().Locate(file_name));

		uint32_t fourcc;
		file->read(&fourcc, sizeof(fourcc));
		Verify(MakeFourCC<'J', 'D', 'T', ' '>::value == fourcc);

		uint32_t version;
		file->read(&version, sizeof(version));
		Verify(JUDA_TEX_VERSION == version);

		file->read(&num_tiles, sizeof(num_tiles));
		file->read(&tile_size, sizeof(tile_size));
		file->read(&format, sizeof(format));

		uint32_t non_empty_nodes;
		file->read(&non_empty_nodes, sizeof(non_empty_nodes));

		uint32_t num_image_entries;
		file->read(&num_image_entries, sizeof(num_image_entries));
		std::vector<JudaTexture::ImageEntry> image_entries(num_image_entries);
		for (uint32_t i = 0; i < num_image_entries; ++ i)
		{
			uint16_t len;
			file->read(&len, sizeof(len));
			image_entries[i].name.resize(len);
			file->read(&image_entries[i].name[0], len);

			file->read(&image_entries[i].x, sizeof(image_entries[i].x));
			file->read(&image_entries[i].y, sizeof(image_entries[i].y));
			file->read(&image_entries[i].w, sizeof(image_entries[i].w));
			file->read(&image_entries[i].h, sizeof(image_entries[i].h));

			file->read(&image_entries[i].addr_u_v, sizeof(image_entries[i].addr_u_v));

			file->read(&image_entries[i].border_clr, sizeof(image_entries[i].border_clr));
		}

		uint32_t data_blocks_offset;
		file->read(&data_blocks_offset, sizeof(data_blocks_offset));

		JudaTexturePtr ret = MakeSharedPtr<JudaTexture>(num_tiles, tile_size, format);

		uint32_t tree_levels = ret->TreeLevels();

		uint32_t data_index = 0;

		std::vector<JudaTexture::quadtree_node_ptr> last_level;
		std::vector<uint32_t> last_start_index_levels;
		for (size_t i = 0; i < tree_levels; ++ i)
		{
			uint32_t size;
			file->read(&size, sizeof(size));

			std::vector<JudaTexture::quadtree_node_ptr> this_level(size);
			std::vector<uint32_t> this_start_index_levels(size);
			file->read(&this_start_index_levels[0], size * sizeof(this_start_index_levels[0]));
			for (size_t j = 0; j < size; ++ j)
			{
				this_level[j] = MakeSharedPtr<JudaTexture::quadtree_node>();

				if (!(this_start_index_levels[j] >> 31))
				{
					this_level[j]->data_index = data_index;
					++ data_index;
				}
			}
			if (i == tree_levels - 1)
			{
				std::vector<uint32_t> this_attr_levels(size);
				file->read(&this_attr_levels[0], size * sizeof(this_attr_levels[0]));
				for (size_t j = 0; j < size; ++ j)
				{
					this_level[j]->attr = this_attr_levels[j];
				}
			}
			else
			{
				for (size_t j = 0; j < size; ++ j)
				{
					this_level[j]->attr = 0xFFFFFFFF;
				}
			}

			if (i > 0)
			{
				for (size_t j = 0; j < last_start_index_levels.size(); ++ j)
				{
					if ((last_start_index_levels[j] & 0x7FFFFFF0) != 0x7FFFFFF0)
					{
						uint32_t start_index = (last_start_index_levels[j] & 0x7FFFFFFF) >> 4;
						uint32_t mask = last_start_index_levels[j] & 0xF;

						int l = 0;
						for (size_t k = 0; k < 4; ++ k)
						{
							if (mask & (1UL << k))
							{
								last_level[j]->children[k] = this_level[start_index + l];
								++ l;
							}
						}
					}
				}
			}
			else
			{
				ret->root_ = this_level[0];
			}

			last_level.swap(this_level);
			last_start_index_levels.swap(this_start_index_levels);
		}

		ret->input_file_ = file;
		ret->data_blocks_offset_ = data_blocks_offset - (non_empty_nodes + 1) * sizeof(uint64_t);
		ret->image_entries_ = image_entries;

		return ret;
	}

	void SaveJudaTexture(JudaTexturePtr const & juda_tex, std::string const & file_name)
	{
		LZMACodec lzma_enc;

		std::vector<JudaTexture::quadtree_node_ptr> this_level;
		std::vector<JudaTexture::quadtree_node_ptr> next_level;

		std::shared_ptr<std::ostream> ofs = MakeSharedPtr<std::ofstream>(file_name.c_str(), std::ios_base::out | std::ios_base::binary);
		
		uint32_t fourcc = MakeFourCC<'J', 'D', 'T', ' '>::value;
		ofs->write(reinterpret_cast<char const *>(&fourcc), sizeof(fourcc));

		uint32_t version = JUDA_TEX_VERSION;
		ofs->write(reinterpret_cast<char const *>(&version), sizeof(version));

		ofs->write(reinterpret_cast<char const *>(&juda_tex->num_tiles_), sizeof(juda_tex->num_tiles_));
		ofs->write(reinterpret_cast<char const *>(&juda_tex->tile_size_), sizeof(juda_tex->tile_size_));
		ofs->write(reinterpret_cast<char const *>(&juda_tex->format_), sizeof(juda_tex->format_));

		uint32_t non_empty_nodes = juda_tex->NumNonEmptyNodes();
		ofs->write(reinterpret_cast<char const *>(&non_empty_nodes), sizeof(non_empty_nodes));

		uint32_t num_image_entries = static_cast<uint32_t>(juda_tex->image_entries_.size());
		ofs->write(reinterpret_cast<char const *>(&num_image_entries), sizeof(num_image_entries));
		for (uint32_t i = 0; i < num_image_entries; ++ i)
		{
			uint16_t len = static_cast<uint16_t>(juda_tex->image_entries_[i].name.size());
			ofs->write(reinterpret_cast<char const *>(&len), sizeof(len));
			ofs->write(juda_tex->image_entries_[i].name.c_str(), len);

			ofs->write(reinterpret_cast<char const *>(&juda_tex->image_entries_[i].x), sizeof(juda_tex->image_entries_[i].x));
			ofs->write(reinterpret_cast<char const *>(&juda_tex->image_entries_[i].y), sizeof(juda_tex->image_entries_[i].y));
			ofs->write(reinterpret_cast<char const *>(&juda_tex->image_entries_[i].w), sizeof(juda_tex->image_entries_[i].w));
			ofs->write(reinterpret_cast<char const *>(&juda_tex->image_entries_[i].h), sizeof(juda_tex->image_entries_[i].h));

			ofs->write(reinterpret_cast<char const *>(&juda_tex->image_entries_[i].addr_u_v), sizeof(juda_tex->image_entries_[i].addr_u_v));

			ofs->write(reinterpret_cast<char const *>(&juda_tex->image_entries_[i].border_clr), sizeof(juda_tex->image_entries_[i].border_clr));
		}

		std::ostream::pos_type data_blocks_offset_pos = ofs->tellp();
		
		uint32_t data_blocks_offset = 0;
		ofs->write(reinterpret_cast<char const *>(&data_blocks_offset), sizeof(data_blocks_offset));

		std::vector<uint32_t> non_empty_block_data_index;
		non_empty_block_data_index.reserve(non_empty_nodes);

		this_level.push_back(juda_tex->root_);
		for (size_t i = 0; i < juda_tex->TreeLevels(); ++ i)
		{
			uint32_t size = static_cast<uint32_t>(this_level.size());
			ofs->write(reinterpret_cast<char const *>(&size), sizeof(size));

			uint32_t num_nodes = 0;
			next_level.reserve(this_level.size() * 4);
			for (size_t j = 0; j < this_level.size(); ++ j)
			{
				uint32_t index;
				if (!this_level[j]->children[0]
					&& !this_level[j]->children[1]
					&& !this_level[j]->children[2]
					&& !this_level[j]->children[3])
				{
					index = 0x7FFFFFFF;
				}
				else
				{
					BOOST_ASSERT(num_nodes <= 0x07FFFFFF);

					index = num_nodes << 4;

					for (size_t k = 0; k < 4; ++ k)
					{
						if (this_level[j]->children[k])
						{
							next_level.push_back(this_level[j]->children[k]);
							index |= (1UL << k);
							++ num_nodes;
						}
					}
				}
				if (JudaTexture::EMPTY_DATA_INDEX == this_level[j]->data_index)
				{
					index |= 1UL << 31;
				}
				else
				{
					non_empty_block_data_index.push_back(this_level[j]->data_index);
				}
				ofs->write(reinterpret_cast<char const *>(&index), sizeof(index));
			}

			if (i == juda_tex->TreeLevels() - 1)
			{
				for (size_t j = 0; j < this_level.size(); ++ j)
				{
					ofs->write(reinterpret_cast<char const *>(&this_level[j]->attr), sizeof(this_level[j]->attr));
				}
			}

			this_level.clear();
			this_level.swap(next_level);
		}

		uint32_t block_start_offset_pos = static_cast<uint32_t>(ofs->tellp());
		data_blocks_offset = block_start_offset_pos + (non_empty_nodes + 1) * sizeof(uint64_t);

		std::vector<uint64_t> block_start_pos(non_empty_nodes + 1);
		ofs->write(reinterpret_cast<char const *>(&block_start_pos[0]), block_start_pos.size() * sizeof(block_start_pos[0]));

		block_start_pos[0] = data_blocks_offset;
		for (size_t i = 0; i < non_empty_block_data_index.size(); ++ i)
		{
			std::vector<uint8_t> const & data = juda_tex->data_blocks_[non_empty_block_data_index[i]];

			std::vector<uint8_t> comed_data;
			lzma_enc.Encode(comed_data, &data[0], data.size());

			uint32_t comed_len = static_cast<uint32_t>(comed_data.size());
			block_start_pos[i + 1] = block_start_pos[i] + comed_len;

			ofs->write(reinterpret_cast<char const *>(&comed_data[0]), comed_data.size() * sizeof(comed_data[0]));
		}

		ofs->seekp(data_blocks_offset_pos, std::ios_base::beg);
		ofs->write(reinterpret_cast<char const *>(&data_blocks_offset), sizeof(data_blocks_offset));

		ofs->seekp(block_start_offset_pos, std::ios_base::beg);
		ofs->write(reinterpret_cast<char const *>(&block_start_pos[0]), block_start_pos.size() * sizeof(block_start_pos[0]));
	}

	void JudaTexture::CacheProperty(uint32_t pages, ElementFormat format, uint32_t border_size, uint32_t cache_tile_size)
	{
		if (!tex_cache_ && tex_cache_array_.empty())
		{
			pages = std::min<uint32_t>(pages, 1024U);
			uint32_t sqrt_pages = static_cast<uint32_t>(sqrt(static_cast<float>(pages)) + 0.5f);

			cache_tile_border_size_ = border_size;
			cache_tile_size_ = (0 == cache_tile_size) ? tile_size_ : cache_tile_size;

			uint32_t const scale = tile_size_ / cache_tile_size_;
			BOOST_ASSERT(scale * cache_tile_size_ == tile_size_);
			BOOST_ASSERT(0 == (scale & (scale - 1)));
			KFL_UNUSED(scale);

			RenderFactory& rf = Context::Instance().RenderFactoryInstance();

			uint32_t tile_with_border_size = cache_tile_size_ + cache_tile_border_size_ * 2;
			uint32_t mipmap = 0;
			while (cache_tile_border_size_ > (1UL << mipmap))
			{
				++ mipmap;
			}
			if (IsCompressedFormat(format))
			{
				switch (format)
				{
				case EF_BC1:
					tex_codec_ = MakeUniquePtr<TexCompressionBC1>();
					break;

				case EF_BC2:
					tex_codec_ = MakeUniquePtr<TexCompressionBC2>();
					break;

				case EF_BC3:
					tex_codec_ = MakeUniquePtr<TexCompressionBC3>();
					break;

				default:
					KFL_UNREACHABLE("Not supported compression format");
				}

				// BC format must be multiply of 4
				while (((tile_with_border_size >> mipmap) & 0x3) != 0)
				{
					-- mipmap;
				}
			}
			++ mipmap;

			RenderDeviceCaps const & caps = rf.RenderEngineInstance().DeviceCaps();

			uint32_t s = std::min(sqrt_pages, std::min(caps.max_texture_width, caps.max_texture_height) / tile_with_border_size);
			uint32_t array_size = std::min(std::min((pages + s * s - 1) / (s * s), static_cast<uint32_t>(caps.max_pixel_texture_units - 1)), 7U);
			if (caps.max_texture_array_length > array_size)
			{
				tex_cache_ = rf.MakeTexture2D(tile_with_border_size * s, tile_with_border_size * s, mipmap,
					std::max(array_size, 2U), format, 1, 0, EAH_GPU_Read);
			}
			else
			{
				tex_cache_array_.resize(array_size);
				for (uint32_t i = 0; i < array_size; ++ i)
				{
					tex_cache_array_[i] = rf.MakeTexture2D(tile_with_border_size * s, tile_with_border_size * s, mipmap, 1, format, 1, 0,
						EAH_GPU_Read);
				}
			}

			tex_indirect_ = rf.MakeTexture2D(num_tiles_, num_tiles_, 1, 1, EF_ABGR8, 1, 0, EAH_GPU_Read);

			tile_free_list_.emplace_back(0, pages);
		}
	}

	TexturePtr const & JudaTexture::CacheTex() const
	{
		return tex_cache_;
	}

	std::vector<TexturePtr> const & JudaTexture::CacheTexArray() const
	{
		return tex_cache_array_;
	}

	TexturePtr const & JudaTexture::IndirectTex() const
	{
		return tex_indirect_;
	}

	void JudaTexture::SetParams(RenderEffect const & effect)
	{
		if (tex_cache_)
		{
			*(effect.ParameterByName("juda_tex_cache")) = tex_cache_;
			*(effect.ParameterByName("inv_juda_tex_cache_size")) = float2(1.0f / tex_cache_->Width(0), 1.0f / tex_cache_->Height(0));
		}
		else
		{
			for (size_t i = 0; i < tex_cache_array_.size(); ++ i)
			{
				*(effect.ParameterByName("juda_tex_cache_" + boost::lexical_cast<std::string>(i)))
					= tex_cache_array_[i];
			}
			*(effect.ParameterByName("inv_juda_tex_cache_size")) = float2(1.0f / tex_cache_array_[0]->Width(0), 1.0f / tex_cache_array_[0]->Height(0));
		}

		*(effect.ParameterByName("juda_tex_indirect")) = tex_indirect_;
		*(effect.ParameterByName("inv_juda_tex_indirect_size")) = float2(1.0f / tex_indirect_->Width(0), 1.0f / tex_indirect_->Height(0));

		*(effect.ParameterByName("tile_size")) = float3(static_cast<float>(cache_tile_size_),
				static_cast<float>(cache_tile_size_ + cache_tile_border_size_ * 2),
				static_cast<float>(cache_tile_border_size_));
	}

	void JudaTexture::UpdateCache(std::vector<uint32_t> const & tile_ids)
	{
		BOOST_ASSERT(tex_cache_ || !tex_cache_array_.empty());

		++ tile_tick_;

		uint32_t const tex_width = tex_cache_ ? tex_cache_->Width(0) : tex_cache_array_[0]->Width(0);
		uint32_t const tex_height = tex_cache_ ? tex_cache_->Height(0) : tex_cache_array_[0]->Height(0);
		uint32_t const tex_layer = tex_cache_ ? tex_cache_->ArraySize() : static_cast<uint32_t>(tex_cache_array_.size());
		uint32_t const tile_with_border_size = cache_tile_size_ + cache_tile_border_size_ * 2;

		uint32_t const num_cache_tiles_a_row = tex_width / tile_with_border_size;
		uint32_t const num_cache_tiles_a_layer = num_cache_tiles_a_row * tex_height / tile_with_border_size;
		uint32_t const num_cache_total_tiles = num_cache_tiles_a_layer * tex_layer;

		std::unordered_map<uint32_t, uint32_t> neighbor_id_map;
		std::vector<uint32_t> all_neighbor_ids;
		std::vector<uint32_t> neighbor_ids;
		std::vector<uint32_t> tile_attrs;
		std::vector<bool> in_same_image;
		auto& tim = tile_info_map_;
		for (size_t i = 0; i < tile_ids.size(); ++ i)
		{
			auto tmiter = tim.find(tile_ids[i]);
			if (tmiter != tim.end())
			{
				// Exists in cache

				tmiter->second.tick = tile_tick_;
			}
			else
			{
				uint32_t level, tile_x, tile_y;
				this->DecodeTileID(level, tile_x, tile_y, tile_ids[i]);

				std::array<uint32_t, 9> new_tile_id_with_neighbors;
				new_tile_id_with_neighbors.fill(0xFFFFFFFF);
				new_tile_id_with_neighbors[0] = tile_ids[i];

				std::array<bool, 9> new_in_same_image;
				new_in_same_image.fill(false);
				new_in_same_image[0] = true;

				uint32_t attr = this->DecodeAAttr(this->Pos2Shuff(level, tile_x, tile_y));
				tile_attrs.push_back(attr);
				if (attr != 0xFFFFFFFF)
				{
					std::array<int32_t, 9> new_tile_id_x;
					std::array<int32_t, 9> new_tile_id_y;

					int32_t left = tile_x - 1;
					int32_t right = tile_x + 1;
					int32_t up = tile_y - 1;
					int32_t down = tile_y + 1;

					ImageEntry const & entry = image_entries_[attr];
					if (TAM_Wrap == (entry.addr_u_v & 0xF))
					{
						left = entry.x + (left - entry.x + entry.w) % entry.w;
						right = entry.x + (right - entry.x + entry.w) % entry.w;
					}
					if (TAM_Wrap == ((entry.addr_u_v >> 4) & 0xF))
					{
						up = entry.y + (up - entry.y + entry.h) % entry.h;
						down = entry.y + (down - entry.y + entry.h) % entry.h;
					}

					new_tile_id_x[1] = left;
					new_tile_id_y[1] = up;
					new_tile_id_x[2] = tile_x;
					new_tile_id_y[2] = up;
					new_tile_id_x[3] = right;
					new_tile_id_y[3] = up;

					new_tile_id_x[4] = left;
					new_tile_id_y[4] = tile_y;
					new_tile_id_x[5] = right;
					new_tile_id_y[5] = tile_y;

					new_tile_id_x[6] = left;
					new_tile_id_y[6] = down;
					new_tile_id_x[7] = tile_x;
					new_tile_id_y[7] = down;
					new_tile_id_x[8] = right;
					new_tile_id_y[8] = down;

					for (int j = 1; j < 9; ++ j)
					{
						if ((new_tile_id_x[j] >= 0) && (new_tile_id_y[j] >= 0)
							&& (new_tile_id_x[j] < static_cast<int32_t>(num_tiles_) - 1)
							&& (new_tile_id_y[j] < static_cast<int32_t>(num_tiles_) - 1))
						{
							new_tile_id_with_neighbors[j] = this->EncodeTileID(level, new_tile_id_x[j], new_tile_id_y[j]);
							if (new_tile_id_with_neighbors[j] != 0xFFFFFFFF)
							{
								if (attr == this->DecodeAAttr(this->Pos2Shuff(level, new_tile_id_x[j], new_tile_id_y[j])))
								{
									new_in_same_image[j] = true;
								}
							}
						}
						else
						{
							new_tile_id_with_neighbors[j] = 0xFFFFFFFF;
						}
					}
				}

				for (size_t j = 0; j < new_tile_id_with_neighbors.size(); ++ j)
				{
					if (new_tile_id_with_neighbors[j] != 0xFFFFFFFF)
					{
						if (neighbor_id_map.find(new_tile_id_with_neighbors[j]) == neighbor_id_map.end())
						{
							neighbor_id_map.emplace(new_tile_id_with_neighbors[j], static_cast<uint32_t>(neighbor_ids.size()));
							neighbor_ids.push_back(new_tile_id_with_neighbors[j]);
						}
					}
					all_neighbor_ids.push_back(new_tile_id_with_neighbors[j]);
					in_same_image.push_back(new_in_same_image[j]);
				}
			}
		}

		uint32_t mipmaps = tex_cache_->NumMipMaps();
		std::vector<std::vector<uint8_t>> neighbor_data;
		this->DecodeTiles(neighbor_data, neighbor_ids, mipmaps);

		TileInfo tile_info;
		tile_info.tick = tile_tick_;
		for (size_t i = 0; i < all_neighbor_ids.size(); i += 9)
		{
			tile_info.attr = tile_attrs[i / 9];
			uint8_t border_clr[4];
			TexAddressingMode addr_u, addr_v;
			if (tile_info.attr != 0xFFFFFFFF)
			{
				ImageEntry const & entry = image_entries_[tile_info.attr];
				addr_u = static_cast<TexAddressingMode>(entry.addr_u_v & 0xF);
				addr_v = static_cast<TexAddressingMode>((entry.addr_u_v >> 4) & 0xF);
				texel_op_.from_float4(border_clr, &entry.border_clr.r());
			}
			else
			{
				addr_u = TAM_Clamp;
				addr_v = TAM_Clamp;
				border_clr[0] = border_clr[1] = border_clr[2] = border_clr[3] = 0;
			}

			if (tile_info_map_.size() < num_cache_total_tiles)
			{
				// Still has space in cache

				uint32_t const s = tile_free_list_.front().first;
				tile_info.z = s / num_cache_tiles_a_layer;
				tile_info.y = (s - tile_info.z * num_cache_tiles_a_layer) / num_cache_tiles_a_row;
				tile_info.x = s - tile_info.z * num_cache_tiles_a_layer - tile_info.y * num_cache_tiles_a_row;

				++ tile_free_list_.front().first;
				if (tile_free_list_.front().first == tile_free_list_.front().second)
				{
					tile_free_list_.pop_front();
				}
			}
			else
			{
				// Find tiles that are not used for the longest time

				uint64_t min_tick = tim.begin()->second.tick;
				auto min_tileiter = tim.begin();
				for (auto tileiter = tim.begin(); tileiter != tim.end(); ++ tileiter)
				{
					if (tileiter->second.tick < min_tick)
					{
						min_tick = tileiter->second.tick;
						min_tileiter = tileiter;
					}
				}

				tile_info.x = min_tileiter->second.x;
				tile_info.y = min_tileiter->second.y;
				tile_info.z = min_tileiter->second.z;

				for (auto tileiter = tim.begin(); tileiter != tim.end();)
				{
					if (tileiter->second.tick == min_tick)
					{
						uint32_t const id = tileiter->second.z * num_cache_tiles_a_layer + tileiter->second.y * num_cache_tiles_a_row + tileiter->second.x;
						auto freeiter = tile_free_list_.begin();
						while ((freeiter != tile_free_list_.end()) && (freeiter->second <= id))
						{
							++ freeiter;
						}
						tile_free_list_.emplace(freeiter, id, id + 1);

						tileiter = tim.erase(tileiter);
					}
					else
					{
						 ++ tileiter;
					}
				}
				for (auto freeiter = tile_free_list_.begin(); freeiter != tile_free_list_.end() - 1;)
				{
					auto nextiter = freeiter;
					++ nextiter;

					if (freeiter->second == nextiter->first)
					{
						freeiter->second = nextiter->second;
						freeiter = tile_free_list_.erase(nextiter);
						-- freeiter;
					}
					else
					{
						++ freeiter;
					}
				}
			}

			std::array<uint32_t, 9> index_with_neighbors = { { 0 } };
			for (size_t j = 0; j < index_with_neighbors.size(); ++ j)
			{
				if (all_neighbor_ids[i + j] != 0xFFFFFFFF)
				{
					BOOST_ASSERT(neighbor_id_map.find(all_neighbor_ids[i + j]) != neighbor_id_map.end());

					index_with_neighbors[j] = neighbor_id_map[all_neighbor_ids[i + j]];
				}
				else
				{
					index_with_neighbors[j] = 0xFFFFFFFF;
				}
			}
			BOOST_ASSERT(index_with_neighbors[0] != 0xFFFFFFFF);

			uint32_t mip_tile_size = cache_tile_size_;
			uint32_t mip_tile_with_border_size = tile_with_border_size;
			uint32_t mip_border_size = cache_tile_border_size_;
			for (uint32_t l = 0; l < mipmaps; ++ l)
			{
#if defined(KLAYGE_COMPILER_MSVC)
				std::array<uint8_t const *, 9> neighbor_data_ptr{};
#else
				std::array<uint8_t const *, 9> neighbor_data_ptr;
#endif
				for (uint32_t j = 0; j < neighbor_data_ptr.size(); ++ j)
				{
					if (index_with_neighbors[j] != 0xFFFFFFFF)
					{
						neighbor_data_ptr[j] = &neighbor_data[index_with_neighbors[j] * mipmaps + l][0];
					}
					else
					{
						neighbor_data_ptr[j] = nullptr;
					}
				}

				std::vector<uint8_t> tex_a_tile_data(mip_tile_with_border_size * mip_tile_with_border_size * texel_size_);
				{
					uint8_t* data_with_border = &tex_a_tile_data[0];
					uint32_t const data_pitch = mip_tile_with_border_size * texel_size_;
				
					for (uint32_t y = 0; y < mip_tile_size; ++ y)
					{
						texel_op_.copy_array(data_with_border + (y + mip_border_size) * data_pitch + mip_border_size * texel_size_,
							neighbor_data_ptr[0] + y * mip_tile_size * texel_size_, mip_tile_size);
					}

					if ((neighbor_data_ptr[1] != nullptr) && in_same_image[i + 1])
					{
						for (uint32_t y = 0; y < mip_border_size; ++ y)
						{
							texel_op_.copy_array(data_with_border + y * data_pitch,
								neighbor_data_ptr[1] + ((y + mip_tile_size - mip_border_size) * mip_tile_size + (mip_tile_size - mip_border_size)) * texel_size_,
								mip_border_size);
						}
					}
					else
					{
						if (tile_info.attr != 0xFFFFFFFF)
						{
							std::vector<int32_t> border_coords_x(mip_border_size * mip_border_size);
							std::vector<int32_t> border_coords_y(mip_border_size * mip_border_size);
							switch (addr_u)
							{
							case TAM_Mirror:
								for (uint32_t y = 0; y < mip_border_size; ++ y)
								{
									for (uint32_t x = 0; x < mip_border_size; ++ x)
									{
										border_coords_x[y * mip_border_size + x] = mip_border_size - 1 - x;
									}
								}
								break;

							case TAM_Clamp:
								for (uint32_t y = 0; y < mip_border_size; ++ y)
								{
									for (uint32_t x = 0; x < mip_border_size; ++ x)
									{
										border_coords_x[y * mip_border_size + x] = 0;
									}
								}
								break;

							case TAM_Border:
								for (uint32_t y = 0; y < mip_border_size; ++ y)
								{
									for (uint32_t x = 0; x < mip_border_size; ++ x)
									{
										border_coords_x[y * mip_border_size + x] = -1;
									}
								}
								break;

							default:
								KFL_UNREACHABLE("Invalid texture addressing mode");
							}
							switch (addr_v)
							{
							case TAM_Mirror:
								for (uint32_t y = 0; y < mip_border_size; ++ y)
								{
									for (uint32_t x = 0; x < mip_border_size; ++ x)
									{
										border_coords_y[y * mip_border_size + x] = mip_border_size - 1 - y;
									}
								}
								break;

							case TAM_Clamp:
								for (uint32_t y = 0; y < mip_border_size; ++ y)
								{
									for (uint32_t x = 0; x < mip_border_size; ++ x)
									{
										border_coords_y[y * mip_border_size + x] = 0;
									}
								}
								break;

							case TAM_Border:
								for (uint32_t y = 0; y < mip_border_size; ++ y)
								{
									for (uint32_t x = 0; x < mip_border_size; ++ x)
									{
										border_coords_y[y * mip_border_size + x] = -1;
									}
								}
								break;

							default:
								KFL_UNREACHABLE("Invalid texture addressing mode");
							}

							for (uint32_t y = 0; y < mip_border_size; ++ y)
							{
								for (uint32_t x = 0; x < mip_border_size; ++ x)
								{
									if ((border_coords_x[y * mip_border_size + x] >= 0) && (border_coords_y[y * mip_border_size + x] >= 0))
									{
										texel_op_.copy(data_with_border + y * data_pitch + x * texel_size_,
											neighbor_data_ptr[0] + border_coords_y[y * mip_border_size + x] * mip_tile_with_border_size + border_coords_x[y * mip_border_size + x]);
									}
									else
									{
										texel_op_.copy(data_with_border + y * data_pitch + x * texel_size_, border_clr);
									}
								}
							}
						}
						else
						{
							for (uint32_t y = 0; y < mip_border_size; ++ y)
							{
								for (uint32_t x = 0; x < mip_border_size; ++ x)
								{
									texel_op_.copy(data_with_border + y * data_pitch + x * texel_size_,
											neighbor_data_ptr[0]);
								}
							}
						}
					}
					if ((neighbor_data_ptr[2] != nullptr) && in_same_image[i + 2])
					{
						for (uint32_t y = 0; y < mip_border_size; ++ y)
						{
							texel_op_.copy_array(data_with_border + y * data_pitch + mip_border_size * texel_size_,
								neighbor_data_ptr[2] + ((y + mip_tile_size - mip_border_size) * mip_tile_size) * texel_size_, mip_tile_size);
						}
					}
					else
					{
						if (tile_info.attr != 0xFFFFFFFF)
						{
							std::vector<int32_t> border_coords_x(mip_tile_size * mip_border_size);
							std::vector<int32_t> border_coords_y(mip_tile_size * mip_border_size);
							switch (addr_u)
							{
							case TAM_Mirror:
								for (uint32_t y = 0; y < mip_border_size; ++ y)
								{
									for (uint32_t x = 0; x < mip_tile_size; ++ x)
									{
										border_coords_x[y * mip_tile_size + x] = x;
									}
								}
								break;

							case TAM_Clamp:
								for (uint32_t y = 0; y < mip_border_size; ++ y)
								{
									for (uint32_t x = 0; x < mip_tile_size; ++ x)
									{
										border_coords_x[y * mip_tile_size + x] = x;
									}
								}
								break;

							case TAM_Border:
								for (uint32_t y = 0; y < mip_border_size; ++ y)
								{
									for (uint32_t x = 0; x < mip_tile_size; ++ x)
									{
										border_coords_x[y * mip_tile_size + x] = -1;
									}
								}
								break;

							default:
								KFL_UNREACHABLE("Invalid texture addressing mode");
							}
							switch (addr_v)
							{
							case TAM_Mirror:
								for (uint32_t y = 0; y < mip_border_size; ++ y)
								{
									for (uint32_t x = 0; x < mip_tile_size; ++ x)
									{
										border_coords_y[y * mip_tile_size + x] = mip_border_size - 1 - y;
									}
								}
								break;

							case TAM_Clamp:
								for (uint32_t y = 0; y < mip_border_size; ++ y)
								{
									for (uint32_t x = 0; x < mip_tile_size; ++ x)
									{
										border_coords_y[y * mip_tile_size + x] = 0;
									}
								}
								break;

							case TAM_Border:
								for (uint32_t y = 0; y < mip_border_size; ++ y)
								{
									for (uint32_t x = 0; x < mip_tile_size; ++ x)
									{
										border_coords_y[y * mip_tile_size + x] = -1;
									}
								}
								break;

							default:
								KFL_UNREACHABLE("Invalid texture addressing mode");
							}

							for (uint32_t y = 0; y < mip_border_size; ++ y)
							{
								for (uint32_t x = 0; x < mip_tile_size; ++ x)
								{
									if ((border_coords_x[y * mip_tile_size + x] >= 0) && (border_coords_y[y * mip_tile_size + x] >= 0))
									{
										texel_op_.copy(data_with_border + y * data_pitch + x * texel_size_,
											neighbor_data_ptr[0] + border_coords_y[y * mip_tile_size + x] * mip_tile_with_border_size + border_coords_x[y * mip_tile_size + x]);
									}
									else
									{
										texel_op_.copy(data_with_border + y * data_pitch + x * texel_size_, border_clr);
									}
								}
							}
						}
						else
						{
							for (uint32_t y = 0; y < mip_border_size; ++ y)
							{
								texel_op_.copy_array(data_with_border + y * data_pitch + mip_border_size * texel_size_,
									neighbor_data_ptr[0], mip_tile_size);
							}
						}
					}
					if ((neighbor_data_ptr[3] != nullptr) && in_same_image[i + 3])
					{
						for (uint32_t y = 0; y < mip_border_size; ++ y)
						{
							texel_op_.copy_array(data_with_border + y * data_pitch + (mip_border_size + mip_tile_size) * texel_size_,
								neighbor_data_ptr[3] + (y + mip_tile_size - mip_border_size) * mip_tile_size * texel_size_, mip_border_size);
						}
					}
					else
					{
						if (tile_info.attr != 0xFFFFFFFF)
						{
							std::vector<int32_t> border_coords_x(mip_border_size * mip_border_size);
							std::vector<int32_t> border_coords_y(mip_border_size * mip_border_size);
							switch (addr_u)
							{
							case TAM_Mirror:
								for (uint32_t y = 0; y < mip_border_size; ++ y)
								{
									for (uint32_t x = 0; x < mip_border_size; ++ x)
									{
										border_coords_x[y * mip_border_size + x] = mip_tile_size - 1 - x;
									}
								}
								break;

							case TAM_Clamp:
								for (uint32_t y = 0; y < mip_border_size; ++ y)
								{
									for (uint32_t x = 0; x < mip_border_size; ++ x)
									{
										border_coords_x[y * mip_border_size + x] = mip_tile_size - 1;
									}
								}
								break;

							case TAM_Border:
								for (uint32_t y = 0; y < mip_border_size; ++ y)
								{
									for (uint32_t x = 0; x < mip_border_size; ++ x)
									{
										border_coords_x[y * mip_border_size + x] = -1;
									}
								}
								break;

							default:
								KFL_UNREACHABLE("Invalid texture addressing mode");
							}
							switch (addr_v)
							{
							case TAM_Mirror:
								for (uint32_t y = 0; y < mip_border_size; ++ y)
								{
									for (uint32_t x = 0; x < mip_border_size; ++ x)
									{
										border_coords_y[y * mip_border_size + x] = mip_border_size - 1 - y;
									}
								}
								break;

							case TAM_Clamp:
								for (uint32_t y = 0; y < mip_border_size; ++ y)
								{
									for (uint32_t x = 0; x < mip_border_size; ++ x)
									{
										border_coords_y[y * mip_border_size + x] = 0;
									}
								}
								break;

							case TAM_Border:
								for (uint32_t y = 0; y < mip_border_size; ++ y)
								{
									for (uint32_t x = 0; x < mip_border_size; ++ x)
									{
										border_coords_y[y * mip_border_size + x] = -1;
									}
								}
								break;

							default:
								KFL_UNREACHABLE("Invalid texture addressing mode");
							}

							for (uint32_t y = 0; y < mip_border_size; ++ y)
							{
								for (uint32_t x = 0; x < mip_border_size; ++ x)
								{
									if ((border_coords_x[y * mip_border_size + x] >= 0) && (border_coords_y[y * mip_border_size + x] >= 0))
									{
										texel_op_.copy(data_with_border + y * data_pitch + x * texel_size_,
											neighbor_data_ptr[0] + border_coords_y[y * mip_border_size + x] * mip_tile_with_border_size + border_coords_x[y * mip_border_size + x]);
									}
									else
									{
										texel_op_.copy(data_with_border + y * data_pitch + x * texel_size_, border_clr);
									}
								}
							}
						}
						else
						{
							for (uint32_t y = 0; y < mip_border_size; ++ y)
							{
								for (uint32_t x = 0; x < mip_border_size; ++ x)
								{
									texel_op_.copy(data_with_border + y * data_pitch + (x + mip_border_size + mip_tile_size) * texel_size_,
										neighbor_data_ptr[0] + (mip_tile_size - 1) * texel_size_);
								}
							}
						}
					}

					if ((neighbor_data_ptr[4] != nullptr) && in_same_image[i + 4])
					{
						for (uint32_t y = 0; y < mip_tile_size; ++ y)
						{
							texel_op_.copy_array(data_with_border + (y + mip_border_size) * data_pitch,
								neighbor_data_ptr[4] + (y * mip_tile_size + (mip_tile_size - mip_border_size)) * texel_size_, mip_border_size);
						}
					}
					else
					{
						if (tile_info.attr != 0xFFFFFFFF)
						{
							std::vector<int32_t> border_coords_x(mip_border_size * mip_tile_size);
							std::vector<int32_t> border_coords_y(mip_border_size * mip_tile_size);
							switch (addr_u)
							{
							case TAM_Mirror:
								for (uint32_t y = 0; y < mip_tile_size; ++ y)
								{
									for (uint32_t x = 0; x < mip_border_size; ++ x)
									{
										border_coords_x[y * mip_border_size + x] = mip_border_size - 1 - x;
									}
								}
								break;

							case TAM_Clamp:
								for (uint32_t y = 0; y < mip_tile_size; ++ y)
								{
									for (uint32_t x = 0; x < mip_border_size; ++ x)
									{
										border_coords_x[y * mip_border_size + x] = 0;
									}
								}
								break;

							case TAM_Border:
								for (uint32_t y = 0; y < mip_tile_size; ++ y)
								{
									for (uint32_t x = 0; x < mip_border_size; ++ x)
									{
										border_coords_x[y * mip_border_size + x] = -1;
									}
								}
								break;

							default:
								KFL_UNREACHABLE("Invalid texture addressing mode");
							}
							switch (addr_v)
							{
							case TAM_Mirror:
								for (uint32_t y = 0; y < mip_tile_size; ++ y)
								{
									for (uint32_t x = 0; x < mip_border_size; ++ x)
									{
										border_coords_y[y * mip_border_size + x] = y;
									}
								}
								break;

							case TAM_Clamp:
								for (uint32_t y = 0; y < mip_tile_size; ++ y)
								{
									for (uint32_t x = 0; x < mip_border_size; ++ x)
									{
										border_coords_y[y * mip_border_size + x] = y;
									}
								}
								break;

							case TAM_Border:
								for (uint32_t y = 0; y < mip_tile_size; ++ y)
								{
									for (uint32_t x = 0; x < mip_border_size; ++ x)
									{
										border_coords_y[y * mip_border_size + x] = -1;
									}
								}
								break;

							default:
								KFL_UNREACHABLE("Invalid texture addressing mode");
							}

							for (uint32_t y = 0; y < mip_tile_size; ++ y)
							{
								for (uint32_t x = 0; x < mip_border_size; ++ x)
								{
									if ((border_coords_x[y * mip_border_size + x] >= 0) && (border_coords_y[y * mip_border_size + x] >= 0))
									{
										texel_op_.copy(data_with_border + y * data_pitch + x * texel_size_,
											neighbor_data_ptr[0] + border_coords_y[y * mip_border_size + x] * mip_tile_with_border_size + border_coords_x[y * mip_border_size + x]);
									}
									else
									{
										texel_op_.copy(data_with_border + y * data_pitch + x * texel_size_, border_clr);
									}
								}
							}
						}
						else
						{
							for (uint32_t y = 0; y < mip_tile_size; ++ y)
							{
								for (uint32_t x = 0; x < mip_border_size; ++ x)
								{
									texel_op_.copy(data_with_border + (y + mip_border_size) * data_pitch + x * texel_size_,
										neighbor_data_ptr[0] + y * mip_tile_size * texel_size_);
								}
							}
						}
					}
					if ((neighbor_data_ptr[5] != nullptr) && in_same_image[i + 5])
					{
						for (uint32_t y = 0; y < mip_tile_size; ++ y)
						{
							texel_op_.copy_array(data_with_border + (y + mip_border_size) * data_pitch + (mip_border_size + mip_tile_size) * texel_size_,
								neighbor_data_ptr[5] + y * mip_tile_size * texel_size_, mip_border_size);
						}
					}
					else
					{
						if (tile_info.attr != 0xFFFFFFFF)
						{
							std::vector<int32_t> border_coords_x(mip_border_size * mip_tile_size);
							std::vector<int32_t> border_coords_y(mip_border_size * mip_tile_size);
							switch (addr_u)
							{
							case TAM_Mirror:
								for (uint32_t y = 0; y < mip_tile_size; ++ y)
								{
									for (uint32_t x = 0; x < mip_border_size; ++ x)
									{
										border_coords_x[y * mip_border_size + x] = mip_tile_size - 1 - x;
									}
								}
								break;

							case TAM_Clamp:
								for (uint32_t y = 0; y < mip_tile_size; ++ y)
								{
									for (uint32_t x = 0; x < mip_border_size; ++ x)
									{
										border_coords_x[y * mip_border_size + x] = mip_tile_size - 1;
									}
								}
								break;

							case TAM_Border:
								for (uint32_t y = 0; y < mip_tile_size; ++ y)
								{
									for (uint32_t x = 0; x < mip_border_size; ++ x)
									{
										border_coords_x[y * mip_border_size + x] = -1;
									}
								}
								break;

							default:
								KFL_UNREACHABLE("Invalid texture addressing mode");
							}
							switch (addr_v)
							{
							case TAM_Mirror:
								for (uint32_t y = 0; y < mip_tile_size; ++ y)
								{
									for (uint32_t x = 0; x < mip_border_size; ++ x)
									{
										border_coords_y[y * mip_border_size + x] = y;
									}
								}
								break;

							case TAM_Clamp:
								for (uint32_t y = 0; y < mip_tile_size; ++ y)
								{
									for (uint32_t x = 0; x < mip_border_size; ++ x)
									{
										border_coords_y[y * mip_border_size + x] = y;
									}
								}
								break;

							case TAM_Border:
								for (uint32_t y = 0; y < mip_tile_size; ++ y)
								{
									for (uint32_t x = 0; x < mip_border_size; ++ x)
									{
										border_coords_y[y * mip_border_size + x] = -1;
									}
								}
								break;

							default:
								KFL_UNREACHABLE("Invalid texture addressing mode");
							}

							for (uint32_t y = 0; y < mip_tile_size; ++ y)
							{
								for (uint32_t x = 0; x < mip_border_size; ++ x)
								{
									if ((border_coords_x[y * mip_border_size + x] >= 0) && (border_coords_y[y * mip_border_size + x] >= 0))
									{
										texel_op_.copy(data_with_border + y * data_pitch + x * texel_size_,
											neighbor_data_ptr[0] + border_coords_y[y * mip_border_size + x] * mip_tile_with_border_size + border_coords_x[y * mip_border_size + x]);
									}
									else
									{
										texel_op_.copy(data_with_border + y * data_pitch + x * texel_size_, border_clr);
									}
								}
							}
						}
						else
						{
							for (uint32_t y = 0; y < mip_tile_size; ++ y)
							{
								for (uint32_t x = 0; x < mip_border_size; ++ x)
								{
									texel_op_.copy(data_with_border + (y + mip_border_size) * data_pitch + (x + mip_border_size + mip_tile_size) * texel_size_,
										neighbor_data_ptr[0] + (y * mip_tile_size + mip_tile_size - 1) * texel_size_);
								}
							}
						}
					}

					if ((neighbor_data_ptr[6] != nullptr) && in_same_image[i + 6])
					{
						for (uint32_t y = 0; y < mip_border_size; ++ y)
						{
							texel_op_.copy_array(data_with_border + (y + mip_border_size + mip_tile_size) * data_pitch,
								neighbor_data_ptr[6] + (y * mip_tile_size + (mip_tile_size - mip_border_size)) * texel_size_, mip_border_size);
						}
					}
					else
					{
						if (tile_info.attr != 0xFFFFFFFF)
						{
							std::vector<int32_t> border_coords_x(mip_border_size * mip_border_size);
							std::vector<int32_t> border_coords_y(mip_border_size * mip_border_size);
							switch (addr_u)
							{
							case TAM_Mirror:
								for (uint32_t y = 0; y < mip_border_size; ++ y)
								{
									for (uint32_t x = 0; x < mip_border_size; ++ x)
									{
										border_coords_x[y * mip_border_size + x] = mip_border_size - 1 - x;
									}
								}
								break;

							case TAM_Clamp:
								for (uint32_t y = 0; y < mip_border_size; ++ y)
								{
									for (uint32_t x = 0; x < mip_border_size; ++ x)
									{
										border_coords_x[y * mip_border_size + x] = 0;
									}
								}
								break;

							case TAM_Border:
								for (uint32_t y = 0; y < mip_border_size; ++ y)
								{
									for (uint32_t x = 0; x < mip_border_size; ++ x)
									{
										border_coords_x[y * mip_border_size + x] = -1;
									}
								}
								break;

							default:
								KFL_UNREACHABLE("Invalid texture addressing mode");
							}
							switch (addr_v)
							{
							case TAM_Mirror:
								for (uint32_t y = 0; y < mip_border_size; ++ y)
								{
									for (uint32_t x = 0; x < mip_border_size; ++ x)
									{
										border_coords_y[y * mip_border_size + x] = mip_tile_size - 1 - y;
									}
								}
								break;

							case TAM_Clamp:
								for (uint32_t y = 0; y < mip_border_size; ++ y)
								{
									for (uint32_t x = 0; x < mip_border_size; ++ x)
									{
										border_coords_y[y * mip_border_size + x] = mip_tile_size - 1;
									}
								}
								break;

							case TAM_Border:
								for (uint32_t y = 0; y < mip_border_size; ++ y)
								{
									for (uint32_t x = 0; x < mip_border_size; ++ x)
									{
										border_coords_y[y * mip_border_size + x] = -1;
									}
								}
								break;

							default:
								KFL_UNREACHABLE("Invalid texture addressing mode");
							}

							for (uint32_t y = 0; y < mip_border_size; ++ y)
							{
								for (uint32_t x = 0; x < mip_border_size; ++ x)
								{
									if ((border_coords_x[y * mip_border_size + x] >= 0) && (border_coords_y[y * mip_border_size + x] >= 0))
									{
										texel_op_.copy(data_with_border + y * data_pitch + x * texel_size_,
											neighbor_data_ptr[0] + border_coords_y[y * mip_border_size + x] * mip_tile_with_border_size + border_coords_x[y * mip_border_size + x]);
									}
									else
									{
										texel_op_.copy(data_with_border + y * data_pitch + x * texel_size_, border_clr);
									}
								}
							}
						}
						else
						{
							for (uint32_t y = 0; y < mip_border_size; ++ y)
							{
								for (uint32_t x = 0; x < mip_border_size; ++ x)
								{
									texel_op_.copy(data_with_border + (y + mip_border_size + mip_tile_size) * data_pitch + x * texel_size_,
										neighbor_data_ptr[0] + (mip_tile_size - 1) * mip_tile_size * texel_size_);
								}
							}
						}
					}
					if ((neighbor_data_ptr[7] != nullptr) && in_same_image[i + 7])
					{
						for (uint32_t y = 0; y < mip_border_size; ++ y)
						{
							texel_op_.copy_array(data_with_border + (y + mip_border_size + mip_tile_size) * data_pitch + mip_border_size * texel_size_,
								neighbor_data_ptr[7] + y * mip_tile_size * texel_size_, mip_tile_size);
						}
					}
					else
					{
						if (tile_info.attr != 0xFFFFFFFF)
						{
							std::vector<int32_t> border_coords_x(mip_tile_size * mip_border_size);
							std::vector<int32_t> border_coords_y(mip_tile_size * mip_border_size);
							switch (addr_u)
							{
							case TAM_Mirror:
								for (uint32_t y = 0; y < mip_border_size; ++ y)
								{
									for (uint32_t x = 0; x < mip_tile_size; ++ x)
									{
										border_coords_x[y * mip_tile_size + x] = x;
									}
								}
								break;

							case TAM_Clamp:
								for (uint32_t y = 0; y < mip_border_size; ++ y)
								{
									for (uint32_t x = 0; x < mip_tile_size; ++ x)
									{
										border_coords_x[y * mip_tile_size + x] = x;
									}
								}
								break;

							case TAM_Border:
								for (uint32_t y = 0; y < mip_border_size; ++ y)
								{
									for (uint32_t x = 0; x < mip_tile_size; ++ x)
									{
										border_coords_x[y * mip_tile_size + x] = -1;
									}
								}
								break;

							default:
								KFL_UNREACHABLE("Invalid texture addressing mode");
							}
							switch (addr_v)
							{
							case TAM_Mirror:
								for (uint32_t y = 0; y < mip_border_size; ++ y)
								{
									for (uint32_t x = 0; x < mip_tile_size; ++ x)
									{
										border_coords_y[y * mip_tile_size + x] = mip_tile_size - 1 - y;
									}
								}
								break;

							case TAM_Clamp:
								for (uint32_t y = 0; y < mip_border_size; ++ y)
								{
									for (uint32_t x = 0; x < mip_tile_size; ++ x)
									{
										border_coords_y[y * mip_tile_size + x] = mip_tile_size - 1;
									}
								}
								break;

							case TAM_Border:
								for (uint32_t y = 0; y < mip_border_size; ++ y)
								{
									for (uint32_t x = 0; x < mip_tile_size; ++ x)
									{
										border_coords_y[y * mip_tile_size + x] = -1;
									}
								}
								break;

							default:
								KFL_UNREACHABLE("Invalid texture addressing mode");
							}

							for (uint32_t y = 0; y < mip_border_size; ++ y)
							{
								for (uint32_t x = 0; x < mip_tile_size; ++ x)
								{
									if ((border_coords_x[y * mip_tile_size + x] >= 0) && (border_coords_y[y * mip_tile_size + x] >= 0))
									{
										texel_op_.copy(data_with_border + y * data_pitch + x * texel_size_,
											neighbor_data_ptr[0] + border_coords_y[y * mip_tile_size + x] * mip_tile_with_border_size + border_coords_x[y * mip_tile_size + x]);
									}
									else
									{
										texel_op_.copy(data_with_border + y * data_pitch + x * texel_size_, border_clr);
									}
								}
							}
						}
						else
						{
							for (uint32_t y = 0; y < mip_border_size; ++ y)
							{
								texel_op_.copy_array(data_with_border + (y + mip_border_size + mip_tile_size) * data_pitch + mip_border_size * texel_size_,
									neighbor_data_ptr[0] + (mip_tile_size - 1) * mip_tile_size * texel_size_, mip_tile_size);
							}
						}
					}			
					if ((neighbor_data_ptr[8] != nullptr) && in_same_image[i + 8])
					{
						for (uint32_t y = 0; y < mip_border_size; ++ y)
						{
							texel_op_.copy_array(data_with_border + (y + mip_border_size + mip_tile_size) * data_pitch + (mip_border_size + mip_tile_size) * texel_size_,
								neighbor_data_ptr[8] + y * mip_tile_size * texel_size_, mip_border_size);
						}
					}
					else
					{
						if (tile_info.attr != 0xFFFFFFFF)
						{
							std::vector<int32_t> border_coords_x(mip_border_size * mip_border_size);
							std::vector<int32_t> border_coords_y(mip_border_size * mip_border_size);
							switch (addr_u)
							{
							case TAM_Mirror:
								for (uint32_t y = 0; y < mip_border_size; ++ y)
								{
									for (uint32_t x = 0; x < mip_border_size; ++ x)
									{
										border_coords_x[y * mip_border_size + x] = mip_tile_size - 1 - x;
									}
								}
								break;

							case TAM_Clamp:
								for (uint32_t y = 0; y < mip_border_size; ++ y)
								{
									for (uint32_t x = 0; x < mip_border_size; ++ x)
									{
										border_coords_x[y * mip_border_size + x] = mip_tile_size - 1;
									}
								}
								break;

							case TAM_Border:
								for (uint32_t y = 0; y < mip_border_size; ++ y)
								{
									for (uint32_t x = 0; x < mip_border_size; ++ x)
									{
										border_coords_x[y * mip_border_size + x] = -1;
									}
								}
								break;

							default:
								KFL_UNREACHABLE("Invalid texture addressing mode");
							}
							switch (addr_v)
							{
							case TAM_Mirror:
								for (uint32_t y = 0; y < mip_border_size; ++ y)
								{
									for (uint32_t x = 0; x < mip_border_size; ++ x)
									{
										border_coords_y[y * mip_border_size + x] = mip_tile_size - 1 - y;
									}
								}
								break;

							case TAM_Clamp:
								for (uint32_t y = 0; y < mip_border_size; ++ y)
								{
									for (uint32_t x = 0; x < mip_border_size; ++ x)
									{
										border_coords_y[y * mip_border_size + x] = mip_tile_size - 1;
									}
								}
								break;

							case TAM_Border:
								for (uint32_t y = 0; y < mip_border_size; ++ y)
								{
									for (uint32_t x = 0; x < mip_border_size; ++ x)
									{
										border_coords_y[y * mip_border_size + x] = -1;
									}
								}
								break;

							default:
								KFL_UNREACHABLE("Invalid texture addressing mode");
							}

							for (uint32_t y = 0; y < mip_border_size; ++ y)
							{
								for (uint32_t x = 0; x < mip_border_size; ++ x)
								{
									if ((border_coords_x[y * mip_border_size + x] >= 0) && (border_coords_y[y * mip_border_size + x] >= 0))
									{
										texel_op_.copy(data_with_border + y * data_pitch + x * texel_size_,
											neighbor_data_ptr[0] + border_coords_y[y * mip_border_size + x] * mip_tile_with_border_size + border_coords_x[y * mip_border_size + x]);
									}
									else
									{
										texel_op_.copy(data_with_border + y * data_pitch + x * texel_size_, border_clr);
									}
								}
							}
						}
						else
						{
							for (uint32_t y = 0; y < mip_border_size; ++ y)
							{
								for (uint32_t x = 0; x < mip_border_size; ++ x)
								{
									texel_op_.copy(data_with_border + (y + mip_border_size + mip_tile_size) * data_pitch + (x + mip_border_size + mip_tile_size) * texel_size_,
										neighbor_data_ptr[0] + (mip_tile_size - 1) * mip_tile_size * texel_size_);
								}
							}
						}
					}
				}

				ElementFormat format;
				if (tex_cache_)
				{
					format = tex_cache_->Format();
				}
				else
				{
					format = tex_cache_array_[tile_info.z]->Format();
				}

				TexturePtr target_tex;
				uint32_t target_array_index;
				if (tex_cache_)
				{
					target_tex = tex_cache_;
					target_array_index = tile_info.z;
				}
				else
				{
					target_tex = tex_cache_array_[tile_info.z];
					target_array_index = 0;
				}

				if (IsCompressedFormat(format))
				{
					uint32_t const block_width = tex_codec_->BlockWidth();
					uint32_t const block_height = tex_codec_->BlockHeight();
					uint32_t const block_bytes = NumFormatBytes(format) * 4;
					uint32_t const bc_row_pitch = (mip_tile_with_border_size + block_width - 1) / block_width * block_bytes;
					uint32_t const bc_slice_pitch = (mip_tile_with_border_size + block_height - 1) / block_height * bc_row_pitch;
					std::vector<uint8_t> bc(bc_slice_pitch);
					{
						uint8_t const * data_with_border = &tex_a_tile_data[0];
						uint32_t const data_row_pitch = mip_tile_with_border_size * texel_size_;
						uint32_t const data_slice_pitch = mip_tile_with_border_size
							* mip_tile_with_border_size * texel_size_;

						uint32_t const * p_argb;
						uint32_t row_pitch;
						uint32_t slice_pitch;
						std::vector<uint32_t> argb_data;
						switch (format_)
						{
						case EF_R8:
							{
								argb_data.resize(mip_tile_with_border_size * mip_tile_with_border_size, 0);
								for (uint32_t y = 0; y < mip_tile_with_border_size; ++ y)
								{
									for (uint32_t x = 0; x < mip_tile_with_border_size; ++ x)
									{
										argb_data[y * mip_tile_with_border_size + x] = data_with_border[y * data_row_pitch + x] << 16;
									}
								}
								p_argb = &argb_data[0];
								row_pitch = mip_tile_with_border_size * 4;
								slice_pitch = mip_tile_with_border_size * row_pitch;
							}
							break;

						case EF_GR8:
							{
								argb_data.resize(mip_tile_with_border_size * mip_tile_with_border_size, 0);
								for (uint32_t y = 0; y < mip_tile_with_border_size; ++ y)
								{
									for (uint32_t x = 0; x < mip_tile_with_border_size; ++ x)
									{
										argb_data[y * mip_tile_with_border_size + x] = (data_with_border[y * data_row_pitch + x * 2 + 0] << 16)
											| (data_with_border[y * data_row_pitch + x * 2 + 1] << 8);
									}
								}
								p_argb = &argb_data[0];
								row_pitch = mip_tile_with_border_size * 4;
								slice_pitch = mip_tile_with_border_size * row_pitch;
							}
							break;

						case EF_ABGR8:
							{
								argb_data.resize(mip_tile_with_border_size * mip_tile_with_border_size, 0);
								for (uint32_t y = 0; y < mip_tile_with_border_size; ++ y)
								{
									for (uint32_t x = 0; x < mip_tile_with_border_size; ++ x)
									{
										argb_data[y * mip_tile_with_border_size + x] = (data_with_border[y * data_row_pitch + x * 4 + 0] << 16)
											| (data_with_border[y * data_row_pitch + x * 4 + 1] << 8)
											| (data_with_border[y * data_row_pitch + x * 4 + 2] << 0)
											| (data_with_border[y * data_row_pitch + x * 4 + 3] << 24);
									}
								}
								p_argb = &argb_data[0];
								row_pitch = mip_tile_with_border_size * 4;
								slice_pitch = mip_tile_with_border_size * row_pitch;
							}
							break;

						case EF_ARGB8:
							p_argb = reinterpret_cast<uint32_t const *>(data_with_border);
							row_pitch = data_row_pitch;
							slice_pitch = data_slice_pitch;
							break;

						default:
							KFL_UNREACHABLE("Not supported element format");
						}

						tex_codec_->EncodeMem(mip_tile_with_border_size, mip_tile_with_border_size,
							&bc[0], bc_row_pitch, bc_slice_pitch, p_argb, row_pitch, slice_pitch, TCM_Quality);
					}

					target_tex->UpdateSubresource2D(target_array_index, l,
						tile_info.x * mip_tile_with_border_size, tile_info.y * mip_tile_with_border_size,
						mip_tile_with_border_size, mip_tile_with_border_size,
						&bc[0], bc_row_pitch);
				}
				else
				{
					target_tex->UpdateSubresource2D(target_array_index, l,
						tile_info.x * mip_tile_with_border_size, tile_info.y * mip_tile_with_border_size,
						mip_tile_with_border_size, mip_tile_with_border_size,
						&tex_a_tile_data[0], mip_tile_with_border_size * texel_size_);
				}

				mip_tile_size /= 2;
				mip_tile_with_border_size /= 2;
				mip_border_size /= 2;
			}

			uint8_t const a_tile_indirect[] =
			{
				static_cast<uint8_t>(tile_info.x),
				static_cast<uint8_t>(tile_info.y),
				static_cast<uint8_t>(tile_info.z),
				0
			};
			uint32_t level, tile_x, tile_y;
			this->DecodeTileID(level, tile_x, tile_y, all_neighbor_ids[i]);
			tex_indirect_->UpdateSubresource2D(0, 0, tile_x, tile_y, 1, 1, a_tile_indirect, sizeof(a_tile_indirect));

			tim.emplace(all_neighbor_ids[i], tile_info);
		}
	}
}
