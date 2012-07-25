#ifndef TEST_DIR
#error you have to define TEST_DIR first
#endif

#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
namespace fs = boost::filesystem;

#define UNICODE_EXAMPLE_FILE "ʨʬжװᇪ"

void createTestDirectory();
void deleteTestDirectory();
const fs::path& getTestDirectory();
/*
 * this will create the following directory tree:
 *  ./0/0
 *  ./0/1.txt
 *  ./0/2.png
 *  ./0/UNICODE_EXAMPLE_FILE
 */
void fillWithTestData();

#define BOOST_REQUIRE_EQUAL_FILES(/* const fs::path& */ file1, /* const fs::path& */ file2) \
{ \
	fs::ifstream file1_stream(file1); \
	fs::ifstream file2_stream(file2); \
 \
	std::string file1_string; \
	std::string file2_string; \
 \
	for (std::string file1_string_buf; std::getline(file1_stream, file1_string_buf);) \
		file1_string += file1_string_buf; \
	for (std::string file2_string_buf; std::getline(file2_stream, file2_string_buf);) \
		file2_string += file2_string_buf; \
 \
	BOOST_REQUIRE_EQUAL( file1_string, file2_string ); \
 \
	file1_stream.close(); \
	file2_stream.close(); \
}


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
	std::vector<std::string> secondLevel = {"0", "1.txt", "2.png", UNICODE_EXAMPLE_FILE};

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
