// Timer.hpp
// KlayGE 定时器 头文件
// Ver 2.0.0
// 版权所有(C) 龚敏敏, 2003
// Homepage: http://www.enginedev.com
//
// 2.0.0
// 初次建立 (2003.8.6)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#ifndef _TIMER_HPP
#define _TIMER_HPP

#pragma comment(lib, "KlayGE_Core.lib")

namespace KlayGE
{
	// 操作定时器，返回单位: 秒，单件模式
	/////////////////////////////////////////////////////////////////////////////////
	class Timer
	{
	public:
		static Timer& Instance();

		void Reset();
		void Start();
		void Stop();
		float AbsoluteTime();
		float AppTime();

	private:
		Timer();

		float Time();

		float	baseTime_;
		float	stopTime_;
		bool	timerStopped_;
	};
}

#endif		// _TIMER_HPP