#include <KlayGE/KlayGE.hpp>
#include <KFL/Math.hpp>

#include "KlayGETests.hpp"

#include <vector>
#include <string>
#include <iostream>

using namespace std;
using namespace KlayGE;

TEST(MathTest, NormalizeFloat2)
{
	float2 v(1, 2);
	v = MathLib::normalize(v);
	EXPECT_LT(MathLib::abs(MathLib::length(v) - 1.0f), 1e-5f);
}

TEST(MathTest, NormalizeFloat3)
{
	float3 v(1, 2, 3);
	v = MathLib::normalize(v);
	EXPECT_LT(MathLib::abs(MathLib::length(v) - 1.0f), 1e-5f);
}

TEST(MathTest, NormalizeFloat4)
{
	float4 v(1, 2, 3, 4);
	v = MathLib::normalize(v);
	EXPECT_LT(MathLib::abs(MathLib::length(v) - 1.0f), 1e-5f);
}
