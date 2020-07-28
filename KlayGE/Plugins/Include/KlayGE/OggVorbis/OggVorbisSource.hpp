// OggVorbisSource.hpp
// KlayGE Ogg音频数据源 头文件
// Ver 3.6.0
// 版权所有(C) 龚敏敏, 2003-2007
// Homepage: http://www.klayge.org
//
// 3.6.0
// 用vorbisfile重写，可以返回原始大小 (2007.7.12)
//
// 2.0.0
// 初次建立 (2003.7.7)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#ifndef _OGGVORBISSOURCE_HPP
#define _OGGVORBISSOURCE_HPP

#pragma once

#include <KlayGE/PreDeclare.hpp>

#include <KlayGE/AudioDataSource.hpp>

#include <istream>
#include <vorbis/codec.h>
#if defined(KLAYGE_COMPILER_GCC)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-variable" // Ignore OV_CALLBACKS_DEFAULT
#endif
#include <vorbis/vorbisfile.h>
#if defined(KLAYGE_COMPILER_GCC)
#pragma GCC diagnostic pop
#endif

namespace KlayGE
{
	class OggVorbisSource final : public AudioDataSource
	{
	public:
		OggVorbisSource() noexcept;
		~OggVorbisSource() override;

		void Open(ResIdentifierPtr const & file) override;
		void Close() override;

		size_t Size() override;

		size_t Read(void* data, size_t size) override;
		void Reset() override;

	private:
		static size_t VorbisRead(void* ptr, size_t byteSize, size_t sizeToRead, void* datasource);
		static int VorbisSeek(void* datasource, ogg_int64_t offset, int whence);
		static int VorbisClose(void* datasource);
		static long VorbisTell(void* datasource);

	private:
		ResIdentifierPtr oggFile_;
		int64_t length_;

		OggVorbis_File vf_;
	};
}

#endif			// _OGGVORBISSOURCE_HPP
