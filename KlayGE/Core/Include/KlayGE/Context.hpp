// Contex.hpp
// KlayGE 引擎场景类 头文件
// Ver 3.11.0
// 版权所有(C) 龚敏敏, 2007-2010
// Homepage: http://www.klayge.org
//
// 3.11.0
// 增加了ContextCfg (2010.8.9)
// 增加了AudioDataSourceFactoryInstance (2010.8.22)
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

#include <KlayGE/PreDeclare.hpp>

#include <string>
#include <boost/assert.hpp>

#include <KlayGE/RenderSettings.hpp>
#include <KFL/DllLoader.hpp>

#ifdef KLAYGE_PLATFORM_ANDROID
struct android_app;
#endif

namespace KlayGE
{
	struct ContextCfg
	{
		std::string render_factory_name;
		std::string audio_factory_name;
		std::string input_factory_name;
		std::string show_factory_name;
		std::string script_factory_name;
		std::string scene_manager_name;
		std::string audio_data_source_factory_name;

		RenderSettings graphics_cfg;
		bool deferred_rendering;

		bool perf_profiler;
		bool location_sensor;
	};

	class KLAYGE_CORE_API Context : boost::noncopyable
	{
	public:
		Context();
		~Context();

		static Context& Instance();
		static void Destroy();
		void Suspend();
		void Resume();
		
#ifdef KLAYGE_PLATFORM_ANDROID
		android_app* AppState() const
		{
			return state_;
		}
#endif

		void LoadCfg(std::string const & cfg_file);
		void SaveCfg(std::string const & cfg_file);

		void Config(ContextCfg const & cfg);
		ContextCfg const & Config() const;

		void LoadRenderFactory(std::string const & rf_name);
		void LoadAudioFactory(std::string const & af_name);
		void LoadInputFactory(std::string const & if_name);
		void LoadShowFactory(std::string const & sf_name);
		void LoadScriptFactory(std::string const & sf_name);
		void LoadSceneManager(std::string const & sm_name);
		void LoadAudioDataSourceFactory(std::string const & adsf_name);

		void AppInstance(App3DFramework& app)
		{
			app_ = &app;
		}
		bool AppValid() const
		{
			return app_ != nullptr;
		}
		App3DFramework& AppInstance()
		{
			BOOST_ASSERT(app_);
			KLAYGE_ASSUME(app_);
			return *app_;
		}

		bool SceneManagerValid() const
		{
			return scene_mgr_.get() != nullptr;
		}
		SceneManager& SceneManagerInstance();

		bool RenderFactoryValid() const
		{
			return render_factory_.get() != nullptr;
		}
		RenderFactory& RenderFactoryInstance();

		bool AudioFactoryValid() const
		{
			return audio_factory_.get() != nullptr;
		}
		AudioFactory& AudioFactoryInstance();

		bool InputFactoryValid() const
		{
			return input_factory_.get() != nullptr;
		}
		InputFactory& InputFactoryInstance();

		bool ShowFactoryValid() const
		{
			return show_factory_.get() != nullptr;
		}
		ShowFactory& ShowFactoryInstance();

		bool ScriptFactoryValid() const
		{
			return script_factory_.get() != nullptr;
		}
		ScriptFactory& ScriptFactoryInstance();

		bool AudioDataSourceFactoryValid() const
		{
			return audio_data_src_factory_.get() != nullptr;
		}
		AudioDataSourceFactory& AudioDataSourceFactoryInstance();

		DeferredRenderingLayer* DeferredRenderingLayerInstance()
		{
			return deferred_rendering_layer_.get();
		}

		thread_pool& ThreadPool()
		{
			return *gtp_instance_;
		}

	private:
		void DestroyAll();

	private:
		static std::unique_ptr<Context> context_instance_;

		ContextCfg cfg_;

#ifdef KLAYGE_PLATFORM_ANDROID
		android_app* state_;
#endif

		App3DFramework*		app_;

		std::unique_ptr<SceneManager> scene_mgr_;

		std::unique_ptr<RenderFactory> render_factory_;
		std::unique_ptr<AudioFactory> audio_factory_;
		std::unique_ptr<InputFactory> input_factory_;
		std::unique_ptr<ShowFactory> show_factory_;
		std::unique_ptr<ScriptFactory> script_factory_;
		std::unique_ptr<AudioDataSourceFactory> audio_data_src_factory_;
		std::unique_ptr<DeferredRenderingLayer> deferred_rendering_layer_;

		DllLoader render_loader_;
		DllLoader audio_loader_;
		DllLoader input_loader_;
		DllLoader show_loader_;
		DllLoader script_loader_;
		DllLoader sm_loader_;
		DllLoader ads_loader_;

		std::unique_ptr<thread_pool> gtp_instance_;
	};
}

#endif		// _CONTEXT_HPP
