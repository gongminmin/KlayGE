#include <KlayGE/KlayGE.hpp>
#include <KFL/Math.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/App3D.hpp>
#include <KlayGE/RenderSettings.hpp>
#include <KlayGE/RenderEffect.hpp>
#include <KFL/XMLDom.hpp>
#include <KlayGE/ResLoader.hpp>
#include <KFL/CXX17/filesystem.hpp>

#include <iostream>
#include <fstream>
#include <string>

using namespace KlayGE;
using namespace std;

class FXML2ShaderApp : public KlayGE::App3DFramework
{
public:
	FXML2ShaderApp()
		: App3DFramework("FXML2Shader")
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

int main(int argc, char* argv[])
{
	using namespace KlayGE;

	if (argc < 2)
	{
		cout << "Usage: FXML2Shader xxx.fxml" << endl;
		return 1;
	}

	std::string fxml_name = argv[1];

	Context::Instance().LoadCfg("KlayGE.cfg");
	ContextCfg context_cfg = Context::Instance().Config();
	context_cfg.render_factory_name = "D3D11";
	context_cfg.graphics_cfg.hide_win = true;
	context_cfg.graphics_cfg.hdr = false;
	Context::Instance().Config(context_cfg);

	FXML2ShaderApp app;
	app.Create();

	fxml_name = ResLoader::Instance().Locate(fxml_name);
	std::string kfx_name = fxml_name.substr(0, fxml_name.rfind(".")) + ".kfx";
	filesystem::remove(kfx_name);

	RenderEffectPtr effect = SyncLoadRenderEffect(fxml_name);

	ofstream ofs((fxml_name + ".shader").c_str());
	ofs << effect->HLSLShaderText();

	return 0;
}
