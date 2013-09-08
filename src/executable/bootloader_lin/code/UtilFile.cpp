/*
Desura is the leading indie game distribution platform
Copyright (C) Jookia <166291@gmail.com>
          (C) Mark Chandler <mark@moddb.com>

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


#include "UtilFile.h"

std::string GetAppPath(std::string extra)
{
	ERROR_OUTPUT(__func__);
	char result[PATH_MAX] = {0};
	ssize_t count = readlink("/proc/self/exe", result, PATH_MAX);

	if (count <= 0)
	{
		ERROR_OUTPUT("Could not read /proc/self/exe!");
		return "";
	}
	
	std::string exePath(result);

	// Cut off the filename.
	for(size_t i = count - 1; i > 0; i--)
	{
		if(result[i] == '/')
		{
			result[i] = '\0';
			break;
		}
	}

	return result;
}

std::string GetAppDataPath(std::string extra)
{
	ERROR_OUTPUT(__func__);

	std::string appDataPath = GetAppPath();
	appDataPath += STR_APPDATA;
	
	if (extra.size() > 0)
	{
		appDataPath += "/";
		appDataPath += extra;
	}
	
	return appDataPath;	
}	

bool ChangeToAppDir()
{
	ERROR_OUTPUT(__func__);
	std::string appPath = GetAppPath();
	ERROR_OUTPUT(appPath.c_str());
	if(chdir(appPath.c_str()) != 0)
	{
		ERROR_OUTPUT("Failed to change working directory!");
		return false;
	}

	return true;
}

bool FileExists(const char* file) 
{
	char buffer[PATH_MAX];
	snprintf(buffer, PATH_MAX, "%s (%s)", __func__, file);
	ERROR_OUTPUT(buffer);
	
	if (!file)
		return false;

	struct stat stFileInfo;
	int intStat;

	// Attempt to get the file attributes
	intStat = stat(file, &stFileInfo);

	if (intStat == 0) 
		return true;
	else 
		return false;
}

bool DeleteFile(const char* file)
{
	char buffer[PATH_MAX];
	snprintf(buffer, PATH_MAX, "%s (%s)", __func__, file);
	ERROR_OUTPUT(buffer);

	if (!file)
		return false;

	return (remove(file) == 0);
}

bool DeleteFile(const wchar_t* file)
{
	char buffer[PATH_MAX];
	snprintf(buffer, PATH_MAX, "%s (%ls)", __func__, file);
	ERROR_OUTPUT(buffer);

	char path[PATH_MAX];
	if (wcstombs(path, file, PATH_MAX) == UINT_MAX)
	{
		ERROR_OUTPUT("Couldn't convert multibyte string to wide-character array!");
		return false;
	}

	DeleteFile(path);

	return true;
}
