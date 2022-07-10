// Show.hpp
// KlayGE 播放引擎类 头文件
// Ver 2.0.0
// 版权所有(C) 龚敏敏, 2003
// Homepage: http://www.klayge.org
//
// 2.0.0
// 初次建立 (2003.9.3)
//
// 修改记录
/////////////////////////////////////////////////////////////////////

#ifndef _SHOW_HPP
#define _SHOW_HPP

#pragma once

#include <KlayGE/PreDeclare.hpp>

#include <string>

namespace KlayGE
{
	enum ShowState
	{
		SS_Unkown,
		SS_Uninit,
		SS_Stopped,
		SS_Paused,
		SS_Playing,
	};

	class KLAYGE_CORE_API ShowEngine : boost::noncopyable
	{
	public:
		ShowEngine() noexcept;
		virtual ~ShowEngine() noexcept;

		void Suspend();
		void Resume();

		bool CanPlay() const;
		bool CanStop() const;
		bool CanPause() const;
		bool IsInitialized() const;

		virtual bool IsComplete() = 0;

		virtual void Load(std::string const & fileName) = 0;
		virtual TexturePtr PresentTexture() = 0;

		void Play();
		void Stop();
		void Pause();

		virtual ShowState State(long timeout = -1) = 0;

	private:
		virtual void DoSuspend() = 0;
		virtual void DoResume() = 0;

		virtual void DoPlay() = 0;
		virtual void DoStop() = 0;
		virtual void DoPause() = 0;

	protected:
		ShowState state_{SS_Uninit};
		bool resume_playing_{false};
	};
}

#endif		// _SHOW_HPP
