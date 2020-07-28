/**
 * @file kfont.cpp
 * @author Minmin Gong
 *
 * @section DESCRIPTION
 *
 * This source file is part of kfont, a subproject of KlayGE
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

#include <KFL/KFL.hpp>
#include <KFL/DllLoader.hpp>
#include <KFL/Thread.hpp>
#include <KFL/ResIdentifier.hpp>
#include <kfont/kfont.hpp>

#include <fstream>
#include <cstring>
#include <algorithm>

#include <boost/assert.hpp>

#include <C/LzmaLib.h>

namespace KlayGE
{
	uint32_t const KFONT_VERSION = 2;

	std::mutex singleton_mutex;

	typedef int (MY_STD_CALL *LzmaCompressFunc)(unsigned char* dest, size_t* destLen, unsigned char const * src, size_t srcLen,
		unsigned char* outProps, size_t* outPropsSize, /* *outPropsSize must be = 5 */
		int level,      /* 0 <= level <= 9, default = 5 */
		unsigned int dictSize,  /* default = (1 << 24) */
		int lc,        /* 0 <= lc <= 8, default = 3  */
		int lp,        /* 0 <= lp <= 4, default = 0  */
		int pb,        /* 0 <= pb <= 4, default = 2  */
		int fb,        /* 5 <= fb <= 273, default = 32 */
		int numThreads /* 1 or 2, default = 2 */);
	typedef int (MY_STD_CALL *LzmaUncompressFunc)(unsigned char* dest, size_t* destLen, unsigned char const * src, SizeT* srcLen,
		unsigned char const * props, size_t propsSize);

	class LZMALoader
	{
	public:
		static LZMALoader& Instance()
		{
			if (!instance_)
			{
				std::lock_guard<std::mutex> lock(singleton_mutex);
				if (!instance_)
				{
					instance_ = MakeUniquePtr<LZMALoader>();
				}
			}
			return *instance_;
		}

		int LzmaCompress(unsigned char* dest, size_t* destLen, unsigned char const * src, size_t srcLen,
			unsigned char* outProps, size_t* outPropsSize, int level, unsigned int dictSize,
			int lc, int lp, int pb, int fb, int numThreads)
		{
			return lzma_compress_func_(dest, destLen, src, srcLen, outProps, outPropsSize, level, dictSize,
				lc, lp, pb, fb, numThreads);
		}
		int LzmaUncompress(unsigned char* dest, size_t* destLen, unsigned char const * src, SizeT* srcLen,
			unsigned char const * props, size_t propsSize)
		{
			return lzma_uncompress_func_(dest, destLen, src, srcLen, props, propsSize);
		}

		LZMALoader()
		{
#if !(defined(KLAYGE_PLATFORM_ANDROID) || defined(KLAYGE_PLATFORM_IOS))
			dll_loader_.Load(DLL_PREFIX "LZMA" DLL_SUFFIX);

			lzma_compress_func_ = (LzmaCompressFunc)dll_loader_.GetProcAddress("LzmaCompress");
			lzma_uncompress_func_ = (LzmaUncompressFunc)dll_loader_.GetProcAddress("LzmaUncompress");
#else
			lzma_compress_func_ = ::LzmaCompress;
			lzma_uncompress_func_ = ::LzmaUncompress;
#endif

			BOOST_ASSERT(lzma_compress_func_);
			BOOST_ASSERT(lzma_uncompress_func_);
		}

	private:
#if !(defined(KLAYGE_PLATFORM_ANDROID) || defined(KLAYGE_PLATFORM_IOS))
		DllLoader dll_loader_;
#endif
		LzmaCompressFunc lzma_compress_func_;
		LzmaUncompressFunc lzma_uncompress_func_;

		static std::unique_ptr<LZMALoader> instance_;
	};
	std::unique_ptr<LZMALoader> LZMALoader::instance_;

	KFont::KFont() = default;

	bool KFont::Load(std::string const & file_name)
	{
		ResIdentifierPtr kfont_input = MakeSharedPtr<ResIdentifier>(file_name, 0,
			MakeSharedPtr<std::ifstream>(file_name.c_str(), std::ios_base::binary | std::ios_base::in));
		return this->Load(kfont_input);
	}

	bool KFont::Load(ResIdentifierPtr const & kfont_input)
	{
		if (kfont_input)
		{
			kfont_input_ = kfont_input;

			kfont_header header;
			kfont_input->read(&header, sizeof(header));
			header.fourcc = LE2Native(header.fourcc);
			header.version = LE2Native(header.version);
			header.start_ptr = LE2Native(header.start_ptr);
			header.validate_chars = LE2Native(header.validate_chars);
			header.non_empty_chars = LE2Native(header.non_empty_chars);
			header.char_size = LE2Native(header.char_size);
			header.base = LE2Native(header.base);
			header.scale = LE2Native(header.scale);
			if ((MakeFourCC<'K', 'F', 'N', 'T'>::value == header.fourcc) && (KFONT_VERSION == header.version))
			{
				char_size_ = header.char_size;
				dist_base_ = header.base;
				dist_scale_ = header.scale;

				kfont_input->seekg(header.start_ptr, std::ios_base::beg);

				std::vector<std::pair<int32_t, int32_t>> temp_char_index(header.non_empty_chars);
				kfont_input->read(&temp_char_index[0], temp_char_index.size() * sizeof(temp_char_index[0]));
				std::vector<std::pair<int32_t, uint32_t>> temp_char_advance(header.validate_chars);
				kfont_input->read(&temp_char_advance[0], temp_char_advance.size() * sizeof(temp_char_advance[0]));

				for (auto& ci : temp_char_index)
				{
					ci.first = LE2Native(ci.first);
					ci.second = LE2Native(ci.second);

					char_index_advance_.emplace(ci.first, std::make_pair(ci.second, 0));
				}
				for (auto& ca : temp_char_advance)
				{
					ca.first = LE2Native(ca.first);
					ca.second = LE2Native(ca.second);

					auto iter = char_index_advance_.find(ca.first);
					if (iter != char_index_advance_.end())
					{
						iter->second.second = ca.second;
					}
					else
					{
						char_index_advance_[ca.first] = std::make_pair(-1, ca.second);
					}
				}

				char_info_.resize(header.non_empty_chars);
				kfont_input->read(&char_info_[0], char_info_.size() * sizeof(char_info_[0]));

				for (auto& ci : char_info_)
				{
					ci.left = LE2Native(ci.left);
					ci.top = LE2Native(ci.top);
					ci.width = LE2Native(ci.width);
					ci.height = LE2Native(ci.height);
				}

				distances_addr_.resize(header.non_empty_chars + 1);
				distances_lzma_start_ = kfont_input->tellg();
				size_t distances_lzma_size = 0;

				for (uint32_t i = 0; i < header.non_empty_chars; ++ i)
				{
					distances_addr_[i] = distances_lzma_size;

					uint64_t len;
					kfont_input->read(&len, sizeof(len));
					len = LE2Native(len);
					distances_lzma_size += static_cast<size_t>(len);

					kfont_input->seekg(len, std::ios_base::cur);
				}

				distances_addr_[header.non_empty_chars] = distances_lzma_size;

				return true;
			}
		}

		return false;
	}

	bool KFont::Save(std::string const & file_name)
	{
		std::ofstream kfont_output(file_name.c_str(), std::ios_base::binary | std::ios_base::out);
		if (kfont_output)
		{
			this->Compact();

			kfont_header header;
			header.fourcc = Native2LE(MakeFourCC<'K', 'F', 'N', 'T'>::value);
			header.version = Native2LE(KFONT_VERSION);
			header.start_ptr = Native2LE(static_cast<uint32_t>(sizeof(header)));
			header.validate_chars = Native2LE(static_cast<uint32_t>(char_index_advance_.size()));
			header.non_empty_chars = Native2LE(static_cast<uint32_t>(char_info_.size()));
			header.char_size = Native2LE(char_size_);
			header.base = Native2LE(dist_base_);
			header.scale = Native2LE(dist_scale_);

			kfont_output.write(reinterpret_cast<char*>(&header), sizeof(header));

			std::vector<int32_t> chars;
			for (auto const & cia : char_index_advance_)
			{
				chars.push_back(cia.first);
			}
			std::sort(chars.begin(), chars.end());

			std::vector<std::pair<int32_t, int32_t>> temp_char_index;
			std::vector<std::pair<int32_t, uint32_t>> temp_char_advance;
			for (auto const & ch : chars)
			{
				std::pair<int32_t, uint32_t> const & ci = char_index_advance_[ch];

				if (ci.first != -1)
				{
					temp_char_index.push_back(std::make_pair(ch, ci.first));
				}
				temp_char_advance.push_back(std::make_pair(ch, ci.second));
			}
			BOOST_ASSERT(temp_char_index.size() == char_info_.size());
			BOOST_ASSERT(temp_char_advance.size() == char_index_advance_.size());

			for (auto const & ci : temp_char_index)
			{
				auto tci = std::make_pair(Native2LE(ci.first), Native2LE(ci.second));
				kfont_output.write(reinterpret_cast<char*>(&tci), sizeof(tci));
			}
			for (auto const & ca : temp_char_advance)
			{
				auto tca = std::make_pair(Native2LE(ca.first), Native2LE(ca.second));
				kfont_output.write(reinterpret_cast<char*>(&tca), sizeof(tca));
			}

			for (auto const & ci : temp_char_index)
			{
				int const index = ci.second;

				int16_t tmp;
				tmp = Native2LE(char_info_[index].top);
				kfont_output.write(reinterpret_cast<char*>(&tmp), sizeof(tmp));
				tmp = Native2LE(char_info_[index].left);
				kfont_output.write(reinterpret_cast<char*>(&tmp), sizeof(tmp));
				tmp = Native2LE(char_info_[index].width);
				kfont_output.write(reinterpret_cast<char*>(&tmp), sizeof(tmp));
				tmp = Native2LE(char_info_[index].height);
				kfont_output.write(reinterpret_cast<char*>(&tmp), sizeof(tmp));
			}

			for (auto const & ci : temp_char_index)
			{
				uint32_t index = ci.second;
				size_t addr = distances_addr_[index];
				uint64_t len = distances_addr_[index + 1] - addr;
				uint64_t len_le = Native2LE(len);
				kfont_output.write(reinterpret_cast<char*>(&len_le), sizeof(len_le));
				kfont_output.write(reinterpret_cast<char*>(&distances_lzma_[addr]),
					static_cast<std::streamsize>(len * sizeof(distances_lzma_[0])));
			}

			return true;
		}

		return false;
	}

	uint32_t KFont::CharSize() const
	{
		return char_size_;
	}

	int16_t KFont::DistBase() const
	{
		return dist_base_;
	}

	int16_t KFont::DistScale() const
	{
		return dist_scale_;
	}

	std::pair<int32_t, uint32_t> const & KFont::CharIndexAdvance(wchar_t ch) const
	{
		auto iter = char_index_advance_.find(ch);
		if (iter != char_index_advance_.end())
		{
			return iter->second;
		}
		else
		{
			static std::pair<int32_t, uint32_t> ret(-1, 0);
			return ret;
		}
	}

	int32_t KFont::CharIndex(wchar_t ch) const
	{
		return this->CharIndexAdvance(ch).first;
	}

	uint32_t KFont::CharAdvance(wchar_t ch) const
	{
		return this->CharIndexAdvance(ch).second;
	}

	KFont::font_info const & KFont::CharInfo(int32_t index) const
	{
		return char_info_[index];
	}

	void KFont::GetDistanceData(uint8_t* p, uint32_t pitch, int32_t index) const
	{
		size_t const decoded_size = char_size_ * char_size_;
		auto decoded = MakeUniquePtr<uint8_t[]>(decoded_size);

		uint32_t size;
		this->GetLZMADistanceData(nullptr, size, index);

		auto in_data = MakeUniquePtr<uint8_t[]>(size);
		this->GetLZMADistanceData(&in_data[0], size, index);

		SizeT s_out_len = decoded_size;

		SizeT s_src_len = static_cast<SizeT>(size - LZMA_PROPS_SIZE);
		LZMALoader::Instance().LzmaUncompress(static_cast<Byte*>(&decoded[0]), &s_out_len, &in_data[LZMA_PROPS_SIZE], &s_src_len,
			&in_data[0], LZMA_PROPS_SIZE);

		uint8_t const * char_data = &decoded[0];
		for (uint32_t y = 0; y < char_size_; ++ y)
		{
			std::memcpy(p, char_data, char_size_);
			p += pitch;
			char_data += char_size_;
		}
	}

	void KFont::GetLZMADistanceData(uint8_t* p, uint32_t& size, int32_t index) const
	{
		size = static_cast<uint32_t>(distances_addr_[index + 1] - distances_addr_[index]);
		if (p != nullptr)
		{
			if (kfont_input_)
			{
				kfont_input_->seekg(distances_lzma_start_ + (index + 1) * sizeof(uint64_t) + distances_addr_[index],
					std::ios_base::beg);
				kfont_input_->read(p, size);
			}
			else
			{
				memcpy(p, &distances_lzma_[distances_addr_[index]], size);
			}
		}
	}

	void KFont::CharSize(uint32_t size)
	{
		char_size_ = size;
	}

	void KFont::DistBase(int16_t base)
	{
		dist_base_ = base;
	}

	void KFont::DistScale(int16_t scale)
	{
		dist_scale_ = scale;
	}

	void KFont::SetDistanceData(wchar_t ch, uint8_t const * p, uint32_t adv, font_info const & fi)
	{
		uint64_t len = char_size_ * char_size_;
		SizeT out_len = static_cast<SizeT>(std::max(len * 11 / 10, static_cast<uint64_t>(32)));
		std::vector<uint8_t> output(LZMA_PROPS_SIZE + out_len);
		SizeT out_props_size = LZMA_PROPS_SIZE;
		LZMALoader::Instance().LzmaCompress(&output[LZMA_PROPS_SIZE], &out_len, static_cast<Byte const *>(p), static_cast<SizeT>(len),
			&output[0], &out_props_size, 5, std::min<uint32_t>(static_cast<uint32_t>(len), 1UL << 24), 3, 0, 2, 32, 1);

		output.resize(LZMA_PROPS_SIZE + out_len);

		this->SetLZMADistanceData(ch, &output[0], static_cast<uint32_t>(output.size()), adv, fi);
	}

	void KFont::SetLZMADistanceData(wchar_t ch, uint8_t const * p, uint32_t size, uint32_t adv, font_info const & fi)
	{
		int32_t ci;
		if (size > 0)
		{
			ci = static_cast<uint32_t>(distances_addr_.size() - 1);
			uint32_t offset = static_cast<uint32_t>(distances_lzma_.size());
			distances_addr_.back() = offset;
			distances_addr_.push_back(offset + size);
			distances_lzma_.insert(distances_lzma_.end(), p, p + size);

			char_info_.push_back(fi);

			BOOST_ASSERT(distances_addr_.size() - 1 == char_info_.size());
			BOOST_ASSERT(distances_addr_.back() == distances_lzma_.size());
		}
		else
		{
			ci = -1;
		}
		char_index_advance_.emplace(ch, std::make_pair(ci, adv));
	}

	void KFont::Compact()
	{
		std::vector<int32_t> chars;
		for (auto const & cia : char_index_advance_)
		{
			chars.push_back(cia.first);
		}
		std::sort(chars.begin(), chars.end());

		std::unordered_map<int32_t, std::pair<int32_t, uint32_t>> new_char_index_advance;
		std::vector<font_info> new_char_info;
		std::vector<size_t> new_distances_addr;
		std::vector<uint8_t> new_distances_lzma;
		for (auto const & ch : chars)
		{
			std::pair<int32_t, uint32_t> const & ci = char_index_advance_[ch];
			int32_t new_ci;
			if (ci.first != -1)
			{
				new_ci = static_cast<int32_t>(new_char_info.size());

				uint32_t old_offset = ci.first;
				new_char_info.push_back(char_info_[old_offset]);
				new_distances_addr.push_back(new_distances_lzma.size());

				size_t addr = distances_addr_[old_offset];
				size_t len = distances_addr_[old_offset + 1] - distances_addr_[old_offset];
				uint8_t const * p = &distances_lzma_[addr];
				new_distances_lzma.insert(new_distances_lzma.end(), p, p + len);
			}
			else
			{
				new_ci = -1;
			}
			new_char_index_advance.emplace(ch, std::make_pair(new_ci, ci.second));
		}
		new_distances_addr.push_back(new_distances_lzma.size());

		char_index_advance_ = new_char_index_advance;
		char_info_ = new_char_info;
		distances_addr_ = new_distances_addr;
		distances_lzma_ = new_distances_lzma;
	}
}
