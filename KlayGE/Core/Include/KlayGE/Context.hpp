// Contex.hpp
// KlayGE ���泡���� ͷ�ļ�
// Ver 3.11.0
// ��Ȩ����(C) ������, 2007-2010
// Homepage: http://www.klayge.org
//
// 3.11.0
// ������ContextCfg (2010.8.9)
// ������AudioDataSourceFactoryInstance (2010.8.22)
//
// 3.8.0
// ������LoadCfg (2008.10.12)
//
// 2.4.0
// ������BOOST_ASSERT
//
// 2.1.1
// ��������
//
// 2.0.0
// ���ν��� (2003.7.23)
//
// �޸ļ�¼
/////////////////////////////////////////////////////////////////////////////////

#ifndef _CONTEXT_HPP
#define _CONTEXT_HPP

#pragma once

#include <KlayGE/PreDeclare.hpp>

#include <string>
#include <boost/assert.hpp>

#ifdef KLAYGE_PLATFORM_ANDROID
#include <android_native_app_glue.h>
#endif

#include <KlayGE/RenderSettings.hpp>
#include <KFL/DllLoader.hpp>

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

	class KLAYGE_CORE_API Context
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

		void SceneManagerInstance(SceneManagerPtr const & mgr)
		{
			scene_mgr_ = mgr;
		}
		bool SceneManagerValid() const
		{
			return !!scene_mgr_;
		}
		SceneManager& SceneManagerInstance();

		void RenderFactoryInstance(RenderFactoryPtr const & factory)
		{
			render_factory_ = factory;
		}
		bool RenderFactoryValid() const
		{
			return !!render_factory_;
		}
		RenderFactory& RenderFactoryInstance();

		void AudioFactoryInstance(AudioFactoryPtr const & factory)
		{
			audio_factory_ = factory;
		}
		bool AudioFactoryValid() const
		{
			return !!audio_factory_;
		}
		AudioFactory& AudioFactoryInstance();

		void InputFactoryInstance(InputFactoryPtr const & factory)
		{
			input_factory_ = factory;
		}
		bool InputFactoryValid() const
		{
			return !!input_factory_;
		}
		InputFactory& InputFactoryInstance();

		void ShowFactoryInstance(ShowFactoryPtr const & factory)
		{
			show_factory_ = factory;
		}
		bool ShowFactoryValid() const
		{
			return !!show_factory_;
		}
		ShowFactory& ShowFactoryInstance();

		void ScriptFactoryInstance(ScriptFactoryPtr const & factory)
		{
			script_factory_ = factory;
		}
		bool ScriptFactoryValid() const
		{
			return !!script_factory_;
		}
		ScriptFactory& ScriptFactoryInstance();

		void AudioDataSourceFactoryInstance(AudioDataSourceFactoryPtr const & factory)
		{
			audio_data_src_factory_ = factory;
		}
		bool AudioDataSourceFactoryValid() const
		{
			return !!audio_data_src_factory_;
		}
		AudioDataSourceFactory& AudioDataSourceFactoryInstance();

		void DeferredRenderingLayerInstance(DeferredRenderingLayerPtr const & drl)
		{
			deferred_rendering_layer_ = drl;
		}
		DeferredRenderingLayerPtr const & DeferredRenderingLayerInstance()
		{
			return deferred_rendering_layer_;
		}

		thread_pool& ThreadPool()
		{
			return *gtp_instance_;
		}

	private:
		void DestroyAll();

	private:
		static std::shared_ptr<Context> context_instance_;

		ContextCfg cfg_;

#ifdef KLAYGE_PLATFORM_ANDROID
		android_app* state_;
#endif

		App3DFramework*		app_;

		SceneManagerPtr		scene_mgr_;

		RenderFactoryPtr	render_factory_;
		AudioFactoryPtr		audio_factory_;
		InputFactoryPtr		input_factory_;
		ShowFactoryPtr		show_factory_;
		ScriptFactoryPtr	script_factory_;
		AudioDataSourceFactoryPtr audio_data_src_factory_;
		DeferredRenderingLayerPtr deferred_rendering_layer_;

		DllLoader render_loader_;
		DllLoader audio_loader_;
		DllLoader input_loader_;
		DllLoader show_loader_;
		DllLoader script_loader_;
		DllLoader sm_loader_;
		DllLoader ads_loader_;

		std::shared_ptr<thread_pool> gtp_instance_;
	};
}

#endif		// _CONTEXT_HPP
