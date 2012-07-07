#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/lexical_cast.hpp>
namespace fs = boost::filesystem;
using boost::lexical_cast;

void createTestDirectory();
void deleteTestDirectory();
const fs::path& getTestDirectory();
template <typename StringType>
/*
 * this will create the following directory tree:
 *  ./0/0
 *  ./0/1
 *  ./1/0
 *  ./1/1
 */
void fillWithTestData(const StringType&);

void createTestDirectory()
{
	fs::create_directories(getTestDirectory());
}

void deleteTestDirectory()
{
	fs::remove_all(getTestDirectory());
}

const fs::path& getTestDirectory()
{
	static const fs::path TEST_DIR_ROOT = fs::current_path()/"unit_test"/"util_fs";
	return TEST_DIR_ROOT;
}

template <typename STR>
void fillWithTestData(const STR& str)
{
	fs::path testDir = getTestDirectory() / str;
	fs::create_directories(testDir);

	for (int i = 0; i < 2; i++)
	{
		fs::create_directory(testDir / lexical_cast<std::string>(i) );
		for(int j = 0; j < 2; j++)
		{
			fs::path newFilePath = testDir / lexical_cast<std::string>(i) / lexical_cast<std::string>(j);
			fs::ofstream newFileStream(newFilePath);
			newFileStream << "this is a test file" << std::endl;
			newFileStream.close();
		}
	}
}
