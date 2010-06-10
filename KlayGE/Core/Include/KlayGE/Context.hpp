// Contex.hpp
// KlayGE 引擎场景类 头文件
// Ver 3.8.0
// 版权所有(C) 龚敏敏, 2007-2008
// Homepage: http://www.klayge.org
//
// 3.8.0
// 增加了LoadCfg (2008.10.12)
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

#pragma once

#ifndef KLAYGE_CORE_SOURCE
#define KLAYGE_LIB_NAME KlayGE_Core
#include <KlayGE/config/auto_link.hpp>
#endif

#include <KlayGE/PreDeclare.hpp>

#include <string>
#include <boost/assert.hpp>

#include <KlayGE/DllLoader.hpp>

namespace KlayGE
{
	class KLAYGE_CORE_API Context
	{
	public:
		~Context();

		static Context& Instance();

		RenderSettings LoadCfg(std::string const & cfg_file);

		void LoadRenderFactory(std::string const & rf_name, XMLNodePtr const & rf_node);
		void LoadAudioFactory(std::string const & af_name, XMLNodePtr const & af_node);
		void LoadInputFactory(std::string const & if_name, XMLNodePtr const & if_node);
		void LoadShowFactory(std::string const & sf_name, XMLNodePtr const & sf_node);
		void LoadSceneManager(std::string const & sm_name, XMLNodePtr const & sm_node);

		void AppInstance(App3DFramework& app)
		{
			app_ = &app;
		}
		App3DFramework& AppInstance()
		{
#ifdef KLAYGE_COMPILER_MSVC
#pragma warning(push)
#pragma warning(disable: 6011)
#endif
			BOOST_ASSERT(app_);
			return *app_;
#ifdef KLAYGE_COMPILER_MSVC
#pragma warning(pop)
#endif
		}

		void SceneManagerInstance(SceneManagerPtr const & mgr)
		{
			sceneMgr_ = mgr;
		}
		SceneManager& SceneManagerInstance()
		{
			BOOST_ASSERT(sceneMgr_);
			return *sceneMgr_;
		}

		void RenderFactoryInstance(RenderFactoryPtr const & factory)
		{
			renderFactory_ = factory;
		}
		RenderFactory& RenderFactoryInstance()
		{
			BOOST_ASSERT(renderFactory_);
			return *renderFactory_;
		}

		void AudioFactoryInstance(AudioFactoryPtr const & factory)
		{
			audioFactory_ = factory;
		}
		AudioFactory& AudioFactoryInstance()
		{
			BOOST_ASSERT(audioFactory_);
			return *audioFactory_;
		}

		void InputFactoryInstance(InputFactoryPtr const & factory)
		{
			inputFactory_ = factory;
		}
		InputFactory& InputFactoryInstance()
		{
			BOOST_ASSERT(inputFactory_);
			return *inputFactory_;
		}

		void ShowFactoryInstance(ShowFactoryPtr const & factory)
		{
			showFactory_ = factory;
		}
		ShowFactory& ShowFactoryInstance()
		{
			BOOST_ASSERT(showFactory_);
			return *showFactory_;
		}

	private:
		Context();

		App3DFramework*		app_;

		SceneManagerPtr		sceneMgr_;

		RenderFactoryPtr	renderFactory_;
		AudioFactoryPtr		audioFactory_;
		InputFactoryPtr		inputFactory_;
		ShowFactoryPtr		showFactory_;

		DllLoader render_loader_;
		DllLoader audio_loader_;
		DllLoader input_loader_;
		DllLoader show_loader_;
		DllLoader sm_loader_;

		std::string dll_suffix_;
	};
}

#endif		// _CONTEXT_HPP
