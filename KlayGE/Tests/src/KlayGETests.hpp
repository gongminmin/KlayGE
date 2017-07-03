#include <gtest/gtest.h>

namespace KlayGE
{
	class KlayGETest : public testing::Test
	{
	protected:
		void SetUp() override;

		void TearDown() override;

	protected:
		std::shared_ptr<App3DFramework> app;
	};
}
