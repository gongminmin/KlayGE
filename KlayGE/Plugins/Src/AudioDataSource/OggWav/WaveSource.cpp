// WaveSource.cpp
// KlayGE wave数据源类 实现文件
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
#include <KlayGE/Util.hpp>

#include <vector>
#include <cassert>

#include <KlayGE/OggWav/WaveSource.hpp>

namespace KlayGE
{
	enum
	{
		WaveFmt_PCM			= 0x01,
		//WaveFmt_IMAADPCM	= 0x11,
		//WaveFmt_MP3			= 0x55,
	};

	enum
	{
		frRIFF	= MakeFourCC<'R', 'I', 'F', 'F'>::value,
		frWAVE	= MakeFourCC<'W', 'A', 'V', 'E'>::value,
		frFmt	= MakeFourCC<'f', 'm', 't', ' '>::value,
		frData	= MakeFourCC<'d', 'a', 't', 'a'>::value,
	};

	#ifdef _MSC_VER
		#pragma pack (push, 1)						// 关闭对齐
	#endif

	struct WAVChunkHdr
	{
		FourCC		id;
		U32			size;
	};

	struct WAVFileHdr
	{
		WAVChunkHdr chuck;
		FourCC		type;
	};

	struct WaveFmt
	{
		U16			formatTag;
		U16			channels;
		U32			samplesPerSec;
		U32			avgBytesPerSec;
		U16			blockAlign;
	};

	struct PCMWaveFmt
	{
		WaveFmt		wf;
		U16			bitsPerSample;
	};

	struct WaveFmtEx
	{
		WaveFmt		wf;
		U16			bitsPerSample;
		U16			extraSize;
	};

	#ifdef _MSC_VER
		#pragma pack (pop)
	#endif

	// 构造函数
	/////////////////////////////////////////////////////////////////////////////////
	WaveSource::WaveSource(ResIdentifierPtr const & file)
	{
		wavFile_ = file;

		this->ReadMMIO();
		this->Reset();
	}

	// 读取文件头
	/////////////////////////////////////////////////////////////////////////////////
	void WaveSource::ReadMMIO()
	{
		wavFile_->seekg(0);

		WAVFileHdr fileHdr;
		wavFile_->read(reinterpret_cast<char*>(&fileHdr), sizeof(fileHdr));

		// 检查是否是一个有效的 Wave 文件
		assert(frRIFF == fileHdr.chuck.id);
		assert(frWAVE == fileHdr.type);

		PCMWaveFmt	pcmWaveFmt;
		WAVChunkHdr chunkHdr;
		for (;;)
		{
			wavFile_->read(reinterpret_cast<char*>(&chunkHdr), sizeof(chunkHdr));

			if (wavFile_->fail())
			{
				wavFile_->clear();
				break;
			}

			switch (chunkHdr.id)
			{
			case frFmt:
				wavFile_->read(reinterpret_cast<char*>(&pcmWaveFmt), sizeof(pcmWaveFmt));
				assert(WaveFmt_PCM == pcmWaveFmt.wf.formatTag);
				wavFile_->seekg(chunkHdr.size - sizeof(pcmWaveFmt), std::ios_base::cur);

				freq_ = pcmWaveFmt.wf.samplesPerSec;
				if (1 == pcmWaveFmt.wf.channels)
				{
					if (8 == pcmWaveFmt.bitsPerSample)
					{
						format_ = AF_Mono8;
					}
					else
					{
						format_ = AF_Mono16;
					}
				}
				else
				{
					if (8 == pcmWaveFmt.bitsPerSample)
					{
						format_ = AF_Stereo8;
					}
					else
					{
						format_ = AF_Stereo16;
					}
				}
				break;

			case frData:
				dataOffset_ = wavFile_->tellg();
				size_ = chunkHdr.size;
				wavFile_->seekg(chunkHdr.size, std::ios_base::cur);
				break;

			default:
				wavFile_->seekg(chunkHdr.size, std::ios_base::cur);
				break;
			}

			wavFile_->seekg(chunkHdr.size & 1, std::ios_base::cur);
		}
	}

	// 读取Wav数据
	/////////////////////////////////////////////////////////////////////////////////
	size_t WaveSource::Read(void* data, size_t size)
	{
		assert(data != NULL);

		std::istream::pos_type offset = wavFile_->tellg();
		wavFile_->read(static_cast<char*>(data), size);
		wavFile_->clear();

		return wavFile_->tellg() - offset;
	}

	// 数据源复位
	/////////////////////////////////////////////////////////////////////////////////
	void WaveSource::Reset()
	{
		wavFile_->seekg(dataOffset_);
	}

	// 返回数据源大小
	/////////////////////////////////////////////////////////////////////////////////
	size_t WaveSource::Size()
	{
		return size_;
	}
}
