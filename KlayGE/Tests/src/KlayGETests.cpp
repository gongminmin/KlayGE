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

	class KlayGETestEnvironment : public testing::Environment
	{
	public:
		void SetUp() override
		{
			Context::Instance().LoadCfg("KlayGE.cfg");
			ContextCfg context_cfg = Context::Instance().Config();
			context_cfg.graphics_cfg.hide_win = true;
			context_cfg.graphics_cfg.hdr = false;
			context_cfg.graphics_cfg.color_grading = false;
			context_cfg.graphics_cfg.gamma = false;
			Context::Instance().Config(context_cfg);

			app_ = MakeSharedPtr<KlayGETestsApp>();
			app_->Create();
		}

		void TearDown() override
		{
			app_.reset();

			Context::Destroy();
		}

	private:
		std::shared_ptr<App3DFramework> app_;
	};

	void KlayGETest::SetUp()
	{
	}

	void KlayGETest::TearDown()
	{
	}
}

int main(int argc, char** argv)
{
	InitGoogleTest(&argc, argv);
	AddGlobalTestEnvironment(new KlayGE::KlayGETestEnvironment);

	int ret_val = RUN_ALL_TESTS();
	if (ret_val != 0)
	{
		getchar();
	}

	return ret_val;
}
