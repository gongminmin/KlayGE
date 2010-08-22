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
#ifdef KLAYGE_COMPILER_MSVC
#pragma warning(push)
#pragma warning(disable: 4244)
#endif
#include <vorbis/vorbisfile.h>
#ifdef KLAYGE_COMPILER_MSVC
#pragma warning(pop)
#endif

namespace KlayGE
{
	class OggVorbisSource : public AudioDataSource
	{
	public:
		OggVorbisSource();
		~OggVorbisSource();

		void Open(ResIdentifierPtr const & file);
		void Close();

		size_t Size();

		size_t Read(void* data, size_t size);
		void Reset();

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
