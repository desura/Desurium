
#include <gtest/gtest.h>

#include "Common.h"
#include "util/UtilFs.h"

using namespace UTIL::STRING;

namespace UnitTest
{
	TEST(sanitizeFilePath, slash1)
	{
		std::string str = "/hi/ho/hihihi\\";

		ASSERT_STREQ("/hi/ho/hihihi/", sanitizeFilePath(str, '/').c_str());
	}

	TEST(sanitizeFilePath, slash_with_dot)
	{
		std::string str = "/hi/./ho/hihihi\\";

		ASSERT_STREQ("/hi/./ho/hihihi/", sanitizeFilePath(str, '/').c_str());
	}

	TEST(sanitizeFilePath, slash_with_dots)
	{
		std::string str = "/hi/../ho/hihihi\\";

		ASSERT_STREQ("/hi/../ho/hihihi/", sanitizeFilePath(str, '/').c_str());
	}
}

