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
#include "MiniDumpGenerator_Extern.h"
#include "SharedObjectLoader.h"

#include <process.h>
#include <Psapi.h>
#include <time.h>


BOOL CALLBACK MiniDumpCallbackFilter(PVOID pParam, const PMINIDUMP_CALLBACK_INPUT pInput, PMINIDUMP_CALLBACK_OUTPUT pOutput) 
{
	MiniDumpGenerator_Extern* mdge = (MiniDumpGenerator_Extern*)pParam;
	return mdge->callbackFilter(pInput, pOutput);
}


MiniDumpGenerator_Extern::MiniDumpGenerator_Extern(const char* exeName, const char* savePath)
:	m_szExeName(exeName),
	m_szSavePath(savePath)
{
	UTIL::FS::recMakeFolder(UTIL::FS::PathWithFile(m_szSavePath));
}

MiniDumpGenerator_Extern::~MiniDumpGenerator_Extern()
{

}


int MiniDumpGenerator_Extern::generate()
{
	generateFileName();
	gcString outPath = m_szSavePath + DIRS_STR + m_szDumpFile;

	DWORD processId = 0;
	HANDLE process = findProcess(processId);

	if (!process || processId == 0)
		return -1;

	char exePath[255];
	GetModuleFileNameExA(process, NULL, exePath, 255);

	UTIL::FS::Path path(exePath, "", true);
	m_szRootPath = path.getFolderPath();

	SharedObjectLoader sol;

	if (!sol.load("dbghelp-desura.dll"))
	{
		//fall back to default one
		if (!sol.load("dbghelp.dll"))
			return false;
	}

	MINIDUMPWRITEDUMP pDump = sol.getFunction<MINIDUMPWRITEDUMP>("MiniDumpWriteDump");

	if (!pDump)
		return 0;

	UTIL::FS::FileHandle fh;

	try
	{
		fh.open(outPath.c_str(), UTIL::FS::FILE_WRITE);
	}
	catch (gcException)
	{
		return 0;
	}

	return saveDump(pDump, process, processId, (HANDLE)fh.getHandle())?1:0;
}

bool MiniDumpGenerator_Extern::saveDump(MINIDUMPWRITEDUMP pDump, HANDLE process, DWORD processId, HANDLE hFile)
{
	if (hFile == INVALID_HANDLE_VALUE)
		return false;

	MINIDUMP_CALLBACK_INFORMATION mci; 

	mci.CallbackRoutine     = MiniDumpCallbackFilter; 
	mci.CallbackParam       = this; 

	MINIDUMP_TYPE mdt       = (MINIDUMP_TYPE)(MiniDumpWithPrivateReadWriteMemory | 
												MiniDumpWithDataSegs | 
												MiniDumpWithHandleData |
												MiniDumpWithFullMemoryInfo | 
												MiniDumpWithThreadInfo | 
												MiniDumpWithProcessThreadData |
												MiniDumpWithUnloadedModules ); 

	//write the dump
	BOOL bOK = pDump(process, processId, hFile, mdt, NULL, NULL, &mci);
	return bOK?true:false;
}



void MiniDumpGenerator_Extern::generateFileName()
{
	time_t rawtime;
	struct tm timeinfo = {0};

	time(&rawtime);
	localtime_s(&timeinfo, &rawtime);

	char dump[255]={0};
	Safe::snprintf(dump, 255, "%s_%04d%02d%02d_%02d%02d%02d.dmp", m_szExeName.c_str(), timeinfo.tm_year+1900, timeinfo.tm_mon, timeinfo.tm_mday,  timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
	m_szDumpFile = dump;
}

HANDLE MiniDumpGenerator_Extern::findProcess(DWORD &processId)
{
	unsigned long aProcesses[1024];
	unsigned long cbNeeded;
	unsigned long cProcesses;

	if(!EnumProcesses(aProcesses, sizeof(aProcesses), &cbNeeded))
		return false;

	cProcesses = cbNeeded / sizeof(unsigned long);

	for (unsigned int i = 0; i<cProcesses; i++)
	{
		if(aProcesses[i] == 0)
			continue;

		HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION|PROCESS_VM_READ|THREAD_ALL_ACCESS, 0, aProcesses[i]);

		if (!hProcess)
			continue;

		char buffer[50] = {0};
		GetModuleBaseNameA(hProcess, 0, buffer, 50);

		if(m_szExeName == buffer)
		{
			processId = aProcesses[i];
			return hProcess;
		}
		else
		{
			CloseHandle(hProcess);
		}
	}

	return NULL;
}

///////////////////////////////////////////////////////////////////////////////
// Custom minidump callback	
// Code from: http://www.debuginfo.com/examples/src/effminidumps/MidiDump.cpp
///////////////////////////////////////////////////////////////////////////////
BOOL MiniDumpGenerator_Extern::callbackFilter(const PMINIDUMP_CALLBACK_INPUT pInput, PMINIDUMP_CALLBACK_OUTPUT pOutput)
{

	BOOL bRet = FALSE; 

	// Check parameters 
	if (pInput == 0) 
		return FALSE; 

	if (pOutput == 0) 
		return FALSE; 

	// Process the callbacks 
	switch(pInput->CallbackType) 
	{
		case IncludeModuleCallback: bRet = TRUE; break;		// Include the module into the dump
		case IncludeThreadCallback: bRet = TRUE; break;		// Include the thread into the dump 
		case ThreadCallback:		bRet = TRUE; break;		// Include all thread information into the minidump
		case ThreadExCallback:		bRet = TRUE; break;		// Include this information 
		case MemoryCallback:		bRet = FALSE; break;	// We do not include any information here -> return FALSE 

		case ModuleCallback: 
		{
			// Are data sections available for this module ? 
			if (pOutput->ModuleWriteFlags & ModuleWriteDataSeg) 
			{
				// Yes, they are, but do we need them? 
				if (!isDataSectionNeeded(pInput->Module.FullPath)) 
					pOutput->ModuleWriteFlags &= (~ModuleWriteDataSeg); 
			}

			bRet = TRUE; 
		}
		break;
	}

	return bRet; 
}

bool MiniDumpGenerator_Extern::isDataSectionNeeded(const wchar_t* pModuleName) 
{
	if(pModuleName == 0) 
		return false; 

	gcString szMoudle(pModuleName);

	if (szMoudle == "ntdll")
		return true;

	gcString binFolder = m_szRootPath + DIRS_STR + "bin";

	std::vector<UTIL::FS::Path> files;
	std::vector<std::string> extFilter;

	extFilter.push_back("dll");
	extFilter.push_back("exe");

	UTIL::FS::getAllFiles(UTIL::FS::PathWithFile(binFolder), files, &extFilter);
	UTIL::FS::getAllFiles(UTIL::FS::PathWithFile(m_szRootPath), files, &extFilter);

	for (size_t x=0; x<files.size(); x++)
	{
		if (files[x].getFile().getFile() == szMoudle)
			return true;
	}

	return false; 
}

const char* MiniDumpGenerator_Extern::getSavedFile()
{
	return m_szDumpFile.c_str();
}
