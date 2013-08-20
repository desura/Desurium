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

#include <process.h>
#include <Psapi.h>

#include <Softpub.h>
#include <wincrypt.h>
#include <wintrust.h>

const char* g_szValidExeNames[] =
{
	"desura.exe",
	NULL,
};

bool FindWorkingDir(std::string &out)
{
	char exePath[255] = {0};

	unsigned long aProcesses[1024], cbNeeded, cProcesses;
	if(!EnumProcesses(aProcesses, sizeof(aProcesses), &cbNeeded))
		return false;

	unsigned long curPID = GetCurrentProcessId();

	bool found = false;

	cProcesses = cbNeeded / sizeof(unsigned long);
	for (unsigned int i = 0; i < cProcesses; i++)
	{
		if(aProcesses[i] == 0)
			continue;

		if (aProcesses[i] == curPID)
			continue;

		HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, 0, aProcesses[i]);

		if (!hProcess)
			continue;

		char buffer[50] = {0};
		GetModuleBaseName(hProcess, 0, buffer, 50);

		size_t x=0;
		while (g_szValidExeNames[x])
		{
			if (strcmp(g_szValidExeNames[x], buffer)==0)
			{
				found = true;
				GetModuleFileNameEx(hProcess, 0, exePath, 255);
				break;
			}

			x++;
		}

		CloseHandle(hProcess);
		
		if (found)
			break;
	}

	if (!found)
		return false;

	size_t exePathLen = strlen(exePath);
	for (size_t x=exePathLen; x>0; x--)
	{
		if (exePath[x] == '\\')
			break;
		else
			exePath[x] = '\0';
	}

	out = exePath;
	return true;
}

bool SetDllPath(const char* wdir)
{
	std::string path;

	if (wdir)
		path = wdir;
	else if (!FindWorkingDir(path))
		return false;
		
	if (path.size() == 0)
		return false;

	std::string binDir = path + DIRS_STR + "bin";

	SetCurrentDirectory(path.c_str());
	SetDllDirectory(binDir.c_str());

	return true;
}


uint32 ValidateCert(const wchar_t* pwszSourceFile, char* message, size_t size)
{
	LONG lStatus;
	DWORD dwLastError;

	// Initialize the WINTRUST_FILE_INFO structurer
	WINTRUST_FILE_INFO FileData;
	memset(&FileData, 0, sizeof(FileData));
	FileData.cbStruct = sizeof(WINTRUST_FILE_INFO);
	FileData.pcwszFilePath = pwszSourceFile;
	FileData.hFile = NULL;
	FileData.pgKnownSubject = NULL;

	/*
	WVTPolicyGUID specifies the policy to apply on the file
	WINTRUST_ACTION_GENERIC_VERIFY_V2 policy checks:
	
	1) The certificate used to sign the file chains up to a root 
	certificate located in the trusted root certificate store. This 
	implies that the identity of the publisher has been verified by 
	a certification authority.
	
	2) In cases where user interface is displayed (which this example
	does not do), WinVerifyTrust will check for whether the  
	end entity certificate is stored in the trusted publisher store,  
	implying that the user trusts content from this publisher.
	
	3) The end entity certificate has sufficient permission to sign 
	code, as indicated by the presence of a code signing EKU or no 
	EKU.
	*/

	GUID WVTPolicyGUID = WINTRUST_ACTION_GENERIC_VERIFY_V2;
	WINTRUST_DATA WinTrustData;

	// Initialize the WinVerifyTrust input data structure.

	// Default all fields to 0.
	memset(&WinTrustData, 0, sizeof(WinTrustData));

	WinTrustData.cbStruct = sizeof(WinTrustData);
	
	// Use default code signing EKU.
	WinTrustData.pPolicyCallbackData = NULL;

	// No data to pass to SIP.
	WinTrustData.pSIPClientData = NULL;

	// Disable WVT UI.
	WinTrustData.dwUIChoice = WTD_UI_NONE;

	// No revocation checking.
	WinTrustData.fdwRevocationChecks = WTD_REVOKE_NONE; 

	// Verify an embedded signature on a file.
	WinTrustData.dwUnionChoice = WTD_CHOICE_FILE;

	// Default verification.
	WinTrustData.dwStateAction = 0;

	// Not applicable for default verification of embedded signature.
	WinTrustData.hWVTStateData = NULL;

	// Not used.
	WinTrustData.pwszURLReference = NULL;

	// Default.
	WinTrustData.dwProvFlags = WTD_SAFER_FLAG;

	// This is not applicable if there is no UI because it changes 
	// the UI to accommodate running applications instead of 
	// installing applications.
	WinTrustData.dwUIContext = 0;

	// Set pFile.
	WinTrustData.pFile = &FileData;

	// WinVerifyTrust verifies signatures as specified by the GUID 
	// and Wintrust_Data.
	lStatus = WinVerifyTrust(NULL, &WVTPolicyGUID, &WinTrustData);

	if (!message)
		return lStatus;

	switch (lStatus) 
	{
		case ERROR_SUCCESS:
			Safe::snprintf(message, size, "The file is signed and the signature was verified.");
			break;
		
		case TRUST_E_NOSIGNATURE:
			// The file was not signed or had a signature 
			// that was not valid.

			// Get the reason for no signature.
			dwLastError = GetLastError();
			if (TRUST_E_NOSIGNATURE == dwLastError ||TRUST_E_SUBJECT_FORM_UNKNOWN == dwLastError || TRUST_E_PROVIDER_UNKNOWN == dwLastError) 
			{
				Safe::snprintf(message, size, "The file is not signed.");
			} 
			else 
			{
				Safe::snprintf(message, size, "An unknown error occurred trying to verify the signature [%d].", dwLastError);
			}
			break;

		case TRUST_E_EXPLICIT_DISTRUST:
			Safe::snprintf(message, size, "The signature is present, but specifically disallowed.");
			break;

		case TRUST_E_SUBJECT_NOT_TRUSTED:
			Safe::snprintf(message, size, "The signature is present, but not trusted.");
			break;

		case CRYPT_E_SECURITY_SETTINGS:
			 Safe::snprintf(message, size, "CRYPT_E_SECURITY_SETTINGS - The hash "
				"representing the subject or the publisher wasn't "
				"explicitly trusted by the admin and admin policy "
				"has disabled user trust. No signature, publisher "
				"or timestamp errors.");
			break;

		default:
			Safe::snprintf(message, size, "Error is: 0x%x.\n", lStatus);
			break;
	}

	return lStatus;
}
