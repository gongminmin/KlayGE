// DShowEngine.cpp
// KlayGE DirectShow �������� ʵ���ļ�
// Ver 2.0.0
// ��Ȩ����(C) ������, 2003
// Homepage: http://www.klayge.org
//
// 2.0.0
// ���ν��� (2003.9.3)
//
// �޸ļ�¼
/////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#define _WIN32_DCOM
#include <KFL/ErrorHandling.hpp>
#include <KFL/Util.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/App3D.hpp>
#include <KlayGE/Window.hpp>

#include <boost/assert.hpp>
#include <windows.h>
#include <combaseapi.h>
#include <uuids.h>
#include <control.h>
#if defined(KLAYGE_COMPILER_GCC)
#pragma GCC diagnostic push
// Those GCC diagnostic ignored lines don't work, because those warnings are emitted by preprocessor
#pragma GCC diagnostic ignored "-Wcomment" // Ignore "/*" within block comment
#pragma GCC diagnostic ignored "-Wunknown-pragmas" // Ignore unknown pragmas
#elif defined(KLAYGE_COMPILER_CLANGCL)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wcomment" // Ignore "/*" within block comment
#endif
#include <d3d9.h>
#if defined(KLAYGE_COMPILER_GCC)
#pragma GCC diagnostic pop
#elif defined(KLAYGE_COMPILER_CLANGCL)
#pragma clang diagnostic pop
#endif
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

#include <KlayGE/DShow/DShowVMR9Allocator.hpp>
#include <KlayGE/DShow/DShow.hpp>

DEFINE_UUID_OF(IBaseFilter);
DEFINE_UUID_OF(IGraphBuilder);
DEFINE_UUID_OF(IMediaControl);
DEFINE_UUID_OF(IMediaEvent);
DEFINE_UUID_OF(IVMRFilterConfig9);
DEFINE_UUID_OF(IVMRSurfaceAllocatorNotify9);

namespace KlayGE
{
	// ���캯��
	/////////////////////////////////////////////////////////////////////////////////
	DShowEngine::DShowEngine()
	{
		if (SUCCEEDED(::CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED)))
		{
			this->Init();
		}
	}

	// ��������
	/////////////////////////////////////////////////////////////////////////////////
	DShowEngine::~DShowEngine()
	{
		this->Free();

		::CoUninitialize();
	}

	// ��ʼ��
	/////////////////////////////////////////////////////////////////////////////////
	void DShowEngine::Init()
	{
		state_ = SS_Uninit;
	}

	// �ͷ�
	/////////////////////////////////////////////////////////////////////////////////
	void DShowEngine::Free()
	{
		this->Stop();

		vmr_allocator_.reset();
		media_event_.reset();
		media_control_.reset();
		filter_.reset();
		graph_.reset();
	}

	void DShowEngine::DoSuspend()
	{
		// TODO
	}

	void DShowEngine::DoResume()
	{
		// TODO
	}

	// ����
	/////////////////////////////////////////////////////////////////////////////////
	void DShowEngine::DoPlay()
	{
		TIFHR(media_control_->Run());
	}

	// ��ͣ����
	/////////////////////////////////////////////////////////////////////////////////
	void DShowEngine::DoPause()
	{
		TIFHR(media_control_->Pause());
	}

	// ֹͣ����
	/////////////////////////////////////////////////////////////////////////////////
	void DShowEngine::DoStop()
	{
		TIFHR(media_control_->Stop());
	}

	// �����ļ�
	/////////////////////////////////////////////////////////////////////////////////
	void DShowEngine::Load(std::string const & fileName)
	{
		this->Free();
		this->Init();

		TIFHR(::CoCreateInstance(CLSID_FilterGraph, nullptr, CLSCTX_ALL,
			UuidOf<IGraphBuilder>(), graph_.put_void()));

		TIFHR(::CoCreateInstance(CLSID_VideoMixingRenderer9, nullptr, CLSCTX_INPROC_SERVER,
			UuidOf<IBaseFilter>(), filter_.put_void()));

		auto filter_config = filter_.as<IVMRFilterConfig9>();

		TIFHR(filter_config->SetRenderingMode(VMR9Mode_Renderless));
		TIFHR(filter_config->SetNumberOfStreams(1));

		auto vmr_surf_alloc_notify = filter_.as<IVMRSurfaceAllocatorNotify9>();

		// create our surface allocator
		vmr_allocator_.reset(new DShowVMR9Allocator(Context::Instance().AppInstance().MainWnd()->HWnd()), false);

		// let the allocator and the notify know about each other
		TIFHR(vmr_surf_alloc_notify->AdviseSurfaceAllocator(static_cast<DWORD_PTR>(DShowVMR9Allocator::USER_ID),
			vmr_allocator_.get()));
		TIFHR(vmr_allocator_->AdviseNotify(vmr_surf_alloc_notify.get()));

		TIFHR(graph_->AddFilter(filter_.get(), L"Video Mixing Renderer 9"));

		graph_.as(media_control_);
		graph_.as(media_event_);

		std::wstring fn;
		Convert(fn, fileName);
		TIFHR(graph_->RenderFile(fn.c_str(), nullptr));

		state_ = SS_Stopped;
	}

	// ��鲥���Ƿ����
	/////////////////////////////////////////////////////////////////////////////////
	bool DShowEngine::IsComplete()
	{
		long lEventCode, lParam1, lParam2;
		bool ret(false);

		HRESULT hr(media_event_->GetEvent(&lEventCode, reinterpret_cast<LONG_PTR*>(&lParam1),
							reinterpret_cast<LONG_PTR*>(&lParam2), 0));
		if (SUCCEEDED(hr))
		{
			if (1 == lEventCode)	// EC_COMPLETE
			{
				ret = true;
			}

			// �ͷź�����¼���ص��ڴ�
			TIFHR(media_event_->FreeEventParams(lEventCode, lParam1, lParam2));
		}

		return ret;
	}

	// ��ȡ��ǰ״̬
	/////////////////////////////////////////////////////////////////////////////////
	ShowState DShowEngine::State(long msTimeout)
	{
		OAFilterState fs;
		HRESULT hr(media_control_->GetState(msTimeout, &fs));
		if (FAILED(hr))
		{
			return SS_Unkown;
		}

		state_ = SS_Unkown;
		switch (fs)
		{
		case State_Stopped:
			state_ = SS_Stopped;
			break;

		case State_Paused:
			state_ = SS_Paused;
			break;

		case State_Running:
			state_ = SS_Playing;
			break;
		}

		return state_;
	}

	// ��ȡ��ʾ������
	/////////////////////////////////////////////////////////////////////////////////
	TexturePtr DShowEngine::PresentTexture()
	{
		return checked_cast<DShowVMR9Allocator*>(vmr_allocator_.get())->PresentTexture();
	}
}
