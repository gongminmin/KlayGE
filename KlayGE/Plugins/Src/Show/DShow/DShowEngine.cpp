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
#include <KFL/COMPtr.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/App3D.hpp>
#include <KlayGE/Window.hpp>

#include <boost/assert.hpp>
#include <uuids.h>

#include <KlayGE/DShow/DShowVMR9Allocator.hpp>
#include <KlayGE/DShow/DShow.hpp>

#ifdef KLAYGE_COMPILER_MSVC
#pragma comment(lib, "strmiids.lib")
#endif

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

		IGraphBuilder* graph;
		TIFHR(::CoCreateInstance(CLSID_FilterGraph, nullptr, CLSCTX_ALL,
			IID_IGraphBuilder, reinterpret_cast<void**>(&graph)));
		graph_ = MakeCOMPtr(graph);

		IBaseFilter* filter;
		TIFHR(::CoCreateInstance(CLSID_VideoMixingRenderer9, nullptr, CLSCTX_INPROC_SERVER,
			IID_IBaseFilter, reinterpret_cast<void**>(&filter)));
		filter_ = MakeCOMPtr(filter);

		std::shared_ptr<IVMRFilterConfig9> filter_config;
		{
			IVMRFilterConfig9* tmp;
			TIFHR(filter_->QueryInterface(IID_IVMRFilterConfig9, reinterpret_cast<void**>(&tmp)));
			filter_config = MakeCOMPtr(tmp);
		}

		TIFHR(filter_config->SetRenderingMode(VMR9Mode_Renderless));
		TIFHR(filter_config->SetNumberOfStreams(1));

		std::shared_ptr<IVMRSurfaceAllocatorNotify9> vmr_surf_alloc_notify;
		{
			IVMRSurfaceAllocatorNotify9* tmp;
			TIFHR(filter_->QueryInterface(IID_IVMRSurfaceAllocatorNotify9, reinterpret_cast<void**>(&tmp)));
			vmr_surf_alloc_notify = MakeCOMPtr(tmp);
		}

		// create our surface allocator
		vmr_allocator_ = MakeCOMPtr(new DShowVMR9Allocator(Context::Instance().AppInstance().MainWnd()->HWnd()));

		// let the allocator and the notify know about each other
		TIFHR(vmr_surf_alloc_notify->AdviseSurfaceAllocator(static_cast<DWORD_PTR>(DShowVMR9Allocator::USER_ID),
			vmr_allocator_.get()));
		TIFHR(vmr_allocator_->AdviseNotify(vmr_surf_alloc_notify.get()));

		TIFHR(graph_->AddFilter(filter_.get(), L"Video Mixing Renderer 9"));

		{
			IMediaControl* tmp;
			TIFHR(graph_->QueryInterface(IID_IMediaControl, reinterpret_cast<void**>(&tmp)));
			media_control_ = MakeCOMPtr(tmp);
		}
		{
			IMediaEvent* tmp;
			TIFHR(graph_->QueryInterface(IID_IMediaEvent, reinterpret_cast<void**>(&tmp)));
			media_event_ = MakeCOMPtr(tmp);
		}

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
		return checked_pointer_cast<DShowVMR9Allocator>(vmr_allocator_)->PresentTexture();
	}
}
