#define BOOST_TEST_MODULE gcBuff_test
#include <boost/test/unit_test.hpp>

#include "util/gcBuff.h"

#define TEST_SIZE 20
#define TEST_CSTRING "this is a test!"
#define TEST_CSTRING_SIZE sizeof(TEST_CSTRING)
const char TEST_STATIC_CSTRING[] = TEST_CSTRING;

BOOST_AUTO_TEST_CASE( gcBuff_constructor )
{
	//construct our objects
	gcBuff buf1(TEST_SIZE);
	gcBuff buf2(TEST_CSTRING, TEST_CSTRING_SIZE);

	BOOST_REQUIRE_EQUAL(buf1.size(), TEST_SIZE);

	BOOST_REQUIRE_EQUAL(buf2.size(), TEST_CSTRING_SIZE);
	BOOST_REQUIRE_EQUAL(buf2.c_ptr(), TEST_STATIC_CSTRING);
}

BOOST_AUTO_TEST_CASE( gcBuff_copy_constructor )
{
	gcBuff buf_ori(TEST_CSTRING, TEST_CSTRING_SIZE);
	gcBuff buf_copy_ref(buf_ori);
	gcBuff buf_copy_ptr(&buf_ori);

	BOOST_REQUIRE_EQUAL(buf_copy_ref.c_ptr(), buf_ori.c_ptr());
	BOOST_REQUIRE_EQUAL(buf_copy_ref.size(), buf_ori.size());
	BOOST_REQUIRE_EQUAL(buf_copy_ptr.c_ptr(), buf_ori.c_ptr());
	BOOST_REQUIRE_EQUAL(buf_copy_ptr.size(), buf_ori.size());
}

BOOST_AUTO_TEST_CASE( gcBuff_cpy )
{
	gcBuff buf(TEST_SIZE);
	buf.cpy(TEST_CSTRING, TEST_CSTRING_SIZE);

	BOOST_REQUIRE_EQUAL(buf.size(), TEST_SIZE);
	BOOST_REQUIRE_EQUAL(buf.c_ptr(), TEST_CSTRING);
}

BOOST_AUTO_TEST_CASE( gcBuff_charArr_cast )
{
	gcBuff buf(TEST_CSTRING, TEST_CSTRING_SIZE);

	BOOST_REQUIRE_EQUAL(static_cast<char*>(buf), TEST_CSTRING);
}

BOOST_AUTO_TEST_CASE( gcBuff_element_access_operator )
{
	gcBuff buf(TEST_STATIC_CSTRING, TEST_CSTRING_SIZE);

	for (int i = 0; i < TEST_CSTRING_SIZE; i++)
		BOOST_REQUIRE_EQUAL(buf[i], TEST_STATIC_CSTRING[i]);
}
