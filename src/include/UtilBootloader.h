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

#ifndef DESURA_BOOTLOADER_UTIL_H
#define DESURA_BOOTLOADER_UTIL_H
#ifdef _WIN32
#pragma once
#endif

#ifndef _WIN32_WINNT		// Allow use of features specific to Windows XP or later.                   
#define _WIN32_WINNT 0x0501	// Change this to the appropriate value to target other versions of Windows.
#endif	


#ifdef _WIN32
#include <windows.h> 
#endif

#include <vector>

namespace BootLoaderUtil
{
	unsigned int GetOSId();

	void SetCurrentDir();
	bool SetDllDir(const char* dir);

	void InitCommonControls();
	void WaitForDebugger();

	bool Restart(const char* args = NULL, bool wait = true);
	bool RestartAsAdmin(const char* args = NULL);
	bool RestartAsNormal(const char* args = NULL);

	void GetLastFolder(char* dest, size_t destSize, const char* src);
	bool IsExeRunning(char* pName);

#ifdef WIN32
	bool CheckForOtherInstances(HINSTANCE hinstant);
	void WaitForOtherInstance(HINSTANCE hinstant);
#else
	bool CheckForOtherInstances(int hinstant);
	void WaitForOtherInstance(int hinstant);
#endif
	void WaitForOtherInstance(char* name);

	bool StartProcess(const char* name, const char* args);

	void PreReadImage(const char* imagePath);
}

#endif //DESURA_BOOTLOADER_UTIL_H
