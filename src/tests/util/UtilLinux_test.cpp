#define BOOST_TEST_MAIN
#include <boost/test/unit_test.hpp>

#include "Common.h"
#include "util/UtilLinux.h"
#include <iostream>

using namespace boost::unit_test;

BOOST_AUTO_TEST_CASE (Util_Lin_String_Output)
{

	std::cout << "-- Testing UTIL::LIN::getAppPath(...) --\n";
	std::wcout << L"UTIL::LIN::getAppPath(L\"\") result: " << UTIL::LIN::getAppPath(L"") << std::endl;
	std::wcout << L"UTIL::LIN::getAppPath(L\"test/path\") result: " << UTIL::LIN::getAppPath(L"test/path") << std::endl;

	std::cout << "-- Testing UTIL::LIN::getDesktopPath(...) --\n";
	std::wcout << L"UTIL::LIN::getDesktopPath(L\"\") result: " << UTIL::LIN::getDesktopPath(L"") << std::endl;
	std::wcout << L"UTIL::LIN::getDesktopPath(L\"test/path\") result: " << UTIL::LIN::getDesktopPath(L"test/path") << std::endl;

	std::cout << "-- Testing UTIL::LIN::getOSString(...) --\n";
	//std::cout << "UTIL::LIN::getOSString() result: " << UTIL::LIN::getOSString() << std::endl;

	std::cout << "-- Testing UTIL::LIN::getFreeSpace(...) --\n";
	std::cout << "UTIL::LIN::getFreeSpace(\"/\") result: " << UTIL::LIN::getFreeSpace("/") << std::endl;

	std::cout << "-- Testing UTIL::LIN::getCmdStdout(...) --\n";
	//std::cout << "UTIL::LIN::getCmdStdout(\"ping -c 4\",0) result: " << UTIL::LIN::getCmdStdout("ping -c 4",0) << std::endl;
}
