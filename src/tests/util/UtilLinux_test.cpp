#define BOOST_TEST_MAIN
#include <boost/test/unit_test.hpp>

#warning todo: add boost checks for all util calls

#include <boost/filesystem.hpp>

#include "Common.h"
#include "util/UtilLinux.h"
#include <iostream>

using namespace boost;
using namespace boost::unit_test;
using namespace UTIL::LIN;

BOOST_AUTO_TEST_CASE (UTIL_LIN_getAppPath)
{
	BOOST_REQUIRE(getAppPath(L"") == filesystem::current_path().wstring() );
	BOOST_REQUIRE(getAppPath(L"test/path") == (filesystem::current_path() / "test/path").wstring() );
}

BOOST_AUTO_TEST_CASE (Util_Lin_String_Output)
{
	std::cout << "-- Testing UTIL::LIN::getDesktopPath(...) --\n";
	std::wcout << L"UTIL::LIN::getDesktopPath(L\"\") result: " << getDesktopPath(L"") << std::endl;
	std::wcout << L"UTIL::LIN::getDesktopPath(L\"test/path\") result: " << getDesktopPath(L"test/path") << std::endl;

	//std::cout << "-- Testing UTIL::LIN::getOSString(...) --\n";
	//std::cout << "UTIL::LIN::getOSString() result: " << UTIL::LIN::getOSString() << std::endl;

	/*std::cout << "-- Testing UTIL::LIN::getFreeSpace(...) --\n";
	std::cout << "UTIL::LIN::getFreeSpace(\"/\") result: " << UTIL::LIN::getFreeSpace("/") << std::endl;
	*/
	BOOST_REQUIRE(getFreeSpace("/") != 0);
	/*std::cout << "-- Testing UTIL::LIN::getCmdStdout(...) --\n";
	std::cout << "UTIL::LIN::getCmdStdout(\"xdg-user-dir DESKTOP\",0) result: " << UTIL::LIN::getCmdStdout("xdg-user-dir DESKTOP",1) << std::endl;
	*/
	//BOOST_REQUIRE(getCmdStdout("xdg-user-dir DESKTOP",0) != "");

	/*std::cout << "-- Testing UTIL::LIN::getApplicationsPath(...) --\n";
	std::wcout << L"UTIL::LIN::getApplicationsPath(L\"\") result: " << getApplicationsPath(L"") << std::endl;
	std::wcout << L"UTIL::LIN::getApplicationsPath(L\"test/path\") result: " << getApplicationsPath(L"test/path") << std::endl;
	*/
	//BOOST_REQUIRE(getApplicationsPath(L"") == L"~/.local/share/applications");

	std::cout << "-- Testing UTIL::LIN::getExecuteDir(...) --\n";
	std::string execpath = getExecuteDir();
	// First output the received path
	std::cout<< "UTIL::LIN::getExecuteDir(L\"\") result: " << execpath << std::endl;
	// Secondly check if getExecuteDir() returns a consistent path
	for(int i = 0; i < 100; ++i) {
		BOOST_REQUIRE(getExecuteDir() == execpath);
	}

	std::cout << "-- Testing UTIL::LIN::getAppPath(...) --\n";
	std::wcout << L"UTIL::LIN::getAppPath(L\"\") result: " << getAppPath(L"") << std::endl;
	std::wcout << L"UTIL::LIN::getAppPath(L\"test/path\") result: " << getAppPath(L"test/path") << std::endl;
	
}
