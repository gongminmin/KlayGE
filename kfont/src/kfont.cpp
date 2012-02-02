#include <kfont/kfont.hpp>

#include <fstream>
#include <algorithm>

#include <boost/assert.hpp>
#include <boost/typeof/typeof.hpp>
#include <boost/foreach.hpp>

#include <C/LzmaLib.h>

#ifdef KFONT_PLATFORM_WINDOWS
#include <windows.h>
#else
#include <dlfcn.h>
#endif

namespace KlayGE
{
	int const KFONT_VERSION = 2;

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
			static LZMALoader ret;
			return ret;
		}

		int LzmaCompress(unsigned char* dest, size_t* destLen, unsigned char const * src, size_t srcLen,
			unsigned char* outProps, size_t* outPropsSize, int level, unsigned int dictSize,
			int lc, int lp, int pb, int fb, int numThreads)
		{
#ifndef KFONT_PLATFORM_ANDROID
			return lzmaCompressFunc_(dest, destLen, src, srcLen, outProps, outPropsSize, level, dictSize,
				lc, lp, pb, fb, numThreads);
#else
			return ::LzmaCompress(dest, destLen, src, srcLen, outProps, outPropsSize, level, dictSize,
				lc, lp, pb, fb, numThreads);
#endif
		}
		int LzmaUncompress(unsigned char* dest, size_t* destLen, unsigned char const * src, SizeT* srcLen,
			unsigned char const * props, size_t propsSize)
		{
#ifndef KFONT_PLATFORM_ANDROID
			return lzmaUncompressFunc_(dest, destLen, src, srcLen, props, propsSize);
#else
			return ::LzmaUncompress(dest, destLen, src, srcLen, props, propsSize);
#endif
		}

	private:
		LZMALoader()
		{
#ifndef KFONT_PLATFORM_ANDROID
#ifdef KFONT_PLATFORM_WINDOWS
			dll_handle_ = static_cast<void*>(::LoadLibraryA("LZMA.dll"));
			lzmaCompressFunc_ = (LzmaCompressFunc)reinterpret_cast<void*>(::GetProcAddress(static_cast<HMODULE>(dll_handle_), "LzmaCompress"));
			lzmaUncompressFunc_ = (LzmaUncompressFunc)reinterpret_cast<void*>(::GetProcAddress(static_cast<HMODULE>(dll_handle_), "LzmaUncompress"));
#else
			dll_handle_ = ::dlopen("LZMA.so", RTLD_LAZY);
			lzmaCompressFunc_ = (LzmaCompressFunc)::dlsym(dll_handle_, "LzmaCompress");
			lzmaUncompressFunc_ = (LzmaUncompressFunc)::dlsym(dll_handle_, "LzmaUncompress");
#endif

			BOOST_ASSERT(lzmaCompressFunc_);
			BOOST_ASSERT(lzmaUncompressFunc_);
#endif
		}

	private:
#ifndef KLAYGE_PLATFORM_ANDROID
		void* dll_handle_;
		LzmaCompressFunc lzmaCompressFunc_;
		LzmaUncompressFunc lzmaUncompressFunc_;
#endif
	};

	template <unsigned char ch0, unsigned char ch1, unsigned char ch2, unsigned char ch3>
	struct MakeFourCC
	{
		enum { value = (ch0 << 0) + (ch1 << 8) + (ch2 << 16) + (ch3 << 24) };
	};

	KFont::KFont()
		: distances_addr_(1, 0)
	{
	}
	
	void KFont::Load(std::string const & file_name)
	{
		std::ifstream kfont_input(file_name.c_str(), std::ios_base::binary | std::ios_base::in);
		BOOST_ASSERT(kfont_input);

		kfont_header header;
		kfont_input.read(reinterpret_cast<char*>(&header), sizeof(header));
		BOOST_ASSERT((MakeFourCC<'K', 'F', 'N', 'T'>::value == header.fourcc));
		BOOST_ASSERT(KFONT_VERSION == header.version);

		char_size_ = header.char_size;
		dist_base_ = header.base;
		dist_scale_ = header.scale;

		kfont_input.seekg(header.start_ptr, std::ios_base::beg);

		std::vector<std::pair<int32_t, int32_t> > temp_char_index(header.non_empty_chars);
		kfont_input.read(reinterpret_cast<char*>(&temp_char_index[0]), static_cast<std::streamsize>(temp_char_index.size() * sizeof(temp_char_index[0])));
		std::vector<std::pair<int32_t, uint32_t> > temp_char_advance(header.validate_chars);
		kfont_input.read(reinterpret_cast<char*>(&temp_char_advance[0]), static_cast<std::streamsize>(temp_char_advance.size() * sizeof(temp_char_advance[0])));

		BOOST_FOREACH(BOOST_TYPEOF(temp_char_index)::reference ci, temp_char_index)
		{
			char_index_advance_.insert(std::make_pair(ci.first, std::make_pair(ci.second, 0)));
		}
		BOOST_FOREACH(BOOST_TYPEOF(temp_char_advance)::reference ca, temp_char_advance)
		{
			BOOST_AUTO(iter, char_index_advance_.find(ca.first));
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
		kfont_input.read(reinterpret_cast<char*>(&char_info_[0]), static_cast<std::streamsize>(char_info_.size() * sizeof(char_info_[0])));

		distances_addr_.resize(header.non_empty_chars + 1);

		std::vector<uint8_t> dist;
		for (uint32_t i = 0; i < header.non_empty_chars; ++ i)
		{
			distances_addr_[i] = distances_lzma_.size();

			uint64_t len;
			kfont_input.read(reinterpret_cast<char*>(&len), sizeof(len));
			distances_lzma_.resize(static_cast<size_t>(distances_lzma_.size() + len));

			kfont_input.read(reinterpret_cast<char*>(&distances_lzma_[distances_addr_[i]]), static_cast<size_t>(len));
		}

		distances_addr_[header.non_empty_chars] = distances_lzma_.size();
	}

	void KFont::Save(std::string const & file_name)
	{
		std::ofstream kfont_output(file_name.c_str(), std::ios_base::binary | std::ios_base::out);
		if (kfont_output)
		{
			this->Compact();

			kfont_header header;
			header.fourcc = MakeFourCC<'K', 'F', 'N', 'T'>::value;
			header.version = KFONT_VERSION;
			header.start_ptr = sizeof(header);
			header.validate_chars = static_cast<uint32_t>(char_index_advance_.size());
			header.non_empty_chars = static_cast<uint32_t>(char_info_.size());
			header.char_size = char_size_;
			header.base = dist_base_;
			header.scale = dist_scale_;

			kfont_output.write(reinterpret_cast<char*>(&header), sizeof(header));

			std::vector<std::pair<int32_t, int32_t> > temp_char_index;
			std::vector<std::pair<int32_t, uint32_t> > temp_char_advance;
			BOOST_FOREACH(BOOST_TYPEOF(char_index_advance_)::reference ci, char_index_advance_)
			{
				if (ci.second.first != -1)
				{
					temp_char_index.push_back(std::make_pair(ci.first, ci.second.first));
				}
				temp_char_advance.push_back(std::make_pair(ci.first, ci.second.second));
			}
			BOOST_ASSERT(temp_char_index.size() == header.non_empty_chars);
			BOOST_ASSERT(temp_char_advance.size() == header.validate_chars);

			kfont_output.write(reinterpret_cast<char*>(&temp_char_index[0]),
				static_cast<std::streamsize>(temp_char_index.size() * sizeof(temp_char_index[0])));

			kfont_output.write(reinterpret_cast<char*>(&temp_char_advance[0]),
				static_cast<std::streamsize>(temp_char_advance.size() * sizeof(temp_char_advance[0])));

			for (size_t i = 0; i < temp_char_index.size(); ++ i)
			{
				int const ch = temp_char_index[i].first;

				kfont_output.write(reinterpret_cast<char*>(&char_info_[ch].top), sizeof(char_info_[ch].top));
				kfont_output.write(reinterpret_cast<char*>(&char_info_[ch].left), sizeof(char_info_[ch].left));
				kfont_output.write(reinterpret_cast<char*>(&char_info_[ch].width), sizeof(char_info_[ch].width));
				kfont_output.write(reinterpret_cast<char*>(&char_info_[ch].height), sizeof(char_info_[ch].height));
			}

			kfont_output.write(reinterpret_cast<char*>(&distances_lzma_[0]),
				static_cast<std::streamsize>(distances_lzma_.size() * sizeof(distances_lzma_[0])));
		}
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
		BOOST_AUTO(iter, char_index_advance_.find(ch));
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
		std::vector<uint8_t> decoded(char_size_ * char_size_);

		uint8_t const * input;
		uint32_t size;
		this->GetLZMADistanceData(input, size, index);

		std::vector<uint8_t> in_data(size);
		std::memcpy(&in_data[0], input, static_cast<std::streamsize>(in_data.size()));

		SizeT s_out_len = static_cast<SizeT>(decoded.size());

		SizeT s_src_len = static_cast<SizeT>(in_data.size() - LZMA_PROPS_SIZE);
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

	void KFont::GetLZMADistanceData(uint8_t const *& p, uint32_t& size, int32_t index) const
	{
		p = &distances_lzma_[distances_addr_[index]];
		size = distances_addr_[index + 1] - distances_addr_[index];
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

		this->SetLZMADistanceData(ch, &output[0], output.size(), adv, fi);
	}

	void KFont::SetLZMADistanceData(wchar_t ch, uint8_t const * p, uint32_t size, uint32_t adv, font_info const & fi)
	{
		int32_t ci;
		if (size > 0)
		{
			ci = distances_addr_.size();
			uint32_t offset = distances_lzma_.size();
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
		char_index_advance_.insert(std::make_pair(ch, std::make_pair(ci, adv)));
	}

	void KFont::Compact()
	{
		std::vector<int32_t> chars;
		BOOST_FOREACH(BOOST_TYPEOF(char_index_advance_)::reference cia, char_index_advance_)
		{
			chars.push_back(cia.first);
		}
		std::sort(chars.begin(), chars.end());

		boost::unordered_map<int32_t, std::pair<int32_t, uint32_t>, boost::hash<int32_t>, std::equal_to<int32_t>,
			boost::fast_pool_allocator<std::pair<int32_t, std::pair<int32_t, uint32_t> > > > new_char_index_advance;
		std::vector<font_info> new_char_info;
		std::vector<size_t> new_distances_addr;
		std::vector<uint8_t> new_distances_lzma;
		BOOST_FOREACH(BOOST_TYPEOF(chars)::reference ch, chars)
		{
			std::pair<int32_t, uint32_t> const & ci = char_index_advance_[ch];
			int32_t new_ci;
			if (ci.first != -1)
			{
				new_ci = static_cast<int32_t>(new_char_info.size());

				uint32_t old_offset = ci.first;
				new_char_info.push_back(char_info_[old_offset]);
				new_distances_addr.push_back(new_distances_lzma.size());

				new_distances_lzma.insert(new_distances_lzma.end(), &distances_lzma_[distances_addr_[old_offset]],
					&distances_lzma_[distances_addr_[old_offset + 1]]);
			}
			else
			{
				new_ci = -1;
			}
			new_char_index_advance.insert(std::make_pair(ch, std::make_pair(new_ci, ci.second)));
		}
		new_distances_addr.push_back(new_distances_lzma.size());

		char_index_advance_ = new_char_index_advance;
		char_info_ = new_char_info;
		distances_addr_ = new_distances_addr;
		distances_lzma_ = new_distances_lzma;
	}
}
