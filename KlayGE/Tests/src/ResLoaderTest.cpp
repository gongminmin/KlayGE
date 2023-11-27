#include <KlayGE/KlayGE.hpp>
#include <KlayGE/ResLoader.hpp>

#include "KlayGETests.hpp"

using namespace KlayGE;

std::string const sanity_string = "This is a test for ResLoader.";

std::string ReadWholeFile(ResIdentifierPtr const & res)
{
	res->seekg(0, std::ios_base::end);
	std::string str;
	str.resize(static_cast<size_t>(res->tellg()));
	res->seekg(0, std::ios_base::beg);
	res->read(&str[0], str.size());
	return str;
}

TEST(ResLoaderTest, AddDelPath)
{
	auto& res_loader = Context::Instance().ResLoaderInstance();

	EXPECT_TRUE(res_loader.Locate("Test.txt").empty());
	res_loader.AddPath("../../Tests/media/ResLoader");
	EXPECT_FALSE(res_loader.Locate("Test.txt").empty());

	auto res = res_loader.Open("Test.txt");
	EXPECT_TRUE(res);
	EXPECT_EQ(ReadWholeFile(res), sanity_string);

	res_loader.DelPath("../../Tests/media/ResLoader");
	EXPECT_TRUE(res_loader.Locate("Test.txt").empty());
}

TEST(ResLoaderTest, MountUnmountPath)
{
	auto& res_loader = Context::Instance().ResLoaderInstance();

	res_loader.Mount("ResLoaderTestData", "../../Tests/media/ResLoader");
	EXPECT_FALSE(res_loader.Locate("ResLoaderTestData/Test.txt").empty());

	auto res = res_loader.Open("ResLoaderTestData/Test.txt");
	EXPECT_TRUE(res);
	EXPECT_EQ(ReadWholeFile(res), sanity_string);

	res_loader.Unmount("ResLoaderTestData", "../../Tests/media/ResLoader");
	EXPECT_TRUE(res_loader.Locate("ResLoaderTestData/Test.txt").empty());
}

TEST(ResLoaderTest, MountUnmount7zPath)
{
	auto& res_loader = Context::Instance().ResLoaderInstance();

	res_loader.Mount("ResLoaderTestData", "../../Tests/media/ResLoader/Test.7z");
	EXPECT_FALSE(res_loader.Locate("ResLoaderTestData/Test.txt").empty());

	auto res = res_loader.Open("ResLoaderTestData/Test.txt");
	EXPECT_TRUE(res);
	EXPECT_EQ(ReadWholeFile(res), sanity_string);

	res_loader.Unmount("ResLoaderTestData", "../../Tests/media/ResLoader/Test.7z");
	EXPECT_TRUE(res_loader.Locate("ResLoaderTestData/Test.txt").empty());
}

TEST(ResLoaderTest, MountUnmountInside7zPath)
{
	auto& res_loader = Context::Instance().ResLoaderInstance();

	res_loader.Mount("ResLoaderTestData", "../../Tests/media/ResLoader/Test.7z/ResLoader");
	EXPECT_FALSE(res_loader.Locate("ResLoaderTestData/Test.txt").empty());

	auto res = res_loader.Open("ResLoaderTestData/Test.txt");
	EXPECT_TRUE(res);
	EXPECT_EQ(ReadWholeFile(res), sanity_string);

	res_loader.Unmount("ResLoaderTestData", "../../Tests/media/ResLoader/Test.7z/ResLoader");
	EXPECT_TRUE(res_loader.Locate("ResLoaderTestData/Test.txt").empty());
}

TEST(ResLoaderTest, MountUnmountEncrypt7zPath)
{
	auto& res_loader = Context::Instance().ResLoaderInstance();

	res_loader.Mount("ResLoaderTestData", "../../Tests/media/ResLoader/TestPassword.7z|1234");
	EXPECT_FALSE(res_loader.Locate("ResLoaderTestData/Test.txt").empty());

	auto res = res_loader.Open("ResLoaderTestData/Test.txt");
	EXPECT_TRUE(res);
	EXPECT_EQ(ReadWholeFile(res), sanity_string);

	res_loader.Unmount("ResLoaderTestData", "../../Tests/media/ResLoader/TestPassword.7z|1234");
	EXPECT_TRUE(res_loader.Locate("ResLoaderTestData/Test.txt").empty());
}

TEST(ResLoaderTest, MountUnmountInsideEncrypt7zPath)
{
	auto& res_loader = Context::Instance().ResLoaderInstance();

	res_loader.Mount("ResLoaderTestData", "../../Tests/media/ResLoader/TestPassword.7z|1234/ResLoader");
	EXPECT_FALSE(res_loader.Locate("ResLoaderTestData/Test.txt").empty());

	auto res = res_loader.Open("ResLoaderTestData/Test.txt");
	EXPECT_TRUE(res);
	EXPECT_EQ(ReadWholeFile(res), sanity_string);

	res_loader.Unmount("ResLoaderTestData", "../../Tests/media/ResLoader/TestPassword.7z|1234/ResLoader");
	EXPECT_TRUE(res_loader.Locate("ResLoaderTestData/Test.txt").empty());
}
