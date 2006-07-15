// DShow.hpp
// KlayGE DirectShow 播放引擎类 头文件
// Ver 2.0.0
// 版权所有(C) 龚敏敏, 2003
// Homepage: http://klayge.sourceforge.net
//
// 2.0.0
// 初次建立 (2003.9.3)
//
// 修改记录
/////////////////////////////////////////////////////////////////////

#ifndef _DSHOW_HPP
#define _DSHOW_HPP

#include <boost/smart_ptr.hpp>
#include <strmif.h>
#include <control.h>
#include <d3d9.h>
#include <strmif.h>
#include <vmr9.h>

#include <string>

#include <boost/utility.hpp>

#include <KlayGE/Show.hpp>

#ifdef KLAYGE_DEBUG
	#pragma comment(lib, "KlayGE_ShowEngine_DShow_d.lib")
#else
	#pragma comment(lib, "KlayGE_ShowEngine_DShow.lib")
#endif

namespace KlayGE
{
	long const OAFALSE = 0;
	long const OATRUE  = -1;

	class DShowEngine : boost::noncopyable, public ShowEngine
	{
	public:
		DShowEngine();
		~DShowEngine();

		bool IsComplete();

		void Load(std::wstring const & fileName, TexturePtr tex);

		ShowState State(long msTimeout = -1);

	private:
		boost::shared_ptr<IGraphBuilder>	graph_;
		boost::shared_ptr<IBaseFilter>      filter_;
		boost::shared_ptr<IMediaControl>	mediaControl_;
		boost::shared_ptr<IMediaEvent>		mediaEvent_;
		boost::shared_ptr<IVMRSurfaceAllocator9> vmr_allocator_;

	private:
		void Init();
		void Free();

		void DoPlay();
		void DoStop();
		void DoPause();
	};
}

#endif		// _DSHOW_HPP
