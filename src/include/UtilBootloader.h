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

#ifndef DONT_INCLUDE_AFXWIN
#ifdef _WIN32
#include <afxwin.h> 
#endif
#else
#ifdef _WIN32
#include <windows.h> 
#endif
#endif


#include <vector>

namespace BootLoaderUtil
{
	class CMDArgInternal;

#ifndef DONT_INCLUDE_AFXWIN
#ifdef _WIN32
	class CDummyWindow : public CWnd
	{
	  public:
		CDummyWindow(HWND hWnd);
		~CDummyWindow();
	};
#endif
#endif

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

	void ConvertToArgs(const char* args, std::vector<char*> &argv);

	class CMDArgs
	{
	public:
		CMDArgs(const char* args);
		~CMDArgs();

		void addValue(const char* name, const char* value = NULL);

		bool hasArg(const char* name);
		bool hasValue(const char* name);

		const char** getArgv();
		int getArgc();

		int getInt(const char* name);
		void getString(const char* name, char* buff, size_t buffSize);

	private:
		void process();
		CMDArgInternal* m_pInternal;
	};
}

#endif //DESURA_BOOTLOADER_UTIL_H
