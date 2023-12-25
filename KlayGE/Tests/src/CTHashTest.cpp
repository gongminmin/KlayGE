#include <KlayGE/KlayGE.hpp>
#include <KFL/Util.hpp>
#include <KFL/Hash.hpp>

#include "KlayGETests.hpp"

using namespace std;
using namespace KlayGE;

TEST(CTHashTest, Basic)
{
	EXPECT_EQ(CtHash("ABCD"), RtHash("ABCD"));
	EXPECT_EQ(CtHash("KlayGE"), RtHash("KlayGE"));
	EXPECT_EQ(CtHash("Test"), RtHash("Test"));
	EXPECT_EQ(CtHash("min_linear_mag_point_mip_linear"), RtHash("min_linear_mag_point_mip_linear"));
}
