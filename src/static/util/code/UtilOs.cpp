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
#ifdef WIN32
	return UTIL::WIN::is64OS();
#else
	return UTIL::LIN::is64OS();
#endif
}

bool isPointOnScreen(int32 x, int32 y)
{
#ifdef WIN32
	return UTIL::WIN::isPointOnScreen(x, y);
#endif
#ifdef NIX
	return true;// TODO LINUX
#endif
}

uint64 getFreeSpace(const char* path)
{
#ifdef WIN32
	return UTIL::WIN::getFreeSpace(path);
#endif
#ifdef NIX
	return UTIL::LIN::getFreeSpace(path);
#endif
}

std::string getOSString()
{
#ifdef WIN32
	char buff[255] = {0};
	UTIL::WIN::getOSString(buff, 255);
 
	return std::string(buff, 255);
#endif

#ifdef NIX
	return UTIL::LIN::getOSString();
#endif
}

void setConfigValue(const std::string &regIndex, const std::string &value, bool expandStr, bool use64bit)
{
#ifdef WIN32
	return UTIL::WIN::setRegValue(regIndex, value, expandStr, use64bit);
#else
	return UTIL::LIN::setConfigValue(regIndex, value);
#endif
}

void setConfigValue(const std::string &regIndex, uint32 value, bool use64bit)
{
#ifdef WIN32
	return UTIL::WIN::setRegValue(regIndex, value, use64bit);
#else
	return UTIL::LIN::setConfigValue(regIndex, value);
#endif
}

void setConfigBinaryValue(const std::string &regIndex, const char* blob, size_t size, bool use64bit)
{
#ifdef WIN32
	return UTIL::WIN::setRegBinaryValue(regIndex, blob, size, use64bit);
#else
	ERROR_NOT_IMPLEMENTED;
#endif
}

std::string getConfigValue(const std::string &configKey, bool use64bit)
{
#ifdef WIN32
	return UTIL::WIN::getRegValue(configKey, use64bit);
#endif

#ifdef NIX
	return UTIL::LIN::getConfigValue(configKey);
#endif
}

std::wstring getCurrentDir(std::wstring extra)
{
#ifdef NIX
	return UTIL::LIN::getAppPath(extra);
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

std::wstring getDataPath(std::wstring extra)
{
	std::wstring dataDir = L""; // Use to be 'data'.
	
	return getCurrentDir(dataDir + extra);
}

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
		std::string cachePath = UTIL::STRING::toStr(getCurrentDir(L"/cache"));
	#endif
	
	return UTIL::STRING::toWStr(cachePath) + extra;
#else
	#error NOT IMPLEMENTED
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
		std::string installPath = UTIL::STRING::toStr(getCurrentDir(L"/games"));
	#endif
	
	return UTIL::STRING::toWStr(installPath) + extra;
#else
	#error NOT IMPLEMENTED
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
		std::string configPath = UTIL::STRING::toStr(getCurrentDir(L"/config"));
	#endif
	
	return UTIL::STRING::toWStr(configPath) + extra;
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
	return UTIL::LIN::getApplicationsPath(extra);
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
	return UTIL::LIN::getDesktopPath(extra);
#endif
}


gcString getAbsPath(const gcString& path)
{
#ifdef WIN32
	return UTIL::WIN::getAbsPath(path);
#else
	return UTIL::LIN::getAbsPath(path);
#endif
}

gcString getRelativePath(const gcString &path)
{
#ifdef WIN32
	return UTIL::WIN::getRelativePath(path);
#else
	return UTIL::LIN::getRelativePath(path);
#endif
}


}
}
