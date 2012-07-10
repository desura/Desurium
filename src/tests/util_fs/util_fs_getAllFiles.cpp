#define BOOST_TEST_MAIN
#include <boost/test/unit_test.hpp>

// interface: void getAllFiles(Path path, std::vector<Path> &outList, std::vector<std::string> *extsFilter);

// set up test env for util_fs testing
#define TEST_DIR "getAllFiles"
#include "testFunctions.cpp"

#include "Common.h"
#include "util/UtilFs.h"
#include "util/UtilFsPath.h"
using namespace UTIL::FS;

START_UTIL_FS_TEST_CASE

BOOST_AUTO_TEST_CASE( getAllFiles_without_extsFilter )
{
	std::vector<Path> content;
	std::vector<Path> content0;

	Path path( getTestDirectory().string(), "", false);
	Path path0( (getTestDirectory()/"0").string(), "", false);

	getAllFiles(path, content, nullptr);
	getAllFiles(path0, content0, nullptr);

	BOOST_REQUIRE_EQUAL(content.size(), 0);
	BOOST_REQUIRE_EQUAL(content0.size(), 4);

	BOOST_REQUIRE_EQUAL(content0[0].getFullPath(), (getTestDirectory()/"0"/"0").string());
	BOOST_REQUIRE_EQUAL(content0[1].getFullPath(), (getTestDirectory()/"0"/"1.txt").string());
	BOOST_REQUIRE_EQUAL(content0[2].getFullPath(), (getTestDirectory()/"0"/"2.png").string());
	BOOST_REQUIRE_EQUAL(content0[3].getFullPath(), (getTestDirectory()/"0"/UNICODE_EXAMPLE_FILE).string());
}

BOOST_AUTO_TEST_CASE( getAllFiles_with_extsfiler )
{
	std::vector<Path> content;
	std::vector<std::string> filter = {"txt"};
	Path path( (getTestDirectory()/"0").string(), "", false);
	getAllFiles(path, content, &filter);

	BOOST_REQUIRE_EQUAL(content.size(), 1);
	BOOST_REQUIRE_EQUAL(content[0].getFullPath(), (getTestDirectory()/"0"/"1.txt").string());
}

END_UTIL_FS_TEST_CASE
