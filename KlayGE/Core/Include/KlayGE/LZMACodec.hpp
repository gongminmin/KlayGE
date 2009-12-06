// LZMACodec.hpp
// KlayGE LZMACodec类 头文件
// Ver 3.9.0
// 版权所有(C) 龚敏敏, 2009
// Homepage: http://klayge.sourceforge.net
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

		uint64_t Encode(boost::shared_ptr<std::ostream> const & os, void const * input, uint64_t len);
		uint64_t Decode(boost::shared_ptr<std::ostream> const & os, ResIdentifierPtr const & res);

	private:
		void* lzma_enc_;
		void* lzma_dec_;
	};
}

#endif			// _LZMACODEC_HPP
