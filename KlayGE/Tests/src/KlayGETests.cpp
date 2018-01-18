#include <KlayGE/KlayGE.hpp>
#include <KlayGE/App3D.hpp>
#include <KlayGE/ResLoader.hpp>

#include "KlayGETests.hpp"

using namespace testing;

namespace KlayGE
{
	class KlayGETestsApp : public App3DFramework
	{
	public:
		KlayGETestsApp()
			: App3DFramework("KlayGETests")
		{
			ResLoader::Instance().AddPath("../../Tests/media");
		}

		virtual void DoUpdateOverlay() override
		{
		}

		virtual uint32_t DoUpdate(uint32_t pass) override
		{
			KFL_UNUSED(pass);
			return URV_Finished;
		}
	};


	void KlayGETest::SetUp()
	{
		Context::Instance().LoadCfg("KlayGE.cfg");
		ContextCfg context_cfg = Context::Instance().Config();
		context_cfg.graphics_cfg.hide_win = true;
		context_cfg.graphics_cfg.hdr = false;
		context_cfg.graphics_cfg.color_grading = false;
		context_cfg.graphics_cfg.gamma = false;
		Context::Instance().Config(context_cfg);

		app = MakeSharedPtr<KlayGETestsApp>();
		app->Create();
	}

	void KlayGETest::TearDown()
	{
	}
}

int main(int argc, char** argv)
{
	InitGoogleTest(&argc, argv);
	int ret_val = RUN_ALL_TESTS();
	if (ret_val != 0)
	{
		getchar();
	}

	return ret_val;
}
