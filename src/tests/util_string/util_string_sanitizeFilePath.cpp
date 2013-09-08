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

#include "Common.h"
#include "util/UtilFs.h"

using namespace UTIL::STRING;
typedef std::string String;

BOOST_AUTO_TEST_CASE( sanitizeFilePath_slash1 )
{
	String str = "/hi/ho/hihihi\\";

	BOOST_REQUIRE_EQUAL("/hi/ho/hihihi/", sanitizeFilePath(str, '/'));
}

BOOST_AUTO_TEST_CASE( sanitizeFilePath_slash_with_dot )
{
	String str = "/hi/./ho/hihihi\\";

	BOOST_REQUIRE_EQUAL("/hi/./ho/hihihi/", sanitizeFilePath(str, '/'));
}

BOOST_AUTO_TEST_CASE( sanitizeFilePath_slash_with_dots )
{
	String str = "/hi/../ho/hihihi\\";

	BOOST_REQUIRE_EQUAL("/hi/../ho/hihihi/", sanitizeFilePath(str, '/'));
}
