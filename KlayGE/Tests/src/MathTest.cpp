#include <KlayGE/KlayGE.hpp>
#include <KFL/Math.hpp>

#include <boost/assert.hpp>
#ifdef KLAYGE_COMPILER_CLANG
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-parameter" // Ignore unused parameter in boost
#endif
#include <boost/test/unit_test.hpp>
#ifdef KLAYGE_COMPILER_CLANG
#pragma clang diagnostic pop
#endif

#include <vector>
#include <string>
#include <iostream>

using namespace std;
using namespace KlayGE;

BOOST_AUTO_TEST_CASE(NormalizeFloat2)
{
	float2 v(1, 2);
	v = MathLib::normalize(v);
	BOOST_CHECK(MathLib::abs(MathLib::length(v) - 1.0f) < 1e-5f);
}

BOOST_AUTO_TEST_CASE(NormalizeFloat3)
{
	float3 v(1, 2, 3);
	v = MathLib::normalize(v);
	BOOST_CHECK(MathLib::abs(MathLib::length(v) - 1.0f) < 1e-5f);
}

BOOST_AUTO_TEST_CASE(NormalizeFloat4)
{
	float4 v(1, 2, 3, 4);
	v = MathLib::normalize(v);
	BOOST_CHECK(MathLib::abs(MathLib::length(v) - 1.0f) < 1e-5f);
}
