// Context.cpp
// KlayGE 引擎场景类 实现文件
// Ver 3.9.0
// 版权所有(C) 龚敏敏, 2007-2009
// Homepage: http://www.klayge.org
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

	Context::Context()
	{
#ifdef KLAYGE_COMPILER_MSVC
#ifdef KLAYGE_DEBUG
		_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif
#endif

		sceneMgr_ = SceneManager::NullObject();

		renderFactory_ = RenderFactory::NullObject();
		audioFactory_ = AudioFactory::NullObject();
		inputFactory_ = InputFactory::NullObject();
		showFactory_ = ShowFactory::NullObject();

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
		sceneMgr_.reset();

		renderFactory_.reset();
		audioFactory_.reset();
		inputFactory_.reset();
		showFactory_.reset();
	}

	Context& Context::Instance()
	{
		static Context context;
		return context;
	}

	ContextCfg Context::LoadCfg(std::string const & cfg_file)
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
		bool stereo_mode = 0;
		float stereo_separation = 0;

#ifdef KLAYGE_PLATFORM_WINDOWS
		std::string rf_name = "D3D9";
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

		XMLDocument cfg_doc;
		XMLNodePtr rf_node;
		XMLNodePtr af_node;
		XMLNodePtr if_node;
		XMLNodePtr sf_node;
		XMLNodePtr sm_node;

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

			af_node = context_node->FirstNode("audio_factory");
			if (af_node)
			{
				af_name = af_node->Attrib("name")->ValueString();
			}

			if_node = context_node->FirstNode("input_factory");
			if (if_node)
			{
				if_name = if_node->Attrib("name")->ValueString();
			}

			sf_node = context_node->FirstNode("show_factory");
			if (sf_node)
			{
				sf_name = sf_node->Attrib("name")->ValueString();
			}

			sm_node = context_node->FirstNode("scene_manager");
			if (sm_node)
			{
				sm_name = sm_node->Attrib("name")->ValueString();
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
			else if ("ABGR8" == color_fmt_str)
			{
				color_fmt = EF_ABGR8;
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

			XMLNodePtr stereo_node = graphics_node->FirstNode("stereo");
			attr = stereo_node->Attrib("enabled");
			if (attr)
			{
				std::string stereo_str = attr->ValueString();
				if (("1" == stereo_str) || ("true" == stereo_str))
				{
					stereo_mode = true;
				}
				else
				{
					stereo_mode = false;
				}
			}
			attr = stereo_node->Attrib("separation");
			if (attr)
			{
				stereo_separation = attr->ValueFloat();
			}
		}

		ContextCfg cfg;

		cfg.render_factory_name = rf_name;
		cfg.audio_factory_name = af_name;
		cfg.input_factory_name = if_name;
		cfg.show_factory_name = sf_name;
		cfg.scene_manager_name = sm_name;

		cfg.graphics_cfg.width = width;
		cfg.graphics_cfg.height = height;
		cfg.graphics_cfg.color_fmt = color_fmt;
		cfg.graphics_cfg.depth_stencil_fmt = depth_stencil_fmt;
		cfg.graphics_cfg.sample_count = sample_count;
		cfg.graphics_cfg.sample_quality = sample_quality;
		cfg.graphics_cfg.full_screen = full_screen;
		cfg.graphics_cfg.sync_interval = sync_interval;
		cfg.graphics_cfg.motion_frames = motion_frames;
		cfg.graphics_cfg.stereo_mode = stereo_mode;
		cfg.graphics_cfg.stereo_separation = stereo_separation;

		return cfg;
	}

	void Context::Config(ContextCfg const & cfg)
	{
		cfg_ = cfg;

		this->LoadRenderFactory(cfg.render_factory_name);
		this->LoadAudioFactory(cfg.audio_factory_name);
		this->LoadInputFactory(cfg.input_factory_name);
		this->LoadShowFactory(cfg.show_factory_name);
		this->LoadSceneManager(cfg.scene_manager_name);
	}

	ContextCfg const & Context::Config() const
	{
		return cfg_;
	}

	void Context::LoadRenderFactory(std::string const & rf_name)
	{
		renderFactory_ = RenderFactory::NullObject();
		render_loader_.Free();

		std::string render_path = ResLoader::Instance().Locate("Render");
		std::string fn = KLAYGE_STRINGIZE(KLAYGE_NAME) + std::string("_RenderEngine_") + rf_name + "_" + dll_suffix_;
		render_loader_.Load(render_path + "/" + fn);

		MakeRenderFactoryFunc mrf = (MakeRenderFactoryFunc)render_loader_.GetProcAddress("MakeRenderFactory");
		if (mrf != NULL)
		{
			mrf(renderFactory_);
		}
		else
		{
			render_loader_.Free();
		}
	}

	void Context::LoadAudioFactory(std::string const & af_name)
	{
		audioFactory_ = AudioFactory::NullObject();
		audio_loader_.Free();

		std::string audio_path = ResLoader::Instance().Locate("Audio");
		std::string fn = KLAYGE_STRINGIZE(KLAYGE_NAME) + std::string("_AudioEngine_") + af_name + "_" + dll_suffix_;
		audio_loader_.Load(audio_path + "/" + fn);

		MakeAudioFactoryFunc maf = (MakeAudioFactoryFunc)audio_loader_.GetProcAddress("MakeAudioFactory");
		if (maf != NULL)
		{
			maf(audioFactory_);
		}
		else
		{
			audio_loader_.Free();
		}
	}

	void Context::LoadInputFactory(std::string const & if_name)
	{
		inputFactory_ = InputFactory::NullObject();
		input_loader_.Free();

		std::string input_path = ResLoader::Instance().Locate("Input");
		std::string fn = KLAYGE_STRINGIZE(KLAYGE_NAME) + std::string("_InputEngine_") + if_name + "_" + dll_suffix_;
		input_loader_.Load(input_path + "/" + fn);

		MakeInputFactoryFunc mif = (MakeInputFactoryFunc)input_loader_.GetProcAddress("MakeInputFactory");
		if (mif != NULL)
		{
			mif(inputFactory_);
		}
		else
		{
			input_loader_.Free();
		}
	}

	void Context::LoadShowFactory(std::string const & sf_name)
	{
		showFactory_ = ShowFactory::NullObject();
		show_loader_.Free();

		std::string show_path = ResLoader::Instance().Locate("Show");
		std::string fn = KLAYGE_STRINGIZE(KLAYGE_NAME) + std::string("_ShowEngine_") + sf_name + "_" + dll_suffix_;
		show_loader_.Load(show_path + "/" + fn);

		MakeShowFactoryFunc msf = (MakeShowFactoryFunc)show_loader_.GetProcAddress("MakeShowFactory");
		if (msf != NULL)
		{
			msf(showFactory_);
		}
		else
		{
			show_loader_.Free();
		}
	}

	void Context::LoadSceneManager(std::string const & sm_name)
	{
		sceneMgr_ = SceneManager::NullObject();
		sm_loader_.Free();

		std::string sm_path = ResLoader::Instance().Locate("Scene");
		std::string fn = KLAYGE_STRINGIZE(KLAYGE_NAME) + std::string("_Scene_") + sm_name + "_" + dll_suffix_;
		sm_loader_.Load(sm_path + "/" + fn);

		MakeSceneManagerFunc msm = (MakeSceneManagerFunc)sm_loader_.GetProcAddress("MakeSceneManager");
		if (msm != NULL)
		{
			msm(sceneMgr_);
		}
		else
		{
			sm_loader_.Free();
		}
	}
}
