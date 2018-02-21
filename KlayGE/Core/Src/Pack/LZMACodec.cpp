/**
 * @file LZMACodec.cpp
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
#include <KFL/ErrorHandling.hpp>
#include <KlayGE/ResLoader.hpp>
#include <KFL/DllLoader.hpp>

#include <cstring>
#include <mutex>

#include <C/LzmaLib.h>

#include <KlayGE/LZMACodec.hpp>

namespace
{
	using namespace KlayGE;

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
}

namespace KlayGE
{
	LZMACodec::LZMACodec()
	{
	}

	LZMACodec::~LZMACodec()
	{
	}

	uint64_t LZMACodec::Encode(std::ostream& os, ResIdentifierPtr const & is, uint64_t len)
	{
		std::vector<uint8_t> input(static_cast<size_t>(len));
		is->read(&input[0], static_cast<size_t>(len));

		std::vector<uint8_t> output;
		this->Encode(output, &input[0], len);
		os.write(reinterpret_cast<char*>(&output[0]), output.size() * sizeof(output[0]));

		return output.size();
	}

	uint64_t LZMACodec::Encode(std::ostream& os, void const * input, uint64_t len)
	{
		std::vector<uint8_t> output;
		this->Encode(output, input, len);
		os.write(reinterpret_cast<char*>(&output[0]), output.size() * sizeof(output[0]));
		return output.size();
	}

	void LZMACodec::Encode(std::vector<uint8_t>& output, ResIdentifierPtr const & is, uint64_t len)
	{
		std::vector<uint8_t> input(static_cast<size_t>(len));
		is->read(&input[0], static_cast<size_t>(len));

		this->Encode(output, &input[0], len);
	}

	void LZMACodec::Encode(std::vector<uint8_t>& output, void const * input, uint64_t len)
	{
		SizeT out_len = static_cast<SizeT>(std::max(len * 11 / 10, static_cast<uint64_t>(32)));
		output.resize(LZMA_PROPS_SIZE + out_len);
		SizeT out_props_size = LZMA_PROPS_SIZE;
		LZMALoader::Instance().LzmaCompress(&output[LZMA_PROPS_SIZE], &out_len, static_cast<Byte const *>(input), static_cast<SizeT>(len),
			&output[0], &out_props_size, 5, std::min<uint32_t>(static_cast<uint32_t>(len), 1UL << 24), 3, 0, 2, 32, 1);

		output.resize(LZMA_PROPS_SIZE + out_len);
	}

	uint64_t LZMACodec::Decode(std::ostream& os, ResIdentifierPtr const & is, uint64_t len, uint64_t original_len)
	{
		std::vector<uint8_t> in_data(static_cast<size_t>(len));
		is->read(&in_data[0], static_cast<size_t>(len));

		std::vector<uint8_t> output;
		this->Decode(output, &in_data[0], len, original_len);

		os.write(reinterpret_cast<char*>(&output[0]), static_cast<std::streamsize>(output.size()));

		return output.size();
	}

	uint64_t LZMACodec::Decode(std::ostream& os, void const * input, uint64_t len, uint64_t original_len)
	{
		std::vector<uint8_t> output;
		this->Decode(output, input, len, original_len);

		os.write(reinterpret_cast<char*>(&output[0]), static_cast<std::streamsize>(output.size()));

		return output.size();
	}

	void LZMACodec::Decode(std::vector<uint8_t>& output, ResIdentifierPtr const & is, uint64_t len, uint64_t original_len)
	{
		std::vector<uint8_t> in_data(static_cast<size_t>(len));
		is->read(&in_data[0], static_cast<size_t>(len));

		this->Decode(output, &in_data[0], len, original_len);
	}

	void LZMACodec::Decode(std::vector<uint8_t>& output, void const * input, uint64_t len, uint64_t original_len)
	{
		output.resize(static_cast<uint32_t>(original_len));
		this->Decode(&output[0], input, len, original_len);
	}

	void LZMACodec::Decode(void* output, void const * input, uint64_t len, uint64_t original_len)
	{
		uint8_t const * p = static_cast<uint8_t const *>(input);

		std::vector<uint8_t> in_data(static_cast<size_t>(len));
		std::memcpy(&in_data[0], p, static_cast<std::streamsize>(in_data.size()));

		SizeT s_out_len = static_cast<SizeT>(original_len);

		SizeT s_src_len = static_cast<SizeT>(len - LZMA_PROPS_SIZE);
		int res = LZMALoader::Instance().LzmaUncompress(static_cast<Byte*>(output), &s_out_len, &in_data[LZMA_PROPS_SIZE], &s_src_len,
			&in_data[0], LZMA_PROPS_SIZE);
		Verify(0 == res);
	}
}
