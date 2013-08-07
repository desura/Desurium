/*
Desura is the leading indie game distribution platform
Copyright (C) 2011 Mark Chandler (Desura Net Pty Ltd)

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>
*/
#include "Common.h"
#include "util/UtilOs.h"

#ifdef NIX
#include "util/UtilLinux.h"
#endif

#ifdef MACOS
  #include "util/UtilMacos.h"
#endif

#ifdef WIN32
  #define  PLATFORM UTIL::WIN
#elif  NIX
  #define  PLATFORM UTIL::LIN
#elif  MACOS
  #define  PLATFORM UTIL::MAC
#else
  #error platform not supported
#endif

#ifdef WIN32
	#include <shlobj.h>
#endif

#ifdef WIN32
#define getSystemPath(id, path) SHGetFolderPathW(NULL, id , NULL, SHGFP_TYPE_CURRENT, path);
#endif

namespace UTIL
{
namespace OS
{
bool is64OS()
{
	return PLATFORM::is64OS();
}

bool isPointOnScreen(int32 x, int32 y)
{
#ifdef WIN32
	return PLATFORM::isPointOnScreen(x, y);
#else
	return true;// TODO LINUX, MACOS
#endif
}

uint64 getFreeSpace(const char* path)
{
	return PLATFORM::getFreeSpace(path);
}

std::string getOSString()
{
#ifdef WIN32
	char buff[255] = {0};
	PLATFORM::getOSString(buff, 255);
 
	return std::string(buff, 255);
#else
	return PLATFORM::getOSString();
#endif
}

void setConfigValue(const std::string &regIndex, const std::string &value, bool expandStr, bool use64bit)
{
#ifdef WIN32
	return PLATFORM::setRegValue(regIndex, value, expandStr, use64bit);
#else
	return PLATFORM::setConfigValue(regIndex, value);
#endif
}

void setConfigValue(const std::string &regIndex, uint32 value, bool use64bit)
{
#ifdef WIN32
	return PLATFORM::setRegValue(regIndex, value, use64bit);
#else
	return PLATFORM::setConfigValue(regIndex, value);
#endif
}

void setConfigBinaryValue(const std::string &regIndex, const char* blob, size_t size, bool use64bit)
{
#ifdef WIN32
	return PLATFORM::setRegBinaryValue(regIndex, blob, size, use64bit);
#else
	ERROR_NOT_IMPLEMENTED;
#endif
}

std::string getConfigValue(const std::string &configKey, bool use64bit)
{
#ifdef WIN32
	return PLATFORM::getRegValue(configKey, use64bit);
#else
	return PLATFORM::getConfigValue(configKey);
#endif
}

std::wstring getCurrentDir(std::wstring extra)
{
#if defined NIX || MACOS
	return PLATFORM::getAppPath(extra);
#else
	wchar_t path[MAX_PATH];
	GetCurrentDirectoryW(MAX_PATH, path);

	gcWString out(path);

	if (extra.size() > 0)
	{
		out += DIRS_WSTR;
		out += extra;
	}

	return out;
#endif
}

#ifdef NIX
	#if \
		!defined(USE_XDG_DIRS) && \
		!defined(USE_SINGLE_HOME_DIR) && \
		!defined(USE_PORTABLE_DIR)
			#error Please select a directory structure for Desura to use!
	#endif
#endif

std::wstring getCachePath(std::wstring extra)
{
#ifdef NIX
	#if defined(USE_XDG_DIRS)
		std::string cachePath = getenv("XDG_CACHE_HOME");
		cachePath.append("/desura");
	#elif defined(USE_SINGLE_HOME_DIR)
		std::string cachePath = getenv("HOME");
		cachePath.append("/.desura/cache");
	#elif defined(USE_PORTABLE_DIR)
		std::string cachePath = UTIL::STRING::toStr(getCurrentDir(L"cache"));
	#endif
	
	if (extra.size() > 0)
		extra.insert(0, DIRS_WSTR);
	
	return UTIL::STRING::toWStr(cachePath) + extra;
#elif MACOS
	return PLATFORM::getCachePath(extra);
#else
	return L"";
//	#error NOT IMPLEMENTED
#endif
}

std::wstring getAppInstallPath(std::wstring extra)
{
#ifdef NIX
	#if defined(USE_XDG_DIRS)
		std::string installPath = getenv("XDG_DATA_HOME");
		installPath.append("/desura");
	#elif defined(USE_SINGLE_HOME_DIR)
		std::string installPath = getenv("HOME");
		installPath.append("/.desura/games");
	#elif defined(USE_PORTABLE_DIR)
		std::string installPath = UTIL::STRING::toStr(getCurrentDir(L"games"));
	#endif
	
	if (extra.size() > 0)
		extra.insert(0, DIRS_WSTR);
	
	return UTIL::STRING::toWStr(installPath) + extra;
#elif MACOS
	return PLATFORM::getAppInstallPath(extra);
#else
	return L"";
//	#error NOT IMPLEMENTED
#endif
}

std::wstring getAppDataPath(std::wstring extra)
{
#ifdef NIX
	#if defined(USE_XDG_DIRS)
		std::string configPath = getenv("XDG_CONFIG_HOME");
		configPath.append("/desura");
	#elif defined(USE_SINGLE_HOME_DIR)
		std::string configPath = getenv("HOME");
		configPath.append("/.desura");
	#elif defined(USE_PORTABLE_DIR)
		std::string configPath = UTIL::STRING::toStr(getCurrentDir(L"config"));
	#endif
	
	if (extra.size() > 0)
		extra.insert(0, DIRS_WSTR);
	
	return UTIL::STRING::toWStr(configPath) + extra;
#elif MACOS
	return PLATFORM::getAppDataPath(extra);
#else
	wchar_t path[MAX_PATH];
	getSystemPath(CSIDL_COMMON_APPDATA, path);

	gcWString out(path);

	out += DIRS_WSTR;
	out += COMMONAPP_PATH_W;

	if (extra.size() > 0)
	{
		out += DIRS_WSTR;
		out += extra;
	}

	return out;
#endif
}

std::wstring getLocalAppDataPath(std::wstring extra)
{
#ifdef WIN32
	wchar_t path[MAX_PATH];
	getSystemPath(CSIDL_LOCAL_APPDATA, path);
	
	std::wstring out(path);

	out += L"\\";
	out += COMMONAPP_PATH_W;

	if (extra.size() > 0)
	{
		out += L"\\";
		out += extra;
	}

	return out;
#elif MACOS
	return PLATFORM::getLocalAppDataPath(extra);
#else //TODO LINUX
	return L"";
#endif
}

std::wstring getTempInternetPath(std::wstring extra)
{
#ifdef WIN32
	wchar_t path[MAX_PATH];
	getSystemPath(CSIDL_INTERNET_CACHE, path);
	
	std::wstring out(path);

	out += L"\\";
	out += COMMONAPP_PATH_W;

	if (extra.size() > 0)
	{
		out += L"\\";
		out += extra;
	}

	return out;
#elif MACOS
	return PLATFORM::getTempInternetPath(extra);
#else //TODO LINUX
	return L"";
#endif
}



std::wstring getCommonProgramFilesPath(std::wstring extra)
{
#ifdef WIN32
	wchar_t path[MAX_PATH];
	getSystemPath(CSIDL_PROGRAM_FILES_COMMON, path);
	
	std::wstring out(path);

	out += L"\\Desura";

	if (extra.size() > 0)
	{
		out += L"\\";
		out += extra;
	}

	return out;
#elif MACOS
	return PLATFORM::getCommonProgramFilesPath(extra);
#else //TODO LINUX
	return L"";
#endif
}

std::wstring getStartMenuProgramsPath(std::wstring extra)
{
#ifdef WIN32
	wchar_t path[MAX_PATH];
	getSystemPath(CSIDL_COMMON_PROGRAMS, path);
	
	std::wstring out(path);

	if (extra.size() > 0)
	{
		out += L"\\";
		out += extra;
	}

	return out;
#else
	return PLATFORM::getApplicationsPath(extra);
#endif
}

std::wstring getDesktopPath(std::wstring extra)
{
#ifdef WIN32
	wchar_t path[MAX_PATH];
	getSystemPath(CSIDL_DESKTOP, path);
	
	std::wstring out(path);

	if (extra.size() > 0)
	{
		out += L"\\";
		out += extra;
	}

	return out;
#else
	return PLATFORM::getDesktopPath(extra);
#endif
}


gcString getAbsPath(const gcString& path)
{
	return PLATFORM::getAbsPath(path);
}

gcString getRelativePath(const gcString &path)
{
	return PLATFORM::getRelativePath(path);
}

std::string getCmdStdout(const char* command, int stdErrDest)
{
	return PLATFORM::getCmdStdout(command, stdErrDest);
}

std::string getExecuteDir()
{
	return PLATFORM::getExecuteDir();
}

bool launchFolder(const char* path)
{
	return PLATFORM::launchFolder(path);
}

BinType getFileType(const char* buff, size_t buffSize)
{
	if (buffSize < 2)
		return BinType::UNKNOWN;	
	
	if (strncmp(buff, "#!", 2) == 0)
		return BinType::SH;
		
	if (strncmp(buff, "MZ", 2) == 0)
		return BinType::WIN32;
	
	if (buffSize < 5)
		return BinType::UNKNOWN;

	if (strncmp(buff+1, "ELF", 3) == 0)
	{
		if (*(buff + 4) == (char)0x01)
			return BinType::ELF32;

		if (*(buff + 4) == (char)0x02)
			return BinType::ELF64;
	}
	
	return BinType::UNKNOWN;
}

bool canLaunchBinary(BinType type)
{
	return PLATFORM::canLaunchBinary(type);
}


}
}
