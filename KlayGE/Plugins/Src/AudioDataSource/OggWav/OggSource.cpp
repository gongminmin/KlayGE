// OggSource.cpp
// KlayGE Ogg Vorbis数据源类 实现文件
// Ver 2.0.0
// 版权所有(C) 龚敏敏, 2003
// Homepage: http://www.enginedev.com
//
// 2.0.0
// 初次建立 (2003.7.7)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KlayGE/ThrowErr.hpp>
#include <KlayGE/SharedPtr.hpp>
#include <KlayGE/Math.hpp>
#include <KlayGE/Memory.hpp>
#include <KlayGE/Engine.hpp>
#include <KlayGE/VFile.hpp>
#include <KlayGE/AudioDataSource.hpp>

#include <vector>
#include <cassert>

#include <KlayGE/OggWav/OggSource.hpp>

#pragma comment(lib, "ogg.lib")
#pragma comment(lib, "vorbis.lib")

const size_t READSIZE(4096);

namespace KlayGE
{
	// 构造函数
	/////////////////////////////////////////////////////////////////////////////////
	OggSource::OggSource(const VFile& file)
				: oggFile_(file.Clone())
	{
		ogg_sync_init(&oy_); // Now we can read pages

		// grab some data at the head of the stream.  We want the first page
		// (which is guaranteed to be small and only contain the Vorbis
		// stream initial header) We need the first page to get the stream
		// serialno.

		// submit a 4k block to libvorbis' Ogg layer
		char* buffer(ogg_sync_buffer(&oy_, READSIZE));
		int bytes(oggFile_->Read(buffer, READSIZE));
		ogg_sync_wrote(&oy_, bytes);

		// Get the first page
		// 确认是不是Vorbis数据
		Verify(1 == ogg_sync_pageout(&oy_, &og_));

		// Get the serial number and set up the rest of decode.
		// serialno first; use it to set up a logical stream
		ogg_stream_init(&os_, ogg_page_serialno(&og_));


		vorbis_info_init(&vi_);
		vorbis_comment_init(&vc_);
		// 确认版本号是否正确
		Verify(ogg_stream_pagein(&os_, &og_) >= 0);

		// 确认是正确的Vorbis
		Verify(1 == ogg_stream_packetout(&os_ ,&op_));

		// 确认是Vorbis头
		Verify(vorbis_synthesis_headerin(&vi_, &vc_, &op_) >= 0);

		format_ = (1 == vi_.channels) ? AF_Mono16 : AF_Stereo16;
		freq_ = vi_.rate;

		int i(0);
		while (i < 2)
		{
			while (i < 2)
			{
				int result(ogg_sync_pageout(&oy_, &og_));

				if (0 == result)
				{
					break; // Need more data
				}

				// Don't complain about missing or corrupt data yet.  We'll catch it at the packet output phase
				if (1 == result)
				{
					ogg_stream_pagein(&os_, &og_); // we can ignore any errors here as they'll also become apparent
					// at packetout
					while (i < 2)
					{
						result = ogg_stream_packetout(&os_,&op_);
						if (0 == result)
						{
							break;
						}
						vorbis_synthesis_headerin(&vi_, &vc_, &op_);
						++ i;
					}
				}
			}

			// no harm in not checking before adding more
			buffer = ogg_sync_buffer(&oy_, 4096);
			bytes = oggFile_->Read(buffer, 4096);
			ogg_sync_wrote(&oy_, bytes);
		}

		// OK, got and parsed all three headers. Initialize the Vorbis packet->PCM decoder
		vorbis_synthesis_init(&vd_, &vi_);	// central decode state
		vorbis_block_init(&vd_, &vb_);		// local state for most of the decode
		// so multiple block decodes can
		// proceed in parallel.  We could init
		// multiple vorbis_block structures
		// for vd here

		this->Reset();
	}

	// 析构函数
	/////////////////////////////////////////////////////////////////////////////////
	OggSource::~OggSource()
	{
		ogg_stream_clear(&os_);

		vorbis_block_clear(&vb_);
		vorbis_dsp_clear(&vd_);
		vorbis_comment_clear(&vc_);
		vorbis_info_clear(&vi_);  // must be called last

		// OK, clean up the framer
		ogg_sync_clear(&oy_);
	}

	// 读取Ogg数据
	/////////////////////////////////////////////////////////////////////////////////
	size_t OggSource::Read(U8* data, size_t size)
	{
		assert(data != NULL);

		std::vector<ogg_int16_t> convbuffer(size / sizeof(ogg_int16_t));
		int leftsamples(convbuffer.size() / vi_.channels);

		size_t cursize(0);

		while (leftsamples > 0)
		{
			while (leftsamples > 0)
			{
				int result(ogg_sync_pageout(&oy_ , &og_));

				if (0 == result)
				{
					// need more data
					break;
				}

				if (result > 0)
				{
					ogg_stream_pagein(&os_, &og_); // can safely ignore errors at this point

					while (leftsamples > 0)
					{
						result = ogg_stream_packetout(&os_, &op_);

						if (0 == result)
						{
							// need more data
							break;
						}

						if (result > 0)
						{
							// we have a packet.  Decode it
							if (0 == vorbis_synthesis(&vb_, &op_)) // test for success!
							{
								vorbis_synthesis_blockin(&vd_, &vb_);
							}

							/* 
							**pcm is a multichannel float vector.  In stereo, for
							example, pcm[0] is left, and pcm[1] is right.  samples is
							the size of each channel.  Convert the float values
							(-1.<=range<=1.) to whatever PCM format and write it out */
							float** pcm;
							int samples;
							while ((leftsamples > 0) && ((samples = vorbis_synthesis_pcmout(&vd_, &pcm)) > 0))
							{
								int bout(std::min(samples, leftsamples));
								leftsamples -= bout;

								// convert floats to 16 bit signed ints (host order) and interleave
								for (int i = 0; i < vi_.channels; ++ i)
								{
									ogg_int16_t* ptr(&convbuffer[i]);
									float* mono(pcm[i]);

									for (int j = 0; j < bout; ++ j)
									{
										*ptr = Engine::MathInstance().Limit<int>(static_cast<int>(mono[j] * 32767.0f), -32768, 32767);
										ptr += vi_.channels;
									}
								}

								vorbis_synthesis_read(&vd_, bout);	// tell libvorbis how
																	// many samples we
																	// actually consumed

								// 把解码后的数据放入缓冲区
								const size_t length(sizeof(ogg_int16_t) * bout * vi_.channels);
								Engine::MemoryInstance().Cpy(&data[cursize], &convbuffer[0], length);
								cursize += length;
							}
						}
					}
				}
			}

			if (leftsamples > 0)
			{
				char* buffer(ogg_sync_buffer(&oy_, READSIZE));
				int bytes(oggFile_->Read(buffer, READSIZE));
				ogg_sync_wrote(&oy_, bytes);

				if (0 == bytes)
				{
					leftsamples = 0;
				}
			}
		}

		return cursize;
	}

	// 返回数据源大小。因为是流式结构，所以返回0
	/////////////////////////////////////////////////////////////////////////////////
	size_t OggSource::Size()
	{
		return 0;
	}

	// 数据源复位
	/////////////////////////////////////////////////////////////////////////////////
	void OggSource::Reset()
	{
		oggFile_->Rewind();
	}
}
