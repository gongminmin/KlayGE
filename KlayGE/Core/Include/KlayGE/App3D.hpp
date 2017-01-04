// App3D.hpp
// KlayGE App3D类 头文件
// Ver 3.11.0
// 版权所有(C) 龚敏敏, 2003-2010
// Homepage: http://www.klayge.org
//
// 3.11.0
// Add ConfirmDevice() (2010.8.15)
//
// 3.8.0
// 移入Core (2008.10.16)
//
// 3.7.0
// 改进了Update (2007.8.14)
//
// 3.6.0
// 增加了MakeWindow (2007.6.26)
//
// 3.1.0
// 增加了OnResize (2005.11.20)
//
// 3.0.0
// 增加了多遍更新 (2005.8.16)
//
// 2.7.0
// 增加了Quit (2005.6.28)
//
// 2.0.0
// 初次建立 (2003.7.10)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#ifndef _APP3D_HPP
#define _APP3D_HPP

#pragma once

#include <KlayGE/PreDeclare.hpp>
#include <KFL/Timer.hpp>

namespace KlayGE
{
	// 一个用于创建3D应用程序框架的基类。建立一个3D应用程序需要继承这个类，
	//			然后重载以下函数:
	//
	//			OnCreate()				- Called when the app is creating.
	//			OnDestroy()				- Called when the app is destroying.
	//			OnSuspend()				- Called when the app is suspending
	//			OnResume()				- Called when the app is resuming.
	//			DoUpdate()				- 刷新场景
	//			DoUpdateOverlay()		- 刷新Overlay物体
	/////////////////////////////////////////////////////////////////////////////////
	class KLAYGE_CORE_API App3DFramework
	{
		friend class SceneManager;

	public:
		enum UpdateRetValue
		{
			URV_NeedFlush = 1UL << 0,
			URV_Finished = 1UL << 1,
			URV_Overlay = 1UL << 2,
			URV_SkipPostProcess = 1UL << 3,
			URV_OpaqueOnly = 1UL << 4,
			URV_TransparencyBackOnly = 1UL << 5,
			URV_TransparencyFrontOnly = 1UL << 6,
			URV_ReflectionOnly = 1UL << 7,
			URV_SpecialShadingOnly = 1UL << 8,
			URV_SimpleForwardOnly = 1UL << 9,
			URV_VDMOnly = 1UL << 10
		};

	public:
		explicit App3DFramework(std::string const & name);
		App3DFramework(std::string const & name, void* native_wnd);
		virtual ~App3DFramework();

		void Create();
		void Destroy();
		void Suspend();
		void Resume();
		void Refresh();

		std::string const & Name() const
		{
			return name_;
		}

		WindowPtr MakeWindow(std::string const & name, RenderSettings const & settings);
		WindowPtr MakeWindow(std::string const & name, RenderSettings const & settings, void* native_wnd);
		WindowPtr const & MainWnd() const
		{
			return main_wnd_;
		}

		virtual bool ConfirmDevice() const;

		Camera const & ActiveCamera() const;
		Camera& ActiveCamera();

		uint32_t TotalNumFrames() const;
		float FPS() const;
		float AppTime() const;
		float FrameTime() const;

		void Run();
		void Quit();

		virtual void OnResize(uint32_t width, uint32_t height);

	protected:
		void LookAt(float3 const & eye, float3 const & lookAt);
		void LookAt(float3 const & eye, float3 const & lookAt, float3 const & up);
		void Proj(float nearPlane, float farPlane);

	protected:
		uint32_t Update(uint32_t pass);
		void UpdateStats();

	private:
		virtual void OnCreate()
		{
		}
		virtual void OnDestroy()
		{
		}
		virtual void OnSuspend()
		{
		}
		virtual void OnResume()
		{
		}

		virtual void DoUpdateOverlay() = 0;
		virtual uint32_t DoUpdate(uint32_t pass) = 0;

	protected:
		std::string name_;

		// Stats
		uint32_t total_num_frames_;
		float	fps_;
		float	accumulate_time_;
		uint32_t num_frames_;

		Timer timer_;
		float app_time_;
		float frame_time_;

		WindowPtr main_wnd_;

#if defined KLAYGE_PLATFORM_WINDOWS_RUNTIME
	public:
		void MetroCreate();
		void MetroRun();
#endif
	};
}

#endif		// _APP3D_HPP
