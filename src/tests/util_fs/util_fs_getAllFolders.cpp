#define BOOST_TEST_MAIN
#include <boost/test/unit_test.hpp>

// interface: void getAllFolders(Path path, std::vector<Path> &outList)

#include "testFunctions.cpp"

#include "Common.h"
#include "util/UtilFs.h"
#include "util/UtilFsPath.h"
using namespace UTIL::FS;

#define TEST_DIR "getAllFolders"

BOOST_AUTO_TEST_CASE( setup_env )
{
	createTestDirectory();
	fillWithTestData(TEST_DIR);
}

BOOST_AUTO_TEST_CASE( getAllFolders1 )
{
	Path path( (getTestDirectory()/TEST_DIR).string(), "", false);
	std::vector<Path> content;

	getAllFolders(path, content);

	BOOST_REQUIRE_EQUAL( content.size(), 1 );
	BOOST_REQUIRE_EQUAL( content[0].getFullPath(), (getTestDirectory()/TEST_DIR/"0").string() );
}

BOOST_AUTO_TEST_CASE( getAllFolder2)
{
	Path path( (getTestDirectory()/TEST_DIR/"0").string(), "", false);
	std::vector<Path> content;

	getAllFolders(path, content);

	BOOST_REQUIRE_EQUAL( content.size(), 0 );
}

BOOST_AUTO_TEST_CASE( destroy_env )
{
	deleteTestDirectory();
}
