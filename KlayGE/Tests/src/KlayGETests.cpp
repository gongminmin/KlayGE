#include <KlayGE/KlayGE.hpp>
#include <KlayGE/App3D.hpp>
#include <KlayGE/ResLoader.hpp>

#if defined(KLAYGE_COMPILER_CLANG)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-parameter" // Ignore unused parameter in boost
#endif
#define BOOST_TEST_MODULE KlayGETests
#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>
#if defined(KLAYGE_COMPILER_CLANG)
#pragma clang diagnostic pop
#endif

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

	class KlayGETestsFixture
	{
	public:
		KlayGETestsFixture()
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

		~KlayGETestsFixture()
		{
		}

	private:
		std::shared_ptr<App3DFramework> app;
	};

	BOOST_GLOBAL_FIXTURE(KlayGETestsFixture);
}
