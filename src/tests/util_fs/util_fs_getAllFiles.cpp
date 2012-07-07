#define BOOST_TEST_MAIN
#include <boost/test/unit_test.hpp>

// interface: void getAllFiles(Path path, std::vector<Path> &outList, std::vector<std::string> *extsFilter);

// set up test env for util_fs testing
#include "testFunctions.cpp"

#include "Common.h"
#include "util/UtilFs.h"
#include "util/UtilFsPath.h"
using namespace UTIL::FS;

#define TEST_DIR "getAllFiles"

BOOST_AUTO_TEST_CASE( setup_env )
{
	createTestDirectory();
	fillWithTestData(TEST_DIR);
}

BOOST_AUTO_TEST_CASE( test_dir_without_files )
{
	createTestDirectory();
	fillWithTestData(TEST_DIR);

	std::vector<Path> content;
	std::vector<Path> content0;
	std::vector<Path> content1;

	Path path( (getTestDirectory()/TEST_DIR).string(), "", false);
	Path path0( (getTestDirectory()/TEST_DIR/"0").string(), "", false);
	Path path1( (getTestDirectory()/TEST_DIR/"1").string(), "", false);
	
	getAllFiles(path, content, nullptr);
	getAllFiles(path0, content0, nullptr);
	getAllFiles(path1, content1, nullptr);

	BOOST_REQUIRE_EQUAL(content.size(), 0);
	BOOST_REQUIRE_EQUAL(content0.size(), 2);
	BOOST_REQUIRE_EQUAL(content1.size(), 2);

	BOOST_REQUIRE_EQUAL(content0[0].getFullPath(), (getTestDirectory()/TEST_DIR/"0"/"0").string());
	BOOST_REQUIRE_EQUAL(content0[1].getFullPath(), (getTestDirectory()/TEST_DIR/"0"/"1").string());
	BOOST_REQUIRE_EQUAL(content1[0].getFullPath(), (getTestDirectory()/TEST_DIR/"1"/"0").string());
	BOOST_REQUIRE_EQUAL(content1[1].getFullPath(), (getTestDirectory()/TEST_DIR/"1"/"1").string());
}

BOOST_AUTO_TEST_CASE( destroy_env )
{
	deleteTestDirectory();
}
