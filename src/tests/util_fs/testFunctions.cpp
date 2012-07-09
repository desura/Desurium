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
 *  ./0/1.txt
 *  ./0/2.png
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

	std::vector<std::string> firstLevel = {"0"};
	std::vector<std::string> secondLevel = {"0", "1.txt", "2.png"};

	for (const std::string& i : firstLevel)
	{
		fs::create_directory(testDir / lexical_cast<std::string>(i) );
		for (const std::string& j : secondLevel)
		{
			fs::path newFilePath = testDir / lexical_cast<std::string>(i) / lexical_cast<std::string>(j);
			fs::ofstream newFileStream(newFilePath);
			newFileStream << "this is a test file" << std::endl;
			newFileStream.close();
		}
	}
}
