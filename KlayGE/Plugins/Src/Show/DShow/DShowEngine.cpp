// DShowEngine.cpp
// KlayGE DirectShow 播放引擎 实现文件
// Ver 2.0.0
// 版权所有(C) 龚敏敏, 2003
// Homepage: http://www.enginedev.com
//
// 2.0.0
// 初次建立 (2003.9.3)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KlayGE/ThrowErr.hpp>

#include <cassert>
#include <uuids.h>

#include <KlayGE/DShow/DShow.hpp>

#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "ole32.lib")
#pragma comment(lib, "strmiids.lib")

namespace KlayGE
{
	// 构造函数
	/////////////////////////////////////////////////////////////////////////////////
	DShowEngine::DShowEngine()
					: hWnd_(NULL)
	{
		::CoInitialize(0);

		this->Init();
	}

	// 析构函数
	/////////////////////////////////////////////////////////////////////////////////
	DShowEngine::~DShowEngine()
	{
		this->Free();

		::CoUninitialize();
	}

	// 初始化
	/////////////////////////////////////////////////////////////////////////////////
	void DShowEngine::Init()
	{
		state_ = SS_Uninit;
	}

	// 释放
	/////////////////////////////////////////////////////////////////////////////////
	void DShowEngine::Free()
	{
		this->Stop();

		videoWnd_.reset();
		mediaEvent_.reset();
		mediaControl_.reset();
		graph_.reset();
	}

	// 检查是否包含视频流
	/////////////////////////////////////////////////////////////////////////////////
	void DShowEngine::CheckVisibility()
	{
		audioOnly_ = false;

		if (!videoWnd_)
		{
			audioOnly_ = true;
			return;
		}

		long visible;
		HRESULT hr(videoWnd_->get_Visible(&visible));
		if (FAILED(hr))
		{
			// 如果是一个只包含音频流的剪辑，get_Visible()不工作
			// 同时，如果视频流是用不支持的方式压缩的，就不能看到视频。
			// 但是如果支持音频流的压缩方式，音频流就可以正常播放
			if (E_NOINTERFACE == hr)
			{
				audioOnly_ = true;
			}
		}
	}

	// 播放
	/////////////////////////////////////////////////////////////////////////////////
	void DShowEngine::DoPlay()
	{
		TIF(mediaControl_->Run());
	}

	// 暂停播放
	/////////////////////////////////////////////////////////////////////////////////
	void DShowEngine::DoPause()
	{
		TIF(mediaControl_->Pause());
	}

	// 停止播放
	/////////////////////////////////////////////////////////////////////////////////
	void DShowEngine::DoStop()
	{
		TIF(mediaControl_->Stop());
	}

	// 载入文件
	/////////////////////////////////////////////////////////////////////////////////
	void DShowEngine::Load(const std::wstring& fileName)
	{
		this->Free();
		this->Init();

		graph_.CoCreateInstance<IID_IGraphBuilder>(CLSID_FilterGraph);

		TIF(graph_->RenderFile(fileName.c_str(), NULL));

		// 获取 DirectShow 接口
		TIF(graph_.QueryInterface<IID_IMediaControl>(mediaControl_));
		TIF(graph_.QueryInterface<IID_IMediaEvent>(mediaEvent_));
		
		// 获取视频接口，如果是音频文件，这就没有用
		TIF(graph_.QueryInterface<IID_IVideoWindow>(videoWnd_));

		this->CheckVisibility();

		hWnd_ = ::GetForegroundWindow();
		if (hWnd_ != NULL)
		{
			if (!audioOnly_)
			{
				videoWnd_->put_Owner(reinterpret_cast<OAHWND>(hWnd_));
				videoWnd_->put_WindowStyle(WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN);
			}

			RECT rect;
			::GetClientRect(hWnd_, &rect);
			TIF(videoWnd_->SetWindowPosition(rect.left, rect.top, rect.right, rect.bottom));

			::UpdateWindow(hWnd_);
			::SetForegroundWindow(hWnd_);
			::SetFocus(hWnd_);
		}

		state_ = SS_Stopped;
	}

	// 检查播放是否完成
	/////////////////////////////////////////////////////////////////////////////////
	bool DShowEngine::IsComplete()
	{
		long lEventCode, lParam1, lParam2;
		bool ret(false);

		HRESULT hr(mediaEvent_->GetEvent(&lEventCode, reinterpret_cast<LONG_PTR*>(&lParam1),
							reinterpret_cast<LONG_PTR*>(&lParam2), 0));
		if (SUCCEEDED(hr))
		{
			if (EC_COMPLETE == lEventCode)
			{
				ret = true;
			}

			// 释放和这个事件相关的内存
			TIF(mediaEvent_->FreeEventParams(lEventCode, lParam1, lParam2));
		}

		return ret;
	}

	// 获取当前状态
	/////////////////////////////////////////////////////////////////////////////////
	ShowState DShowEngine::State(long msTimeout)
	{
		OAFilterState fs;
		HRESULT hr(mediaControl_->GetState(msTimeout, &fs));
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

	// 全屏 / 窗口模式切换
	/////////////////////////////////////////////////////////////////////////////////
	void DShowEngine::ToggleFullScreen()
	{
		static OAHWND hDrain(NULL);

		// 如果只是音频文件，就不用切换
		if (audioOnly_)
		{
			return;
		}

		assert(videoWnd_.Get() != NULL);

		// 获取当前状态
		long mode;
		videoWnd_->get_FullScreenMode(&mode);

		if (0 == mode)
		{
			// 保存当前消息流水线
			TIF(videoWnd_->get_MessageDrain(&hDrain));

			// 把消息流水线设置到主窗口
			TIF(videoWnd_->put_MessageDrain(reinterpret_cast<OAHWND>(hWnd_)));

			// 切换到全屏模式
			TIF(videoWnd_->put_FullScreenMode(-1));
		}
		else
		{
			// 切换回窗口模式
			TIF(videoWnd_->put_FullScreenMode(0));

			// 恢复消息流水线
			TIF(videoWnd_->put_MessageDrain(hDrain));

			// 视频窗口复位
			TIF(videoWnd_->SetWindowForeground(-1));

			// 回收键盘焦点
			::UpdateWindow(hWnd_);
			::SetForegroundWindow(hWnd_);
			::SetFocus(hWnd_);
		}
	}
}