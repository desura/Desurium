#include <gtest/gtest.h>

#include "Common.h"
#include "util/UtilMisc.h"

namespace UnitTest
{
	TEST(UtilDateTime, fixDateTimeString_valid_string)
	{
		std::string input_string("20131025T123456");
		std::string expected_result(input_string);

		std::string s = UTIL::MISC::fixDateTimeString(input_string.c_str());

		ASSERT_STREQ(expected_result.c_str(), s.c_str());
	}

	TEST(UtilDateTime, fixDateTimeString_missing_t)
	{
		std::string input_string("20150410345612");
		std::string expected_result("20150410T345612");

		std::string s = UTIL::MISC::fixDateTimeString(input_string.c_str());

		ASSERT_STREQ(expected_result.c_str(), s.c_str());
	}

	TEST(UtilDateTime, fixDateTimeString_short_string)
	{
		std::string input_string("20110708");
		std::string expected_result("20110708T000000");

		std::string s = UTIL::MISC::fixDateTimeString(input_string.c_str());

		ASSERT_STREQ(expected_result.c_str(), s.c_str());
	}

	TEST(UtilDateTime, fixDateTimeString_very_short_string)
	{
		std::string input_string("201007");
		std::string expected_result("");

		std::string s = UTIL::MISC::fixDateTimeString(input_string.c_str());

		ASSERT_STREQ(expected_result.c_str(), s.c_str());
	}

	TEST(UtilDateTime, fixDateTimeString_simple_day_diff)
	{
		std::string dateTime1("20100101T000000");
		std::string dateTime2("20100102T000000");

		unsigned int days = 9999;
		unsigned int hours = 1000;
		bool dtIsUTC = false;

		UTIL::MISC::getTimeDiff(dateTime1.c_str(), dateTime2.c_str(), days, hours, dtIsUTC);

		int expected_days = 1;
		int expected_hours = 0;

		ASSERT_EQ(expected_days, days);
		ASSERT_EQ(expected_hours, hours);
	}

	TEST(UtilDateTime, fixDateTimeString_less_simple_day_diff)
	{
		std::string dateTime1("20100430T000000");
		std::string dateTime2("20100502T000000");

		unsigned int days = 9999;
		unsigned int hours = 1000;
		bool dtIsUTC = false;

		UTIL::MISC::getTimeDiff(dateTime1.c_str(), dateTime2.c_str(), days, hours, dtIsUTC);

		int expected_days = 2;
		int expected_hours = 0;

		ASSERT_EQ(expected_days, days);
		ASSERT_EQ(expected_hours, hours);
	}

	TEST(UtilDateTime, fixDateTimeString_less_simple_day_diff_invalid_dates)
	{
		std::string dateTime1("20100731000000");
		std::string dateTime2("20100805000000");

		unsigned int days = 9999;
		unsigned int hours = 1000;
		bool dtIsUTC = false;

		UTIL::MISC::getTimeDiff(dateTime1.c_str(), dateTime2.c_str(), days, hours, dtIsUTC);

		int expected_days = 5;
		int expected_hours = 0;

		ASSERT_EQ(expected_days, days);
		ASSERT_EQ(expected_hours, hours);
	}

	TEST(UtilDateTime, fixDateTimeString_less_simple_day_diff_short_dates)
	{
		std::string dateTime1("20101130");
		std::string dateTime2("20101220");

		unsigned int days = 9999;
		unsigned int hours = 1000;

		bool dtIsUTC = false;
		UTIL::MISC::getTimeDiff(dateTime1.c_str(), dateTime2.c_str(), days, hours, dtIsUTC);

		int expected_days = 20;
		int expected_hours = 0;

		ASSERT_EQ(expected_days, days);
		ASSERT_EQ(expected_hours, hours);

		dtIsUTC = true;
		UTIL::MISC::getTimeDiff(dateTime1.c_str(), dateTime2.c_str(), days, hours, dtIsUTC);

		expected_days = 20;
		expected_hours = 0;

		ASSERT_EQ(expected_days, days);
		ASSERT_EQ(expected_hours, hours);
	}
}
