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


#include "windows.h"
#include "ShlObj.h"
#include "Common.h"
#include "UpdateFunctions.h"
#include "umcf/UMcf.h"

extern void SetRegValues();
extern void InstallService();

INT_PTR DisplayUpdateWindow(int updateType);

void InitUpdateLog();
void StopUpdateLog();
void Log(const char* format, ...);

bool CheckCert()
{
#ifndef DESURA_OFFICIAL_BUILD
	return false;
#endif

#ifndef WITH_CODESIGN
	return false;
#endif

	wchar_t exePath[255];
	GetModuleFileNameW(NULL, exePath, 255);

	size_t exePathLen = Safe::wcslen(exePath, 255);
	for (size_t x=exePathLen; x>0; x--)
	{
		if (exePath[x] == '\\')
			break;
		else
			exePath[x] = '\0';
	}

	wchar_t *modules[] = 
	{
		L"desura.exe",
		L"desura_service.exe",
		L"bin\\uicore.dll",
		L"bin\\usercore.dll",
		L"bin\\webcore.dll",
		L"bin\\mcfcore.dll",
		L"bin\\servicecore_c.dll",
	};

	char *moduleName[] = 
	{
		"desura.exe",
		"desura_service.exe",
		"uicore.dll",
		"usercore.dll",
		"webcore.dll",
		"mcfcore.dll",
		"servicecore_c.dll",
	};

	bool allGood = true;
	char msgMsg[1024];
	
	char* curPos = msgMsg;
	size_t curSize = 1024;

	Safe::snprintf(curPos, curSize, "There has been an error validating the Digital Signature for:\n"); 

	curPos+= 62;
	curSize+= 62;

	for (size_t x=0; x<6; x++)
	{
		wchar_t path[255];

		gcString mod(modules[x]);
		Safe::snwprintf(path, 255, L"%s\\%s", exePath, modules[x]);

		uint32 res = UTIL::WIN::validateCert(path);

		if (res == TRUST_E_NOSIGNATURE)
		{
			size_t size = strlen(moduleName[x]);
			Safe::snprintf(curPos, curSize, "\t%s\n", moduleName[x]);
			curPos += size+2;
			curSize -= size+2;
			allGood = false;
		}
	}

	if (allGood)
		return false;

	Safe::snprintf(curPos, curSize,	"\n"
								"It is adviseable to do a full update instead of proceding as\n"
								"you might be running unoffical code.\n"
								"\n"
								"Do you want to do a full update now?");


	LONG res = ::MessageBoxA(NULL, msgMsg, "Desura Error: Certificate Error", MB_YESNOCANCEL|MB_ICONSTOP);

	if (res == IDCANCEL)
		exit(200);
	
	return (res == IDYES);
}

class UpdateLogHandle
{
public:
	UpdateLogHandle()
	{
		InitUpdateLog();
	}

	~UpdateLogHandle()
	{
		StopUpdateLog();
	}
};

int NeedUpdateNonGpl()
{
	UpdateLogHandle ulh;

	std::wstring path = UTIL::OS::getAppDataPath(UPDATEFILE_W);

	if (!FileExists(UPDATEXML_W))
	{
		Log("NeedUpdate: Missing Xml");
		return UPDATE_XML;
	}
	else
	{
		if (FileExists(path.c_str()) && CheckUpdate(path.c_str()))
		{
			Log("NeedUpdate: Missing Mcf");
			return UPDATE_MCF;
		}

		if (!CheckInstall())
		{
			Log("NeedUpdate: Bad Files");
			return UPDATE_FILES;
		}
	}

#ifdef WITH_CODESIGN
	if (CheckCert())
	{
		Log("NeedUpdate: Bad Cert");
		return UPDATE_CERT;
	}
#endif

	return UPDATE_NONE;
}

bool CheckUpdate(const wchar_t* path)
{
	UMcf updateMcf;
	updateMcf.setFile(path);

	return (updateMcf.parseMCF() == UMCF_OK && updateMcf.isValidInstaller());
}

class BadFileLogger : public IBadFileCallback
{
public:
	bool foundBadFile(const wchar_t* szFileName, const wchar_t* szPath) override
	{
		gcString strFullPath("{0}\\{1}", szPath, szFileName);
		Log("Found bad file: %s", strFullPath.c_str());
		
		return false;
	}
};


bool CheckInstall()
{
	UMcf updateMcf;
	
	if (updateMcf.loadFromFile(UPDATEXML_W) != MCF_OK)
		return false;

	BadFileLogger bfl;
	return updateMcf.checkFiles(&bfl);
}

//with full updates we should have admin rights
void FullUpdate()
{
	DeleteFileW(L"desura_old.exe");
	DeleteFileW(L"desura_service_old.exe");

	std::wstring updateFile = UTIL::OS::getAppDataPath(UPDATEFILE_W);
	DeleteFileW(updateFile.c_str());

	int nRes = DisplayUpdateWindow(UPDATE_FILES);
	
	//Critical failure
	if (nRes == -1)
		exit(0);
	
	if (nRes == 2)
		exit(0);

#ifndef DEBUG
	try
	{
		SetRegValues();
		InstallService();
		ServiceUpdate(false);
	}
	catch (gcException &e)
	{
		char msg[255];
		Safe::snprintf(msg, 255, "Failed to Update Desura: %s [%d.%d]", e.getErrMsg(), e.getErrId(), e.getSecErrId());
		::MessageBox(NULL, msg, "Desura Critical Error", MB_OK);
	
		exit(-4);
	}
#endif
}

void McfUpdate()
{
	int nRes = DisplayUpdateWindow(UPDATE_MCF);
	
	//Critical failure
	if (nRes == -1)
		exit(0);
}


void CheckForBadUninstaller()
{
	char exePath[255];
	GetModuleFileName(NULL, exePath, 255);

	size_t exePathLen = strlen(exePath);
	for (size_t x=exePathLen; x>0; x--)
	{
		char c = exePath[x];

		exePath[x] = '\0';
		exePathLen = x;

		if (c == '\\')
			break;
	}

	char * folder = exePath;

	for (size_t x=exePathLen; x>0; x--)
	{
		if (exePath[x] == '\\')
		{
			folder = &exePath[x+1];
			break;
		}
	}	

	if (strcmp(folder, "Desura") != 0)
		DeleteFile("Desura_Uninstall.exe");
}


FILE* g_pUpdateLog = NULL;

void InitUpdateLog()
{
	if (g_pUpdateLog)
		return;

	char path[MAX_PATH];
	SHGetFolderPathA(NULL, CSIDL_COMMON_APPDATA, NULL, SHGFP_TYPE_CURRENT, path);

	char file[255];

	_snprintf_s(file, 255, "%s\\Desura", path);
	CreateDirectory(file, NULL);

	_snprintf_s(file, 255, "%s\\Desura\\DesuraApp", path);
	CreateDirectory(file, NULL);

	_snprintf_s(file, 255, "%s\\Desura\\DesuraApp\\update_log.txt", path);
	fopen_s(&g_pUpdateLog, file, "a");
}

void StopUpdateLog()
{
	if (!g_pUpdateLog)
		return;

	fclose(g_pUpdateLog);
}

void Log(const char* format, ...)
{
	if (!g_pUpdateLog)
		return;

	time_t rawtime;
	struct tm timeinfo;
	char buffer[255];

	time(&rawtime);
	localtime_s(&timeinfo, &rawtime);

	strftime(buffer, 255, "%c:", &timeinfo);
	fprintf(g_pUpdateLog, buffer);

	va_list args;
	va_start(args, format);
	vfprintf(g_pUpdateLog, format, args);
	va_end(args);
}