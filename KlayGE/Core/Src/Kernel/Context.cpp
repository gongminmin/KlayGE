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

#include <fstream>
#include <boost/filesystem.hpp>
#ifdef KLAYGE_COMPILER_MSVC
#pragma warning(push)
#pragma warning(disable: 4702)
#endif
#include <boost/lexical_cast.hpp>
#ifdef KLAYGE_COMPILER_MSVC
#pragma warning(pop)
#endif

#include <rapidxml/rapidxml.hpp>

#include <KlayGE/Context.hpp>

namespace KlayGE
{
	typedef void (*MakeRenderFactoryFunc)(RenderFactoryPtr& ptr, void* extra_param);
	typedef void (*MakeAudioFactoryFunc)(AudioFactoryPtr& ptr, void* extra_param);
	typedef void (*MakeInputFactoryFunc)(InputFactoryPtr& ptr, void* extra_param);
	typedef void (*MakeShowFactoryFunc)(ShowFactoryPtr& ptr, void* extra_param);
	typedef void (*MakeSceneManagerFunc)(SceneManagerPtr& ptr, void* extra_param);

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
		using boost::lexical_cast;
		using namespace rapidxml;

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

		xml_node<>* rf_node = NULL;
		xml_node<>* af_node = NULL;
		xml_node<>* if_node = NULL;
		xml_node<>* sf_node = NULL;
		xml_node<>* sm_node = NULL;

		std::ifstream file(ResLoader::Instance().Locate(cfg_file).c_str());
		if (file)
		{
			file.seekg(0, std::ios_base::end);
			int len = static_cast<int>(file.tellg());
			file.seekg(0, std::ios_base::beg);
			std::vector<char> str(len + 1, 0);
			file.read(&str[0], len);

			xml_document<> cfg_doc;
			cfg_doc.parse<0>(&str[0]);

			xml_node<>* cfg_root = cfg_doc.first_node("configure");

			xml_node<>* context_node = cfg_root->first_node("context");
			xml_node<>* screen_node = cfg_root->first_node("screen");

#ifdef KLAYGE_PLATFORM_WINDOWS
			xml_node<>* rf_node = context_node->first_node("render_factory");
			if (rf_node != NULL)
			{
				rf_name = rf_node->first_attribute("name")->value();
			}
#endif

			xml_node<>* af_node = context_node->first_node("audio_factory");
			if (af_node != NULL)
			{
				af_name = af_node->first_attribute("name")->value();
			}

			xml_node<>* if_node = context_node->first_node("input_factory");
			if (if_node != NULL)
			{
				if_name = if_node->first_attribute("name")->value();
			}

			xml_node<>* sf_node = context_node->first_node("show_factory");
			if (sf_node != NULL)
			{
				sf_name = sf_node->first_attribute("name")->value();
			}

			xml_node<>* sm_node = context_node->first_node("scene_manager");
			if (sm_node != NULL)
			{
				sm_name = sm_node->first_attribute("name")->value();
			}

			xml_node<>* frame_node = screen_node->first_node("frame");
			xml_attribute<>* attr;
			attr = frame_node->first_attribute("width");
			if (attr != NULL)
			{
				width = lexical_cast<int>(attr->value());
			}
			attr = frame_node->first_attribute("height");
			if (attr != NULL)
			{
				height = lexical_cast<int>(attr->value());
			}
			std::string color_fmt_str = "ARGB8";
			attr = frame_node->first_attribute("color_fmt");
			if (attr != NULL)
			{
				color_fmt_str = attr->value();
			}
			std::string depth_stencil_fmt_str = "D16";
			attr = frame_node->first_attribute("depth_stencil_fmt");
			if (attr != NULL)
			{
				depth_stencil_fmt_str = attr->value();
			}
			attr = frame_node->first_attribute("fullscreen");
			if (attr != NULL)
			{
				std::string fs_str = attr->value();
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

			xml_node<>* sample_node = frame_node->first_node("sample");
			attr = sample_node->first_attribute("count");
			if (attr != NULL)
			{
				sample_count = lexical_cast<int>(attr->value());
			}
			attr = sample_node->first_attribute("quality");
			if (attr != NULL)
			{
				sample_quality = lexical_cast<int>(attr->value());
			}
		}

		std::string dll_suffix = KLAYGE_STRINGIZE(KLAYGE_COMPILER_NAME);
		dll_suffix += "_";
		dll_suffix += KLAYGE_STRINGIZE(KLAYGE_COMPILER_TARGET);
#ifdef KLAYGE_DEBUG
		dll_suffix += "_d";
#endif
#ifdef KLAYGE_PLATFORM_WINDOWS
		dll_suffix += ".dll";
#else
		dll_suffix += ".so";
#endif

		{
			std::string render_path = ResLoader::Instance().Locate("Render");
			for (boost::filesystem::directory_iterator iter(render_path); iter != boost::filesystem::directory_iterator(); ++ iter)
			{
				std::string fn = iter->path().filename();
				std::string suffix = rf_name + "_" + dll_suffix;
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

		{
			std::string audio_path = ResLoader::Instance().Locate("Audio");
			for (boost::filesystem::directory_iterator iter(audio_path); iter != boost::filesystem::directory_iterator(); ++ iter)
			{
				std::string fn = iter->path().filename();
				std::string suffix = af_name + "_" + dll_suffix;
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

		{
			std::string input_path = ResLoader::Instance().Locate("Input");
			for (boost::filesystem::directory_iterator iter(input_path); iter != boost::filesystem::directory_iterator(); ++ iter)
			{
				std::string fn = iter->path().filename();
				std::string suffix = if_name + "_" + dll_suffix;
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

		{
			std::string show_path = ResLoader::Instance().Locate("Show");
			for (boost::filesystem::directory_iterator iter(show_path); iter != boost::filesystem::directory_iterator(); ++ iter)
			{
				std::string fn = iter->path().filename();
				std::string suffix = sf_name + "_" + dll_suffix;
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

		{
			std::string sm_path = ResLoader::Instance().Locate("Scene");
			for (boost::filesystem::directory_iterator iter(sm_path); iter != boost::filesystem::directory_iterator(); ++ iter)
			{
				std::string fn = iter->path().filename();
				std::string suffix = sm_name + "_" + dll_suffix;
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
}
