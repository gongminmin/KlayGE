// LZMACodec.cpp
// KlayGE LZMACodec类 实现文件
// Ver 3.9.0
// 版权所有(C) 龚敏敏, 2009
// Homepage: http://www.klayge.org
//
// 3.9.0
// 初次建立 (2009.12.6)
//
// 修改记录
//////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KlayGE/ThrowErr.hpp>
#include <KlayGE/ResLoader.hpp>
#include <KlayGE/DllLoader.hpp>

#include <cstring>

#include "LzTypes.hpp"
#include <LzmaEnc.h>
#include <LzmaDec.h>

#include <KlayGE/LZMACodec.hpp>

namespace
{
	using namespace KlayGE;

	void* SzAlloc(void* /*p*/, size_t size)
	{
		return malloc(size);
	}

	void SzFree(void* /*p*/, void* address)
	{
		free(address);
	}

	ISzAlloc lzma_alloc = { SzAlloc, SzFree };


	typedef void (*LzmaEncProps_InitFunc)(CLzmaEncProps* p);
	typedef SRes (*LzmaEncodeFunc)(Byte* dest, SizeT* destLen, Byte const * src, SizeT srcLen,
		CLzmaEncProps const * props, Byte* propsEncoded, SizeT* propsSize, int writeEndMark,
		ICompressProgress* progress, ISzAlloc* alloc, ISzAlloc* allocBig);
	typedef SRes (*LzmaDecodeFunc)(Byte* dest, SizeT* destLen, Byte const * src, SizeT* srcLen,
		Byte const * propData, unsigned int propSize, ELzmaFinishMode finishMode,
		ELzmaStatus* status, ISzAlloc* alloc);

	class SevenZipLoader
	{
	public:
		static SevenZipLoader& Instance()
		{
			static SevenZipLoader ret;
			return ret;
		}

		void LzmaEncProps_Init(CLzmaEncProps* p)
		{
			return lzmaEncProps_InitFunc_(p);
		}

		SRes LzmaEncode(Byte* dest, SizeT* destLen, Byte const * src, SizeT srcLen,
			CLzmaEncProps const * props, Byte* propsEncoded, SizeT* propsSize, int writeEndMark,
			ICompressProgress* progress, ISzAlloc* alloc, ISzAlloc* allocBig)
		{
			return lzmaEncodeFuncFunc_(dest, destLen, src, srcLen, props, propsEncoded, propsSize, writeEndMark,
				progress, alloc, allocBig);
		}
		SRes LzmaDecode(Byte* dest, SizeT* destLen, Byte const * src, SizeT* srcLen,
			Byte const * propData, unsigned int propSize, ELzmaFinishMode finishMode,
			ELzmaStatus* status, ISzAlloc* alloc)
		{
			return lzmaDecodeFuncFunc_(dest, destLen, src, srcLen, propData, propSize, finishMode,
				status, alloc);
		}

	private:
		SevenZipLoader()
		{
#ifdef KLAYGE_PLATFORM_WINDOWS
			dll_loader_.Load("7z.dll");
#elif defined KLAYGE_PLATFORM_LINUX || defined KLAYGE_PLATFORM_ANDROID
			dll_loader_.Load("7z.so");
#endif
			lzmaEncProps_InitFunc_ = (LzmaEncProps_InitFunc)dll_loader_.GetProcAddress("LzmaEncProps_Init");
			lzmaEncodeFuncFunc_ = (LzmaEncodeFunc)dll_loader_.GetProcAddress("LzmaEncode");
			lzmaDecodeFuncFunc_ = (LzmaDecodeFunc)dll_loader_.GetProcAddress("LzmaDecode");

			BOOST_ASSERT(lzmaEncProps_InitFunc_);
			BOOST_ASSERT(lzmaEncodeFuncFunc_);
			BOOST_ASSERT(lzmaDecodeFuncFunc_);
		}

	private:
		DllLoader dll_loader_;
		LzmaEncProps_InitFunc lzmaEncProps_InitFunc_;
		LzmaEncodeFunc lzmaEncodeFuncFunc_;
		LzmaDecodeFunc lzmaDecodeFuncFunc_;
	};
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
		CLzmaEncProps props;
		SevenZipLoader::Instance().LzmaEncProps_Init(&props);
		props.level = 5;
		props.dictSize = std::min<uint32_t>(static_cast<uint32_t>(len), 1UL << 24);

		SizeT out_len = static_cast<SizeT>(std::max(len * 11 / 10, static_cast<uint64_t>(32)));
		output.resize(LZMA_PROPS_SIZE + out_len);
		SizeT out_props_size = LZMA_PROPS_SIZE;
		SevenZipLoader::Instance().LzmaEncode(&output[LZMA_PROPS_SIZE], &out_len, static_cast<Byte const *>(input), static_cast<SizeT>(len),
			&props, &output[0], &out_props_size, 0, NULL, &lzma_alloc, &lzma_alloc);

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
		ELzmaStatus status;
		int res = SevenZipLoader::Instance().LzmaDecode(static_cast<Byte*>(output), &s_out_len, &in_data[LZMA_PROPS_SIZE], &s_src_len,
			&in_data[0], LZMA_PROPS_SIZE, LZMA_FINISH_ANY,
			&status, &lzma_alloc);

		Verify(0 == res);
		Verify(status != LZMA_STATUS_NEEDS_MORE_INPUT);
	}
}
