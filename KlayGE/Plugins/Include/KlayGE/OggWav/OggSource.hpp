// OggSource.hpp
// KlayGE Ogg音频数据源 头文件
// Ver 2.0.0
// 版权所有(C) 龚敏敏, 2003
// Homepage: http://www.enginedev.com
//
// 2.0.0
// 初次建立 (2003.7.7)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#ifndef _OGGSOURCE_HPP
#define _OGGSOURCE_HPP

#include <KlayGE/PreDeclare.hpp>

#include <KlayGE/AudioDataSource.hpp>

#include <vorbis/codec.h>

#pragma comment(lib, "KlayGE_AudioDataSource_OggWav.lib")

namespace KlayGE
{
	class OggSource : public AudioDataSource
	{
	public:
		explicit OggSource(ResIdentifierPtr const & file);
		~OggSource();

		size_t Size();

		size_t Read(void* data, size_t size);
		void Reset();

	private:
		ResIdentifierPtr	oggFile_;

		ogg_sync_state   oy_;	// sync and verify incoming physical bitstream
		ogg_stream_state os_;	// take physical pages, weld into a logical stream of packets
		ogg_page         og_;	// one Ogg bitstream page.  Vorbis packets are inside
		ogg_packet       op_;	// one raw packet of data for decode

		vorbis_info      vi_;	// struct that stores all the static vorbis bitstream settings
		vorbis_comment   vc_;	// struct that stores all the bitstream user comments
		vorbis_dsp_state vd_;	// central working state for the packet->PCM decoder
		vorbis_block     vb_;	// local working space for packet->PCM decode
	};
}

#endif			// _OGGSOURCE_HPP