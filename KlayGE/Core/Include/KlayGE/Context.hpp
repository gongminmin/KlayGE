// Contex.hpp
// KlayGE 引擎场景类 头文件
// Ver 2.4.0
// 版权所有(C) 龚敏敏, 2003-2005
// Homepage: http://klayge.sourceforge.nets
//
// 2.4.0
// 增加了assert
//
// 2.1.1
// 改了类名
//
// 2.0.0
// 初次建立 (2003.7.23)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#ifndef _CONTEXT_HPP
#define _CONTEXT_HPP

#include <KlayGE/PreDeclare.hpp>
#include <KlayGE/SceneManager.hpp>

#include <cassert>

#ifdef KLAYGE_DEBUG
	#pragma comment(lib, "KlayGE_Core_d.lib")
#else
	#pragma comment(lib, "KlayGE_Core.lib")
#endif

namespace KlayGE
{
	class Context
	{
	public:
		static Context& Instance()
		{
			static Context context;
			return context;
		}

		void AppInstance(App3DFramework& app)
			{ app_ = &app; }
		App3DFramework& AppInstance()
		{
			assert(app_ != NULL);
			return *app_;
		}

		void SceneManagerInstance(SceneManager& mgr)
			{ sceneMgr_ = &mgr; }
		SceneManager& SceneManagerInstance()
		{
			assert(sceneMgr_ != NULL);
			return *sceneMgr_;
		}

		void RenderFactoryInstance(RenderFactory& factory)
			{ renderFactory_ = &factory; }
		RenderFactory& RenderFactoryInstance()
		{
			assert(renderFactory_ != NULL);
			return *renderFactory_;
		}

		void AudioFactoryInstance(AudioFactory& factory)
			{ audioFactory_ = &factory; }
		AudioFactory& AudioFactoryInstance()
		{
			assert(audioFactory_ != NULL);
			return *audioFactory_;
		}

		void InputFactoryInstance(InputFactory& factory)
			{ inputFactory_ = &factory; }
		InputFactory& InputFactoryInstance()
		{
			assert(inputFactory_ != NULL);
			return *inputFactory_;
		}

	private:
		Context()
			{ }

		App3DFramework* app_;

		SceneManager* sceneMgr_;

		RenderFactory*	renderFactory_;
		AudioFactory*	audioFactory_;
		InputFactory*	inputFactory_;
	};
}

#endif		// _CONTEXT_HPP
