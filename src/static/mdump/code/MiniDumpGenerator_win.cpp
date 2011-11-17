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
#include "MiniDumpGenerator.h"

#include <shlobj.h>
#include <time.h>
#include "client/windows/handler/exception_handler.h"
#include "common/windows/string_utils-inl.h"
#include "common/windows/guid_string.h"
#include "../../../branding/branding.h"

google_breakpad::ExceptionHandler* MiniDumpGenerator::s_pExceptionHandler = NULL;

class FilterWrapper
{
public:
	EXCEPTION_POINTERS* exinfo;
	MDRawAssertionInfo* assertion;
};

bool FilterCallback(void* context, EXCEPTION_POINTERS* exinfo, MDRawAssertionInfo* assertion)
{
	FilterWrapper fw;
	fw.exinfo = exinfo;
	fw.assertion = assertion;

	MiniDumpGenerator* mdg = (MiniDumpGenerator*)context;
	return mdg->filter(&fw);
}

bool CompleteCallback(const wchar_t* dump_path, const wchar_t* minidump_id, void* context, EXCEPTION_POINTERS* exinfo, MDRawAssertionInfo* assertion, bool succeeded)
{
	FilterWrapper fw;
	fw.exinfo = exinfo;
	fw.assertion = assertion;

	MiniDumpGenerator* mdg = (MiniDumpGenerator*)context;
	return mdg->complete(dump_path, minidump_id, &fw, succeeded);	
}

MiniDumpGenerator::MiniDumpGenerator()
{
	m_szUser = 0;
	
#ifndef DESURA_OFFICAL_BUILD
	m_bNoUpload = true;
#else
	m_bNoUpload = false;
#endif

	m_bCreatedHandle = false;

	if (s_pExceptionHandler)
		return;
		
	setDumpLevel(3);
	m_pCrashCallback = NULL;
	m_bCreatedHandle = true;
	
	wchar_t szDumpPath[MAX_PATH];
	getDumpPath(szDumpPath, MAX_PATH);

	s_pExceptionHandler = new google_breakpad::ExceptionHandler(szDumpPath, &FilterCallback, &CompleteCallback, this, google_breakpad::ExceptionHandler::HANDLER_ALL);
}

MiniDumpGenerator::~MiniDumpGenerator()
{
	if (m_bCreatedHandle)
	{
		delete s_pExceptionHandler;
		s_pExceptionHandler = NULL;
	}
}

void MiniDumpGenerator::showMessageBox(bool state)
{
	m_bShowMsgBox = state;
}

void MiniDumpGenerator::setUpload(bool state)
{
	m_bNoUpload = !state;
}

void MiniDumpGenerator::setUser(const wchar_t* user)
{
	if (m_szUser)
		delete [] m_szUser;

	if (!user)
	{
		m_szUser = NULL;
	}
	else
	{
		m_szUser = new wchar_t[255];
		wcsncpy_s(m_szUser, 255, user, 255);
	}
}

void MiniDumpGenerator::setDumpLevel(unsigned char level)
{
	if (!s_pExceptionHandler)
		return;

	MINIDUMP_TYPE level4 = (MINIDUMP_TYPE)(MiniDumpWithPrivateReadWriteMemory | 
												MiniDumpWithDataSegs | 
												MiniDumpWithHandleData |
												MiniDumpWithFullMemoryInfo | 
												MiniDumpWithThreadInfo | 
												MiniDumpWithProcessThreadData |
												MiniDumpWithUnloadedModules|
												MiniDumpScanMemory ); 

	MINIDUMP_TYPE level3 = (MINIDUMP_TYPE)(		MiniDumpWithDataSegs | 
												MiniDumpWithHandleData |
												MiniDumpScanMemory|
												MiniDumpWithIndirectlyReferencedMemory|
												MiniDumpWithDataSegs); 

	MINIDUMP_TYPE level2 = (MINIDUMP_TYPE)(		MiniDumpNormal | 
												MiniDumpWithHandleData |
												MiniDumpScanMemory |
												MiniDumpWithIndirectlyReferencedMemory ); 

	switch (level)
	{
	default:
	case 1:
		s_pExceptionHandler->dump_type_ = MiniDumpNormal;
		break;

	case 2:
		s_pExceptionHandler->dump_type_ = level2;
		break;

	case 3:
		s_pExceptionHandler->dump_type_ = level3;
		break;

	case 4:
		s_pExceptionHandler->dump_type_ = level4;
		break;
	}
}

void MiniDumpGenerator::getDumpPath(wchar_t *buffer, size_t bufSize)
{
	wchar_t comAppPath[MAX_PATH];
	SHGetFolderPathW(NULL, CSIDL_COMMON_APPDATA , NULL, SHGFP_TYPE_CURRENT, comAppPath);

	//COMMONAPP_PATH
	_snwprintf_s(buffer, bufSize, _TRUNCATE, L"%s\\Desura", comAppPath);
	CreateDirectoryW(buffer, NULL);

	_snwprintf_s(buffer, bufSize, _TRUNCATE, L"%s\\Desura\\DesuraApp", comAppPath);
	CreateDirectoryW(buffer, NULL);

	_snwprintf_s(buffer, bufSize, _TRUNCATE, L"%s\\Desura\\DesuraApp\\dumps", comAppPath);
	CreateDirectoryW(buffer, NULL);
}

bool MiniDumpGenerator::filter(FilterWrapper* fw)
{
	return true;
}

bool MiniDumpGenerator::complete(const wchar_t* dump_path, const wchar_t* minidump_id, FilterWrapper* fw, bool succeeded)
{
	wchar_t fullPath[255] = {0};

	wcscat_s(fullPath, 255, dump_path);
	wcscat_s(fullPath, 255, L"\\");
	wcscat_s(fullPath, 255, minidump_id);
	wcscat_s(fullPath, 255, L".dmp");


	if (dumpreport(fullPath) == false && m_bShowMsgBox && succeeded)
	{
		char msg[1024];

		_snprintf_s(msg, 1024, _TRUNCATE,	"An unrecoverable error has occurred.\n"
											"\n"
											"For some reason the normal crashdump tool could not be used to upload the\n"
											"report. Please use the DumpGen utility (Start Menu -> Desura) to upload\n"
											"the dump and then use the force update shortcut to recover Desura into a\n"
											"usable state.\n"
											"\n"
											"The information in the dump will help the developers fix the issue in \n"
											"future releases.");

		::MessageBox(NULL, msg, "Desura Crash", MB_OK);
	}

	return succeeded;
}

bool MiniDumpGenerator::dumpreport(const wchar_t* file)
{
	wchar_t exePath[255];
	GetModuleFileNameW(NULL, exePath, 255);

	size_t exePathLen = wcslen(exePath);
	for (size_t x=exePathLen; x>0; x--)
	{
		if (exePath[x] == '\\')
			break;
		else
			exePath[x] = '\0';
	}

	PROCESS_INFORMATION ProcInfo = {0};
	STARTUPINFOW StartupInfo = {0};

	wchar_t launchArg[512] = {0};

#ifdef _DEBUG
	const wchar_t* exeName = L"dumpgen-d.exe";
#else
	const wchar_t* exeName = L"dumpgen.exe";
#endif

	_snwprintf_s(launchArg, 512, _TRUNCATE, L"%s -crashreport -file \"%s\"", exeName, file);

	if (m_bShowMsgBox)
		wcscat_s(launchArg, 512, L" -msgbox");

	if (m_bNoUpload)
		wcscat_s(launchArg, 512, L" -noupload");

	if (m_szUser)
	{
		wcscat_s(launchArg, 512, L" -user ");
		wcscat_s(launchArg, 512, m_szUser);
	}

	BOOL res = CreateProcessW(NULL, launchArg, NULL, NULL, false, NORMAL_PRIORITY_CLASS, NULL, exePath, &StartupInfo, &ProcInfo );

	return res?true:false;
}

void MiniDumpGenerator::setCrashCallback(CrashCallbackFn callback)
{
	m_pCrashCallback = callback;
}

class CustomClientInfo;

namespace google_breakpad
{


CrashGenerationClient::CrashGenerationClient(const wchar_t* pipe_name, MINIDUMP_TYPE dump_type, const CustomClientInfo* custom_info)
{
}

CrashGenerationClient::~CrashGenerationClient()
{
}

bool CrashGenerationClient::Register()
{
	return false;
}

bool CrashGenerationClient::RequestDump(EXCEPTION_POINTERS* ex_info, MDRawAssertionInfo* assert_info)
{
	return false;
}

bool CrashGenerationClient::RequestDump(EXCEPTION_POINTERS* ex_info)
{
	return false;
}

bool CrashGenerationClient::RequestDump(MDRawAssertionInfo* assert_info)
{
	return false;
}


// static
wstring GUIDString::GUIDToWString(GUID *guid) {
  wchar_t guid_string[37];
  swprintf(
	  guid_string, sizeof(guid_string) / sizeof(guid_string[0]),
	  L"%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x",
	  guid->Data1, guid->Data2, guid->Data3,
	  guid->Data4[0], guid->Data4[1], guid->Data4[2],
	  guid->Data4[3], guid->Data4[4], guid->Data4[5],
	  guid->Data4[6], guid->Data4[7]);

  // remove when VC++7.1 is no longer supported
  guid_string[sizeof(guid_string) / sizeof(guid_string[0]) - 1] = L'\0';

  return wstring(guid_string);
}

// static
wstring GUIDString::GUIDToSymbolServerWString(GUID *guid) {
  wchar_t guid_string[33];
  swprintf(
	  guid_string, sizeof(guid_string) / sizeof(guid_string[0]),
	  L"%08X%04X%04X%02X%02X%02X%02X%02X%02X%02X%02X",
	  guid->Data1, guid->Data2, guid->Data3,
	  guid->Data4[0], guid->Data4[1], guid->Data4[2],
	  guid->Data4[3], guid->Data4[4], guid->Data4[5],
	  guid->Data4[6], guid->Data4[7]);

  // remove when VC++7.1 is no longer supported
  guid_string[sizeof(guid_string) / sizeof(guid_string[0]) - 1] = L'\0';

  return wstring(guid_string);
}
}


/*
MiniDumpNormal                         = 0x00000000,
	stack traces
	
MiniDumpWithDataSegs                   = 0x00000001,
	global vars
	
MiniDumpWithFullMemory                 = 0x00000002,
	all memory
	
MiniDumpWithHandleData                 = 0x00000004,
	handle list
	
MiniDumpFilterMemory                   = 0x00000008,
	removes stack variables and zeros them
	
MiniDumpScanMemory                     = 0x00000010,
	remove modules
	
MiniDumpWithUnloadedModules            = 0x00000020,
	saves unloaded modules
	
MiniDumpWithIndirectlyReferencedMemory = 0x00000040,
	pointers are resolved upto 1k
	
MiniDumpFilterModulePaths              = 0x00000080,
	remove module paths
	
MiniDumpWithProcessThreadData          = 0x00000100,
	private process and thread data

MiniDumpWithPrivateReadWriteMemory     = 0x00000200,
	include all private memory
	
MiniDumpWithoutOptionalData            = 0x00000400,
	removes memory data
	
MiniDumpWithFullMemoryInfo             = 0x00000800,
	layout of memory
	
MiniDumpWithThreadInfo                 = 0x00001000,
	extra thread info
	
MiniDumpWithCodeSegs                   = 0x00002000,
	code segments
	
MiniDumpWithoutAuxiliaryState          = 0x00004000,
	
	
MiniDumpWithFullAuxiliaryState         = 0x00008000,
	
	
MiniDumpWithPrivateWriteCopyMemory     = 0x00010000,
	
	
MiniDumpIgnoreInaccessibleMemory       = 0x00020000,
	
	
MiniDumpWithTokenInformation           = 0x00040000,
	
	
MiniDumpValidTypeFlags                 = 0x0007ffff,
	
*/
