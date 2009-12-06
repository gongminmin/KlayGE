// LZMACodec.cpp
// KlayGE LZMACodec类 实现文件
// Ver 3.9.0
// 版权所有(C) 龚敏敏, 2009
// Homepage: http://klayge.sourceforge.net
//
// 3.9.0
// 初次建立 (2009.12.6)
//
// 修改记录
//////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KlayGE/ThrowErr.hpp>
#include <KlayGE/ResLoader.hpp>

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

	uint64_t LZMACodec::Encode(boost::shared_ptr<std::ostream> const & os, void const * input, uint64_t len)
	{
		SizeT out_len = static_cast<SizeT>(len * 11 / 10);
		std::vector<uint8_t> out_buf(LZMA_PROPS_SIZE + out_len);
		SizeT out_props_size = LZMA_PROPS_SIZE;
		int res = LzmaEnc_WriteProperties(lzma_enc_, &out_buf[0], &out_props_size);
		Verify(0 == res);
		res = LzmaEnc_MemEncode(lzma_enc_, &out_buf[LZMA_PROPS_SIZE], &out_len, static_cast<Byte const *>(input), static_cast<SizeT>(len),
			0, NULL, &lzma_alloc, &lzma_alloc);
		Verify(0 == res);

		uint64_t u64_out_len = static_cast<uint64_t>(out_len);
		if (os)
		{
			os->write(reinterpret_cast<char*>(&len), sizeof(len));
			os->write(reinterpret_cast<char*>(&u64_out_len), sizeof(u64_out_len));
			os->write(reinterpret_cast<char*>(&out_buf[0]), static_cast<std::streamsize>(LZMA_PROPS_SIZE + u64_out_len));
		}
		return u64_out_len;
	}

	uint64_t LZMACodec::Decode(boost::shared_ptr<std::ostream> const & os, ResIdentifierPtr const & is)
	{
		uint64_t out_len;
		is->read(&out_len, sizeof(out_len));

		uint64_t in_len;
		is->read(&in_len, sizeof(in_len));

		std::vector<uint8_t> in_data(static_cast<size_t>(LZMA_PROPS_SIZE + in_len));
		is->read(&in_data[0], static_cast<std::streamsize>(in_data.size()));

		SizeT s_out_len = static_cast<SizeT>(out_len);
		std::vector<uint8_t> out_data(s_out_len);

		CLzmaDec p;
		LzmaDec_Construct(&p);
		int res = LzmaDec_AllocateProbs(&p, &in_data[0], LZMA_PROPS_SIZE, &lzma_alloc);
		Verify(0 == res);

		p.dic = &out_data[0];
		p.dicBufSize = s_out_len;
		LzmaDec_Init(&p);

		SizeT s_src_len = static_cast<SizeT>(in_len);
		ELzmaStatus status;
		res = LzmaDec_DecodeToDic(&p, s_out_len, &in_data[LZMA_PROPS_SIZE], &s_src_len, LZMA_FINISH_ANY, &status);
		Verify(0 == res);
		Verify(status != LZMA_STATUS_NEEDS_MORE_INPUT);

		s_out_len = p.dicPos;
		LzmaDec_FreeProbs(&p, &lzma_alloc);

		if (os)
		{
			os->write(reinterpret_cast<char*>(&out_data[0]), s_out_len);
		}

		return s_out_len;
	}
}
