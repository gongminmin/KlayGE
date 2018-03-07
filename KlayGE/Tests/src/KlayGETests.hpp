#if defined(KLAYGE_COMPILER_CLANGC2)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations" // Ignore POSIX function declaration
#endif
#include <gtest/gtest.h>
#if defined(KLAYGE_COMPILER_CLANGC2)
#pragma clang diagnostic pop
#endif

namespace KlayGE
{
	class KlayGETest : public testing::Test
	{
	protected:
		void SetUp() override;
		void TearDown() override;
	};
}
