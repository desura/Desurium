#define BOOST_TEST_MAIN
#include <boost/test/unit_test.hpp>

// interface: void copyFolder(Path src, Path dest, std::vector<std::string> *vIgnoreList = NULL, bool copyOverExisting = true);
//            void copyFolder(std::string src, std::string dest, std::vector<std::string> *vIgnoreList = NULL, bool copyOverExisting = true)

// set up test env for util_fs testing
#define TEST_DIR "copyFolder"
#include "testFunctions.cpp"

#include "Common.h"
#include "util/UtilFs.h"
#include "util/UtilFsPath.h"
using namespace UTIL::FS;

#define SRC (getTestDirectory()/"0")
#define DES1 (getTestDirectory()/"1")
#define DES2 (getTestDirectory()/"2")
#define DES3 (getTestDirectory()/"3")
#define DES4 (getTestDirectory()/"4")
#define SRC5 (getTestDirectory()/"S5")
#define DES5 (getTestDirectory()/"D5")

START_UTIL_FS_TEST_CASE

BOOST_AUTO_TEST_CASE( copyFolder_Path_NULL_default )
{
	Path src(SRC.string(), "", false);
	Path des(DES1.string(), "", false);

	BOOST_REQUIRE( !fs::exists(DES1) );
	copyFolder(src, des);
	BOOST_REQUIRE( fs::exists(DES1) );
	BOOST_REQUIRE_EQUAL_FILES( SRC/"0", DES1/"0" );
	BOOST_REQUIRE_EQUAL_FILES( SRC/"1.txt", DES1/"1.txt" );
	BOOST_REQUIRE_EQUAL_FILES( SRC/"2.png", DES1/"2.png" );
	BOOST_REQUIRE_EQUAL_FILES( SRC/UNICODE_EXAMPLE_FILE, DES1/UNICODE_EXAMPLE_FILE );
}

BOOST_AUTO_TEST_CASE( copyFolder_Path_vector_default )
{
	Path src(SRC.string(), "", false);
	Path des(DES2.string(), "", false);
	std::vector<std::string> ignoreList = {"1.txt"};

	BOOST_REQUIRE( !fs::exists(DES2) );
	copyFolder(src, des, &ignoreList);
	BOOST_REQUIRE( fs::exists(DES2) );
	BOOST_REQUIRE_EQUAL_FILES( SRC/"0", DES2/"0" );
	BOOST_REQUIRE( !fs::exists(DES2/"1.txt") );
	BOOST_REQUIRE_EQUAL_FILES( SRC/"2.png", DES2/"2.png" );
	BOOST_REQUIRE_EQUAL_FILES( SRC/UNICODE_EXAMPLE_FILE, DES2/UNICODE_EXAMPLE_FILE );
}

BOOST_AUTO_TEST_CASE( copyFolder_string_NULL_default )
{
	BOOST_REQUIRE( !fs::exists(DES3) );
	copyFolder(SRC.string(), DES3.string());
	BOOST_REQUIRE( fs::exists(DES3) );
	BOOST_REQUIRE_EQUAL_FILES( SRC/"0", DES3/"0" );
	BOOST_REQUIRE_EQUAL_FILES( SRC/"1.txt", DES3/"1.txt" );
	BOOST_REQUIRE_EQUAL_FILES( SRC/"2.png", DES3/"2.png" );
	BOOST_REQUIRE_EQUAL_FILES( SRC/UNICODE_EXAMPLE_FILE, DES3/UNICODE_EXAMPLE_FILE );
}

BOOST_AUTO_TEST_CASE( copyFolder_string_vector_default )
{
	std::vector<std::string> ignoreList = {"1.txt"};
	BOOST_REQUIRE( !fs::exists(DES4) );
	copyFolder(SRC.string(), DES4.string(), &ignoreList);
	BOOST_REQUIRE( fs::exists(DES4) );
	BOOST_REQUIRE_EQUAL_FILES( SRC/"0", DES4/"0" );
	BOOST_REQUIRE( !fs::exists(DES4/"1.txt") );
	BOOST_REQUIRE_EQUAL_FILES( SRC/"2.png", DES4/"2.png" );
	BOOST_REQUIRE_EQUAL_FILES( SRC/UNICODE_EXAMPLE_FILE, DES4/UNICODE_EXAMPLE_FILE );
}

BOOST_AUTO_TEST_CASE( copyFolder_string_rec )
{
	// here we copy a directory with files into a directory ...
	copyFolder(SRC.string(), (SRC5/"0").string());

	BOOST_REQUIRE( !fs::exists(DES5) );
	// ... so we can copy the directory with the directory with files ;)
	copyFolder(SRC5.string(), DES5.string() );
	BOOST_REQUIRE( fs::exists(DES5) );
	BOOST_REQUIRE( fs::exists(DES5/"0") );
	BOOST_REQUIRE_EQUAL_FILES( SRC/"0", DES5/"0"/"0" );
	BOOST_REQUIRE_EQUAL_FILES( SRC/"1.txt", DES5/"0"/"1.txt" );
	BOOST_REQUIRE_EQUAL_FILES( SRC/"2.png", DES5/"0"/"2.png" );
	BOOST_REQUIRE_EQUAL_FILES( SRC/UNICODE_EXAMPLE_FILE, DES5/"0"/UNICODE_EXAMPLE_FILE );
}

END_UTIL_FS_TEST_CASE
