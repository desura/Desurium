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

// interface: void copyFile(Path src, Path dest);
//            void copyFile(std::string src, std::string dest)

// set up test env for util_fs testing
#define TEST_DIR "copyFile"
#include "testFunctions.cpp"

#include "Common.h"
#include "util/UtilFs.h"
#include "util/UtilFsPath.h"
using namespace UTIL::FS;

#define SRC (getTestDirectory()/"0"/"1.txt")
#define DES1 (getTestDirectory()/"0"/"jsdakl1")
#define DES2 (getTestDirectory()/"0"/"jsdakl2")

START_UTIL_FS_TEST_CASE

BOOST_AUTO_TEST_CASE( copyFile_Path )
{
	Path src(SRC.string(), "", false);
	Path des(DES1.string(), "", false);

	BOOST_REQUIRE( !fs::exists(DES1) );
	copyFile(src, des);
	BOOST_REQUIRE( fs::exists(DES1) );
	BOOST_REQUIRE_EQUAL_FILES(SRC, DES1);
}

BOOST_AUTO_TEST_CASE( copyFile_string )
{
	BOOST_REQUIRE( !fs::exists(DES2) );
	copyFile(SRC.string(), DES2.string());
	BOOST_REQUIRE( fs::exists(DES2) );
	BOOST_REQUIRE_EQUAL_FILES (SRC, DES2);
}

END_UTIL_FS_TEST_CASE
