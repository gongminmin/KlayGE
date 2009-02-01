// Context.cpp
// KlayGE 引擎场景类 实现文件
// Ver 3.8.0
// 版权所有(C) 龚敏敏, 2007-2008
// Homepage: http://klayge.sourceforge.net
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
#ifdef KLAYGE_COMPILER_MSVC
#pragma warning(push)
#pragma warning(disable: 4251 4275 4512 4702)
#endif
#include <boost/program_options.hpp>
#ifdef KLAYGE_COMPILER_MSVC
#pragma warning(pop)
#endif
#include <boost/filesystem.hpp>

#include <KlayGE/Context.hpp>

namespace KlayGE
{
	typedef bool (*MatchFunc)(char const * name, char const * compiler);
	typedef void (*MakeRenderFactoryFunc)(RenderFactoryPtr& ptr, boost::program_options::variables_map const & vm);
	typedef void (*MakeAudioFactoryFunc)(AudioFactoryPtr& ptr, boost::program_options::variables_map const & vm);
	typedef void (*MakeInputFactoryFunc)(InputFactoryPtr& ptr, boost::program_options::variables_map const & vm);
	typedef void (*MakeShowFactoryFunc)(ShowFactoryPtr& ptr, boost::program_options::variables_map const & vm);
	typedef void (*MakeSceneManagerFunc)(SceneManagerPtr& ptr, boost::program_options::variables_map const & vm);

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
		int octree_depth = 3;
		int width = 800;
		int height = 600;
		std::string color_fmt_str = "ARGB8";
		std::string depth_stencil_fmt_str = "D16";
		int sample_count = 1;
		int sample_quality = 0;
		bool full_screen = false;

		boost::program_options::options_description desc("Configuration");
		desc.add_options()
			("context.render_factory", boost::program_options::value<std::string>(), "Render Factory")
			("context.audio_factory", boost::program_options::value<std::string>(), "Audio Factory")
			("context.input_factory", boost::program_options::value<std::string>(), "Input Factory")
			("context.show_factory", boost::program_options::value<std::string>(), "Show Factory")
			("context.scene_manager", boost::program_options::value<std::string>(), "Scene Manager")
			("octree.depth", boost::program_options::value<int>(&octree_depth)->default_value(3), "Octree depth")
			("screen.width", boost::program_options::value<int>(&width)->default_value(800), "Screen Width")
			("screen.height", boost::program_options::value<int>(&height)->default_value(600), "Screen Height")
			("screen.color_fmt", boost::program_options::value<std::string>(&color_fmt_str)->default_value("ARGB8"), "Screen Color Format")
			("screen.depth_stencil_fmt", boost::program_options::value<std::string>(&depth_stencil_fmt_str)->default_value("D16"), "Screen Depth Stencil Format")
			("screen.sample_count", boost::program_options::value<int>(&sample_count)->default_value(0), "Screen Sample Count")
			("screen.sample_quality", boost::program_options::value<int>(&sample_quality)->default_value(0), "Screen Sample Quality")
			("screen.fullscreen", boost::program_options::value<bool>(&full_screen)->default_value(false), "Full Screen");

		ElementFormat color_fmt = EF_ARGB8;
		if ("ARGB8" == color_fmt_str)
		{
			color_fmt = EF_ARGB8;
		}
		if ("ABGR8" == color_fmt_str)
		{
			color_fmt = EF_ABGR8;
		}
		if ("A2BGR10" == color_fmt_str)
		{
			color_fmt = EF_A2BGR10;
		}

		ElementFormat depth_stencil_fmt = EF_D16;
		if ("D16" == depth_stencil_fmt_str)
		{
			depth_stencil_fmt = EF_D16;
		}
		if ("D24S8" == depth_stencil_fmt_str)
		{
			depth_stencil_fmt = EF_D24S8;
		}
		if ("D32F" == depth_stencil_fmt_str)
		{
			depth_stencil_fmt = EF_D32F;
		}

		std::string rf_name;
		std::string af_name;
		std::string if_name;
		std::string sf_name;
		std::string sm_name;

		boost::program_options::variables_map vm;

		std::ifstream cfg_fs(ResLoader::Instance().Locate(cfg_file).c_str());
		if (cfg_fs)
		{
			boost::program_options::store(boost::program_options::parse_config_file(cfg_fs, desc), vm);
			boost::program_options::notify(vm);

			if (vm.count("context.render_factory"))
			{
				rf_name = vm["context.render_factory"].as<std::string>();
			}
			else
			{
#ifdef KLAYGE_PLATFORM_WINDOWS
				rf_name = "D3D9";
#else
				rf_name = "OpenGL";
#endif
			}

			if (vm.count("context.audio_factory"))
			{
				af_name = vm["context.audio_factory"].as<std::string>();
			}
			else
			{
				af_name = "OpenAL";
			}

			if (vm.count("context.input_factory"))
			{
				if_name = vm["context.input_factory"].as<std::string>();
			}
			else
			{
#ifdef KLAYGE_PLATFORM_WINDOWS
				if_name = "DInput";
#endif
			}

			if (vm.count("context.show_factory"))
			{
				sf_name = vm["context.show_factory"].as<std::string>();
			}
			else
			{
#ifdef KLAYGE_PLATFORM_WINDOWS
				sf_name = "DShow";
#endif
			}

			if (vm.count("context.scene_manager"))
			{
				sm_name = vm["context.scene_manager"].as<std::string>();
			}
		}

		std::string compiler_str = KLAYGE_COMPILER_TOOLSET;
#ifdef KLAYGE_DEBUG
		compiler_str += "_d";
#endif


		{
			std::string render_path = ResLoader::Instance().Locate("Render");
			for (boost::filesystem::directory_iterator iter(render_path); iter != boost::filesystem::directory_iterator(); ++ iter)
			{
				std::string fn = render_path + "/" + iter->path().filename();
				if (".dll" == fn.substr(fn.length() - 4))
				{
					render_loader_.Load(fn);

					MatchFunc match_func = (MatchFunc)render_loader_.GetProcAddress("Match");
					if ((match_func != NULL) && match_func(rf_name.c_str(), compiler_str.c_str()))
					{
						MakeRenderFactoryFunc mrf = (MakeRenderFactoryFunc)render_loader_.GetProcAddress("MakeRenderFactory");
						if (mrf != NULL)
						{
							mrf(renderFactory_, vm);
							break;
						}
						else
						{
							render_loader_.Free();
						}
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
				std::string fn = audio_path + "/" + iter->path().filename();
				if (".dll" == fn.substr(fn.length() - 4))
				{
					audio_loader_.Load(fn);

					MatchFunc match_func = (MatchFunc)audio_loader_.GetProcAddress("Match");
					if ((match_func != NULL) && match_func(af_name.c_str(), compiler_str.c_str()))
					{
						MakeAudioFactoryFunc maf = (MakeAudioFactoryFunc)audio_loader_.GetProcAddress("MakeAudioFactory");
						if (maf != NULL)
						{
							maf(audioFactory_, vm);
							break;
						}
						else
						{
							audio_loader_.Free();
						}
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
				std::string fn = input_path + "/" + iter->path().filename();
				if (".dll" == fn.substr(fn.length() - 4))
				{
					input_loader_.Load(fn);

					MatchFunc match_func = (MatchFunc)input_loader_.GetProcAddress("Match");
					if ((match_func != NULL) && match_func(if_name.c_str(), compiler_str.c_str()))
					{
						MakeInputFactoryFunc mif = (MakeInputFactoryFunc)input_loader_.GetProcAddress("MakeInputFactory");
						if (mif != NULL)
						{
							mif(inputFactory_, vm);
							break;
						}
						else
						{
							input_loader_.Free();
						}
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
				std::string fn = show_path + "/" + iter->path().filename();
				if (".dll" == fn.substr(fn.length() - 4))
				{
					show_loader_.Load(fn);

					MatchFunc match_func = (MatchFunc)show_loader_.GetProcAddress("Match");
					if ((match_func != NULL) && match_func(sf_name.c_str(), compiler_str.c_str()))
					{
						MakeShowFactoryFunc msf = (MakeShowFactoryFunc)show_loader_.GetProcAddress("MakeShowFactory");
						if (msf != NULL)
						{
							msf(showFactory_, vm);
							break;
						}
						else
						{
							show_loader_.Free();
						}
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
				std::string fn = sm_path + "/" + iter->path().filename();
				if (".dll" == fn.substr(fn.length() - 4))
				{
					sm_loader_.Load(fn);

					MatchFunc match_func = (MatchFunc)sm_loader_.GetProcAddress("Match");
					if ((match_func != NULL) && match_func(sm_name.c_str(), compiler_str.c_str()))
					{
						MakeSceneManagerFunc msm = (MakeSceneManagerFunc)sm_loader_.GetProcAddress("MakeSceneManager");
						if (msm != NULL)
						{
							msm(sceneMgr_, vm);
							break;
						}
						else
						{
							sm_loader_.Free();
						}
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
