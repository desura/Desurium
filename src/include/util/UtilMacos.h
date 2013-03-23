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

#ifndef DESURA_UTIL_MACOS_H
#define DESURA_UTIL_MACOS_H

#ifdef MACOS

namespace UTIL
{
namespace MAC
{
	bool is64OS();
	uint64 getFreeSpace(const char* path);
	void setConfigValue(const std::string &configKey, const std::string &value);
	void setConfigValue(const std::string &configKey, uint32 value);
	std::wstring getApplicationsPath(std::wstring extra);
	std::wstring getDesktopPath(std::wstring extra);
	gcString getAbsPath(const gcString& path);
	gcString getRelativePath(const gcString &path);
	std::wstring getAppPath(std::wstring extra);
	std::wstring getAppDataPath(std::wstring extra);
	std::wstring getCommonProgramFilesPath(std::wstring extra);
	std::wstring getTempInternetPath(std::wstring extra);
	std::wstring getLocalAppDataPath(std::wstring extra);
	std::wstring getAppInstallPath(std::wstring extra);
	std::wstring getCachePath(std::wstring extra);
	std::string getConfigValue(const std::string &configKey);
	std::string getOSString();
	std::string getCmdStdout(const char* command, int stdErrDest = 0);
	std::string getExecuteDir();
	bool launchFolder(const char* path);
	bool canLaunchBinary(OS::BinType type);
}
}

#endif

#endif