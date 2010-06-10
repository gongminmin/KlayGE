// LZMACodec.hpp
// KlayGE LZMACodec类 头文件
// Ver 3.9.0
// 版权所有(C) 龚敏敏, 2009
// Homepage: http://www.klayge.org
//
// 3.9.0
// 初次建立 (2009.12.6)
//
// 修改记录
//////////////////////////////////////////////////////////////////////////////////

#ifndef _LZMACODEC_HPP
#define _LZMACODEC_HPP

#pragma once

#ifndef KLAYGE_CORE_SOURCE
#define KLAYGE_LIB_NAME KlayGE_Core
#include <KlayGE/config/auto_link.hpp>
#endif

#include <KlayGE/PreDeclare.hpp>

namespace KlayGE
{
	class KLAYGE_CORE_API LZMACodec
	{
	public:
		LZMACodec();
		~LZMACodec();

		void EncodeProps(int level, uint32_t dict_size);

		uint64_t Encode(std::ostream& os, ResIdentifierPtr const & res, uint64_t len);
		uint64_t Encode(std::ostream& os, void const * input, uint64_t len);
		void Encode(std::vector<uint8_t>& output, ResIdentifierPtr const & res, uint64_t len);
		void Encode(std::vector<uint8_t>& output, void const * input, uint64_t len);

		uint64_t Decode(std::ostream& os, ResIdentifierPtr const & res, uint64_t len, uint64_t original_len);
		uint64_t Decode(std::ostream& os, void const * input, uint64_t len, uint64_t original_len);
		void Decode(std::vector<uint8_t>& output, ResIdentifierPtr const & res, uint64_t len, uint64_t original_len);
		void Decode(std::vector<uint8_t>& output, void const * input, uint64_t len, uint64_t original_len);

	private:
		void* lzma_enc_;
		void* lzma_dec_;
	};
}

#endif			// _LZMACODEC_HPP
