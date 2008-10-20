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
	typedef std::string const & (*NameFunc)();
	typedef void (*RenderFactoryInstanceFunc)(RenderFactoryPtr& ptr);
	typedef void (*AudioFactoryInstanceFunc)(AudioFactoryPtr& ptr);
	typedef void (*InputFactoryInstanceFunc)(InputFactoryPtr& ptr);
	typedef void (*ShowFactoryInstanceFunc)(ShowFactoryPtr& ptr);
	typedef void (*SceneManagerFactoryInstanceFunc)(SceneManagerPtr& ptr, boost::program_options::variables_map const & vm);

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
		int color_fmt = EF_ARGB8;
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
			("screen.color_fmt", boost::program_options::value<int>(&color_fmt)->default_value(13), "Screen Color Format")
			("screen.fullscreen", boost::program_options::value<bool>(&full_screen)->default_value(false), "Full Screen");

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
#ifdef KLAYGE_COMPILER_MSVC
				// DShow plugin can only be compiled by vc for now
				sf_name = vm["context.show_factory"].as<std::string>();
#endif
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


		{
			std::string render_path = ResLoader::Instance().Locate("Render");
			for (boost::filesystem::directory_iterator iter(render_path); iter != boost::filesystem::directory_iterator(); ++ iter)
			{
				std::string fn = render_path + "/" + iter->path().filename();
				if (".dll" == fn.substr(fn.length() - 4))
				{
					render_loader_.Load(fn);

					NameFunc name_func = (NameFunc)render_loader_.GetProcAddress("Name");
					if ((name_func != NULL) && (rf_name == name_func()))
					{
						RenderFactoryInstanceFunc rfi = (RenderFactoryInstanceFunc)render_loader_.GetProcAddress("RenderFactoryInstance");
						if (rfi != NULL)
						{
							rfi(renderFactory_);
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

					NameFunc name_func = (NameFunc)audio_loader_.GetProcAddress("Name");
					if ((name_func != NULL) && (af_name == name_func()))
					{
						AudioFactoryInstanceFunc afi = (AudioFactoryInstanceFunc)audio_loader_.GetProcAddress("AudioFactoryInstance");
						if (afi != NULL)
						{
							afi(audioFactory_);
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

					NameFunc name_func = (NameFunc)input_loader_.GetProcAddress("Name");
					if ((name_func != NULL) && (if_name == name_func()))
					{
						InputFactoryInstanceFunc ifi = (InputFactoryInstanceFunc)input_loader_.GetProcAddress("InputFactoryInstance");
						if (ifi != NULL)
						{
							ifi(inputFactory_);
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

					NameFunc name_func = (NameFunc)show_loader_.GetProcAddress("Name");
					if ((name_func != NULL) && (sf_name == name_func()))
					{
						ShowFactoryInstanceFunc sfi = (ShowFactoryInstanceFunc)show_loader_.GetProcAddress("ShowFactoryInstance");
						if (sfi != NULL)
						{
							sfi(showFactory_);
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

					NameFunc name_func = (NameFunc)sm_loader_.GetProcAddress("Name");
					if ((name_func != NULL) && (sm_name == name_func()))
					{
						SceneManagerFactoryInstanceFunc smi = (SceneManagerFactoryInstanceFunc)sm_loader_.GetProcAddress("SceneManagerFactoryInstance");
						if (smi != NULL)
						{
							smi(sceneMgr_, vm);
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
		settings.color_fmt = static_cast<ElementFormat>(color_fmt);
		settings.full_screen = full_screen;

		return settings;
	}
}
