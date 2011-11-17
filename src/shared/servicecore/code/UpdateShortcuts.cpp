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

#ifdef WIN32
#include <ShlObj.h>
#endif

#ifdef WIN32
void UpdateShortCuts()
{
	char curDir[255];
	GetCurrentDirectory(255, curDir);

	gcString exe(curDir);
	gcString wd(curDir);

#ifdef DEBUG
	exe += "\\desura-d.exe";
#else
	exe += "\\desura.exe";
#endif

	gcWString folder = UTIL::OS::getConfigValue("HKEY_LOCAL_MACHINE\\SOFTWARE\\Desura\\DesuraApp\\StartMenuFolder");

	if (folder == L"")
		folder == L"Desura";

	gcString startFolder = UTIL::OS::getStartMenuProgramsPath(folder);

	gcString spOne = startFolder + "\\Desura.lnk";
	gcString spTwo = startFolder + "\\Desura (force update).lnk";

	OS_VERSION ver = UTIL::WIN::getOSId();

	bool force = ((ver == WINDOWS_XP || ver == WINDOWS_XP64) && UTIL::FS::isValidFolder(startFolder));

	if (UTIL::FS::isValidFile(spOne) || force)
	{
		UTIL::FS::delFile(spOne);
		UTIL::WIN::createShortCut(gcWString(spOne).c_str(), exe.c_str(), wd.c_str(), "", false);
	}

	if (UTIL::FS::isValidFile(spTwo) || force)
	{
		UTIL::FS::delFile(spTwo);
		UTIL::WIN::createShortCut(gcWString(spTwo).c_str(), exe.c_str(), wd.c_str(), "-forceupdate", true);
	}
}
#else

void UpdateShortCuts()
{
}

#endif
