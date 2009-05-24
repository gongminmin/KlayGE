// Context.cpp
// KlayGE 引擎场景类 实现文件
// Ver 3.9.0
// 版权所有(C) 龚敏敏, 2007-2009
// Homepage: http://klayge.sourceforge.net
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
#include <KlayGE/RenderSettings.hpp>
#include <KlayGE/XMLDom.hpp>

#include <fstream>
#include <boost/filesystem.hpp>

#include <KlayGE/Context.hpp>

namespace KlayGE
{
	typedef void (*MakeRenderFactoryFunc)(RenderFactoryPtr& ptr, XMLNodePtr const & extra_param);
	typedef void (*MakeAudioFactoryFunc)(AudioFactoryPtr& ptr, XMLNodePtr const & extra_param);
	typedef void (*MakeInputFactoryFunc)(InputFactoryPtr& ptr, XMLNodePtr const & extra_param);
	typedef void (*MakeShowFactoryFunc)(ShowFactoryPtr& ptr, XMLNodePtr const & extra_param);
	typedef void (*MakeSceneManagerFunc)(SceneManagerPtr& ptr, XMLNodePtr const & extra_param);

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

	RenderSettings Context::LoadCfg(std::string const & cfg_file)
	{
		int width = 800;
		int height = 600;
		ElementFormat color_fmt = EF_ARGB8;
		ElementFormat depth_stencil_fmt = EF_D16;
		int sample_count = 1;
		int sample_quality = 0;
		bool full_screen = false;

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

		XMLNodePtr rf_node;
		XMLNodePtr af_node;
		XMLNodePtr if_node;
		XMLNodePtr sf_node;
		XMLNodePtr sm_node;

		ResIdentifierPtr file = ResLoader::Instance().Load(cfg_file);
		if (file)
		{
			XMLDocument cfg_doc;
			XMLNodePtr cfg_root = cfg_doc.Parse(file);

			XMLNodePtr context_node = cfg_root->FirstNode("context");
			XMLNodePtr screen_node = cfg_root->FirstNode("screen");

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

			XMLNodePtr frame_node = screen_node->FirstNode("frame");
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
		}

		this->LoadRenderFactory(rf_name, rf_node);
		this->LoadAudioFactory(af_name, af_node);
		this->LoadInputFactory(if_name, if_node);
		this->LoadShowFactory(sf_name, sf_node);
		this->LoadSceneManager(sm_name, sm_node);

		RenderSettings settings;
		settings.width = width;
		settings.height = height;
		settings.color_fmt = color_fmt;
		settings.depth_stencil_fmt = depth_stencil_fmt;
		settings.sample_count = sample_count;
		settings.sample_quality = sample_quality;
		settings.full_screen = full_screen;

		return settings;
	}

	void Context::LoadRenderFactory(std::string const & rf_name, XMLNodePtr const & rf_node)
	{
		renderFactory_ = RenderFactory::NullObject();
		render_loader_.Free();

		std::string render_path = ResLoader::Instance().Locate("Render");
		for (boost::filesystem::directory_iterator iter(render_path); iter != boost::filesystem::directory_iterator(); ++ iter)
		{
			std::string fn = iter->path().filename();
			std::string suffix = rf_name + "_" + dll_suffix_;
			std::string::size_type n = fn.rfind(suffix);
			if ((n != std::string::npos) && (n + suffix.length() == fn.length()))
			{
				render_loader_.Load(render_path + "/" + fn);

				MakeRenderFactoryFunc mrf = (MakeRenderFactoryFunc)render_loader_.GetProcAddress("MakeRenderFactory");
				if (mrf != NULL)
				{
					mrf(renderFactory_, rf_node);
					break;
				}
				else
				{
					render_loader_.Free();
				}
			}
		}
	}

	void Context::LoadAudioFactory(std::string const & af_name, XMLNodePtr const & af_node)
	{
		audioFactory_ = AudioFactory::NullObject();
		audio_loader_.Free();

		std::string audio_path = ResLoader::Instance().Locate("Audio");
		for (boost::filesystem::directory_iterator iter(audio_path); iter != boost::filesystem::directory_iterator(); ++ iter)
		{
			std::string fn = iter->path().filename();
			std::string suffix = af_name + "_" + dll_suffix_;
			std::string::size_type n = fn.rfind(suffix);
			if ((n != std::string::npos) && (n + suffix.length() == fn.length()))
			{
				audio_loader_.Load(audio_path + "/" + fn);

				MakeAudioFactoryFunc maf = (MakeAudioFactoryFunc)audio_loader_.GetProcAddress("MakeAudioFactory");
				if (maf != NULL)
				{
					maf(audioFactory_, af_node);
					break;
				}
				else
				{
					audio_loader_.Free();
				}
			}
		}
	}

	void Context::LoadInputFactory(std::string const & if_name, XMLNodePtr const & if_node)
	{
		inputFactory_ = InputFactory::NullObject();
		input_loader_.Free();

		std::string input_path = ResLoader::Instance().Locate("Input");
		for (boost::filesystem::directory_iterator iter(input_path); iter != boost::filesystem::directory_iterator(); ++ iter)
		{
			std::string fn = iter->path().filename();
			std::string suffix = if_name + "_" + dll_suffix_;
			std::string::size_type n = fn.rfind(suffix);
			if ((n != std::string::npos) && (n + suffix.length() == fn.length()))
			{
				input_loader_.Load(input_path + "/" + fn);

				MakeInputFactoryFunc mif = (MakeInputFactoryFunc)input_loader_.GetProcAddress("MakeInputFactory");
				if (mif != NULL)
				{
					mif(inputFactory_, if_node);
					break;
				}
				else
				{
					input_loader_.Free();
				}
			}
		}
	}

	void Context::LoadShowFactory(std::string const & sf_name, XMLNodePtr const & sf_node)
	{
		showFactory_ = ShowFactory::NullObject();
		show_loader_.Free();

		std::string show_path = ResLoader::Instance().Locate("Show");
		for (boost::filesystem::directory_iterator iter(show_path); iter != boost::filesystem::directory_iterator(); ++ iter)
		{
			std::string fn = iter->path().filename();
			std::string suffix = sf_name + "_" + dll_suffix_;
			std::string::size_type n = fn.rfind(suffix);
			if ((n != std::string::npos) && (n + suffix.length() == fn.length()))
			{
				show_loader_.Load(show_path + "/" + fn);

				MakeShowFactoryFunc msf = (MakeShowFactoryFunc)show_loader_.GetProcAddress("MakeShowFactory");
				if (msf != NULL)
				{
					msf(showFactory_, sf_node);
					break;
				}
				else
				{
					show_loader_.Free();
				}
			}
		}
	}

	void Context::LoadSceneManager(std::string const & sm_name, XMLNodePtr const & sm_node)
	{
		sceneMgr_ = SceneManager::NullObject();
		sm_loader_.Free();

		std::string sm_path = ResLoader::Instance().Locate("Scene");
		for (boost::filesystem::directory_iterator iter(sm_path); iter != boost::filesystem::directory_iterator(); ++ iter)
		{
			std::string fn = iter->path().filename();
			std::string suffix = sm_name + "_" + dll_suffix_;
			std::string::size_type n = fn.rfind(suffix);
			if ((n != std::string::npos) && (n + suffix.length() == fn.length()))
			{
				sm_loader_.Load(sm_path + "/" + fn);

				MakeSceneManagerFunc msm = (MakeSceneManagerFunc)sm_loader_.GetProcAddress("MakeSceneManager");
				if (msm != NULL)
				{
					msm(sceneMgr_, sm_node);
					break;
				}
				else
				{
					sm_loader_.Free();
				}
			}
		}
	}
}
