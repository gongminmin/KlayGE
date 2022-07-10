// DShow.hpp
// KlayGE DirectShow ���������� ͷ�ļ�
// Ver 2.0.0
// ��Ȩ����(C) ������, 2003
// Homepage: http://www.klayge.org
//
// 2.0.0
// ���ν��� (2003.9.3)
//
// �޸ļ�¼
/////////////////////////////////////////////////////////////////////

#ifndef _DSHOW_HPP
#define _DSHOW_HPP

#pragma once

#include <string>

#include <KFL/com_ptr.hpp>
#include <KlayGE/Show.hpp>

struct IGraphBuilder;
struct IBaseFilter;
struct IMediaControl;
struct IMediaEvent;
struct IVMRSurfaceAllocator9;

namespace KlayGE
{
	class DShowEngine final : public ShowEngine
	{
	public:
		DShowEngine();
		~DShowEngine() override;

		bool IsComplete() override;

		void Load(std::string const & fileName) override;
		TexturePtr PresentTexture() override;

		ShowState State(long msTimeout = -1) override;

	private:
		com_ptr<IGraphBuilder> graph_;
		com_ptr<IBaseFilter> filter_;
		com_ptr<IMediaControl> media_control_;
		com_ptr<IMediaEvent> media_event_;
		com_ptr<IVMRSurfaceAllocator9> vmr_allocator_;

	private:
		void Init();
		void Free();

		void DoSuspend() override;
		void DoResume() override;

		void DoPlay() override;
		void DoStop() override;
		void DoPause() override;
	};
}

#endif		// _DSHOW_HPP
