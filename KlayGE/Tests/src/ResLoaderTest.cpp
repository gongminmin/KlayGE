#include <KlayGE/KlayGE.hpp>
#include <KlayGE/ResLoader.hpp>

#include "KlayGETests.hpp"

using namespace KlayGE;

std::string const sanity_string = "This is a test for ResLoader.";

std::string ReadWholeFile(ResIdentifierPtr const & res)
{
	res->seekg(0, std::ios_base::end);
	std::string str;
	str.resize(res->tellg());
	res->seekg(0, std::ios_base::beg);
	res->read(str.data(), str.size());
	return str;
}

TEST(ResLoaderTest, AddDelPath)
{
	EXPECT_TRUE(ResLoader::Instance().Locate("Test.txt").empty());
	ResLoader::Instance().AddPath("../../Tests/media/ResLoader");
	EXPECT_FALSE(ResLoader::Instance().Locate("Test.txt").empty());

	auto res = ResLoader::Instance().Open("Test.txt");
	EXPECT_TRUE(res);
	EXPECT_EQ(ReadWholeFile(res), sanity_string);

	ResLoader::Instance().DelPath("../../Tests/media/ResLoader");
	EXPECT_TRUE(ResLoader::Instance().Locate("Test.txt").empty());
}

TEST(ResLoaderTest, MountUnmountPath)
{
	ResLoader::Instance().Mount("ResLoaderTestData", "../../Tests/media/ResLoader");
	EXPECT_FALSE(ResLoader::Instance().Locate("ResLoaderTestData/Test.txt").empty());

	auto res = ResLoader::Instance().Open("ResLoaderTestData/Test.txt");
	EXPECT_TRUE(res);
	EXPECT_EQ(ReadWholeFile(res), sanity_string);

	ResLoader::Instance().Unmount("ResLoaderTestData", "../../Tests/media/ResLoader");
	EXPECT_TRUE(ResLoader::Instance().Locate("ResLoaderTestData/Test.txt").empty());
}

TEST(ResLoaderTest, MountUnmount7zPath)
{
	ResLoader::Instance().Mount("ResLoaderTestData", "../../Tests/media/ResLoader/Test.7z");
	EXPECT_FALSE(ResLoader::Instance().Locate("ResLoaderTestData/Test.txt").empty());

	auto res = ResLoader::Instance().Open("ResLoaderTestData/Test.txt");
	EXPECT_TRUE(res);
	EXPECT_EQ(ReadWholeFile(res), sanity_string);

	ResLoader::Instance().Unmount("ResLoaderTestData", "../../Tests/media/ResLoader/Test.7z");
	EXPECT_TRUE(ResLoader::Instance().Locate("ResLoaderTestData/Test.txt").empty());
}

TEST(ResLoaderTest, MountUnmountInside7zPath)
{
	ResLoader::Instance().Mount("ResLoaderTestData", "../../Tests/media/ResLoader/Test.7z/ResLoader");
	EXPECT_FALSE(ResLoader::Instance().Locate("ResLoaderTestData/Test.txt").empty());

	auto res = ResLoader::Instance().Open("ResLoaderTestData/Test.txt");
	EXPECT_TRUE(res);
	EXPECT_EQ(ReadWholeFile(res), sanity_string);

	ResLoader::Instance().Unmount("ResLoaderTestData", "../../Tests/media/ResLoader/Test.7z/ResLoader");
	EXPECT_TRUE(ResLoader::Instance().Locate("ResLoaderTestData/Test.txt").empty());
}

TEST(ResLoaderTest, MountUnmountEncrypt7zPath)
{
	ResLoader::Instance().Mount("ResLoaderTestData", "../../Tests/media/ResLoader/TestPassword.7z|1234");
	EXPECT_FALSE(ResLoader::Instance().Locate("ResLoaderTestData/Test.txt").empty());

	auto res = ResLoader::Instance().Open("ResLoaderTestData/Test.txt");
	EXPECT_TRUE(res);
	// BUG: Can't read from encrypt package, Github #164
	//EXPECT_EQ(ReadWholeFile(res), sanity_string);

	ResLoader::Instance().Unmount("ResLoaderTestData", "../../Tests/media/ResLoader/TestPassword.7z|1234");
	EXPECT_TRUE(ResLoader::Instance().Locate("ResLoaderTestData/Test.txt").empty());
}

TEST(ResLoaderTest, MountUnmountInsideEncrypt7zPath)
{
	ResLoader::Instance().Mount("ResLoaderTestData", "../../Tests/media/ResLoader/TestPassword.7z|1234/ResLoader");
	EXPECT_FALSE(ResLoader::Instance().Locate("ResLoaderTestData/Test.txt").empty());

	auto res = ResLoader::Instance().Open("ResLoaderTestData/Test.txt");
	EXPECT_TRUE(res);
	// BUG: Can't read from encrypt package, Github #164
	//EXPECT_EQ(ReadWholeFile(res), sanity_string);

	ResLoader::Instance().Unmount("ResLoaderTestData", "../../Tests/media/ResLoader/TestPassword.7z|1234/ResLoader");
	EXPECT_TRUE(ResLoader::Instance().Locate("ResLoaderTestData/Test.txt").empty());
}
