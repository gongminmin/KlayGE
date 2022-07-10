// OggVorbisSource.cpp
// KlayGE Ogg Vorbis数据源类 实现文件
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

#include <KlayGE/KlayGE.hpp>
#include <KFL/ErrorHandling.hpp>
#include <KlayGE/ResLoader.hpp>
#include <KlayGE/AudioDataSource.hpp>

#include <vector>
#include <cstring>
#include <boost/assert.hpp>

#include <KlayGE/OggVorbis/OggVorbisSource.hpp>

namespace KlayGE
{
	// 构造函数
	/////////////////////////////////////////////////////////////////////////////////
	OggVorbisSource::OggVorbisSource() noexcept = default;

	// 析构函数
	/////////////////////////////////////////////////////////////////////////////////
	OggVorbisSource::~OggVorbisSource()
	{
		ov_clear(&vf_);
	}

	void OggVorbisSource::Open(ResIdentifierPtr const & file)
	{
		oggFile_ = file;

		oggFile_->seekg(0, std::ios_base::end);
		length_ = oggFile_->tellg();
		oggFile_->seekg(0, std::ios_base::beg);

		ov_callbacks vorbis_callbacks;
		vorbis_callbacks.read_func = OggVorbisSource::VorbisRead;
		vorbis_callbacks.close_func = OggVorbisSource::VorbisClose;
		vorbis_callbacks.seek_func = OggVorbisSource::VorbisSeek;
		vorbis_callbacks.tell_func = OggVorbisSource::VorbisTell;

		Verify(0 == ov_open_callbacks(this, &vf_, nullptr, 0, vorbis_callbacks));

		vorbis_info* vorbis_info = ov_info(&vf_, -1);
		format_ = (1 == vorbis_info->channels) ? AF_Mono16 : AF_Stereo16;
		freq_ = vorbis_info->rate;

		this->Reset();
	}

	void OggVorbisSource::Close()
	{
		oggFile_.reset();
		this->Reset();
	}

	// 读取Ogg数据
	/////////////////////////////////////////////////////////////////////////////////
	size_t OggVorbisSource::Read(void* data, size_t size)
	{
		BOOST_ASSERT(data != nullptr);

		char* pcm = static_cast<char*>(data);

		size_t cur_size = 0;
		int section;
		while (cur_size < size)
		{
			int result = ov_read(&vf_, pcm + cur_size, static_cast<int>(size - cur_size), 0, 2, 1, &section);

			if (result > 0)
			{
				cur_size += result;
			}
			else
			{
				break;
			}
		}

		return cur_size;
	}

	// 返回数据源大小
	/////////////////////////////////////////////////////////////////////////////////
	size_t OggVorbisSource::Size()
	{
		vorbis_info* vorbis_info = ov_info(&vf_, -1);
		return static_cast<size_t>(ov_pcm_total(&vf_, -1) * vorbis_info->channels * sizeof(ogg_int16_t));
	}

	// 数据源复位
	/////////////////////////////////////////////////////////////////////////////////
	void OggVorbisSource::Reset()
	{
		oggFile_->clear();
		ov_pcm_seek(&vf_, 0);
	}

	size_t OggVorbisSource::VorbisRead(void* ptr, size_t byte_size, size_t size_to_read, void* datasource)
	{
		// Get the data in the right format
		OggVorbisSource* vorbis_data = static_cast<OggVorbisSource*>(datasource);

		size_t actual_size_to_read;	// How much data we are actually going to read from memory
		// Calculate how much we need to read.  This can be sizeToRead*byteSize or less depending on how near the EOF marker we are
		size_t space_to_eof = static_cast<size_t>(vorbis_data->length_ - vorbis_data->oggFile_->tellg());
		if (size_to_read * byte_size < space_to_eof)
		{
			actual_size_to_read = size_to_read * byte_size;
		}
		else
		{
			actual_size_to_read = space_to_eof;
		}

		// A simple copy of the data from memory to the datastruct that the vorbis libs will use
		if (actual_size_to_read > 0)
		{
			vorbis_data->oggFile_->read(ptr, static_cast<std::streamsize>(actual_size_to_read));
		}

		// Return how much we read (in the same way fread would)
		return actual_size_to_read;
	}

	int OggVorbisSource::VorbisSeek(void* datasource, ogg_int64_t offset, int whence)
	{
		OggVorbisSource* vorbis_data = static_cast<OggVorbisSource*>(datasource);

		std::ios_base::seekdir dir;
		switch (whence)
		{
		case SEEK_SET: // Seek to the start of the data file
			dir = std::ios_base::beg;
			break;

		case SEEK_CUR: // Seek from where we are
			dir = std::ios_base::cur;
			break;

		case SEEK_END: // Seek from the end of the file
			dir = std::ios_base::end;
			break;

		default:
			KFL_UNREACHABLE("Invalid whence");
		};

		vorbis_data->oggFile_->seekg(static_cast<long>(offset), dir);

		return 0;
	}

	int OggVorbisSource::VorbisClose(void* datasource)
	{
		OggVorbisSource* vorbis_data = static_cast<OggVorbisSource*>(datasource);
		vorbis_data->oggFile_.reset();
		return 1;
	}

	long OggVorbisSource::VorbisTell(void* datasource)
	{
		OggVorbisSource* vorbis_data = static_cast<OggVorbisSource*>(datasource);
		return static_cast<long>(vorbis_data->oggFile_->tellg());
	}
}
