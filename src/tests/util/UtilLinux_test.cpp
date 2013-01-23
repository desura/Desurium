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
