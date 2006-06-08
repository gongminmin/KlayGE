// App3Dp.hpp
// KlayGE App3D类 头文件
// Ver 3.1.0
// 版权所有(C) 龚敏敏, 2003-2005
// Homepage: http://klayge.sourceforge.net
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

#include <KlayGE/Camera.hpp>
#include <KlayGE/RenderEngine.hpp>

#ifdef KLAYGE_DEBUG
	#pragma comment(lib, "KlayGE_AppLayer_d.lib")
#else
	#pragma comment(lib, "KlayGE_AppLayer.lib")
#endif

namespace KlayGE
{
	// 一个用于创建3D应用程序框架的基类。要建立一个D3D应用程序只要继承这个类，
	//			然后重载以下函数:
	//
	//			InitObjects()			- 初始化3D设备
	//			NumPasses()				- 返回场景需要绘制的遍数
	//			Update()				- 刷新场景
	//			DelObjects()			- 清除3D场景
	/////////////////////////////////////////////////////////////////////////////////
	class App3DFramework
	{
		friend class SceneManager;

	public:
		App3DFramework();
		virtual ~App3DFramework();

		virtual void Create(std::string const & name, RenderSettings const & settings);

		Camera const & ActiveCamera() const;
		Camera& ActiveCamera();

		void Run();
		void Quit();

		virtual void OnResize(uint32_t width, uint32_t height);

	protected:
		void LookAt(float3 const & eye, float3 const & lookAt, float3 const & up = float3(0, 1, 0));
		void Proj(float nearPlane, float farPlane);

	protected:
		virtual void InitObjects()
		{
		}

		virtual uint32_t NumPasses() const
		{
			return 1;
		}
		void Update(uint32_t pass);
		virtual void DoUpdate(uint32_t pass) = 0;

		virtual void RenderOver()
		{
		}

		virtual void DelObjects()
		{
		}
	};
}

#endif		// _APP3D_HPP
