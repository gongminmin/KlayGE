/**
 * @file App3D.cpp
 * @author Minmin Gong
 *
 * @section DESCRIPTION
 *
 * This source file is part of KlayGE
 * For the latest info, see http://www.klayge.org
 *
 * @section LICENSE
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published
 * by the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 * You may alternatively use this source under the terms of
 * the KlayGE Proprietary License (KPL). You can obtained such a license
 * from http://www.klayge.org/licensing/.
 */

#ifndef KLAYGE_CORE_APP_3D_HPP
#define KLAYGE_CORE_APP_3D_HPP

#pragma once

#include <KlayGE/PreDeclare.hpp>
#include <KFL/Timer.hpp>

#include <KlayGE/Signal.hpp>

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
	class KLAYGE_CORE_API App3DFramework : boost::noncopyable
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

		Camera const & ActiveCamera() const;
		Camera& ActiveCamera();

		uint32_t TotalNumFrames() const;
		float FPS() const;
		float AppTime() const;
		float FrameTime() const;

		void Run();
		void Quit();

		virtual void OnResize(uint32_t width, uint32_t height);

	public:
		struct AndCombiner
		{
			using ResultType = bool;

			bool operator()(bool result)
			{
				combined_result_ &= result;
				return true;
			}

			ResultType Result()
			{
				return combined_result_;
			}

		private:
			bool combined_result_ = true;
		};

		using ConfirmDeviceSignal = Signal::Signal<bool(), AndCombiner>;

		ConfirmDeviceSignal& OnConfirmDevice()
		{
			return confirm_device_;
		}

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

	private:
		ConfirmDeviceSignal confirm_device_;

#if defined KLAYGE_PLATFORM_WINDOWS_STORE
	public:
		void MetroCreate();
		void MetroRun();
#endif
	};
}

#endif		// KLAYGE_CORE_APP_3D_HPP
