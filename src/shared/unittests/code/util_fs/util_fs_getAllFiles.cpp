
#include <gtest/gtest.h>

// interface: void getAllFiles(Path path, std::vector<Path> &outList, std::vector<std::string> *extsFilter);

// set up test env for util_fs testing
#define TEST_DIR "getAllFiles"
#include "util_fs/testFunctions.cpp"
#include "helper_functions.h"

#include "Common.h"
#include "util/UtilFs.h"
#include "util/UtilFsPath.h"
using namespace UTIL::FS;

namespace UnitTest
{
	TEST_F(FSTestFixture, getAllFiles_without_extsFilter)
	{
		std::vector<Path> content;
		std::vector<Path> content0;

		Path path(getTestDirectory().string(), "", false);
		Path path0((getTestDirectory() / "0").string(), "", false);

		getAllFiles(path, content, nullptr);
		getAllFiles(path0, content0, nullptr);

		ASSERT_EQ(content.size(), 0);
		ASSERT_EQ(content0.size(), 4);


		ASSERT_TRUE(isInVector(Path((getTestDirectory() / "0" / "0").string(), "", true), content0));
		ASSERT_TRUE(isInVector(Path((getTestDirectory() / "0" / "1.txt").string(), "", true), content0));
		ASSERT_TRUE(isInVector(Path((getTestDirectory() / "0" / "2.png").string(), "", true), content0));
		ASSERT_TRUE(isInVector(Path((getTestDirectory() / "0" / UNICODE_EXAMPLE_FILE).string(), "", true), content0));
	}

	TEST_F(FSTestFixture, getAllFiles_with_extsfiler)
	{
		std::vector<Path> content;
		std::vector<std::string> filter;
		filter.push_back("txt");

		Path path((getTestDirectory() / "0").string(), "", false);
		getAllFiles(path, content, &filter);

		ASSERT_EQ(content.size(), 1);
		ASSERT_EQ(content[0].getFullPath(), (getTestDirectory() / "0" / "1.txt").string());
	}
}

