// Engine.hpp
// KlayGE 引擎 头文件
// Ver 2.0.0
// 版权所有(C) 龚敏敏, 2003
// Homepage: http://www.enginedev.com
//
// 2.0.0
// 初次建立 (2003.7.23)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#ifndef _ENGINE_HPP
#define _ENGINE_HPP

#include <KlayGE/PreDeclare.hpp>

#include <KlayGE/SharedPtr.hpp>

#pragma comment(lib, "KlayGE_Core.lib")

namespace KlayGE
{
	class Engine
	{
	public:
		Engine();
		~Engine();

		static MathLib& MathInstance()
			{ return *math_; }
		static MemoryLib& MemoryInstance()
			{ return *memory_; }

		static void AppInstance(App3DFramework& app)
			{ app_ = &app; }
		static App3DFramework& AppInstance()
			{ return *app_; }
		static SceneManager& SceneManagerInstance()
			{ return *sceneManager_; }

		static void RenderFactoryInstance(RenderFactory& factory)
			{ renderFactory_ = &factory; }
		static RenderFactory& RenderFactoryInstance()
			{ return *renderFactory_; }

		static void AudioFactoryInstance(AudioFactory& factory)
			{ audioFactory_ = &factory; }
		static AudioFactory& AudioFactoryInstance()
			{ return *audioFactory_; }

		static void InputFactoryInstance(InputFactory& factory)
			{ inputFactory_ = &factory; }
		static InputFactory& InputFactoryInstance()
			{ return *inputFactory_; }

	private:
		static MathLib*	math_;
		static MemoryLib*	memory_;

		static App3DFramework* app_;
		static SceneManager*	sceneManager_;

		static RenderFactory*	renderFactory_;
		static AudioFactory*	audioFactory_;
		static InputFactory*	inputFactory_;
	};
}

#endif		// _ENGINE_HPP