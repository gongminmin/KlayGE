// DShow.hpp
// KlayGE DirectShow 播放引擎类 头文件
// Ver 2.0.0
// 版权所有(C) 龚敏敏, 2003
// Homepage: http://www.klayge.org
//
// 2.0.0
// 初次建立 (2003.9.3)
//
// 修改记录
/////////////////////////////////////////////////////////////////////

#ifndef _DSHOW_HPP
#define _DSHOW_HPP

#pragma once

#include <windows.h>
#include <control.h>
#include <d3d9.h>
#ifdef KLAYGE_COMPILER_GCC
#define _WIN32_WINNT_BACKUP _WIN32_WINNT
#undef _WIN32_WINNT
#define _WIN32_WINNT 0x0501
#endif
#include <strmif.h>
#ifdef KLAYGE_COMPILER_GCC
#undef _WIN32_WINNT
#define _WIN32_WINNT _WIN32_WINNT_BACKUP
#endif
#include <vmr9.h>

#include <string>

#include <KlayGE/Show.hpp>

namespace KlayGE
{
	class DShowEngine : public ShowEngine
	{
	public:
		DShowEngine();
		~DShowEngine();

		bool IsComplete();

		void Load(std::string const & fileName);
		TexturePtr PresentTexture();

		ShowState State(long msTimeout = -1);

	private:
		std::shared_ptr<IGraphBuilder>		graph_;
		std::shared_ptr<IBaseFilter>		filter_;
		std::shared_ptr<IMediaControl>		media_control_;
		std::shared_ptr<IMediaEvent>		media_event_;
		std::shared_ptr<IVMRSurfaceAllocator9> vmr_allocator_;

	private:
		void Init();
		void Free();

		virtual void DoSuspend() override;
		virtual void DoResume() override;

		void DoPlay();
		void DoStop();
		void DoPause();
	};
}

#endif		// _DSHOW_HPP
