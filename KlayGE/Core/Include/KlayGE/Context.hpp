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

#pragma once

#include <string>

#include <KlayGE/RenderSettings.hpp>
#include <KFL/Noncopyable.hpp>

#ifdef KLAYGE_PLATFORM_ANDROID
struct android_app;
#endif

namespace KlayGE
{
	class ThreadPool;
	class App3DFramework;
	class AudioDataSourceFactory;
	class AudioFactory;
	class DeferredRenderingLayer;
	class InputFactory;
	class RenderFactory;
	class SceneManager;
	class ScriptFactory;
	class ShowFactory;
#if KLAYGE_IS_DEV_PLATFORM
	class DevHelper;
#endif
	class PerfProfiler;
	class UIManager;

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

	class KLAYGE_CORE_API Context final
	{
		KLAYGE_NONCOPYABLE(Context);

	public:
		Context();
		~Context() noexcept;

		static Context& Instance();
		static void Destroy() noexcept;
		void Suspend();
		void Resume();
		
#ifdef KLAYGE_PLATFORM_ANDROID
		android_app* AppState() const;
		void AppState(android_app* state);
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

#if KLAYGE_IS_DEV_PLATFORM
		void LoadDevHelper();
#endif

		void AppInstance(App3DFramework& app) noexcept;
		bool AppValid() const noexcept;
		App3DFramework& AppInstance() noexcept;

		bool SceneManagerValid() const noexcept;
		SceneManager& SceneManagerInstance();

		bool RenderFactoryValid() const noexcept;
		RenderFactory& RenderFactoryInstance();

		bool AudioFactoryValid() const noexcept;
		AudioFactory& AudioFactoryInstance();

		bool InputFactoryValid() const noexcept;
		InputFactory& InputFactoryInstance();

		bool ShowFactoryValid() const noexcept;
		ShowFactory& ShowFactoryInstance();

		bool ScriptFactoryValid() const noexcept;
		ScriptFactory& ScriptFactoryInstance();

		bool AudioDataSourceFactoryValid() const noexcept;
		AudioDataSourceFactory& AudioDataSourceFactoryInstance();

		bool DeferredRenderingLayerValid() const noexcept;
		DeferredRenderingLayer& DeferredRenderingLayerInstance();

#if KLAYGE_IS_DEV_PLATFORM
		bool DevHelperValid() const noexcept;
		DevHelper& DevHelperInstance();
#endif

		ThreadPool& ThreadPoolInstance();

		PerfProfiler& PerfProfilerInstance();
		UIManager& UIManagerInstance();

	private:
		class Impl;
		std::unique_ptr<Impl> pimpl_;
	};
}
