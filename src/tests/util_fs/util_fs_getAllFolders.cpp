#define BOOST_TEST_MAIN
#include <boost/test/unit_test.hpp>

// interface: void getAllFolders(Path path, std::vector<Path> &outList)

#define TEST_DIR "getAllFolders"
#include "testFunctions.cpp"

#include "Common.h"
#include "util/UtilFs.h"
#include "util/UtilFsPath.h"
using namespace UTIL::FS;


START_UTIL_FS_TEST_CASE

BOOST_AUTO_TEST_CASE( getAllFolders1 )
{
	Path path( getTestDirectory().string(), "", false);
	std::vector<Path> content;

	getAllFolders(path, content);

	BOOST_REQUIRE_EQUAL( content.size(), 1 );
	BOOST_REQUIRE_EQUAL( content[0].getFullPath(), (getTestDirectory()/"0").string() );
}

BOOST_AUTO_TEST_CASE( getAllFolder2)
{
	Path path( (getTestDirectory()/"0").string(), "", false);
	std::vector<Path> content;

	getAllFolders(path, content);

	BOOST_REQUIRE_EQUAL( content.size(), 0 );
}

END_UTIL_FS_TEST_CASE
