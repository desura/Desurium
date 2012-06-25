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
