// 3DApp.hpp
// KlayGE 3DApp类 头文件
// Ver 1.2.8.11
// 版权所有(C) 龚敏敏, 2001--2002
// Homepage: http://www.enginedev.com
//
// 1.2.8.8
// 把inline放入类声明 (2002.9.24)
//
// 1.2.8.11
// 抽出了基类 CAppFramework (2002.11.7)
// 改用UNICODE核心 (2002.11.18)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#ifndef _APP3D_HPP
#define _APP3D_HPP

#include <KlayGE/Camera.hpp>
#include <KlayGE/RenderEngine.hpp>

#pragma comment(lib, "KlayGE_AppLayer.lib")

namespace KlayGE
{
	// 一个用于创建3D应用程序框架的基类。要建立一个D3D应用程序只要继承这个类，
	//			然后重载以下函数:
	//
	//			InitObjects()			- 初始化3D设备
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

		void Run();

	protected:
		void LookAt(Vector3 const & eye, Vector3 const & lookAt, Vector3 const & up = Vector3(0, 1, 0));
		void Proj(float nearPlane, float farPlane);

	protected:
		virtual void InitObjects()
			{ }

		virtual void Update()
			{ }

		virtual void RenderOver()
			{ }

		virtual void DelObjects()
			{ }
	};
}

#endif		// _APP3D_HPP
