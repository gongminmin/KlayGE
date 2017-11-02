#include <KlayGE/KlayGE.hpp>
#include <KFL/Math.hpp>
#include <KFL/SIMDMath.hpp>

#include "KlayGETests.hpp"

#include <vector>
#include <string>
#include <iostream>

using namespace std;
using namespace KlayGE;

TEST(SIMDMathTest, NormalizeVector2)
{
	SIMDVectorF4 v = SIMDMathLib::SetVector(1, 2, 0, 0);
	v = SIMDMathLib::NormalizeVector2(v);
	EXPECT_LT(MathLib::abs(SIMDMathLib::GetX(SIMDMathLib::LengthVector2(v)) - 1.0f), 1e-3f);
}

TEST(SIMDMathTest, NormalizeVector3)
{
	SIMDVectorF4 v = SIMDMathLib::SetVector(1, 2, 3, 0);
	v = SIMDMathLib::NormalizeVector3(v);
	EXPECT_LT(MathLib::abs(SIMDMathLib::GetX(SIMDMathLib::LengthVector3(v)) - 1.0f), 1e-3f);
}

TEST(SIMDMathTest, NormalizeVector4)
{
	SIMDVectorF4 v = SIMDMathLib::SetVector(1, 2, 3, 4);
	v = SIMDMathLib::NormalizeVector4(v);
	EXPECT_LT(MathLib::abs(SIMDMathLib::GetX(SIMDMathLib::LengthVector4(v)) - 1.0f), 1e-3f);
}
