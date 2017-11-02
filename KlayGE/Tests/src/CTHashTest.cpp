#include <KlayGE/KlayGE.hpp>
#include <KFL/Util.hpp>
#include <KFL/Hash.hpp>

#include "KlayGETests.hpp"

using namespace std;
using namespace KlayGE;

TEST(CTHashTest, Basic)
{
	EXPECT_EQ(CT_HASH("ABCD"), RT_HASH("ABCD"));
	EXPECT_EQ(CT_HASH("KlayGE"), RT_HASH("KlayGE"));
	EXPECT_EQ(CT_HASH("Test"), RT_HASH("Test"));
	EXPECT_EQ(CT_HASH("min_linear_mag_point_mip_linear"), RT_HASH("min_linear_mag_point_mip_linear"));
}
