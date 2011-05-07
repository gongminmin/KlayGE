#include <KlayGE/KlayGE.hpp>
#include <KlayGE/Math.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/ResLoader.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/App3D.hpp>
#include <KlayGE/RenderSettings.hpp>
#include <KlayGE/Script.hpp>
#include <KlayGE/DllLoader.hpp>

#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <fstream>
#include <sstream>

using namespace KlayGE;

namespace
{
	class information
	{
	public:
		information()
		{
			RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();

			re.GetCustomAttrib("VENDOR", &vendor_);
			re.GetCustomAttrib("RENDERER", &renderer_);

			std::string glsl_ver_str;
			re.GetCustomAttrib("SHADING_LANGUAGE_VERSION", &glsl_ver_str);
			if (!glsl_ver_str.empty())
			{
				std::string const glsl_ver = glsl_ver_str;
				std::string::size_type const glsl_dot_pos = glsl_ver.find(".");
				glsl_major_ver_ = glsl_ver[glsl_dot_pos - 1] - '0';
				glsl_minor_ver_ = glsl_ver[glsl_dot_pos + 1] - '0';
			}
			else
			{
				glsl_major_ver_ = 0;
				glsl_minor_ver_ = 0;
			}

			int num_exts;
			re.GetCustomAttrib("NUM_FEATURES", &num_exts);
			for (int i = 0; i < num_exts; ++ i)
			{
				std::ostringstream oss;
				oss << "FEATURE_NAME_" << i;

				std::string name;
				re.GetCustomAttrib(oss.str(), &name);
				std::string::size_type p = name.find("GL_VERSION_");
				if (std::string::npos == p)
				{
					extensions_.push_back(name);
				}
				else
				{
					major_ver_ = name[11] - '0';
					minor_ver_ = name[13] - '0';
				}
			}
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
	EmptyApp()
		: App3DFramework("GL Compatibility")
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

	Context::Instance().LoadCfg("KlayGE.cfg");
	ContextCfg context_cfg = Context::Instance().Config();
	context_cfg.render_factory_name = "OpenGL";
	context_cfg.graphics_cfg.hdr = false;
	Context::Instance().Config(context_cfg);

	EmptyApp app;
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
