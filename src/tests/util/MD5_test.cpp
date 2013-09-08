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

#define BOOST_TEST_MODULE MD5_test
#include <boost/test/unit_test.hpp>

#include "Common.h"
#include "util/MD5Progressive.h"

#define EMPTY_MD5 "d41d8cd98f00b204e9800998ecf8427e"

static const char TEST_CSTRING[] = "this is a little test";
#define TEST_CSTRING_SIZE sizeof(TEST_CSTRING)
#define TEST_MD5_RESULT "419d085dcebc10a7e6d7115216e9820d"

BOOST_AUTO_TEST_CASE( MD5Progressive_cons )
{
	MD5Progressive md5;
	BOOST_REQUIRE_EQUAL(md5.finish(), EMPTY_MD5);
}

BOOST_AUTO_TEST_CASE( MD5Progressive_update )
{
	MD5Progressive md5;
	md5.update(TEST_CSTRING, TEST_CSTRING_SIZE - 1);
	BOOST_REQUIRE_EQUAL(md5.finish(), TEST_MD5_RESULT);
}
