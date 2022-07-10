/**
 * @file StringUtilTest.cpp
 * @author Minmin Gong
 *
 * @section DESCRIPTION
 *
 * This source file is part of KFL, a subproject of KlayGE
 * For the latest info, see http://www.klayge.org
 *
 * @section LICENSE
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published
 * by the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 * You may alternatively use this source under the terms of
 * the KlayGE Proprietary License (KPL). You can obtained such a license
 * from http://www.klayge.org/licensing/.
 */

#include <KlayGE/KlayGE.hpp>
#include <KFL/StringUtil.hpp>
#include <KFL/Util.hpp>

#include <string_view>

#include "KlayGETests.hpp"

using namespace std;
using namespace KlayGE;

TEST(StringUtilTest, NarrowSplit)
{
    std::vector<std::string_view> narrow_result = StringUtil::Split("split Test", StringUtil::EqualTo(' '));
    EXPECT_EQ(narrow_result.size(), 2U);
    EXPECT_EQ(narrow_result[0], "split");
    EXPECT_EQ(narrow_result[1], "Test");

    narrow_result = StringUtil::Split(" split Test ", StringUtil::EqualTo(' '));
    EXPECT_EQ(narrow_result.size(), 2U);
    EXPECT_EQ(narrow_result[0], "split");
    EXPECT_EQ(narrow_result[1], "Test");

    narrow_result = StringUtil::Split(" split\tTest ", StringUtil::IsAnyOf(" \t"));
    EXPECT_EQ(narrow_result.size(), 2U);
    EXPECT_EQ(narrow_result[0], "split");
    EXPECT_EQ(narrow_result[1], "Test");
}

TEST(StringUtilTest, WideSplit)
{
    std::vector<std::wstring_view> result = StringUtil::Split(L"split Test", StringUtil::EqualTo(L' '));
    EXPECT_EQ(result.size(), 2U);
    EXPECT_EQ(result[0], L"split");
    EXPECT_EQ(result[1], L"Test");

    result = StringUtil::Split(L" split Test ", StringUtil::EqualTo(L' '));
    EXPECT_EQ(result.size(), 2U);
    EXPECT_EQ(result[0], L"split");
    EXPECT_EQ(result[1], L"Test");

    result = StringUtil::Split(L" split\tTest ", StringUtil::IsAnyOf(L" \t"));
    EXPECT_EQ(result.size(), 2U);
    EXPECT_EQ(result[0], L"split");
    EXPECT_EQ(result[1], L"Test");
}

TEST(StringUtilTest, Trim)
{
    std::string_view result = StringUtil::TrimLeft("");
    EXPECT_EQ(result, "");

    result = StringUtil::TrimLeft(" trim Test ");
    EXPECT_EQ(result, "trim Test ");

    result = StringUtil::TrimRight(" trim Test ");
    EXPECT_EQ(result, " trim Test");

    result = StringUtil::Trim(" trim Test ");
    EXPECT_EQ(result, "trim Test");
}

TEST(StringUtilTest, UpperLower)
{
    std::string str = "lOwEr cAsE";
    StringUtil::ToLower(str);
    EXPECT_EQ(str, "lower case");

    str = "UppEr cAsE";
    StringUtil::ToUpper(str);
    EXPECT_EQ(str, "UPPER CASE");
}

TEST(StringUtilTest, CaseInsensitiveCompare)
{
    EXPECT_FALSE(StringUtil::CaseInsensitiveLexicographicalCompare("String1", "sTRING1"));
    EXPECT_TRUE(StringUtil::CaseInsensitiveLexicographicalCompare("String1", "sTRING2"));
}
