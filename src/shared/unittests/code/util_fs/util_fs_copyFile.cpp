#include <gtest/gtest.h>

// interface: void copyFile(Path src, Path dest);
//            void copyFile(std::string src, std::string dest)

// set up test env for util_fs testing
#define TEST_DIR "copyFile"
#include "util_fs/testFunctions.cpp"

#include "Common.h"
#include "util/UtilFs.h"
#include "util/UtilFsPath.h"
using namespace UTIL::FS;

#define SRC (getTestDirectory()/"0"/"1.txt")
#define DES1 (getTestDirectory()/"0"/"jsdakl1")
#define DES2 (getTestDirectory()/"0"/"jsdakl2")

namespace UnitTest
{
	TEST_F(FSTestFixture, copyFile_Path)
	{
		Path src(SRC.string(), "", false);
		Path des(DES1.string(), "", false);

		ASSERT_TRUE(!fs::exists(DES1));
		copyFile(src, des);
		ASSERT_TRUE(fs::exists(DES1));
		ASSERT_EQ_FILES(SRC, DES1);
	}

	TEST_F(FSTestFixture, copyFile_string)
	{
		ASSERT_TRUE(!fs::exists(DES2));
		copyFile(SRC.string(), DES2.string());
		ASSERT_TRUE(fs::exists(DES2));
		ASSERT_EQ_FILES(SRC, DES2);
	}
}

