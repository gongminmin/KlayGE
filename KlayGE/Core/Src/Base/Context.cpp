// Context.cpp
// KlayGE 引擎场景类 实现文件
// Ver 3.11.0
// 版权所有(C) 龚敏敏, 2007-2010
// Homepage: http://www.klayge.org
//
// 3.11.0
// 增加了AudioDataSourceFactoryInstance (2010.8.22)
//
// 3.9.0
// XML格式的配置文件 (2009.4.26)
//
// 3.8.0
// 增加了LoadCfg (2008.10.12)
//
// 3.7.0
// 初次建立 (2007.12.19)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KFL/CXX20/span.hpp>
#include <KFL/StringUtil.hpp>
#include <KFL/Util.hpp>
#include <KFL/Math.hpp>
#include <KFL/Log.hpp>
#include <KlayGE/DevHelper.hpp>
#include <KlayGE/SceneManager.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/AudioFactory.hpp>
#include <KlayGE/InputFactory.hpp>
#include <KlayGE/ShowFactory.hpp>
#include <KlayGE/ScriptFactory.hpp>
#include <KlayGE/AudioDataSource.hpp>
#include <KlayGE/ResLoader.hpp>
#include <KFL/XMLDom.hpp>
#include <KlayGE/DeferredRenderingLayer.hpp>
#include <KlayGE/PerfProfiler.hpp>
#include <KlayGE/UI.hpp>
#include <KFL/Hash.hpp>

#include <fstream>
#include <mutex>
#include <sstream>
#include <string>

#if defined(KLAYGE_PLATFORM_WINDOWS)
#include <windows.h>
#if defined(KLAYGE_PLATFORM_WINDOWS_DESKTOP)
#if (_WIN32_WINNT >= _WIN32_WINNT_WINBLUE)
#include <VersionHelpers.h>
#endif
#endif
#elif defined(KLAYGE_PLATFORM_ANDROID)
#include <android_native_app_glue.h>
#endif

#if defined(KLAYGE_PLATFORM_ANDROID) || defined(KLAYGE_PLATFORM_IOS)
	#define KLAYGE_STATIC_LINK_PLUGINS
#endif

#ifdef KLAYGE_STATIC_LINK_PLUGINS
extern "C"
{
	void MakeRenderFactory(std::unique_ptr<KlayGE::RenderFactory>& ptr);
	void MakeAudioFactory(std::unique_ptr<KlayGE::AudioFactory>& ptr);
	void MakeInputFactory(std::unique_ptr<KlayGE::InputFactory>& ptr);
	void MakeShowFactory(std::unique_ptr<KlayGE::ShowFactory>& ptr);
	void MakeScriptFactory(std::unique_ptr<KlayGE::ScriptFactory>& ptr);
	void MakeSceneManager(std::unique_ptr<KlayGE::SceneManager>& ptr);
	void MakeAudioDataSourceFactory(std::unique_ptr<KlayGE::AudioDataSourceFactory>& ptr);
}
#endif

#include <KlayGE/Context.hpp>

#define KLAYGE_DLL_PREFIX DLL_PREFIX KFL_STRINGIZE(KLAYGE_NAME)

namespace
{
	std::mutex singleton_mutex;
}

namespace KlayGE
{
	std::unique_ptr<Context> Context::context_instance_;

	typedef void (*MakeRenderFactoryFunc)(std::unique_ptr<RenderFactory>& ptr);
	typedef void (*MakeAudioFactoryFunc)(std::unique_ptr<AudioFactory>& ptr);
	typedef void (*MakeInputFactoryFunc)(std::unique_ptr<InputFactory>& ptr);
	typedef void (*MakeShowFactoryFunc)(std::unique_ptr<ShowFactory>& ptr);
	typedef void (*MakeScriptFactoryFunc)(std::unique_ptr<ScriptFactory>& ptr);
	typedef void (*MakeSceneManagerFunc)(std::unique_ptr<SceneManager>& ptr);
	typedef void (*MakeAudioDataSourceFactoryFunc)(std::unique_ptr<AudioDataSourceFactory>& ptr);
#if KLAYGE_IS_DEV_PLATFORM
	typedef void (*MakeDevHelperFunc)(std::unique_ptr<DevHelper>& ptr);
#endif

	Context::Context()
		: app_(nullptr)
	{
#ifdef KLAYGE_COMPILER_MSVC
#ifdef KLAYGE_DEBUG
		_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif
#endif

		gtp_instance_ = MakeUniquePtr<ThreadPool>(1, 16);
	}

	Context::~Context()
	{
	}

	void Context::DestroyAll()
	{
		scene_mgr_.reset();

		ResLoader::Destroy();
		PerfProfiler::Destroy();
		UIManager::Destroy();

		deferred_rendering_layer_.reset();
		show_factory_.reset();
		render_factory_.reset();
		audio_factory_.reset();
		input_factory_.reset();
		script_factory_.reset();
		audio_data_src_factory_.reset();

#if KLAYGE_IS_DEV_PLATFORM
		dev_helper_.reset();
#endif

		app_ = nullptr;

		gtp_instance_.reset();
	}

	Context& Context::Instance()
	{
		if (!context_instance_)
		{
			std::lock_guard<std::mutex> lock(singleton_mutex);
			if (!context_instance_)
			{
				context_instance_ = MakeUniquePtr<Context>();
			}
		}
		return *context_instance_;
	}

	void Context::Destroy()
	{
		std::lock_guard<std::mutex> lock(singleton_mutex);
		if (context_instance_)
		{
			context_instance_->DestroyAll();
			context_instance_.reset();
		}
	}

	void Context::Suspend()
	{
		if (scene_mgr_)
		{
			scene_mgr_->Suspend();
		}

		ResLoader::Instance().Suspend();
		PerfProfiler::Instance().Suspend();
		UIManager::Instance().Suspend();

		if (deferred_rendering_layer_)
		{
			deferred_rendering_layer_->Suspend();
		}
		if (show_factory_)
		{
			show_factory_->Suspend();
		}
		if (render_factory_)
		{
			render_factory_->Suspend();
		}
		if (audio_factory_)
		{
			audio_factory_->Suspend();
		}
		if (input_factory_)
		{
			input_factory_->Suspend();
		}
		if (script_factory_)
		{
			script_factory_->Suspend();
		}
		if (audio_data_src_factory_)
		{
			audio_data_src_factory_->Suspend();
		}
	}

	void Context::Resume()
	{
		if (scene_mgr_)
		{
			scene_mgr_->Resume();
		}

		ResLoader::Instance().Resume();
		PerfProfiler::Instance().Resume();
		UIManager::Instance().Resume();

		if (deferred_rendering_layer_)
		{
			deferred_rendering_layer_->Resume();
		}
		if (show_factory_)
		{
			show_factory_->Resume();
		}
		if (render_factory_)
		{
			render_factory_->Resume();
		}
		if (audio_factory_)
		{
			audio_factory_->Resume();
		}
		if (input_factory_)
		{
			input_factory_->Resume();
		}
		if (script_factory_)
		{
			script_factory_->Resume();
		}
		if (audio_data_src_factory_)
		{
			audio_data_src_factory_->Resume();
		}
	}

	void Context::LoadCfg(std::string const & cfg_file)
	{
#if defined(KLAYGE_PLATFORM_WINDOWS_DESKTOP)
		static char const * available_rfs_array[] = { "D3D11", "OpenGL", "OpenGLES", "D3D12" };
		static char const * available_afs_array[] = { "OpenAL", "XAudio" };
		static char const * available_adsfs_array[] = { "OggVorbis" };
		static char const * available_ifs_array[] = { "MsgInput" };
		static char const * available_sfs_array[] = { "DShow", "MFShow" };
		static char const * available_scfs_array[] = { "Python" };
#elif defined(KLAYGE_PLATFORM_WINDOWS_STORE)
		static char const * available_rfs_array[] = { "D3D11", "D3D12" };
		static char const * available_afs_array[] = { "XAudio" };
		static char const * available_adsfs_array[] = { "OggVorbis" };
		static char const * available_ifs_array[] = { "MsgInput" };
		static char const * available_sfs_array[] = { "MFShow" };
		static char const * available_scfs_array[] = { "Python" };
#elif defined(KLAYGE_PLATFORM_LINUX)
		static char const * available_rfs_array[] = { "OpenGL" };
		static char const * available_afs_array[] = { "OpenAL" };
		static char const * available_adsfs_array[] = { "OggVorbis" };
		static char const * available_ifs_array[] = { "NullInput" };
		static char const * available_sfs_array[] = { "NullShow" };
		static char const * available_scfs_array[] = { "Python" };
#elif defined(KLAYGE_PLATFORM_ANDROID)
		static char const * available_rfs_array[] = { "OpenGLES" };
		static char const * available_afs_array[] = { "OpenAL" };
		static char const * available_adsfs_array[] = { "OggVorbis" };
		static char const * available_ifs_array[] = { "MsgInput" };
		static char const * available_sfs_array[] = { "NullShow" };
		static char const * available_scfs_array[] = { "NullScript" };
#elif defined(KLAYGE_PLATFORM_IOS)
		static char const * available_rfs_array[] = { "OpenGLES" };
		static char const * available_afs_array[] = { "OpenAL" };
		static char const * available_adsfs_array[] = { "OggVorbis" };
		static char const * available_ifs_array[] = { "MsgInput" };
		static char const * available_sfs_array[] = { "NullShow" };
		static char const * available_scfs_array[] = { "NullScript" };
#elif defined(KLAYGE_PLATFORM_DARWIN)
		static char const * available_rfs_array[] = { "OpenGL" };
		static char const * available_afs_array[] = { "OpenAL" };
		static char const * available_adsfs_array[] = { "OggVorbis" };
		static char const * available_ifs_array[] = { "MsgInput" };
		static char const * available_sfs_array[] = { "NullShow" };
		static char const * available_scfs_array[] = { "Python" };
#endif
		static char const * available_sms_array[] = { "OCTree" };

		int width = 800;
		int height = 600;
		ElementFormat color_fmt = EF_ARGB8;
		ElementFormat depth_stencil_fmt = EF_D16;
		int sample_count = 1;
		int sample_quality = 0;
		bool full_screen = false;
		int sync_interval = 0;
		bool hdr = false;
		bool ppaa = false;
		bool gamma = false;
		bool color_grading = false;
		float bloom = 0.25f;
		bool blue_shift = true;
		bool keep_screen_on = true;
		StereoMethod stereo_method = STM_None;
		float stereo_separation = 0;
		DisplayOutputMethod display_output_method = DOM_sRGB;
		uint32_t paper_white = 100;
		uint32_t display_max_luminance = 100;
		std::vector<std::pair<std::string, std::string>> graphics_options;
		bool debug_context = false;
		bool perf_profiler = false;
		bool location_sensor = false;

		std::string rf_name;
		std::string af_name;
		std::string if_name;
		std::string sf_name;
		std::string scf_name;
		std::string sm_name;
		std::string adsf_name;

		ResIdentifierPtr file = ResLoader::Instance().Open(cfg_file);
		if (file)
		{
			std::unique_ptr<XMLDocument> cfg_doc = LoadXml(*file);
			XMLNode const* cfg_root = cfg_doc->RootNode();

			XMLNode const* context_node = cfg_root->FirstNode("context");
			XMLNode const* graphics_node = cfg_root->FirstNode("graphics");

			if (XMLNode const* rf_node = context_node->FirstNode("render_factory"))
			{
				rf_name = std::string(rf_node->Attrib("name")->ValueString());
			}
			if (XMLNode const* af_node = context_node->FirstNode("audio_factory"))
			{
				af_name = std::string(af_node->Attrib("name")->ValueString());
			}
			if (XMLNode const* if_node = context_node->FirstNode("input_factory"))
			{
				if_name = std::string(if_node->Attrib("name")->ValueString());
			}
			if (XMLNode const* sf_node = context_node->FirstNode("show_factory"))
			{
				sf_name = std::string(sf_node->Attrib("name")->ValueString());
			}
			if (XMLNode const* scf_node = context_node->FirstNode("script_factory"))
			{
				scf_name = std::string(scf_node->Attrib("name")->ValueString());
			}
			if (XMLNode const* sm_node = context_node->FirstNode("scene_manager"))
			{
				sm_name = std::string(sm_node->Attrib("name")->ValueString());
			}
			if (XMLNode const* adsf_node = context_node->FirstNode("audio_data_source_factory"))
			{
				adsf_name = std::string(adsf_node->Attrib("name")->ValueString());
			}
			if (XMLNode const* perf_profiler_node = context_node->FirstNode("perf_profiler"))
			{
				perf_profiler = perf_profiler_node->Attrib("enabled")->ValueInt() ? true : false;
			}
			if (XMLNode const* location_sensor_node = context_node->FirstNode("location_sensor"))
			{
				location_sensor = location_sensor_node->Attrib("enabled")->ValueInt() ? true : false;
			}

			XMLNode const* frame_node = graphics_node->FirstNode("frame");
			if (XMLAttribute const* attr = frame_node->Attrib("width"))
			{
				width = attr->ValueInt();
			}
			if (XMLAttribute const* attr = frame_node->Attrib("height"))
			{
				height = attr->ValueInt();
			}
			std::string color_fmt_str = "ARGB8";
			if (XMLAttribute const* attr = frame_node->Attrib("color_fmt"))
			{
				color_fmt_str = std::string(attr->ValueString());
			}
			std::string depth_stencil_fmt_str = "D16";
			if (XMLAttribute const* attr = frame_node->Attrib("depth_stencil_fmt"))
			{
				depth_stencil_fmt_str = std::string(attr->ValueString());
			}
			if (XMLAttribute const* attr = frame_node->Attrib("fullscreen"))
			{
				full_screen = attr->ValueBool();
			}
			if (XMLAttribute const* attr = frame_node->Attrib("keep_screen_on"))
			{
				keep_screen_on = attr->ValueBool();
			}

			size_t const color_fmt_str_hash = RT_HASH(color_fmt_str.c_str());
			if (CT_HASH("ARGB8") == color_fmt_str_hash)
			{
				color_fmt = EF_ARGB8;
			}
			else if (CT_HASH("ABGR8") == color_fmt_str_hash)
			{
				color_fmt = EF_ABGR8;
			}
			else if (CT_HASH("A2BGR10") == color_fmt_str_hash)
			{
				color_fmt = EF_A2BGR10;
			}
			else if (CT_HASH("ABGR16F") == color_fmt_str_hash)
			{
				color_fmt = EF_ABGR16F;
			}

			size_t const depth_stencil_fmt_str_hash = RT_HASH(depth_stencil_fmt_str.c_str());
			if (CT_HASH("D16") == depth_stencil_fmt_str_hash)
			{
				depth_stencil_fmt = EF_D16;
			}
			else if (CT_HASH("D24S8") == depth_stencil_fmt_str_hash)
			{
				depth_stencil_fmt = EF_D24S8;
			}
			else if (CT_HASH("D32F") == depth_stencil_fmt_str_hash)
			{
				depth_stencil_fmt = EF_D32F;
			}
			else
			{
				depth_stencil_fmt = EF_Unknown;
			}

			XMLNode const* sample_node = frame_node->FirstNode("sample");
			if (XMLAttribute const* attr = sample_node->Attrib("count"))
			{
				sample_count = attr->ValueInt();
			}
			if (XMLAttribute const* attr = sample_node->Attrib("quality"))
			{
				sample_quality = attr->ValueInt();
			}

			XMLNode const* sync_interval_node = graphics_node->FirstNode("sync_interval");
			if (XMLAttribute const* attr = sync_interval_node->Attrib("value"))
			{
				sync_interval = attr->ValueInt();
			}

			XMLNode const* hdr_node = graphics_node->FirstNode("hdr");
			if (XMLAttribute const* attr = hdr_node->Attrib("value"))
			{
				hdr = attr->ValueBool();
			}
			if (XMLAttribute const* attr = hdr_node->Attrib("bloom"))
			{
				bloom = attr->ValueFloat();
			}
			if (XMLAttribute const* attr = hdr_node->Attrib("blue_shift"))
			{
				blue_shift = attr->ValueBool();
			}

			XMLNode const* ppaa_node = graphics_node->FirstNode("ppaa");
			if (XMLAttribute const* attr = ppaa_node->Attrib("value"))
			{
				ppaa = attr->ValueBool();
			}

			XMLNode const* gamma_node = graphics_node->FirstNode("gamma");
			if (XMLAttribute const* attr = gamma_node->Attrib("value"))
			{
				gamma = attr->ValueBool();
			}

			XMLNode const* color_grading_node = graphics_node->FirstNode("color_grading");
			if (XMLAttribute const* attr = color_grading_node->Attrib("value"))
			{
				color_grading = attr->ValueBool();
			}

			XMLNode const* stereo_node = graphics_node->FirstNode("stereo");
			if (XMLAttribute const* attr = stereo_node->Attrib("method"))
			{
				size_t const method_str_hash = HashValue(attr->ValueString());
				if (CT_HASH("none") == method_str_hash)
				{
					stereo_method = STM_None;
				}
				else if (CT_HASH("red_cyan") == method_str_hash)
				{
					stereo_method = STM_ColorAnaglyph_RedCyan;
				}
				else if (CT_HASH("yellow_blue") == method_str_hash)
				{
					stereo_method = STM_ColorAnaglyph_YellowBlue;
				}
				else if (CT_HASH("green_red") == method_str_hash)
				{
					stereo_method = STM_ColorAnaglyph_GreenRed;
				}
				else if (CT_HASH("lcd_shutter") == method_str_hash)
				{
					stereo_method = STM_LCDShutter;
				}
				else if (CT_HASH("hor_interlacing") == method_str_hash)
				{
					stereo_method = STM_HorizontalInterlacing;
				}
				else if (CT_HASH("ver_interlacing") == method_str_hash)
				{
					stereo_method = STM_VerticalInterlacing;
				}
				else if (CT_HASH("horizontal") == method_str_hash)
				{
					stereo_method = STM_Horizontal;
				}
				else if (CT_HASH("vertical") == method_str_hash)
				{
					stereo_method = STM_Vertical;
				}
				else if (CT_HASH("oculus_vr") == method_str_hash)
				{
					stereo_method = STM_OculusVR;
				}
				else
				{
					stereo_method = STM_ColorAnaglyph_RedCyan;
				}
			}
			else
			{
				stereo_method = STM_None;
			}
			if (XMLAttribute const* attr = stereo_node->Attrib("separation"))
			{
				stereo_separation = attr->ValueFloat();
			}

			XMLNode const* output_node = graphics_node->FirstNode("output");
			if (XMLAttribute const* attr = output_node->Attrib("method"))
			{
				size_t const method_str_hash = HashValue(attr->ValueString());
				if (CT_HASH("hdr10") == method_str_hash)
				{
					display_output_method = DOM_HDR10;
				}
				else
				{
					display_output_method = DOM_sRGB;
				}
			}

			if (XMLAttribute const* attr = output_node->Attrib("white"))
			{
				paper_white = attr->ValueUInt();
			}
			if (XMLAttribute const* attr = output_node->Attrib("max_lum"))
			{
				display_max_luminance = attr->ValueUInt();
			}

			if (display_output_method == DOM_sRGB)
			{
				paper_white = display_max_luminance = 100;
			}
			else if ((color_fmt == EF_ARGB8) || (color_fmt == EF_ABGR8))
			{
				color_fmt = EF_A2BGR10;
			}

			if (XMLNode const* options_node = graphics_node->FirstNode("options"))
			{
				if (XMLAttribute const* attr = options_node->Attrib("str"))
				{
					std::string_view const options_str = attr->ValueString();

					std::vector<std::string_view> strs = StringUtil::Split(options_str, StringUtil::EqualTo(','));
					for (size_t index = 0; index < strs.size(); ++ index)
					{
						std::string_view opt = StringUtil::Trim(strs[index]);
						std::string::size_type const loc = opt.find(':');
						std::string_view opt_name = opt.substr(0, loc);
						std::string_view opt_val = opt.substr(loc + 1);
						graphics_options.emplace_back(std::string(opt_name), std::string(opt_val));
					}
				}
			}

			XMLNode const* debug_context_node = graphics_node->FirstNode("debug_context");
			if (XMLAttribute const* attr = debug_context_node->Attrib("value"))
			{
				debug_context = attr->ValueBool();
			}
		}

		std::span<char const *> const available_rfs = available_rfs_array;
		std::span<char const *> const available_afs = available_afs_array;
		std::span<char const *> const available_adsfs = available_adsfs_array;
		std::span<char const *> const available_ifs = available_ifs_array;
		std::span<char const *> const available_sfs = available_sfs_array;
		std::span<char const *> const available_scfs = available_scfs_array;
		std::span<char const *> const available_sms = available_sms_array;

		if (std::find(available_rfs.begin(), available_rfs.end(), rf_name) == available_rfs.end())
		{
			rf_name = available_rfs[0];
		}
		if (std::find(available_afs.begin(), available_afs.end(), af_name) == available_afs.end())
		{
			af_name = available_afs[0];
		}
		if (std::find(available_adsfs.begin(), available_adsfs.end(), adsf_name) == available_adsfs.end())
		{
			adsf_name = available_adsfs[0];
		}
		if (std::find(available_ifs.begin(), available_ifs.end(), if_name) == available_ifs.end())
		{
			if_name = available_ifs[0];
		}
		if (std::find(available_sfs.begin(), available_sfs.end(), sf_name) == available_sfs.end())
		{
			sf_name = available_sfs[0];
		}
		if (std::find(available_scfs.begin(), available_scfs.end(), scf_name) == available_scfs.end())
		{
			scf_name = available_scfs[0];
		}
		if (std::find(available_sms.begin(), available_sms.end(), sm_name) == available_sms.end())
		{
			sm_name = available_sms[0];
		}

		cfg_.render_factory_name = std::move(rf_name);
		cfg_.audio_factory_name = std::move(af_name);
		cfg_.input_factory_name = std::move(if_name);
		cfg_.show_factory_name = std::move(sf_name);
		cfg_.script_factory_name = std::move(scf_name);
		cfg_.scene_manager_name = std::move(sm_name);
		cfg_.audio_data_source_factory_name = std::move(adsf_name);

		cfg_.graphics_cfg.left = cfg_.graphics_cfg.top = 0;
		cfg_.graphics_cfg.width = width;
		cfg_.graphics_cfg.height = height;
		cfg_.graphics_cfg.color_fmt = color_fmt;
		cfg_.graphics_cfg.depth_stencil_fmt = depth_stencil_fmt;
		cfg_.graphics_cfg.sample_count = sample_count;
		cfg_.graphics_cfg.sample_quality = sample_quality;
		cfg_.graphics_cfg.full_screen = full_screen;
		cfg_.graphics_cfg.sync_interval = sync_interval;
		cfg_.graphics_cfg.hdr = hdr;
		cfg_.graphics_cfg.ppaa = ppaa;
		cfg_.graphics_cfg.gamma = gamma;
		cfg_.graphics_cfg.color_grading = color_grading;
		cfg_.graphics_cfg.bloom = bloom;
		cfg_.graphics_cfg.blue_shift = blue_shift;
		cfg_.graphics_cfg.keep_screen_on = keep_screen_on;
		cfg_.graphics_cfg.stereo_method = stereo_method;
		cfg_.graphics_cfg.stereo_separation = stereo_separation;
		cfg_.graphics_cfg.display_output_method = display_output_method;
		cfg_.graphics_cfg.paper_white = paper_white;
		cfg_.graphics_cfg.display_max_luminance = display_max_luminance;
		cfg_.graphics_cfg.options = std::move(graphics_options);
		cfg_.graphics_cfg.debug_context = debug_context;

		cfg_.deferred_rendering = false;
		cfg_.perf_profiler = perf_profiler;
		cfg_.location_sensor = location_sensor;
	}

	void Context::SaveCfg(std::string const & cfg_file)
	{
		XMLDocument cfg_doc;
		auto root = cfg_doc.AllocNode(XMLNodeType::Element, "configure");

		{
			auto context_node = cfg_doc.AllocNode(XMLNodeType::Element, "context");
			{
				auto rf_node = cfg_doc.AllocNode(XMLNodeType::Element, "render_factory");
				rf_node->AppendAttrib(cfg_doc.AllocAttribString("name", cfg_.render_factory_name));
				context_node->AppendNode(std::move(rf_node));

				auto af_node = cfg_doc.AllocNode(XMLNodeType::Element, "audio_factory");
				af_node->AppendAttrib(cfg_doc.AllocAttribString("name", cfg_.audio_factory_name));
				context_node->AppendNode(std::move(af_node));

				auto if_node = cfg_doc.AllocNode(XMLNodeType::Element, "input_factory");
				if_node->AppendAttrib(cfg_doc.AllocAttribString("name", cfg_.input_factory_name));
				context_node->AppendNode(std::move(if_node));

				auto sm_node = cfg_doc.AllocNode(XMLNodeType::Element, "scene_manager");
				sm_node->AppendAttrib(cfg_doc.AllocAttribString("name", cfg_.scene_manager_name));
				context_node->AppendNode(std::move(sm_node));

				auto sf_node = cfg_doc.AllocNode(XMLNodeType::Element, "show_factory");
				sf_node->AppendAttrib(cfg_doc.AllocAttribString("name", cfg_.show_factory_name));
				context_node->AppendNode(std::move(sf_node));

				auto scf_node = cfg_doc.AllocNode(XMLNodeType::Element, "script_factory");
				scf_node->AppendAttrib(cfg_doc.AllocAttribString("name", cfg_.script_factory_name));
				context_node->AppendNode(std::move(scf_node));

				auto adsf_node = cfg_doc.AllocNode(XMLNodeType::Element, "audio_data_source_factory");
				adsf_node->AppendAttrib(cfg_doc.AllocAttribString("name", cfg_.audio_data_source_factory_name));
				context_node->AppendNode(std::move(adsf_node));

				auto perf_profiler_node = cfg_doc.AllocNode(XMLNodeType::Element, "perf_profiler");
				perf_profiler_node->AppendAttrib(cfg_doc.AllocAttribInt("enabled", cfg_.perf_profiler));
				context_node->AppendNode(std::move(perf_profiler_node));

				auto location_sensor_node = cfg_doc.AllocNode(XMLNodeType::Element, "location_sensor");
				location_sensor_node->AppendAttrib(cfg_doc.AllocAttribInt("enabled", cfg_.location_sensor));
				context_node->AppendNode(std::move(location_sensor_node));
			}
			root->AppendNode(std::move(context_node));
		}
		{
			auto graphics_node = cfg_doc.AllocNode(XMLNodeType::Element, "graphics");
			{
				{
					auto frame_node = cfg_doc.AllocNode(XMLNodeType::Element, "frame");
					frame_node->AppendAttrib(cfg_doc.AllocAttribInt("width", cfg_.graphics_cfg.width));
					frame_node->AppendAttrib(cfg_doc.AllocAttribInt("height", cfg_.graphics_cfg.height));

					std::string color_fmt_str;
					switch (cfg_.graphics_cfg.color_fmt)
					{
					case EF_ARGB8:
						color_fmt_str = "ARGB8";
						break;

					case EF_ABGR8:
						color_fmt_str = "ABGR8";
						break;

					case EF_A2BGR10:
						color_fmt_str = "A2BGR10";
						break;

					case EF_ABGR16F:
						color_fmt_str = "ABGR16F";
						break;

					default:
						color_fmt_str = "ARGB8";
						break;
					}
					frame_node->AppendAttrib(cfg_doc.AllocAttribString("color_fmt", color_fmt_str));

					std::string depth_stencil_fmt_str;
					switch (cfg_.graphics_cfg.depth_stencil_fmt)
					{
					case EF_D16:
						depth_stencil_fmt_str = "D16";
						break;

					case EF_D24S8:
						depth_stencil_fmt_str = "D24S8";
						break;

					case EF_D32F:
						depth_stencil_fmt_str = "D32F";
						break;

					default:
						depth_stencil_fmt_str = "None";
						break;
					}
					frame_node->AppendAttrib(cfg_doc.AllocAttribString("depth_stencil_fmt", depth_stencil_fmt_str));

					frame_node->AppendAttrib(cfg_doc.AllocAttribInt("fullscreen", cfg_.graphics_cfg.full_screen));
					frame_node->AppendAttrib(cfg_doc.AllocAttribInt("keep_screen_on", cfg_.graphics_cfg.keep_screen_on));

					{
						auto sample_node = cfg_doc.AllocNode(XMLNodeType::Element, "sample");
						sample_node->AppendAttrib(cfg_doc.AllocAttribInt("count", cfg_.graphics_cfg.sample_count));
						sample_node->AppendAttrib(cfg_doc.AllocAttribInt("quality", cfg_.graphics_cfg.sample_quality));
						frame_node->AppendNode(std::move(sample_node));
					}

					graphics_node->AppendNode(std::move(frame_node));
				}
				{
					auto sync_interval_node = cfg_doc.AllocNode(XMLNodeType::Element, "sync_interval");
					sync_interval_node->AppendAttrib(cfg_doc.AllocAttribInt("value", cfg_.graphics_cfg.sync_interval));
					graphics_node->AppendNode(std::move(sync_interval_node));
				}
				{
					auto hdr_node = cfg_doc.AllocNode(XMLNodeType::Element, "hdr");
					hdr_node->AppendAttrib(cfg_doc.AllocAttribInt("value", cfg_.graphics_cfg.hdr));
					hdr_node->AppendAttrib(cfg_doc.AllocAttribFloat("bloom", cfg_.graphics_cfg.bloom));
					hdr_node->AppendAttrib(cfg_doc.AllocAttribInt("blue_shift", cfg_.graphics_cfg.blue_shift));
					graphics_node->AppendNode(std::move(hdr_node));
				}
				{
					auto ppaa_node = cfg_doc.AllocNode(XMLNodeType::Element, "ppaa");
					ppaa_node->AppendAttrib(cfg_doc.AllocAttribInt("value", cfg_.graphics_cfg.ppaa));
					graphics_node->AppendNode(std::move(ppaa_node));
				}
				{
					auto gamma_node = cfg_doc.AllocNode(XMLNodeType::Element, "gamma");
					gamma_node->AppendAttrib(cfg_doc.AllocAttribInt("value", cfg_.graphics_cfg.gamma));
					graphics_node->AppendNode(std::move(gamma_node));
				}
				{
					auto color_grading_node = cfg_doc.AllocNode(XMLNodeType::Element, "color_grading");
					color_grading_node->AppendAttrib(cfg_doc.AllocAttribInt("value", cfg_.graphics_cfg.color_grading));
					graphics_node->AppendNode(std::move(color_grading_node));
				}
				{
					auto stereo_node = cfg_doc.AllocNode(XMLNodeType::Element, "stereo");
					std::string method_str;
					switch (cfg_.graphics_cfg.stereo_method)
					{
					case STM_None:
						method_str = "none";
						break;

					case STM_ColorAnaglyph_RedCyan:
						method_str = "red_cyan";
						break;

					case STM_ColorAnaglyph_YellowBlue:
						method_str = "yellow_blue";
						break;

					case STM_ColorAnaglyph_GreenRed:
						method_str = "green_red";
						break;

					case STM_LCDShutter:
						method_str = "lcd_shutter";
						break;

					case STM_HorizontalInterlacing:
						method_str = "hor_interlacing";
						break;

					case STM_VerticalInterlacing:
						method_str = "ver_interlacing";
						break;

					case STM_Horizontal:
						method_str = "horizontal";
						break;

					case STM_Vertical:
						method_str = "vertical";
						break;

					case STM_OculusVR:
						method_str = "oculus_vr";
						break;

					default:
						method_str = "none";
						break;
					}
					stereo_node->AppendAttrib(cfg_doc.AllocAttribString("method", method_str));

					std::ostringstream oss;
					oss.precision(2);
					oss << std::fixed << cfg_.graphics_cfg.stereo_separation;
					stereo_node->AppendAttrib(cfg_doc.AllocAttribString("separation", oss.str()));

					graphics_node->AppendNode(std::move(stereo_node));
				}
				{
					auto output_node = cfg_doc.AllocNode(XMLNodeType::Element, "output");
					std::string method_str;
					switch (cfg_.graphics_cfg.display_output_method)
					{
					case DOM_HDR10:
						method_str = "hdr10";
						break;

					default:
						method_str = "srgb";
						break;
					}
					output_node->AppendAttrib(cfg_doc.AllocAttribString("method", method_str));

					output_node->AppendAttrib(cfg_doc.AllocAttribString("white", std::to_string(cfg_.graphics_cfg.paper_white)));
					output_node->AppendAttrib(
						cfg_doc.AllocAttribString("max_lum", std::to_string(cfg_.graphics_cfg.display_max_luminance)));

					graphics_node->AppendNode(std::move(output_node));
				}
				{
					auto debug_context_node = cfg_doc.AllocNode(XMLNodeType::Element, "debug_context");
					debug_context_node->AppendAttrib(cfg_doc.AllocAttribInt("value", cfg_.graphics_cfg.debug_context));
					graphics_node->AppendNode(std::move(debug_context_node));
				}
			}
			root->AppendNode(std::move(graphics_node));
		}

		cfg_doc.RootNode(std::move(root));

		std::ofstream ofs(cfg_file.c_str());
		SaveXml(cfg_doc, ofs);
	}

	void Context::Config(ContextCfg const & cfg)
	{
		cfg_ = cfg;

		if (this->RenderFactoryValid())
		{
			if (cfg_.deferred_rendering)
			{
				if (!deferred_rendering_layer_)
				{
					deferred_rendering_layer_ = MakeUniquePtr<DeferredRenderingLayer>();
				}
			}
			else
			{
				deferred_rendering_layer_.reset();
			}
		}
	}

	ContextCfg const & Context::Config() const
	{
		return cfg_;
	}

	void Context::LoadRenderFactory(std::string const & rf_name)
	{
		render_factory_.reset();

#ifndef KLAYGE_STATIC_LINK_PLUGINS
		render_loader_.Free();

		std::string render_path = ResLoader::Instance().Locate("Render");
		std::string fn = KLAYGE_DLL_PREFIX"_RenderEngine_" + rf_name + DLL_SUFFIX;

		std::string path = render_path + "/" + fn;
		render_loader_.Load(ResLoader::Instance().Locate(path));

		MakeRenderFactoryFunc mrf = (MakeRenderFactoryFunc)render_loader_.GetProcAddress("MakeRenderFactory");
		if (mrf != nullptr)
		{
			mrf(render_factory_);
		}
		else
		{
			LogError() << "Loading " << path << " failed" << std::endl;
			render_loader_.Free();
		}
#else
		KFL_UNUSED(rf_name);
		MakeRenderFactory(render_factory_);
#endif
	}

	void Context::LoadAudioFactory(std::string const & af_name)
	{
		audio_factory_.reset();

#ifndef KLAYGE_STATIC_LINK_PLUGINS
		audio_loader_.Free();

		std::string audio_path = ResLoader::Instance().Locate("Audio");
		std::string fn = KLAYGE_DLL_PREFIX"_AudioEngine_" + af_name + DLL_SUFFIX;

		std::string path = audio_path + "/" + fn;
		audio_loader_.Load(ResLoader::Instance().Locate(path));

		MakeAudioFactoryFunc maf = (MakeAudioFactoryFunc)audio_loader_.GetProcAddress("MakeAudioFactory");
		if (maf != nullptr)
		{
			maf(audio_factory_);
		}
		else
		{
			LogError() << "Loading " << path << " failed" << std::endl;
			audio_loader_.Free();
		}
#else
		KFL_UNUSED(af_name);
		MakeAudioFactory(audio_factory_);
#endif
	}

	void Context::LoadInputFactory(std::string const & if_name)
	{
		input_factory_.reset();

#ifndef KLAYGE_STATIC_LINK_PLUGINS
		input_loader_.Free();

		std::string input_path = ResLoader::Instance().Locate("Input");
		std::string fn = KLAYGE_DLL_PREFIX"_InputEngine_" + if_name + DLL_SUFFIX;

		std::string path = input_path + "/" + fn;
		input_loader_.Load(ResLoader::Instance().Locate(path));

		MakeInputFactoryFunc mif = (MakeInputFactoryFunc)input_loader_.GetProcAddress("MakeInputFactory");
		if (mif != nullptr)
		{
			mif(input_factory_);
		}
		else
		{
			LogError() << "Loading " << path << " failed" << std::endl;
			input_loader_.Free();
		}
#else
		KFL_UNUSED(if_name);
		MakeInputFactory(input_factory_);
#endif
	}

	void Context::LoadShowFactory(std::string const & sf_name)
	{
		show_factory_.reset();

#ifndef KLAYGE_STATIC_LINK_PLUGINS
		show_loader_.Free();

		std::string show_path = ResLoader::Instance().Locate("Show");
		std::string fn = KLAYGE_DLL_PREFIX"_ShowEngine_" + sf_name + DLL_SUFFIX;

		std::string path = show_path + "/" + fn;
		show_loader_.Load(ResLoader::Instance().Locate(path));

		MakeShowFactoryFunc msf = (MakeShowFactoryFunc)show_loader_.GetProcAddress("MakeShowFactory");
		if (msf != nullptr)
		{
			msf(show_factory_);
		}
		else
		{
			LogError() << "Loading " << path << " failed" << std::endl;
			show_loader_.Free();
		}
#else
		KFL_UNUSED(sf_name);
		MakeShowFactory(show_factory_);
#endif
	}

	void Context::LoadScriptFactory(std::string const & sf_name)
	{
		script_factory_.reset();

#ifndef KLAYGE_STATIC_LINK_PLUGINS
		script_loader_.Free();

		std::string script_path = ResLoader::Instance().Locate("Script");
		std::string fn = KLAYGE_DLL_PREFIX"_ScriptEngine_" + sf_name + DLL_SUFFIX;

		std::string path = script_path + "/" + fn;
		script_loader_.Load(ResLoader::Instance().Locate(path));

		MakeScriptFactoryFunc msf = (MakeScriptFactoryFunc)script_loader_.GetProcAddress("MakeScriptFactory");
		if (msf != nullptr)
		{
			msf(script_factory_);
		}
		else
		{
			LogError() << "Loading " << path << " failed" << std::endl;
			script_loader_.Free();
		}
#else
		KFL_UNUSED(sf_name);
		MakeScriptFactory(script_factory_);
#endif
	}

	void Context::LoadSceneManager(std::string const & sm_name)
	{
		scene_mgr_.reset();

#ifndef KLAYGE_STATIC_LINK_PLUGINS
		sm_loader_.Free();

		std::string sm_path = ResLoader::Instance().Locate("Scene");
		std::string fn = KLAYGE_DLL_PREFIX"_Scene_" + sm_name + DLL_SUFFIX;

		std::string path = sm_path + "/" + fn;
		sm_loader_.Load(ResLoader::Instance().Locate(path));

		MakeSceneManagerFunc msm = (MakeSceneManagerFunc)sm_loader_.GetProcAddress("MakeSceneManager");
		if (msm != nullptr)
		{
			msm(scene_mgr_);
		}
		else
		{
			LogError() << "Loading " << path << " failed" << std::endl;
			sm_loader_.Free();
		}
#else
		KFL_UNUSED(sm_name);
		MakeSceneManager(scene_mgr_);
#endif
	}

	void Context::LoadAudioDataSourceFactory(std::string const & adsf_name)
	{
		audio_data_src_factory_.reset();

#ifndef KLAYGE_STATIC_LINK_PLUGINS
		ads_loader_.Free();

		std::string adsf_path = ResLoader::Instance().Locate("Audio");
		std::string fn = KLAYGE_DLL_PREFIX"_AudioDataSource_" + adsf_name + DLL_SUFFIX;

		std::string path = adsf_path + "/" + fn;
		ads_loader_.Load(ResLoader::Instance().Locate(path));

		MakeAudioDataSourceFactoryFunc madsf = (MakeAudioDataSourceFactoryFunc)ads_loader_.GetProcAddress("MakeAudioDataSourceFactory");
		if (madsf != nullptr)
		{
			madsf(audio_data_src_factory_);
		}
		else
		{
			LogError() << "Loading " << path << " failed" << std::endl;
			ads_loader_.Free();
		}
#else
		KFL_UNUSED(adsf_name);
		MakeAudioDataSourceFactory(audio_data_src_factory_);
#endif
	}

#if KLAYGE_IS_DEV_PLATFORM
	void Context::LoadDevHelper()
	{
		dev_helper_.reset();

		dev_helper_loader_.Free();

		std::string path = KLAYGE_DLL_PREFIX"_DevHelper" DLL_SUFFIX;

		dev_helper_loader_.Load(ResLoader::Instance().Locate(path));

		MakeDevHelperFunc mdh = (MakeDevHelperFunc)dev_helper_loader_.GetProcAddress("MakeDevHelper");
		if (mdh != nullptr)
		{
			mdh(dev_helper_);
		}
		else
		{
			LogError() << "Loading " << path << " failed" << std::endl;
			dev_helper_loader_.Free();
		}
	}
#endif

	SceneManager& Context::SceneManagerInstance()
	{
		if (!scene_mgr_)
		{
			std::lock_guard<std::mutex> lock(singleton_mutex);
			if (!scene_mgr_)
			{
				this->LoadSceneManager(cfg_.scene_manager_name);
			}
		}
		return *scene_mgr_;
	}

	RenderFactory& Context::RenderFactoryInstance()
	{
		if (!render_factory_)
		{
			std::lock_guard<std::mutex> lock(singleton_mutex);
			if (!render_factory_)
			{
				this->LoadRenderFactory(cfg_.render_factory_name);
			}
		}
		return *render_factory_;
	}

	AudioFactory& Context::AudioFactoryInstance()
	{
		if (!audio_factory_)
		{
			std::lock_guard<std::mutex> lock(singleton_mutex);
			if (!audio_factory_)
			{
				this->LoadAudioFactory(cfg_.audio_factory_name);
			}
		}
		return *audio_factory_;
	}

	InputFactory& Context::InputFactoryInstance()
	{
		if (!input_factory_)
		{
			std::lock_guard<std::mutex> lock(singleton_mutex);
			if (!input_factory_)
			{
				this->LoadInputFactory(cfg_.input_factory_name);
			}
		}
		return *input_factory_;
	}

	ShowFactory& Context::ShowFactoryInstance()
	{
		if (!show_factory_)
		{
			std::lock_guard<std::mutex> lock(singleton_mutex);
			if (!show_factory_)
			{
				this->LoadShowFactory(cfg_.show_factory_name);
			}
		}
		return *show_factory_;
	}

	ScriptFactory& Context::ScriptFactoryInstance()
	{
		if (!script_factory_)
		{
			std::lock_guard<std::mutex> lock(singleton_mutex);
			if (!script_factory_)
			{
				this->LoadScriptFactory(cfg_.script_factory_name);
			}
		}
		return *script_factory_;
	}

	AudioDataSourceFactory& Context::AudioDataSourceFactoryInstance()
	{
		if (!audio_data_src_factory_)
		{
			std::lock_guard<std::mutex> lock(singleton_mutex);
			if (!audio_data_src_factory_)
			{
				this->LoadAudioDataSourceFactory(cfg_.audio_data_source_factory_name);
			}
		}
		return *audio_data_src_factory_;
	}

#if KLAYGE_IS_DEV_PLATFORM
	DevHelper& Context::DevHelperInstance()
	{
		if (!dev_helper_)
		{
			std::lock_guard<std::mutex> lock(singleton_mutex);
			if (!dev_helper_)
			{
				this->LoadDevHelper();
			}
		}
		return *dev_helper_;
	}
#endif
}
