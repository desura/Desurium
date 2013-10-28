#include <gtest/gtest.h>

#include "Common.h"
#include "util/MD5Progressive.h"

#define EMPTY_MD5 "d41d8cd98f00b204e9800998ecf8427e"

static const char TEST_CSTRING[] = "this is a little test";
#define TEST_CSTRING_SIZE sizeof(TEST_CSTRING)
#define TEST_MD5_RESULT "419d085dcebc10a7e6d7115216e9820d"

namespace UnitTest
{
	TEST(MD5Progressive, cons)
	{
		MD5Progressive md5;
		ASSERT_STREQ(md5.finish().c_str(), EMPTY_MD5);
	}

	TEST(MD5Progressive, update)
	{
		MD5Progressive md5;
		md5.update(TEST_CSTRING, TEST_CSTRING_SIZE - 1);
		ASSERT_STREQ(md5.finish().c_str(), TEST_MD5_RESULT);
	}
}

