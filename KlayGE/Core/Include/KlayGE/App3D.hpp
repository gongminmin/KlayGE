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
#include <KlayGE/Timer.hpp>
#include <KlayGE/RenderSettings.hpp>

namespace KlayGE
{
	// 一个用于创建3D应用程序框架的基类。建立一个3D应用程序需要继承这个类，
	//			然后重载以下函数:
	//
	//			InitObjects()			- 初始化3D设备
	//			Update()				- 刷新场景
	//			DelObjects()			- 清除3D场景
	/////////////////////////////////////////////////////////////////////////////////
	class KLAYGE_CORE_API App3DFramework
	{
		friend class SceneManager;

	public:
		enum UpdateRetVal
		{
			URV_Need_Flush = 1UL << 0,
			URV_Flushed = 1UL << 1,
			URV_Finished = 1UL << 2
		};

	public:
		explicit App3DFramework(std::string const & name);
		virtual ~App3DFramework();

		virtual void Create();
		virtual void Destroy();

		WindowPtr MakeWindow(std::string const & name, RenderSettings const & settings);
		WindowPtr MainWnd() const
		{
			return main_wnd_;
		}

		virtual bool ConfirmDevice() const
		{
			return true;
		}

		Camera const & ActiveCamera() const;
		Camera& ActiveCamera();

		float FPS() const;

		void Run();
		void Quit();

		virtual void OnResize(uint32_t width, uint32_t height);

	protected:
		void LookAt(float3 const & eye, float3 const & lookAt);
		void LookAt(float3 const & eye, float3 const & lookAt, float3 const & up);
		void Proj(float nearPlane, float farPlane);

	protected:
		virtual void InitObjects()
		{
		}

		uint32_t Update(uint32_t pass);
		void UpdateStats();

		virtual void RenderOver()
		{
		}

		virtual void DelObjects()
		{
		}

	private:
		virtual void DoUpdateOverlay() = 0;
		virtual uint32_t DoUpdate(uint32_t pass) = 0;

	protected:
		std::string name_;

		// Stats
		float	fps_;
		float	frame_time_;
		float	accumulate_time_;
		uint32_t num_frames_;

		Timer timer_;

		WindowPtr main_wnd_;
	};
}

#endif		// _APP3D_HPP
