#include <KlayGE/KlayGE.hpp>
#include <KFL/Util.hpp>

#include <boost/assert.hpp>
#ifdef KLAYGE_COMPILER_CLANG
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-parameter" // Ignore unused parameter in boost
#endif
#include <boost/test/unit_test.hpp>
#ifdef KLAYGE_COMPILER_CLANG
#pragma clang diagnostic pop
#endif

using namespace std;
using namespace KlayGE;

BOOST_AUTO_TEST_CASE(CTHash)
{
	BOOST_CHECK(CT_HASH("ABCD") == RT_HASH("ABCD"));
	BOOST_CHECK(CT_HASH("KlayGE") == RT_HASH("KlayGE"));
	BOOST_CHECK(CT_HASH("Test") == RT_HASH("Test"));
	BOOST_CHECK(CT_HASH("min_linear_mag_point_mip_linear") == RT_HASH("min_linear_mag_point_mip_linear"));
}
