#ifndef TEST_DIR
#error you have to define TEST_DIR first
#endif

#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
namespace fs = boost::filesystem;

void createTestDirectory();
void deleteTestDirectory();
const fs::path& getTestDirectory();
/*
 * this will create the following directory tree:
 *  ./0/0
 *  ./0/1.txt
 *  ./0/2.png
 */
void fillWithTestData();

// some macros
#define START_UTIL_FS_TEST_CASE BOOST_AUTO_TEST_CASE( setup_env ) \
{ \
	createTestDirectory(); \
}
#define END_UTIL_FS_TEST_CASE BOOST_AUTO_TEST_CASE( destroy_env ) \
{ \
	deleteTestDirectory(); \
}

void createTestDirectory()
{
	fs::create_directories(getTestDirectory());
	fillWithTestData();
}

void deleteTestDirectory()
{
	fs::remove_all(getTestDirectory());
}

const fs::path& getTestDirectory()
{
	static const fs::path TEST_DIR_ROOT = fs::current_path()/"unit_test"/"util_fs"/TEST_DIR;
	return TEST_DIR_ROOT;
}

void fillWithTestData()
{
	const fs::path &testDir = getTestDirectory();
	fs::create_directories(testDir);

	std::vector<std::string> firstLevel = {"0"};
	std::vector<std::string> secondLevel = {"0", "1.txt", "2.png"};

	for (const std::string& i : firstLevel)
	{
		fs::create_directory(testDir / i );
		for (const std::string& j : secondLevel)
		{
			fs::path newFilePath = testDir / i / j;
			fs::ofstream newFileStream(newFilePath);
			newFileStream << "this is a test file" << std::endl;
			newFileStream.close();
		}
	}
}
