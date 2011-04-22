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
#include <KlayGE/Util.hpp>
#include <KlayGE/Math.hpp>
#include <KlayGE/SceneManager.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/AudioFactory.hpp>
#include <KlayGE/InputFactory.hpp>
#include <KlayGE/ShowFactory.hpp>
#include <KlayGE/AudioDataSource.hpp>
#include <KlayGE/ResLoader.hpp>
#include <KlayGE/XMLDom.hpp>

#include <fstream>

#include <KlayGE/Context.hpp>

namespace KlayGE
{
	typedef void (*MakeRenderFactoryFunc)(RenderFactoryPtr& ptr);
	typedef void (*MakeAudioFactoryFunc)(AudioFactoryPtr& ptr);
	typedef void (*MakeInputFactoryFunc)(InputFactoryPtr& ptr);
	typedef void (*MakeShowFactoryFunc)(ShowFactoryPtr& ptr);
	typedef void (*MakeSceneManagerFunc)(SceneManagerPtr& ptr);
	typedef void (*MakeAudioDataSourceFactoryFunc)(AudioDataSourceFactoryPtr& ptr);

	Context::Context()
	{
#ifdef KLAYGE_COMPILER_MSVC
#ifdef KLAYGE_DEBUG
		_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif
#endif

		scene_mgr_ = SceneManager::NullObject();

		render_factory_ = RenderFactory::NullObject();
		audio_factory_ = AudioFactory::NullObject();
		input_factory_ = InputFactory::NullObject();
		show_factory_ = ShowFactory::NullObject();
		audio_data_src_factory_ = AudioDataSourceFactory::NullObject();

		dll_suffix_ = KLAYGE_STRINGIZE(KLAYGE_COMPILER_NAME);
		dll_suffix_ += "_";
		dll_suffix_ += KLAYGE_STRINGIZE(KLAYGE_COMPILER_TARGET);
#ifdef KLAYGE_DEBUG
		dll_suffix_ += "_d";
#endif
#ifdef KLAYGE_PLATFORM_WINDOWS
		dll_suffix_ += ".dll";
#else
		dll_suffix_ += ".so";
#endif
	}

	Context::~Context()
	{
		scene_mgr_.reset();

		render_factory_.reset();
		audio_factory_.reset();
		input_factory_.reset();
		show_factory_.reset();
		audio_data_src_factory_.reset();
	}

	Context& Context::Instance()
	{
		static Context context;
		return context;
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
		int stereo_method = 0;
		float stereo_separation = 0;

#ifdef KLAYGE_PLATFORM_WINDOWS
		std::string rf_name = "D3D11";
#else
		std::string rf_name = "OpenGL";
#endif
		std::string af_name = "OpenAL";
#ifdef KLAYGE_PLATFORM_WINDOWS
		std::string if_name = "DInput";
#else
		std::string if_name;
#endif
#ifdef KLAYGE_PLATFORM_WINDOWS
		std::string sf_name = "DShow";
#else
		std::string sf_name;
#endif
		std::string sm_name;
		std::string adsf_name;

		XMLDocument cfg_doc;
		XMLNodePtr rf_node;
		XMLNodePtr af_node;
		XMLNodePtr if_node;
		XMLNodePtr sf_node;
		XMLNodePtr sm_node;
		XMLNodePtr adsf_node;

		ResIdentifierPtr file = ResLoader::Instance().Load(cfg_file);
		if (file)
		{
			XMLNodePtr cfg_root = cfg_doc.Parse(file);

			XMLNodePtr context_node = cfg_root->FirstNode("context");
			XMLNodePtr graphics_node = cfg_root->FirstNode("graphics");

#ifdef KLAYGE_PLATFORM_WINDOWS
			rf_node = context_node->FirstNode("render_factory");
			if (rf_node)
			{
				rf_name = rf_node->Attrib("name")->ValueString();
			}
#endif

#ifdef KLAYGE_PLATFORM_WINDOWS
			af_node = context_node->FirstNode("audio_factory");
			if (af_node)
			{
				af_name = af_node->Attrib("name")->ValueString();
			}
#endif

#ifdef KLAYGE_PLATFORM_WINDOWS
			if_node = context_node->FirstNode("input_factory");
			if (if_node)
			{
				if_name = if_node->Attrib("name")->ValueString();
			}
#endif

#ifdef KLAYGE_PLATFORM_WINDOWS
			sf_node = context_node->FirstNode("show_factory");
			if (sf_node)
			{
				sf_name = sf_node->Attrib("name")->ValueString();
			}
#endif

			sm_node = context_node->FirstNode("scene_manager");
			if (sm_node)
			{
				sm_name = sm_node->Attrib("name")->ValueString();
			}

			adsf_node = context_node->FirstNode("audio_data_source_factory");
			if (adsf_node)
			{
				adsf_name = adsf_node->Attrib("name")->ValueString();
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
				std::string fs_str = attr->ValueString();
				if (("1" == fs_str) || ("true" == fs_str))
				{
					full_screen = true;
				}
				else
				{
					full_screen = false;
				}
			}

			if ("ARGB8" == color_fmt_str)
			{
				color_fmt = EF_ARGB8;
			}
			else if ("ARGB8_SRGB" == color_fmt_str)
			{
				color_fmt = EF_ARGB8_SRGB;
			}
			else if ("ABGR8" == color_fmt_str)
			{
				color_fmt = EF_ABGR8;
			}
			else if ("ABGR8_SRGB" == color_fmt_str)
			{
				color_fmt = EF_ABGR8_SRGB;
			}
			else if ("A2BGR10" == color_fmt_str)
			{
				color_fmt = EF_A2BGR10;
			}

			if ("D16" == depth_stencil_fmt_str)
			{
				depth_stencil_fmt = EF_D16;
			}
			else if ("D24S8" == depth_stencil_fmt_str)
			{
				depth_stencil_fmt = EF_D24S8;
			}
			else if ("D32F" == depth_stencil_fmt_str)
			{
				depth_stencil_fmt = EF_D32F;
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
				std::string fs_str = attr->ValueString();
				if (("1" == fs_str) || ("true" == fs_str))
				{
					hdr = true;
				}
				else
				{
					hdr = false;
				}
			}

			XMLNodePtr stereo_node = graphics_node->FirstNode("stereo");
			attr = stereo_node->Attrib("method");
			if (attr)
			{
				std::string method_str = attr->ValueString();
				if ("none" == method_str)
				{
					stereo_method = STM_None;
				}
				else if ("red_cyan" == method_str)
				{
					stereo_method = STM_ColorAnaglyph_RedCyan;
				}
				else if ("yellow_blue" == method_str)
				{
					stereo_method = STM_ColorAnaglyph_YellowBlue;
				}
				else if ("green_red" == method_str)
				{
					stereo_method = STM_ColorAnaglyph_GreenRed;
				}
				else if ("lcd_shutter" == method_str)
				{
					stereo_method = STM_LCDShutter;
				}
				else if ("hor_interlacing" == method_str)
				{
					stereo_method = STM_HorizontalInterlacing;
				}
				else if ("ver_interlacing" == method_str)
				{
					stereo_method = STM_VerticalInterlacing;
				}
				else if ("horizontal" == method_str)
				{
					stereo_method = STM_Horizontal;
				}
				else if ("vertical" == method_str)
				{
					stereo_method = STM_Vertical;
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
		}

		cfg_.render_factory_name = rf_name;
		cfg_.audio_factory_name = af_name;
		cfg_.input_factory_name = if_name;
		cfg_.show_factory_name = sf_name;
		cfg_.scene_manager_name = sm_name;
		cfg_.audio_data_source_factory_name = adsf_name;

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
		cfg_.graphics_cfg.stereo_method = static_cast<StereoMethod>(stereo_method);
		cfg_.graphics_cfg.stereo_separation = stereo_separation;
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

			XMLNodePtr adsf_node = cfg_doc.AllocNode(XNT_Element, "audio_data_source_factory");
			adsf_node->AppendAttrib(cfg_doc.AllocAttribString("name", cfg_.audio_data_source_factory_name));
			context_node->AppendNode(adsf_node);
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
				depth_stencil_fmt_str = "D16";
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
			}
			stereo_node->AppendAttrib(cfg_doc.AllocAttribString("method", method_str));
			stereo_node->AppendAttrib(cfg_doc.AllocAttribFloat("separation", cfg_.graphics_cfg.stereo_separation));

			graphics_node->AppendNode(stereo_node);
		}
		root->AppendNode(graphics_node);

		std::ofstream ofs(cfg_file.c_str());
		cfg_doc.Print(ofs);
	}

	void Context::Config(ContextCfg const & cfg)
	{
		cfg_ = cfg;

		if ((cfg_.render_factory_name != cfg_.render_factory_name) || (RenderFactory::NullObject() == render_factory_))
		{
			this->LoadRenderFactory(cfg_.render_factory_name);
		}
		if ((cfg_.audio_factory_name != cfg_.audio_factory_name) || (AudioFactory::NullObject() == audio_factory_))
		{
			this->LoadAudioFactory(cfg_.audio_factory_name);
		}
		if ((cfg_.input_factory_name != cfg_.input_factory_name) || (InputFactory::NullObject() == input_factory_))
		{
			this->LoadInputFactory(cfg_.input_factory_name);
		}
		if ((cfg_.show_factory_name != cfg_.show_factory_name) || (ShowFactory::NullObject() == show_factory_))
		{
			this->LoadShowFactory(cfg_.show_factory_name);
		}
		if ((cfg_.scene_manager_name != cfg_.scene_manager_name) || (SceneManager::NullObject() == scene_mgr_))
		{
			this->LoadSceneManager(cfg_.scene_manager_name);
		}
		if ((cfg_.audio_data_source_factory_name != cfg_.audio_data_source_factory_name) || (AudioDataSourceFactory::NullObject() == audio_data_src_factory_))
		{
			this->LoadAudioDataSourceFactory(cfg_.audio_data_source_factory_name);
		}
	}

	ContextCfg const & Context::Config() const
	{
		return cfg_;
	}

	void Context::LoadRenderFactory(std::string const & rf_name)
	{
		render_factory_ = RenderFactory::NullObject();
		render_loader_.Free();

		std::string render_path = ResLoader::Instance().Locate("Render");
		std::string fn = KLAYGE_STRINGIZE(KLAYGE_NAME) + std::string("_RenderEngine_") + rf_name + "_" + dll_suffix_;
#ifdef KLAYGE_PLATFORM_LINUX
		fn = "lib" + fn;
#endif
		render_loader_.Load(render_path + "/" + fn);

		MakeRenderFactoryFunc mrf = (MakeRenderFactoryFunc)render_loader_.GetProcAddress("MakeRenderFactory");
		if (mrf != NULL)
		{
			mrf(render_factory_);
		}
		else
		{
			render_loader_.Free();
		}
	}

	void Context::LoadAudioFactory(std::string const & af_name)
	{
		audio_factory_ = AudioFactory::NullObject();
		audio_loader_.Free();

		std::string audio_path = ResLoader::Instance().Locate("Audio");
		std::string fn = KLAYGE_STRINGIZE(KLAYGE_NAME) + std::string("_AudioEngine_") + af_name + "_" + dll_suffix_;
#ifdef KLAYGE_PLATFORM_LINUX
		fn = "lib" + fn;
#endif
		audio_loader_.Load(audio_path + "/" + fn);

		MakeAudioFactoryFunc maf = (MakeAudioFactoryFunc)audio_loader_.GetProcAddress("MakeAudioFactory");
		if (maf != NULL)
		{
			maf(audio_factory_);
		}
		else
		{
			audio_loader_.Free();
		}
	}

	void Context::LoadInputFactory(std::string const & if_name)
	{
		input_factory_ = InputFactory::NullObject();
		input_loader_.Free();

		std::string input_path = ResLoader::Instance().Locate("Input");
		std::string fn = KLAYGE_STRINGIZE(KLAYGE_NAME) + std::string("_InputEngine_") + if_name + "_" + dll_suffix_;
#ifdef KLAYGE_PLATFORM_LINUX
		fn = "lib" + fn;
#endif
		input_loader_.Load(input_path + "/" + fn);

		MakeInputFactoryFunc mif = (MakeInputFactoryFunc)input_loader_.GetProcAddress("MakeInputFactory");
		if (mif != NULL)
		{
			mif(input_factory_);
		}
		else
		{
			input_loader_.Free();
		}
	}

	void Context::LoadShowFactory(std::string const & sf_name)
	{
		show_factory_ = ShowFactory::NullObject();
		show_loader_.Free();

		std::string show_path = ResLoader::Instance().Locate("Show");
		std::string fn = KLAYGE_STRINGIZE(KLAYGE_NAME) + std::string("_ShowEngine_") + sf_name + "_" + dll_suffix_;
#ifdef KLAYGE_PLATFORM_LINUX
		fn = "lib" + fn;
#endif
		show_loader_.Load(show_path + "/" + fn);

		MakeShowFactoryFunc msf = (MakeShowFactoryFunc)show_loader_.GetProcAddress("MakeShowFactory");
		if (msf != NULL)
		{
			msf(show_factory_);
		}
		else
		{
			show_loader_.Free();
		}
	}

	void Context::LoadSceneManager(std::string const & sm_name)
	{
		scene_mgr_ = SceneManager::NullObject();
		sm_loader_.Free();

		std::string sm_path = ResLoader::Instance().Locate("Scene");
		std::string fn = KLAYGE_STRINGIZE(KLAYGE_NAME) + std::string("_Scene_") + sm_name + "_" + dll_suffix_;
#ifdef KLAYGE_PLATFORM_LINUX
		fn = "lib" + fn;
#endif
		sm_loader_.Load(sm_path + "/" + fn);

		MakeSceneManagerFunc msm = (MakeSceneManagerFunc)sm_loader_.GetProcAddress("MakeSceneManager");
		if (msm != NULL)
		{
			msm(scene_mgr_);
		}
		else
		{
			sm_loader_.Free();
		}
	}

	void Context::LoadAudioDataSourceFactory(std::string const & adsf_name)
	{
		audio_data_src_factory_ = AudioDataSourceFactory::NullObject();
		ads_loader_.Free();

		std::string adsf_path = ResLoader::Instance().Locate("Audio");
		std::string fn = KLAYGE_STRINGIZE(KLAYGE_NAME) + std::string("_AudioDataSource_") + adsf_name + "_" + dll_suffix_;
#ifdef KLAYGE_PLATFORM_LINUX
		fn = "lib" + fn;
#endif
		ads_loader_.Load(adsf_path + "/" + fn);

		MakeAudioDataSourceFactoryFunc madsf = (MakeAudioDataSourceFactoryFunc)ads_loader_.GetProcAddress("MakeAudioDataSourceFactory");
		if (madsf != NULL)
		{
			madsf(audio_data_src_factory_);
		}
		else
		{
			ads_loader_.Free();
		}
	}
}
