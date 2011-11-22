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


#include "files.h"

std::string GetAppPath(std::string extra)
{
	ERROR_OUTPUT(__func__);
	char result[PATH_MAX] = {0};
	ssize_t count = readlink("/proc/self/exe", result, PATH_MAX);

	if (count <= 0)
	{
		ERROR_OUTPUT("Could not get program directory!");
		return "";
	}
	
	std::string exePath(result);

	for(size_t i = count - 1; i > 0; i--)
	{
		if(result[i] == '/')
		{
			result[i] = '\0';
			break;
		}
	}

	std::string sResult(result);
	std::string parent;
	std::string nextParent;
	
	
	size_t pos = sResult.find_last_of('/');
	size_t nextPos = sResult.find_last_of('/', pos-1);
	size_t size = (pos-nextPos-1);
	
	if (pos != std::string::npos)
	{
		parent = sResult.substr(pos+1, std::string::npos);
	
		if (nextPos != std::string::npos)
			nextParent = sResult.substr(nextPos+1, size);
	}
	
	if (parent == "desura")
	{
		mkdir("bin", S_IRWXU|S_IRWXG|S_IROTH|S_IXOTH);
		
		if (FileExists("bin/desura"))
			rename("bin/desura", "bin/desura_old");
			
		rename(exePath.c_str(), "bin/desura");

		if(extra.size() > 0)
		{
			sResult += "/";
			sResult += extra;
		}
		
		return sResult;
	}

#ifdef DEBUG
	nextParent = "desura";
#endif

	std::string ver = sResult + "/../version";

	//if we are the bootloader and not in a desura folder (note: need to check for version to not screw existing installs)
	if (parent != "bin" || (nextParent != "desura" && !FileExists(ver.c_str())))
	{
		if (FileExists("desura_bin"))
			rename("desura_bin", "desura_bin_old");
		
		// If the end of the path (our name) is not desura, and a 'desura' exists
		std::vector<std::string> pathTokens;
		UTIL::STRING::tokenize(exePath, pathTokens, "/");
		if ((pathTokens.size() > 0 && pathTokens[pathTokens.size() - 1] != "desura") && FileExists("desura"))
			rename("desura", "desura_old");
			
		rename(exePath.c_str(), "desura_bin");
		mkdir("desura", S_IRWXU|S_IRWXG|S_IROTH|S_IXOTH);
		mkdir("desura/bin", S_IRWXU|S_IRWXG|S_IROTH|S_IXOTH);
		
		if (FileExists("bin/desura"))
			rename("bin/desura", "bin/desura_old");
			
		rename("desura_bin", "desura/bin/desura");
	

		if(extra.size() > 0)
		{
			sResult += "/";
			sResult += extra;
		}
		return sResult + "/desura";
	}
	
	// we actually want one folder above where we are due to executable being in desura/bin/
	sResult = sResult.substr(0, pos);

	if(extra.size() > 0)
	{
		sResult += "/";
		sResult += extra;
	}

	ERROR_OUTPUT(sResult.c_str());
	return sResult;
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
