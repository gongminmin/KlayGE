// Contex.hpp
// KlayGE 引擎场景类 头文件
// Ver 2.4.0
// 版权所有(C) 龚敏敏, 2003-2005
// Homepage: http://klayge.sourceforge.nets
//
// 2.4.0
// 增加了BOOST_ASSERT
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

#define KLAYGE_LIB_NAME KlayGE_Core
#include <KlayGE/config/auto_link.hpp>

#include <KlayGE/PreDeclare.hpp>
#include <KlayGE/SceneManager.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/AudioFactory.hpp>
#include <KlayGE/InputFactory.hpp>
#include <KlayGE/ShowFactory.hpp>

#include <boost/assert.hpp>

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
			BOOST_ASSERT(app_ != NULL);
			return *app_;
		}

		void SceneManagerInstance(SceneManager& mgr)
			{ sceneMgr_ = &mgr; }
		SceneManager& SceneManagerInstance()
		{
			BOOST_ASSERT(sceneMgr_ != NULL);
			return *sceneMgr_;
		}

		void RenderFactoryInstance(RenderFactory& factory)
			{ renderFactory_ = &factory; }
		RenderFactory& RenderFactoryInstance()
		{
			BOOST_ASSERT(renderFactory_ != NULL);
			return *renderFactory_;
		}

		void AudioFactoryInstance(AudioFactory& factory)
			{ audioFactory_ = &factory; }
		AudioFactory& AudioFactoryInstance()
		{
			BOOST_ASSERT(audioFactory_ != NULL);
			return *audioFactory_;
		}

		void InputFactoryInstance(InputFactory& factory)
			{ inputFactory_ = &factory; }
		InputFactory& InputFactoryInstance()
		{
			BOOST_ASSERT(inputFactory_ != NULL);
			return *inputFactory_;
		}

		void ShowFactoryInstance(ShowFactory& factory)
			{ showFactory_ = &factory; }
		ShowFactory& ShowFactoryInstance()
		{
			BOOST_ASSERT(showFactory_ != NULL);
			return *showFactory_;
		}

	private:
		Context()
		{
#ifdef KLAYGE_COMPILER_MSVC
#ifdef KLAYGE_DEBUG
			_CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#endif
#endif

			sceneMgr_ = SceneManager::NullObject().get();

			renderFactory_ = RenderFactory::NullObject().get();
			audioFactory_ = AudioFactory::NullObject().get();
			inputFactory_ = InputFactory::NullObject().get();
			showFactory_ = ShowFactory::NullObject().get();
		}

		App3DFramework* app_;

		SceneManager* sceneMgr_;

		RenderFactory*	renderFactory_;
		AudioFactory*	audioFactory_;
		InputFactory*	inputFactory_;
		ShowFactory*    showFactory_;
	};
}

#endif		// _CONTEXT_HPP
