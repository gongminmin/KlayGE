// AudioDataSource.hpp
// KlayGE 音频数据源引擎 头文件
// Ver 2.0.0
// 版权所有(C) 龚敏敏, 2003
// Homepage: http://www.enginedev.com
//
// 2.0.0
// 初次建立 (2003.7.7)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#ifndef _AUDIODATASOURCE_HPP
#define _AUDIODATASOURCE_HPP

#include <KlayGE/PreDeclare.hpp>

namespace KlayGE
{
	enum AudioFormat
	{
		AF_Mono8,
		AF_Mono16,
		AF_Stereo8,
		AF_Stereo16
	};

	class AudioDataSource
	{
	public:
		AudioFormat Format() const
			{ return this->format_; }
		U32 Freq() const
			{ return this->freq_; }

		virtual size_t Size() = 0;

		virtual size_t Read(U8* data, size_t size) = 0;
		virtual void Reset() = 0;

		virtual ~AudioDataSource()
			{ }

	protected:
		AudioFormat		format_;
		U32				freq_;
	};
}

#endif			// _AUDIODATASOURCE_HPP