// DShow.hpp
// KlayGE DirectShow 播放引擎类 头文件
// Ver 2.0.0
// 版权所有(C) 龚敏敏, 2003
// Homepage: http://www.enginedev.com
//
// 2.0.0
// 初次建立 (2003.9.3)
//
// 修改记录
/////////////////////////////////////////////////////////////////////

#ifndef _DSHOW_HPP
#define _DSHOW_HPP

#include <KlayGE/COMPtr.hpp>
#include <KlayGE/Show.hpp>
#include <strmif.h>
#include <control.h>
#include <evcode.h>

#include <string>

#include <boost/utility.hpp>

#pragma comment(lib, "KlayGE_ShowEngine_DShow.lib")

namespace KlayGE
{
	const long OAFALSE = 0;
	const long OATRUE  = -1;

	class DShowEngine : boost::noncopyable, public ShowEngine
	{
	public:
		DShowEngine();
		~DShowEngine();

		bool IsComplete();

		void Load(const std::wstring& fileName);

		ShowState State(long msTimeout = -1);
		void ToggleFullScreen();

	private:
		HWND		hWnd_;
		bool		audioOnly_;

		COMPtr<IGraphBuilder>	graph_;
		COMPtr<IMediaControl>	mediaControl_;
		COMPtr<IMediaEvent>		mediaEvent_;
		COMPtr<IVideoWindow>	videoWnd_;

	private:
		void CheckVisibility();

		void Init();
		void Free();

		void DoPlay();
		void DoStop();
		void DoPause();
	};
}

#endif		// _DSHOW_HPP
