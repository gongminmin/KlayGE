#include <KlayGE/KlayGE.hpp>
#include <KlayGE/Math.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/ResLoader.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/App3D.hpp>
#include <KlayGE/RenderSettings.hpp>
#include <KlayGE/Script.hpp>
#include <KlayGE/DllLoader.hpp>
#include <KlayGE/XMLDom.hpp>

#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <fstream>

#include <boost/bind.hpp>
#ifdef KLAYGE_COMPILER_MSVC
#pragma warning(push)
#pragma warning(disable: 4127)
#endif
#include <boost/algorithm/string/split.hpp>
#ifdef KLAYGE_COMPILER_MSVC
#pragma warning(pop)
#endif
#include <boost/filesystem.hpp>

#include <glloader/glloader.h>

#ifdef KLAYGE_COMPILER_MSVC
#pragma comment(lib, "opengl32.lib")
#endif

using namespace KlayGE;

namespace
{
	class information
	{
	public:
		information()
		{
			vendor_ = reinterpret_cast<char const *>(::glGetString(GL_VENDOR));
			renderer_ = reinterpret_cast<char const *>(::glGetString(GL_RENDERER));

			std::string const ver(reinterpret_cast<char const *>(::glGetString(GL_VERSION)));
			std::string::size_type const dot_pos(ver.find("."));
			major_ver_ = ver[dot_pos - 1] - '0';
			minor_ver_ = ver[dot_pos + 1] - '0';

			char const * glsl_ver_str = reinterpret_cast<char const *>(::glGetString(GL_SHADING_LANGUAGE_VERSION));
			if (glsl_ver_str != NULL)
			{
				std::string const glsl_ver(glsl_ver_str);
				std::string::size_type const glsl_dot_pos(ver.find("."));
				glsl_major_ver_ = glsl_ver[glsl_dot_pos - 1] - '0';
				glsl_minor_ver_ = glsl_ver[glsl_dot_pos + 1] - '0';
			}
			else
			{
				glsl_major_ver_ = 0;
				glsl_minor_ver_ = 0;
			}

			std::string const extension_str(reinterpret_cast<char const *>(::glGetString(GL_EXTENSIONS)));
			boost::algorithm::split(extensions_, extension_str,
					boost::bind(std::equal_to<char>(), ' ', _1));
			extensions_.erase(std::remove_if(extensions_.begin(), extensions_.end(),
				boost::bind(&std::string::empty, _1)), extensions_.end());
		}

		friend std::ostream& operator<<(std::ostream& os, information const & info)
		{
			os << "<?xml version='1.0' encoding='utf-8'?>" << std::endl;

			os << "<info vendor='" << info.vendor_
				<< "' renderer='" << info.renderer_
				<< "' major_ver='" << info.major_ver_
				<< "' minor_ver='" << info.minor_ver_
				<< "' glsl_major_ver='" << info.glsl_major_ver_
				<< "' glsl_minor_ver='" << info.glsl_minor_ver_ << "'>" << std::endl;

			for (std::vector<std::string>::const_iterator iter = info.extensions_.begin();
					iter != info.extensions_.end(); ++ iter)
			{
				os << "\t<extension name='" << *iter << "'/>" << std::endl;
			}

			os << "</info>" << std::endl;

			return os;
		}

	private:
		std::string vendor_;
		std::string renderer_;

		int major_ver_;
		int minor_ver_;

		int glsl_major_ver_;
		int glsl_minor_ver_;

		std::vector<std::string> extensions_;
	};
}

class EmptyApp : public KlayGE::App3DFramework
{
public:
	EmptyApp(std::string const & name, KlayGE::RenderSettings const & settings)
		: App3DFramework(name, settings)
	{
	}

	void DoUpdateOverlay()
	{
	}

	uint32_t DoUpdate(uint32_t /*pass*/)
	{
		return URV_Finished;
	}
};

int main()
{
	using namespace KlayGE;

	ResLoader::Instance().AddPath("../../../bin");

	RenderSettings settings = Context::Instance().LoadCfg("KlayGE.cfg");
	Context::Instance().LoadRenderFactory("OpenGL", XMLNodePtr());

	EmptyApp app("GL Compatibility", settings);
	app.Create();

	std::string const info_file_name("info.xml");

	information info;

	std::ofstream ofs(info_file_name.c_str());
	ofs << info;

	ScriptEngine scriptEng;
	ScriptModule module("GLCompatibility");

	module.Call("gl_compatibility", boost::make_tuple(info_file_name));

	return 0;
}
