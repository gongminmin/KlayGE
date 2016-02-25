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
#include <KFL/Util.hpp>
#include <KFL/Math.hpp>
#include <KFL/Log.hpp>
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
#include <KFL/Thread.hpp>
#include <KlayGE/PerfProfiler.hpp>
#include <KlayGE/UI.hpp>

#include <fstream>
#include <sstream>
#if defined(KLAYGE_COMPILER_GCC)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations" // Ignore auto_ptr declaration
#endif
#include <boost/algorithm/string/split.hpp>
#if defined(KLAYGE_COMPILER_GCC)
#pragma GCC diagnostic pop
#endif
#include <boost/algorithm/string/trim.hpp>

#ifdef KLAYGE_PLATFORM_WINDOWS
#include <windows.h>
#if defined(KLAYGE_PLATFORM_WINDOWS_DESKTOP)
#if (_WIN32_WINNT >= _WIN32_WINNT_WINBLUE)
#include <VersionHelpers.h>
#endif
#endif
#endif

#if defined(KLAYGE_PLATFORM_ANDROID) || defined(KLAYGE_PLATFORM_IOS)
#include <KlayGE/OpenGLES/OGLESRenderFactory.hpp>
#include <KlayGE/OCTree/OCTreeFactory.hpp>
#include <KlayGE/MsgInput/MInputFactory.hpp>
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

	Context::Context()
		: app_(nullptr)
	{
#ifdef KLAYGE_PLATFORM_ANDROID
		state_ = get_app();
#endif

#ifdef KLAYGE_COMPILER_MSVC
#ifdef KLAYGE_DEBUG
		_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif
#endif

		gtp_instance_ = MakeUniquePtr<thread_pool>(1, 16);
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
		int width = 800;
		int height = 600;
		ElementFormat color_fmt = EF_ARGB8;
		ElementFormat depth_stencil_fmt = EF_D16;
		int sample_count = 1;
		int sample_quality = 0;
		bool full_screen = false;
		int sync_interval = 0;
		int motion_frames = 0;
		bool hdr = false;
		bool ppaa = false;
		bool gamma = false;
		bool color_grading = false;
		int stereo_method = 0;
		float stereo_separation = 0;
		std::vector<std::pair<std::string, std::string>> graphics_options;
		bool perf_profiler = false;
		bool location_sensor = false;

		std::string rf_name = "D3D11";
		std::string af_name = "OpenAL";
		std::string if_name = "MsgInput";
		std::string sf_name = "DShow";
		std::string scf_name = "Python";
		std::string sm_name;
		std::string adsf_name;

		ResIdentifierPtr file = ResLoader::Instance().Open(cfg_file);
		if (file)
		{
			XMLDocument cfg_doc;
			XMLNodePtr cfg_root = cfg_doc.Parse(file);

			XMLNodePtr context_node = cfg_root->FirstNode("context");
			XMLNodePtr graphics_node = cfg_root->FirstNode("graphics");

			XMLNodePtr rf_node = context_node->FirstNode("render_factory");
			if (rf_node)
			{
				rf_name = rf_node->Attrib("name")->ValueString();
			}

			XMLNodePtr af_node = context_node->FirstNode("audio_factory");
			if (af_node)
			{
				af_name = af_node->Attrib("name")->ValueString();
			}

			XMLNodePtr if_node = context_node->FirstNode("input_factory");
			if (if_node)
			{
				if_name = if_node->Attrib("name")->ValueString();
			}

			XMLNodePtr sf_node = context_node->FirstNode("show_factory");
			if (sf_node)
			{
				sf_name = sf_node->Attrib("name")->ValueString();
			}

			XMLNodePtr scf_node = context_node->FirstNode("script_factory");
			if (scf_node)
			{
				scf_name = scf_node->Attrib("name")->ValueString();
			}

			XMLNodePtr sm_node = context_node->FirstNode("scene_manager");
			if (sm_node)
			{
				sm_name = sm_node->Attrib("name")->ValueString();
			}

			XMLNodePtr adsf_node = context_node->FirstNode("audio_data_source_factory");
			if (adsf_node)
			{
				adsf_name = adsf_node->Attrib("name")->ValueString();
			}

			XMLNodePtr perf_profiler_node = context_node->FirstNode("perf_profiler");
			if (perf_profiler_node)
			{
				perf_profiler = perf_profiler_node->Attrib("enabled")->ValueInt() ? true : false;
			}

			XMLNodePtr location_sensor_node = context_node->FirstNode("location_sensor");
			if (location_sensor_node)
			{
				location_sensor = location_sensor_node->Attrib("enabled")->ValueInt() ? true : false;
			}

			XMLNodePtr frame_node = graphics_node->FirstNode("frame");
			XMLAttributePtr attr;
			attr = frame_node->Attrib("width");
			if (attr)
			{
				width = attr->ValueInt();
			}
			attr = frame_node->Attrib("height");
			if (attr)
			{
				height = attr->ValueInt();
			}
			std::string color_fmt_str = "ARGB8";
			attr = frame_node->Attrib("color_fmt");
			if (attr)
			{
				color_fmt_str = attr->ValueString();
			}
			std::string depth_stencil_fmt_str = "D16";
			attr = frame_node->Attrib("depth_stencil_fmt");
			if (attr)
			{
				depth_stencil_fmt_str = attr->ValueString();
			}
			attr = frame_node->Attrib("fullscreen");
			if (attr)
			{
				std::string const & fs_str = attr->ValueString();
				if (("1" == fs_str) || ("true" == fs_str))
				{
					full_screen = true;
				}
				else
				{
					full_screen = false;
				}
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

			XMLNodePtr sample_node = frame_node->FirstNode("sample");
			attr = sample_node->Attrib("count");
			if (attr)
			{
				sample_count = attr->ValueInt();
			}
			attr = sample_node->Attrib("quality");
			if (attr)
			{
				sample_quality = attr->ValueInt();
			}

			XMLNodePtr sync_interval_node = graphics_node->FirstNode("sync_interval");
			attr = sync_interval_node->Attrib("value");
			if (attr)
			{
				sync_interval = attr->ValueInt();
			}

			XMLNodePtr motion_blur_node = graphics_node->FirstNode("motion_blur");
			attr = motion_blur_node->Attrib("frames");
			if (attr)
			{
				motion_frames = attr->ValueInt();
			}

			XMLNodePtr hdr_node = graphics_node->FirstNode("hdr");
			attr = hdr_node->Attrib("value");
			if (attr)
			{
				std::string const & hdr_str = attr->ValueString();
				if (("1" == hdr_str) || ("true" == hdr_str))
				{
					hdr = true;
				}
				else
				{
					hdr = false;
				}
			}

			XMLNodePtr ppaa_node = graphics_node->FirstNode("ppaa");
			attr = hdr_node->Attrib("value");
			if (attr)
			{
				std::string const & ppaa_str = attr->ValueString();
				if (("1" == ppaa_str) || ("true" == ppaa_str))
				{
					ppaa = true;
				}
				else
				{
					ppaa = false;
				}
			}

			XMLNodePtr gamma_node = graphics_node->FirstNode("gamma");
			attr = gamma_node->Attrib("value");
			if (attr)
			{
				std::string const & gamma_str = attr->ValueString();
				if (("1" == gamma_str) || ("true" == gamma_str))
				{
					gamma = true;
				}
				else
				{
					gamma = false;
				}
			}

			XMLNodePtr color_grading_node = graphics_node->FirstNode("color_grading");
			attr = color_grading_node->Attrib("value");
			if (attr)
			{
				std::string const & color_grading_str = attr->ValueString();
				if (("1" == color_grading_str) || ("true" == color_grading_str))
				{
					color_grading = true;
				}
				else
				{
					color_grading = false;
				}
			}

			XMLNodePtr stereo_node = graphics_node->FirstNode("stereo");
			attr = stereo_node->Attrib("method");
			if (attr)
			{
				std::string const & method_str = attr->ValueString();
				size_t const method_str_hash = RT_HASH(method_str.c_str());
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
			attr = stereo_node->Attrib("separation");
			if (attr)
			{
				stereo_separation = attr->ValueFloat();
			}

			XMLNodePtr options_node = graphics_node->FirstNode("options");
			if (options_node)
			{
				attr = options_node->Attrib("str");
				if (attr)
				{
					std::string const & options_str = attr->ValueString();

					std::vector<std::string> strs;
					boost::algorithm::split(strs, options_str, boost::is_any_of(","));
					for (size_t index = 0; index < strs.size(); ++ index)
					{
						std::string& opt = strs[index];
						boost::algorithm::trim(opt);
						std::string::size_type const loc = opt.find(':');
						std::string opt_name = opt.substr(0, loc);
						std::string opt_val = opt.substr(loc + 1);
						graphics_options.emplace_back(opt_name, opt_val);
					}
				}
			}
		}

#if defined(KLAYGE_PLATFORM_WINDOWS_DESKTOP)
#if (_WIN32_WINNT >= _WIN32_WINNT_WINBLUE)
		if (!IsWindowsVistaOrGreater())
#else
		OSVERSIONINFO os_ver_info;
		memset(&os_ver_info, 0, sizeof(os_ver_info));
		os_ver_info.dwOSVersionInfoSize = sizeof(os_ver_info);
		::GetVersionEx(&os_ver_info);

		if (os_ver_info.dwMajorVersion < 6)
#endif
		{
			if ("D3D11" == rf_name)
			{
				rf_name = "OpenGL";
			}
		}

#elif defined(KLAYGE_PLATFORM_WINDOWS_RUNTIME)
		rf_name = "D3D11";
#elif defined(KLAYGE_PLATFORM_LINUX)
		if ("D3D11" == rf_name)
		{
			rf_name = "OpenGL";
		}
		if ("DSound" == af_name)
		{
			af_name = "OpenAL";
		}
#elif defined(KLAYGE_PLATFORM_ANDROID)
		rf_name = "OpenGLES";
		af_name = "OpenAL";
#elif defined(KLAYGE_PLATFORM_IOS)
		rf_name = "OpenGLES";
		af_name = "OpenAL";
#elif defined(KLAYGE_PLATFORM_DARWIN)
		if ("D3D11" == rf_name)
		{
			rf_name = "OpenGL";
		}
		af_name = "OpenAL";
#endif

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
		cfg_.graphics_cfg.motion_frames = motion_frames;
		cfg_.graphics_cfg.hdr = hdr;
		cfg_.graphics_cfg.ppaa = ppaa;
		cfg_.graphics_cfg.gamma = gamma;
		cfg_.graphics_cfg.color_grading = color_grading;
		cfg_.graphics_cfg.stereo_method = static_cast<StereoMethod>(stereo_method);
		cfg_.graphics_cfg.stereo_separation = stereo_separation;
		cfg_.graphics_cfg.options = std::move(graphics_options);

		cfg_.deferred_rendering = false;
		cfg_.perf_profiler = perf_profiler;
		cfg_.location_sensor = location_sensor;
	}

	void Context::SaveCfg(std::string const & cfg_file)
	{
		XMLDocument cfg_doc;
		XMLNodePtr root = cfg_doc.AllocNode(XNT_Element, "configure");
		cfg_doc.RootNode(root);

		XMLNodePtr context_node = cfg_doc.AllocNode(XNT_Element, "context");
		{
			XMLNodePtr rf_node = cfg_doc.AllocNode(XNT_Element, "render_factory");
			rf_node->AppendAttrib(cfg_doc.AllocAttribString("name", cfg_.render_factory_name));
			context_node->AppendNode(rf_node);

			XMLNodePtr af_node = cfg_doc.AllocNode(XNT_Element, "audio_factory");
			af_node->AppendAttrib(cfg_doc.AllocAttribString("name", cfg_.audio_factory_name));
			context_node->AppendNode(af_node);

			XMLNodePtr if_node = cfg_doc.AllocNode(XNT_Element, "input_factory");
			if_node->AppendAttrib(cfg_doc.AllocAttribString("name", cfg_.input_factory_name));
			context_node->AppendNode(if_node);

			XMLNodePtr sm_node = cfg_doc.AllocNode(XNT_Element, "scene_manager");
			sm_node->AppendAttrib(cfg_doc.AllocAttribString("name", cfg_.scene_manager_name));
			context_node->AppendNode(sm_node);

			XMLNodePtr sf_node = cfg_doc.AllocNode(XNT_Element, "show_factory");
			sf_node->AppendAttrib(cfg_doc.AllocAttribString("name", cfg_.show_factory_name));
			context_node->AppendNode(sf_node);

			XMLNodePtr scf_node = cfg_doc.AllocNode(XNT_Element, "script_factory");
			scf_node->AppendAttrib(cfg_doc.AllocAttribString("name", cfg_.script_factory_name));
			context_node->AppendNode(scf_node);

			XMLNodePtr adsf_node = cfg_doc.AllocNode(XNT_Element, "audio_data_source_factory");
			adsf_node->AppendAttrib(cfg_doc.AllocAttribString("name", cfg_.audio_data_source_factory_name));
			context_node->AppendNode(adsf_node);

			XMLNodePtr perf_profiler_node = cfg_doc.AllocNode(XNT_Element, "perf_profiler");
			perf_profiler_node->AppendAttrib(cfg_doc.AllocAttribInt("enabled", cfg_.perf_profiler));
			context_node->AppendNode(perf_profiler_node);

			XMLNodePtr location_sensor_node = cfg_doc.AllocNode(XNT_Element, "location_sensor");
			location_sensor_node->AppendAttrib(cfg_doc.AllocAttribInt("enabled", cfg_.location_sensor));
			context_node->AppendNode(location_sensor_node);
		}
		root->AppendNode(context_node);

		XMLNodePtr graphics_node = cfg_doc.AllocNode(XNT_Element, "graphics");
		{
			XMLNodePtr frame_node = cfg_doc.AllocNode(XNT_Element, "frame");
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

			{
				XMLNodePtr sample_node = cfg_doc.AllocNode(XNT_Element, "sample");
				sample_node->AppendAttrib(cfg_doc.AllocAttribInt("count", cfg_.graphics_cfg.sample_count));
				sample_node->AppendAttrib(cfg_doc.AllocAttribInt("quality", cfg_.graphics_cfg.sample_quality));
				frame_node->AppendNode(sample_node);
			}

			graphics_node->AppendNode(frame_node);

			XMLNodePtr sync_interval_node = cfg_doc.AllocNode(XNT_Element, "sync_interval");
			sync_interval_node->AppendAttrib(cfg_doc.AllocAttribInt("value", cfg_.graphics_cfg.sync_interval));
			graphics_node->AppendNode(sync_interval_node);

			XMLNodePtr motion_blur_node = cfg_doc.AllocNode(XNT_Element, "motion_blur");
			motion_blur_node->AppendAttrib(cfg_doc.AllocAttribInt("frames", cfg_.graphics_cfg.motion_frames));
			graphics_node->AppendNode(motion_blur_node);

			XMLNodePtr hdr_node = cfg_doc.AllocNode(XNT_Element, "hdr");
			hdr_node->AppendAttrib(cfg_doc.AllocAttribInt("value", cfg_.graphics_cfg.hdr));
			graphics_node->AppendNode(hdr_node);

			XMLNodePtr ppaa_node = cfg_doc.AllocNode(XNT_Element, "ppaa");
			ppaa_node->AppendAttrib(cfg_doc.AllocAttribInt("value", cfg_.graphics_cfg.ppaa));
			graphics_node->AppendNode(ppaa_node);

			XMLNodePtr gamma_node = cfg_doc.AllocNode(XNT_Element, "gamma");
			gamma_node->AppendAttrib(cfg_doc.AllocAttribInt("value", cfg_.graphics_cfg.gamma));
			graphics_node->AppendNode(gamma_node);

			XMLNodePtr color_grading_node = cfg_doc.AllocNode(XNT_Element, "color_grading");
			color_grading_node->AppendAttrib(cfg_doc.AllocAttribInt("value", cfg_.graphics_cfg.color_grading));
			graphics_node->AppendNode(color_grading_node);

			XMLNodePtr stereo_node = cfg_doc.AllocNode(XNT_Element, "stereo");
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

			graphics_node->AppendNode(stereo_node);
		}
		root->AppendNode(graphics_node);

		std::ofstream ofs(cfg_file.c_str());
		cfg_doc.Print(ofs);
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

#if !(defined(KLAYGE_PLATFORM_ANDROID) || defined(KLAYGE_PLATFORM_IOS))
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
			LogError("Loading %s failed", path.c_str());
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
			LogError("Loading %s failed", path.c_str());
			audio_loader_.Free();
		}
	}

	void Context::LoadInputFactory(std::string const & if_name)
	{
		input_factory_.reset();

#if !(defined(KLAYGE_PLATFORM_ANDROID) || defined(KLAYGE_PLATFORM_IOS))
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
			LogError("Loading %s failed", path.c_str());
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
			LogError("Loading %s failed", path.c_str());
			show_loader_.Free();
		}
	}

	void Context::LoadScriptFactory(std::string const & sf_name)
	{
		script_factory_.reset();
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
			LogError("Loading %s failed", path.c_str());
			script_loader_.Free();
		}
	}

	void Context::LoadSceneManager(std::string const & sm_name)
	{
		scene_mgr_.reset();

#if !(defined(KLAYGE_PLATFORM_ANDROID) || defined(KLAYGE_PLATFORM_IOS))
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
			LogError("Loading %s failed", path.c_str());
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
			LogError("Loading %s failed", path.c_str());
			ads_loader_.Free();
		}
	}

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
}
