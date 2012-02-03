#include <kfont/kfont.hpp>

#include <fstream>
#include <cstring>
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

namespace
{
	template <int size>
	void EndianSwitch(void* p);

	template <>
	void EndianSwitch<2>(void* p)
	{
		uint8_t* bytes = static_cast<uint8_t*>(p);
		std::swap(bytes[0], bytes[1]);
	}
	template <>
	void EndianSwitch<4>(void* p)
	{
		uint8_t* bytes = static_cast<uint8_t*>(p);
		std::swap(bytes[0], bytes[3]);
		std::swap(bytes[1], bytes[2]);
	}
	template <>
	void EndianSwitch<8>(void* p)
	{
		uint8_t* bytes = static_cast<uint8_t*>(p);
		std::swap(bytes[0], bytes[7]);
		std::swap(bytes[1], bytes[6]);
		std::swap(bytes[2], bytes[5]);
		std::swap(bytes[3], bytes[4]);
	}

	template <int size>
	void NativeToBigEndian(void* p)
	{
#ifdef KFONT_LITTLE_ENDIAN
		EndianSwitch<size>(p);
#else
		p;
#endif
	}
	template <int size>
	void NativeToLittleEndian(void* p)
	{
#ifdef KFONT_LITTLE_ENDIAN
		p;
#else
		EndianSwitch<size>(p);
#endif
	}

	template <int size>
	void BigEndianToNative(void* p)
	{
		NativeToBigEndian<size>(p);
	}
	template <int size>
	void LittleEndianToNative(void* p)
	{
		NativeToLittleEndian<size>(p);
	}
}

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
	
	bool KFont::Load(std::string const & file_name)
	{
		std::ifstream kfont_input(file_name.c_str(), std::ios_base::binary | std::ios_base::in);
		if (kfont_input)
		{
			kfont_header header;
			kfont_input.read(reinterpret_cast<char*>(&header), sizeof(header));
			LittleEndianToNative<sizeof(header.fourcc)>(&header.fourcc);
			LittleEndianToNative<sizeof(header.version)>(&header.version);
			LittleEndianToNative<sizeof(header.start_ptr)>(&header.start_ptr);
			LittleEndianToNative<sizeof(header.validate_chars)>(&header.validate_chars);
			LittleEndianToNative<sizeof(header.non_empty_chars)>(&header.non_empty_chars);
			LittleEndianToNative<sizeof(header.char_size)>(&header.char_size);
			LittleEndianToNative<sizeof(header.base)>(&header.base);
			LittleEndianToNative<sizeof(header.scale)>(&header.scale);
			if ((MakeFourCC<'K', 'F', 'N', 'T'>::value == header.fourcc) && (KFONT_VERSION == header.version))
			{
				char_size_ = header.char_size;
				dist_base_ = header.base;
				dist_scale_ = header.scale;

				kfont_input.seekg(header.start_ptr, std::ios_base::beg);

				std::vector<std::pair<int32_t, int32_t> > temp_char_index(header.non_empty_chars);
				kfont_input.read(reinterpret_cast<char*>(&temp_char_index[0]), static_cast<std::streamsize>(temp_char_index.size() * sizeof(temp_char_index[0])));
				std::vector<std::pair<int32_t, uint32_t> > temp_char_advance(header.validate_chars);
				kfont_input.read(reinterpret_cast<char*>(&temp_char_advance[0]), static_cast<std::streamsize>(temp_char_advance.size() * sizeof(temp_char_advance[0])));

				typedef BOOST_TYPEOF(temp_char_index) TCIType;
				BOOST_FOREACH(TCIType::reference ci, temp_char_index)
				{
					LittleEndianToNative<sizeof(ci.first)>(&ci.first);
					LittleEndianToNative<sizeof(ci.second)>(&ci.second);

					char_index_advance_.insert(std::make_pair(ci.first, std::make_pair(ci.second, 0)));
				}
				typedef BOOST_TYPEOF(temp_char_advance) TCAType;
				BOOST_FOREACH(TCAType::reference ca, temp_char_advance)
				{
					LittleEndianToNative<sizeof(ca.first)>(&ca.first);
					LittleEndianToNative<sizeof(ca.second)>(&ca.second);

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

				typedef BOOST_TYPEOF(char_info_) CIType;
				BOOST_FOREACH(CIType::reference ci, char_info_)
				{
					LittleEndianToNative<sizeof(ci.left)>(&ci.left);
					LittleEndianToNative<sizeof(ci.top)>(&ci.top);
					LittleEndianToNative<sizeof(ci.width)>(&ci.width);
					LittleEndianToNative<sizeof(ci.height)>(&ci.height);
				}

				distances_addr_.resize(header.non_empty_chars + 1);

				std::vector<uint8_t> dist;
				for (uint32_t i = 0; i < header.non_empty_chars; ++ i)
				{
					distances_addr_[i] = distances_lzma_.size();

					uint64_t len;
					kfont_input.read(reinterpret_cast<char*>(&len), sizeof(len));
					LittleEndianToNative<sizeof(len)>(&len);
					distances_lzma_.resize(static_cast<size_t>(distances_lzma_.size() + len));

					kfont_input.read(reinterpret_cast<char*>(&distances_lzma_[distances_addr_[i]]), static_cast<size_t>(len));
				}

				distances_addr_[header.non_empty_chars] = distances_lzma_.size();

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
			header.fourcc = MakeFourCC<'K', 'F', 'N', 'T'>::value;
			header.version = KFONT_VERSION;
			header.start_ptr = sizeof(header);
			header.validate_chars = static_cast<uint32_t>(char_index_advance_.size());
			header.non_empty_chars = static_cast<uint32_t>(char_info_.size());
			header.char_size = char_size_;
			header.base = dist_base_;
			header.scale = dist_scale_;
			NativeToLittleEndian<sizeof(header.fourcc)>(&header.fourcc);
			NativeToLittleEndian<sizeof(header.version)>(&header.version);
			NativeToLittleEndian<sizeof(header.start_ptr)>(&header.start_ptr);
			NativeToLittleEndian<sizeof(header.validate_chars)>(&header.validate_chars);
			NativeToLittleEndian<sizeof(header.non_empty_chars)>(&header.non_empty_chars);
			NativeToLittleEndian<sizeof(header.char_size)>(&header.char_size);
			NativeToLittleEndian<sizeof(header.base)>(&header.base);
			NativeToLittleEndian<sizeof(header.scale)>(&header.scale);

			kfont_output.write(reinterpret_cast<char*>(&header), sizeof(header));

			std::vector<int32_t> chars;
			typedef BOOST_TYPEOF(char_index_advance_) CIAType;
			BOOST_FOREACH(CIAType::reference cia, char_index_advance_)
			{
				chars.push_back(cia.first);
			}
			std::sort(chars.begin(), chars.end());

			std::vector<std::pair<int32_t, int32_t> > temp_char_index;
			std::vector<std::pair<int32_t, uint32_t> > temp_char_advance;
			typedef BOOST_TYPEOF(chars) CharsType;
			BOOST_FOREACH(CharsType::reference ch, chars)
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

			typedef BOOST_TYPEOF(temp_char_index) TCIType;
			BOOST_FOREACH(TCIType::reference ci, temp_char_index)
			{
				TCIType::value_type tci = ci;
				NativeToLittleEndian<sizeof(tci.first)>(&tci.first);
				NativeToLittleEndian<sizeof(tci.second)>(&tci.second);

				kfont_output.write(reinterpret_cast<char*>(&tci), sizeof(tci));
			}
			typedef BOOST_TYPEOF(temp_char_advance) TCAType;
			BOOST_FOREACH(TCAType::reference ca, temp_char_advance)
			{
				TCAType::value_type tca = ca;
				NativeToLittleEndian<sizeof(tca.first)>(&tca.first);
				NativeToLittleEndian<sizeof(tca.second)>(&tca.second);

				kfont_output.write(reinterpret_cast<char*>(&tca), sizeof(tca));
			}

			for (size_t i = 0; i < temp_char_index.size(); ++ i)
			{
				int const index = temp_char_index[i].second;

				int16_t tmp = char_info_[index].top;
				NativeToLittleEndian<sizeof(tmp)>(&tmp);
				kfont_output.write(reinterpret_cast<char*>(&tmp), sizeof(tmp));
				tmp = char_info_[index].left;
				NativeToLittleEndian<sizeof(tmp)>(&tmp);
				kfont_output.write(reinterpret_cast<char*>(&tmp), sizeof(tmp));
				tmp = char_info_[index].width;
				NativeToLittleEndian<sizeof(tmp)>(&tmp);
				kfont_output.write(reinterpret_cast<char*>(&tmp), sizeof(tmp));
				tmp = char_info_[index].height;
				NativeToLittleEndian<sizeof(tmp)>(&tmp);
				kfont_output.write(reinterpret_cast<char*>(&tmp), sizeof(tmp));
			}

			for (size_t i = 0; i < temp_char_index.size(); ++ i)
			{
				uint32_t index = temp_char_index[i].second;
				size_t addr = distances_addr_[index];
				uint64_t len = distances_addr_[index + 1] - addr;
				NativeToLittleEndian<sizeof(len)>(&len);
				kfont_output.write(reinterpret_cast<char*>(&len), sizeof(len));
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
			ci = distances_addr_.size() - 1;
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
		typedef BOOST_TYPEOF(char_index_advance_) CIAType;
		BOOST_FOREACH(CIAType::reference cia, char_index_advance_)
		{
			chars.push_back(cia.first);
		}
		std::sort(chars.begin(), chars.end());

		boost::unordered_map<int32_t, std::pair<int32_t, uint32_t>, boost::hash<int32_t>, std::equal_to<int32_t>,
			boost::fast_pool_allocator<std::pair<int32_t, std::pair<int32_t, uint32_t> > > > new_char_index_advance;
		std::vector<font_info> new_char_info;
		std::vector<size_t> new_distances_addr;
		std::vector<uint8_t> new_distances_lzma;
		typedef BOOST_TYPEOF(chars) CharsType;
		BOOST_FOREACH(CharsType::reference ch, chars)
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
			new_char_index_advance.insert(std::make_pair(ch, std::make_pair(new_ci, ci.second)));
		}
		new_distances_addr.push_back(new_distances_lzma.size());

		char_index_advance_ = new_char_index_advance;
		char_info_ = new_char_info;
		distances_addr_ = new_distances_addr;
		distances_lzma_ = new_distances_lzma;
	}
}
