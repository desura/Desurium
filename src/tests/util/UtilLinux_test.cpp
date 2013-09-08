/*
Desura is the leading indie game distribution platform
Copyright (C) Ian T. Jacobsen <iantj92@gmail.com>
          (C) Karol Herbst <git@karolherbst.de>
          (C) matthiaskrgr <matthias.krueger@famsik.de>

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

#warning todo: add boost checks for all util calls

#include <boost/filesystem.hpp>

#include "Common.h"
#include "util/UtilString.h"
#include "util/UtilLinux.h"
#include <iostream>

using namespace boost;
using namespace boost::unit_test;
using namespace UTIL::LIN;

BOOST_AUTO_TEST_CASE (Util_Lin_String_Output)
{
	BOOST_REQUIRE(getFreeSpace("/") != 0);
	std::string execpath = getExecuteDir();
	// check if getExecuteDir() returns a consistent path
	for(int i = 0; i < 100; ++i) {
		BOOST_REQUIRE_EQUAL(getExecuteDir(), execpath);
	}
}
