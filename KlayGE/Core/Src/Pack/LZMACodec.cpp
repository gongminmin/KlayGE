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

#include <cstring>

#include "LzTypes.hpp"
#include "LzmaEnc.hpp"
#include "LzmaDec.hpp"

#include <KlayGE/LZMACodec.hpp>

namespace
{
	void* SzAlloc(void* /*p*/, size_t size)
	{
		return malloc(size);
	}

	void SzFree(void* /*p*/, void* address)
	{
		free(address);
	}

	ISzAlloc lzma_alloc = { SzAlloc, SzFree };
}

namespace KlayGE
{
	LZMACodec::LZMACodec()
	{
		lzma_enc_ = LzmaEnc_Create(&lzma_alloc);
		this->EncodeProps(5, 1UL << 24);
	}

	LZMACodec::~LZMACodec()
	{
		LzmaEnc_Destroy(lzma_enc_, &lzma_alloc, &lzma_alloc);
	}

	void LZMACodec::EncodeProps(int level, uint32_t dict_size)
	{
		CLzmaEncProps props;
		LzmaEncProps_Init(&props);
		props.level = level;
		props.dictSize = dict_size;
		LzmaEnc_SetProps(lzma_enc_, &props);
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
		int res = LzmaEnc_WriteProperties(lzma_enc_, &output[0], &out_props_size);
		Verify(0 == res);
		res = LzmaEnc_MemEncode(lzma_enc_, &output[LZMA_PROPS_SIZE], &out_len, static_cast<Byte const *>(input), static_cast<SizeT>(len),
			0, NULL, &lzma_alloc, &lzma_alloc);
		Verify(0 == res);

		uint64_t u64_out_len = static_cast<uint64_t>(out_len);
		output.resize(static_cast<size_t>(LZMA_PROPS_SIZE + u64_out_len));
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

		CLzmaDec dec;
		LzmaDec_Construct(&dec);
		int res = LzmaDec_AllocateProbs(&dec, &in_data[0], LZMA_PROPS_SIZE, &lzma_alloc);
		Verify(0 == res);

		dec.dic = static_cast<uint8_t*>(output);
		dec.dicBufSize = s_out_len;
		LzmaDec_Init(&dec);

		SizeT s_src_len = static_cast<SizeT>(len - LZMA_PROPS_SIZE);
		ELzmaStatus status;
		res = LzmaDec_DecodeToDic(&dec, s_out_len, &in_data[LZMA_PROPS_SIZE], &s_src_len, LZMA_FINISH_ANY, &status);
		Verify(0 == res);
		Verify(status != LZMA_STATUS_NEEDS_MORE_INPUT);
		Verify(dec.dicPos == original_len);

		LzmaDec_FreeProbs(&dec, &lzma_alloc);
	}
}
