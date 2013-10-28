#include <gtest/gtest.h>

#include "Common.h"
#include "util/gcBuff.h"

#define TEST_SIZE 20
#define TEST_CSTRING "this is a test!"
#define TEST_CSTRING_SIZE sizeof(TEST_CSTRING)
const char TEST_STATIC_CSTRING[] = TEST_CSTRING;

namespace UnitTest
{
	TEST(gcBuff, constructor)
	{
		//construct our objects
		gcBuff buf1(TEST_SIZE);
		gcBuff buf2(TEST_CSTRING, TEST_CSTRING_SIZE);

		ASSERT_EQ(buf1.size(), TEST_SIZE);

		ASSERT_EQ(buf2.size(), TEST_CSTRING_SIZE);
		ASSERT_STREQ(buf2.c_ptr(), TEST_STATIC_CSTRING);
	}

	TEST(gcBuff, copy_constructor)
	{
		gcBuff buf_ori(TEST_CSTRING, TEST_CSTRING_SIZE);
		gcBuff buf_copy_ref(buf_ori);
		gcBuff buf_copy_ptr(&buf_ori);

		ASSERT_STREQ(buf_copy_ref.c_ptr(), buf_ori.c_ptr());
		ASSERT_EQ(buf_copy_ref.size(), buf_ori.size());
		ASSERT_STREQ(buf_copy_ptr.c_ptr(), buf_ori.c_ptr());
		ASSERT_EQ(buf_copy_ptr.size(), buf_ori.size());
	}

	TEST(gcBuff, cpy)
	{
		gcBuff buf(TEST_SIZE);
		buf.cpy(TEST_CSTRING, TEST_CSTRING_SIZE);

		ASSERT_EQ(buf.size(), TEST_SIZE);
		ASSERT_STREQ(buf.c_ptr(), TEST_CSTRING);
	}

	TEST(gcBuff, charArr_cast)
	{
		gcBuff buf(TEST_CSTRING, TEST_CSTRING_SIZE);

		ASSERT_STREQ(static_cast<char*>(buf), TEST_CSTRING);
	}

	TEST(gcBuff, element_access_operator)
	{
		gcBuff buf(TEST_STATIC_CSTRING, TEST_CSTRING_SIZE);

		for (uint32 i = 0; i < TEST_CSTRING_SIZE; i++)
			ASSERT_EQ(buf[i], TEST_STATIC_CSTRING[i]);
	}

}
