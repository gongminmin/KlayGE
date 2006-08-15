// App3D.cpp
// KlayGE App3D类 实现文件
// Ver 3.1.0
// 版权所有(C) 龚敏敏, 2003-2005
// Homepage: http://klayge.sourceforge.net
//
// 3.1.0
// 增加了OnResize (2005.11.20)
//
// 2.7.0
// 增加了Quit (2005.6.28)
//
// 2.0.0
// 初次建立 (2003.7.10)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KlayGE/Util.hpp>
#include <KlayGE/ThrowErr.hpp>
#include <KlayGE/Math.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/RenderWindow.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/RenderSettings.hpp>
#include <KlayGE/Input.hpp>
#include <KlayGE/InputFactory.hpp>

#include <boost/assert.hpp>
#include <windows.h>

#include <KlayGE/App3D.hpp>

namespace KlayGE
{
	// 构造函数
	/////////////////////////////////////////////////////////////////////////////////
	App3DFramework::App3DFramework()
						: fps_(0), accumulate_time_(0), num_frames_(0)
	{
		Context::Instance().AppInstance(*this);
	}

	App3DFramework::~App3DFramework()
	{
		this->DelObjects();
	}

	// 建立应用程序窗口和D3D接口
	/////////////////////////////////////////////////////////////////////////////////
	void App3DFramework::Create(std::string const & name, RenderSettings const & settings)
	{
		Context::Instance().RenderFactoryInstance().RenderEngineInstance().CreateRenderWindow(name, settings);

		this->InitObjects();
		this->OnResize(settings.width, settings.height);
	}

	void App3DFramework::Run()
	{
		Context::Instance().RenderFactoryInstance().RenderEngineInstance().StartRendering();

		this->DelObjects();
	}

	// 获取当前摄像机
	/////////////////////////////////////////////////////////////////////////////////
	Camera const & App3DFramework::ActiveCamera() const
	{
		RenderEngine& renderEngine(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		RenderTarget& activeRenderTarget(*renderEngine.CurRenderTarget());
		CameraPtr camera = activeRenderTarget.GetViewport().camera;
		BOOST_ASSERT(camera);

		return *camera;
	}

	Camera& App3DFramework::ActiveCamera()
	{
		RenderEngine& renderEngine(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		RenderTarget& activeRenderTarget(*renderEngine.CurRenderTarget());
		CameraPtr camera = activeRenderTarget.GetViewport().camera;
		BOOST_ASSERT(camera);

		return *camera;
	}

	// 设置观察矩阵
	/////////////////////////////////////////////////////////////////////////////////
	void App3DFramework::LookAt(float3 const & vEye, float3 const & vLookAt,
												float3 const & vUp)
	{
		this->ActiveCamera().ViewParams(vEye, vLookAt, vUp);
	}

	// 设置投射矩阵
	/////////////////////////////////////////////////////////////////////////////////
	void App3DFramework::Proj(float nearPlane, float farPlane)
	{
		BOOST_ASSERT(nearPlane != 0);
		BOOST_ASSERT(farPlane != 0);
		BOOST_ASSERT(farPlane > nearPlane);

		RenderEngine& renderEngine(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		RenderTarget& activeRenderTarget(*renderEngine.CurRenderTarget());

		this->ActiveCamera().ProjParams(PI / 4, static_cast<float>(activeRenderTarget.Width()) / activeRenderTarget.Height(),
			nearPlane, farPlane);
 	}

	// 退出程序
	/////////////////////////////////////////////////////////////////////////////////
	void App3DFramework::Quit()
	{
#ifdef KLAYGE_PLATFORM_WINDOWS
		::PostQuitMessage(0);
#else
		exit(0);
#endif
	}

	// 更新场景
	/////////////////////////////////////////////////////////////////////////////////
	void App3DFramework::Update(uint32_t pass)
	{
		if (0 == pass)
		{
			this->UpdateStats();

			InputEngine& inputEngine = Context::Instance().InputFactoryInstance().InputEngineInstance();
			inputEngine.Update();
		}

		this->DoUpdate(pass);
	}

	// 响应窗口大小变化
	/////////////////////////////////////////////////////////////////////////////////
	void App3DFramework::OnResize(uint32_t /*width*/, uint32_t /*height*/)
	{
	}

	// 更新状态
	/////////////////////////////////////////////////////////////////////////////////
	void App3DFramework::UpdateStats()
	{
		// measure statistics
		++ num_frames_;
		accumulate_time_ += static_cast<float>(timer_.elapsed());

		// check if new second
		if (accumulate_time_ > 1)
		{
			// new second - not 100% precise
			fps_ = num_frames_ / accumulate_time_;

			accumulate_time_ = 0;
			num_frames_  = 0;
		}

		timer_.restart();
	}

	// 获取渲染目标的每秒帧数
	/////////////////////////////////////////////////////////////////////////////////
	float App3DFramework::FPS() const
	{
		return fps_;
	}
}
