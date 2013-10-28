
#include <gtest/gtest.h>

// interface: void getAllFolders(Path path, std::vector<Path> &outList)

#define TEST_DIR "getAllFolders"
#include "util_fs/testFunctions.cpp"

#include "Common.h"
#include "util/UtilFs.h"
#include "util/UtilFsPath.h"
using namespace UTIL::FS;

namespace UnitTest
{
	TEST_F(FSTestFixture, getAllFolders1)
	{
		Path path(getTestDirectory().string(), "", false);
		std::vector<Path> content;

		getAllFolders(path, content);

		ASSERT_EQ(content.size(), 1);
		ASSERT_EQ(content[0].getFullPath(), (getTestDirectory() / "0").string());
	}

	TEST_F(FSTestFixture, getAllFolder2)
	{
		Path path((getTestDirectory() / "0").string(), "", false);
		std::vector<Path> content;

		getAllFolders(path, content);

		ASSERT_EQ(content.size(), 0);
	}
}

