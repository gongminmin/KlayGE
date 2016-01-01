#include <KlayGE/KlayGE.hpp>
#include <KFL/Math.hpp>
#include <KFL/SIMDMath.hpp>

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

BOOST_AUTO_TEST_CASE(NormalizeVector2)
{
	SIMDVectorF4 v = SIMDMathLib::SetVector(1, 2, 0, 0);
	v = SIMDMathLib::NormalizeVector2(v);
	BOOST_CHECK(MathLib::abs(SIMDMathLib::GetX(SIMDMathLib::LengthVector2(v)) - 1.0f) < 1e-3f);
}

BOOST_AUTO_TEST_CASE(NormalizeVector3)
{
	SIMDVectorF4 v = SIMDMathLib::SetVector(1, 2, 3, 0);
	v = SIMDMathLib::NormalizeVector3(v);
	BOOST_CHECK(MathLib::abs(SIMDMathLib::GetX(SIMDMathLib::LengthVector3(v)) - 1.0f) < 1e-3f);
}

BOOST_AUTO_TEST_CASE(NormalizeVector4)
{
	SIMDVectorF4 v = SIMDMathLib::SetVector(1, 2, 3, 4);
	v = SIMDMathLib::NormalizeVector4(v);
	BOOST_CHECK(MathLib::abs(SIMDMathLib::GetX(SIMDMathLib::LengthVector4(v)) - 1.0f) < 1e-3f);
}
