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

#ifndef DLB_MAIN_H
#define DLB_MAIN_H

#include "Common.h" // ERROR_OUTPUT(), define NIX
#include "SharedObjectLoader.h" // SharedObjectLoader

#define VERSION_MAJOR 1
#define VERSION_MINOR 0
#define VERSION_BUILDNO 0
#define VERSION_EXTEND 0

typedef bool (*UploadCrashFn)(const char* path, const char* user, int build, int branch);

void ShowHelpDialog(const char* msg, const char* url = NULL, const char* type = "--error");

typedef struct
{
	char type;
	char args[1024];
} RestartArg_s;

typedef struct
{
	char type;
	char user[64];
	int branch;
	int build;
	char file[952];
} CrashArg_s;

class UICoreI;

class MainApp
{
public:
	MainApp(int argc, char** argv);
	~MainApp();
	
	int run();
	bool testDeps();
	
protected:
	bool loadUICore();
	void shutdownUICore();
	
	bool loadDumpUploader();
	
	void restartReal(const char* args);
	void restartShib(const char* args);
	static void restart(const char* args, bool real = false);
	static void restartFromUICore(const char* args);
	
	void onCrashShib(const char* path);
	static bool onCrash(const char* path);
	
	void processCrash(CrashArg_s* args = NULL);
	void setUser(const char* user);
	
	static void setCrashSettings(const char* user, bool upload);
	
	friend bool RestartBootloader(const char* args);
	
	bool rootTest();
	bool permTest();
	bool utf8Test();

	bool loadCrashHelper();
	void sendArgs();
	
	void checkUnityWhitelist();
	void setupSharedMem();
	
	int runParent(int pid);
	int runChild(bool usingGDB);
	
private:
	int m_Argc;
	char** m_Argv;

	int m_RestartMem;
	UICoreI* m_pUICore;
	CrashArg_s* m_pCrashArgs;
	
	SharedObjectLoader g_UICoreDll;
	char m_szUser[255];
};

bool RestartBootloader(const char* args = NULL);

#endif
