#define BOOST_TEST_MAIN
#include <boost/test/unit_test.hpp>

// interface: void copyFile(Path src, Path dest);
//            void copyFile(std::string src, std::string dest)

// set up test env for util_fs testing
#define TEST_DIR "copyFile"
#include "testFunctions.cpp"

#include "Common.h"
#include "util/UtilFs.h"
#include "util/UtilFsPath.h"
using namespace UTIL::FS;

#define SRC (getTestDirectory()/"0"/"1.txt")
#define DES1 (getTestDirectory()/"0"/"jsdakl1")
#define DES2 (getTestDirectory()/"0"/"jsdakl2")

START_UTIL_FS_TEST_CASE

BOOST_AUTO_TEST_CASE( copyFile_Path )
{
	Path src(SRC.string(), "", false);
	Path des(DES1.string(), "", false);

	BOOST_REQUIRE( !fs::exists(DES1) );
	copyFile(src, des);
	BOOST_REQUIRE( fs::exists(DES1) );
	BOOST_REQUIRE_EQUAL_FILES(SRC, DES1);
}

BOOST_AUTO_TEST_CASE( copyFile_string )
{
	BOOST_REQUIRE( !fs::exists(DES2) );
	copyFile(SRC.string(), DES2.string());
	BOOST_REQUIRE( fs::exists(DES2) );
	BOOST_REQUIRE_EQUAL_FILES (SRC, DES2);
}

END_UTIL_FS_TEST_CASE
