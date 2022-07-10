#include <KlayGE/KlayGE.hpp>

#include <KFL/CXX20/format.hpp>
#include <KFL/Math.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/App3D.hpp>
#include <KlayGE/RenderSettings.hpp>
#include <KlayGE/Script.hpp>
#include <KlayGE/ScriptFactory.hpp>

#include <iostream>
#include <vector>
#include <string>
#include <algorithm>

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
			if (glsl_ver_str.empty())
			{
				glsl_major_ver_ = 0;
				glsl_minor_ver_ = 0;
			}
			else
			{
				std::string::size_type const glsl_dot_pos = glsl_ver_str.find(".");
				glsl_major_ver_ = glsl_ver_str[glsl_dot_pos - 1] - '0';
				glsl_minor_ver_ = glsl_ver_str[glsl_dot_pos + 1] - '0';
			}

			int num_exts;
			re.GetCustomAttrib("NUM_FEATURES", &num_exts);
			for (int i = 0; i < num_exts; ++ i)
			{
				std::string name;
				re.GetCustomAttrib(std::format("FEATURE_NAME_{}", i), &name);
				std::string::size_type p = name.find("GLES_VERSION_");
				if (std::string::npos == p)
				{
					extensions_.push_back(name);
				}
				else
				{
					major_ver_ = name[13] - '0';
					minor_ver_ = name[15] - '0';
				}
			}
		}

		std::vector<ScriptVariablePtr> StoreToPy(ScriptModule const& module)
		{
			std::vector<ScriptVariablePtr> ret;

			ret.push_back(module.MakeVariable(vendor_));
			ret.push_back(module.MakeVariable(renderer_));
			ret.push_back(module.MakeVariable(major_ver_));
			ret.push_back(module.MakeVariable(minor_ver_));
			ret.push_back(module.MakeVariable(glsl_major_ver_));
			ret.push_back(module.MakeVariable(glsl_minor_ver_));

			std::string ext_str;
			for (auto const & ext : extensions_)
			{
				ext_str += ext + ' ';
			}
			ret.push_back(module.MakeVariable(ext_str));

			return ret;
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

class GLESCompatibilityApp : public KlayGE::App3DFramework
{
public:
	GLESCompatibilityApp()
		: App3DFramework("GLESCompatibility")
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

	Context::Instance().LoadCfg("KlayGE.cfg");
	ContextCfg context_cfg = Context::Instance().Config();
	context_cfg.render_factory_name = "OpenGLES";
	context_cfg.script_factory_name = "Python";
	context_cfg.graphics_cfg.hide_win = true;
	context_cfg.graphics_cfg.hdr = false;
	context_cfg.graphics_cfg.ppaa = false;
	context_cfg.graphics_cfg.color_grading = false;
	context_cfg.graphics_cfg.gamma = false;
	Context::Instance().Config(context_cfg);

	GLESCompatibilityApp app;
	app.Create();

	ScriptEngine& scriptEng = Context::Instance().ScriptFactoryInstance().ScriptEngineInstance();
	ScriptModulePtr module = scriptEng.CreateModule("GLESCompatibility");

	information info;
	std::vector<ScriptVariablePtr> for_py = info.StoreToPy(*module);

	module->Call("gles_compatibility", for_py);

	return 0;
}
