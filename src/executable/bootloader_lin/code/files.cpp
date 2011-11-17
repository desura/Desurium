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

void UpdateIcons(bool updateDesktop)
{
	ERROR_OUTPUT(__func__);

	gcString curPath = UTIL::OS::getCurrentDir();
	gcString savedPath;
	
	try
	{
		savedPath = UTIL::OS::getConfigValue("InstallPath");
	}
	catch (std::exception &e)
	{
	}
	
	if (savedPath == curPath)
		return;

	gcString out;

	if (FileExists("desura.desktop"))
	{
#ifndef DEBUG
		gcString cmd("xdg/xdg-desktop-menu uninstall desura.desktop");
		system(cmd.c_str());
#endif
	}	
	
	out.reserve(2000);
	
	out += "#!/usr/bin/env xdg-open\n";
	out += "[Desktop Entry]\n";
	out += "Version=1.0\n";
	out += "Type=Application\n";
	out += "Name=Desura\n";
	out += "GenericName=Game Manager\n";
	out += "Comment=Download and play the best games\n";
	out += gcString("Exec={0}/desura {1}U\n", curPath, '%');
	out += gcString("TryExec={0}/bin/desura\n", curPath);
	out += gcString("Path={0}\n", curPath);
	out += "MimeType=x-scheme-handler/desura;\n";
	out += gcString("Icon={0}/data/desura.png\n", curPath);
	out += "Terminal=false\n";
	out += "Categories=Game;\n";
	out += "StartupNotify=false\n";

	FILE *fh = fopen("desura.desktop", "w");
	if (fh)
	{
		unsigned int size = fprintf(fh, out.c_str());
		if (size != out.size())
			fprintf(stderr, "Failed to update menu icon. Error writing to file.");
		fclose(fh);
		system("chmod a+x desura.desktop 2>/dev/null 1>/dev/null");

	#ifndef DEBUG
		system("xdg/xdg-desktop-menu install --novendor --mode user desura.desktop 2>/dev/null 1>/dev/null");
	
		system(gcString("gconftool-2 -s /desktop/gnome/url-handlers/desura/enabled --type bool true 2>/dev/null 1>/dev/null").c_str());
		system(gcString("gconftool-2 -s /desktop/gnome/url-handlers/desura/command --type string \"{0}/desura {1}s\" 2>/dev/null 1>/dev/null", curPath, '%').c_str());
		system(gcString("gconftool-2 -s /desktop/gnome/url-handlers/desura/needs_terminal --type bool false 2>/dev/null 1>/dev/null").c_str());
		
		// Update menus if we can
		system("update-menus 2>/dev/null 1>/dev/null");
	#endif
		try
		{
			UTIL::OS::setConfigValue("InstallPath", curPath);
		}
		catch (std::exception &e)
		{
		}
	}
	else
	{
		fprintf(stderr, "Failed to update desktop icon. Could not open file for writing");
	}
	
	char *env = getenv("XDG_DESKTOP_DIR");
	std::string desktopFile;
	
	if (env)
		desktopFile = env;
	
	if (desktopFile.size() == 0)
	{
		env = getenv("HOME");
		if (env)
		{
			desktopFile = env;
			desktopFile += "/Desktop";
		}
		else
		{
			return;
		}
	}
		
	desktopFile += "/desura.desktop";
	
	if(FileExists(desktopFile.c_str()) || updateDesktop)
	{
		FILE *fh = fopen(desktopFile.c_str(), "w");
		if (fh)
		{
			unsigned int size = fprintf(fh, "%s", out.c_str());
			if (size != out.size())
				fprintf(stderr, "Failed to update menu icon. Error writing to file.");
			fclose(fh);
			system(gcString("chmod a+x {0} 2>/dev/null 1>/dev/null", desktopFile.c_str()).c_str());
		}
		else
		{
			fprintf(stderr, "Failed to update desktop icon. Could not open file for writing");
		}
	}
}
