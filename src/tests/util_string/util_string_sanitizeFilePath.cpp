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
