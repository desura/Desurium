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
#define DES (getTestDirectory()/"0"/"jsdakl")

START_UTIL_FS_TEST_CASE

BOOST_AUTO_TEST_CASE( copyFile_Path )
{
	Path src(SRC.string(), "", false);
	Path des(DES.string(), "", false);

	copyFile(src, des);

	BOOST_REQUIRE( fs::exists(DES) );
}

BOOST_AUTO_TEST_CASE( copyFile_string )
{
	
}

END_UTIL_FS_TEST_CASE
