/*
Desura is the leading indie game distribution platform
Copyright (C) Karol Herbst <git@karolherbst.de>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>
*/

#define BOOST_TEST_MAIN
#include <boost/test/unit_test.hpp>

// interface: void getAllFiles(Path path, std::vector<Path> &outList, std::vector<std::string> *extsFilter);

// set up test env for util_fs testing
#define TEST_DIR "getAllFiles"
#include "testFunctions.cpp"
#include "helper_functions.h"

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


	BOOST_REQUIRE( isInVector(Path((getTestDirectory()/"0"/"0").string(), "", true), content0) );
	BOOST_REQUIRE( isInVector(Path((getTestDirectory()/"0"/"1.txt").string(), "", true), content0) );
	BOOST_REQUIRE( isInVector(Path((getTestDirectory()/"0"/"2.png").string(), "", true), content0) );
	BOOST_REQUIRE( isInVector(Path((getTestDirectory()/"0"/UNICODE_EXAMPLE_FILE).string(), "", true), content0) );
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
