// AudioDataSource.hpp
// KlayGE 音频数据源引擎 头文件
// Ver 2.0.4
// 版权所有(C) 龚敏敏, 2003-2004
// Homepage: http://www.klayge.org
//
// 2.0.4
// 增加了NullObject (2004.4.7)
//
// 2.0.0
// 初次建立 (2003.7.7)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#ifndef _AUDIODATASOURCE_HPP
#define _AUDIODATASOURCE_HPP

#pragma once

#include <KlayGE/PreDeclare.hpp>

namespace KlayGE
{
	enum AudioFormat
	{
		AF_Mono8,
		AF_Mono16,
		AF_Stereo8,
		AF_Stereo16,

		AF_Unknown,
	};

	class KLAYGE_CORE_API AudioDataSource
	{
	public:
		virtual void Open(ResIdentifierPtr const & file) = 0;
		virtual void Close() = 0;

		AudioFormat Format() const;
		uint32_t Freq() const;

		virtual size_t Size() = 0;

		virtual size_t Read(void* data, size_t size) = 0;
		virtual void Reset() = 0;

		virtual ~AudioDataSource();

	protected:
		AudioFormat		format_;
		uint32_t				freq_;
	};

	class KLAYGE_CORE_API AudioDataSourceFactory
	{
	public:
		virtual ~AudioDataSourceFactory()
		{
		}

		void Suspend();
		void Resume();

		virtual std::wstring const & Name() const = 0;

		virtual AudioDataSourcePtr MakeAudioDataSource() = 0;

	private:
		virtual void DoSuspend() = 0;
		virtual void DoResume() = 0;
	};
}

#endif			// _AUDIODATASOURCE_HPP
