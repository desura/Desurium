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
#include "util/UtilWindows.h"

#ifdef WIN32

#include <winioctl.h> 
#include <Softpub.h>
#include <wincrypt.h>
#include <wintrust.h>
#include <shlobj.h>
#include <Msi.h>
#include <Shlwapi.h>
#include <Accctrl.h>
#include <Aclapi.h>
#include <Propkey.h>
#include <olectl.h>

#include <Sddl.h>
#define getSystemPath(id, path) SHGetFolderPathW(NULL, id , NULL, SHGFP_TYPE_CURRENT, path);

namespace UTIL
{
namespace WIN
{

//http://msdn.microsoft.com/en-us/library/ms684139(VS.85).aspx
bool is64OS()
{
	typedef BOOL (WINAPI *LPFN_ISWOW64PROCESS) (HANDLE, PBOOL);

	LPFN_ISWOW64PROCESS fnIsWow64Process;


	BOOL bIsWow64 = FALSE;
	fnIsWow64Process = (LPFN_ISWOW64PROCESS) GetProcAddress(GetModuleHandle(TEXT("kernel32")),"IsWow64Process");
  
	if (NULL != fnIsWow64Process)
	{
		if (!fnIsWow64Process(GetCurrentProcess(),&bIsWow64))
		{
			return false;
		}
	}
	return (bIsWow64 == TRUE);
}


bool getRegSections(const std::string &regIndex, std::vector<std::wstring> &parts, HKEY &rootKey)
{
	std::vector<std::string> subParts;
	UTIL::STRING::tokenize(regIndex, subParts, "\\");

	//account for default reg key
	if (regIndex.size() > 0 && *(regIndex.end()-1) == '\\')
		subParts.push_back("");

	if (subParts.size() < 3)
		return false;

	parts.push_back(gcWString(subParts[0]));
	parts.push_back(L"");

	for (size_t x=1; x<subParts.size()-1; x++)
	{
		if (x != 1)
			parts[1] += L"\\";

		parts[1] += gcWString(subParts[x]);
	}

	parts.push_back(gcWString(UTIL::STRING::urlDecode(*(subParts.end()-1))));

	if (parts[0] == L"HKEY_CLASSES_ROOT")
	{
		rootKey = HKEY_CLASSES_ROOT;
	}
	else if (parts[0] == L"HKEY_CURRENT_USER")
	{
		rootKey = HKEY_CURRENT_USER;
	}
	else if (parts[0] == L"HKEY_LOCAL_MACHINE")
	{
		rootKey = HKEY_LOCAL_MACHINE;
	}
	else if (parts[0] == L"HKEY_USERS")
	{
		rootKey = HKEY_USERS;
	}
	else
	{
		return false;
	}

	return true;
}

std::string getRegValue(const std::string &regIndex, bool use64bit)
{
	std::vector<std::wstring> parts;
	HKEY rootKey;

	if (!getRegSections(regIndex, parts, rootKey))
		return "";

	DWORD res;
	HKEY hk;

	DWORD dwType=REG_SZ;
	DWORD dwSize=0;

	DWORD flags = KEY_READ|(use64bit?KEY_WOW64_64KEY:0);

	res = RegOpenKeyEx(rootKey, parts[1].c_str(), 0, flags, &hk);

	std::string resStr;

	if (res == ERROR_SUCCESS)
	{
		DWORD err = RegQueryValueExW(hk, parts[2].c_str(), NULL, &dwType, NULL, &dwSize);

		if (dwSize != 0)
		{
			wchar_t* buff = new wchar_t[(dwSize/sizeof(wchar_t))+1];
			dwSize += 1;

			err = RegQueryValueExW(hk, parts[2].c_str(), NULL, &dwType, (LPBYTE)buff, &dwSize);
			resStr = gcString(gcWString(buff, dwSize));

			safe_delete(buff);
		}

		RegCloseKey(hk);
	}

	return resStr;
}

int getRegValueInt(const std::string &regIndex, bool use64bit)
{
	std::vector<std::wstring> parts;
	HKEY rootKey;

	if (!getRegSections(regIndex, parts, rootKey))
		return -1;

	DWORD res;
	HKEY hk;

	DWORD dwType=REG_DWORD;
	DWORD dwSize=sizeof(PDWORD);

	DWORD flags = KEY_READ|(use64bit?KEY_WOW64_64KEY:0);

	res = RegOpenKeyEx(rootKey, parts[1].c_str(), 0, flags, &hk);

	int resInt = 0;
	if (res == ERROR_SUCCESS)
	{
		DWORD err = RegQueryValueExW(hk, parts[2].c_str(), NULL, &dwType, NULL, &dwSize);

		if (dwSize != 0)
		{
			dwSize += 1;

			err = RegQueryValueExW(hk, parts[2].c_str(), NULL, &dwType, (PBYTE)&resInt, &dwSize);
		}

		RegCloseKey(hk);
	}

	return resInt;
}

void setRegValue(const std::string &regIndex, const std::string &value, bool expandStr, bool use64bit)
{
	if (regIndex.size() == 0)
		return;

	std::vector<std::wstring> parts;
	HKEY rootKey;

	if (!getRegSections(regIndex, parts, rootKey))
		return;

	HKEY hk;

	gcWString v(value);

	DWORD dwSize	= (DWORD)(v.size()+1) * sizeof(wchar_t);
	DWORD flags		= KEY_WRITE|(use64bit?KEY_WOW64_64KEY:0);
	DWORD res		= RegCreateKeyEx(rootKey, parts[1].c_str(), 0, NULL, REG_OPTION_NON_VOLATILE, flags, NULL, &hk, NULL) ;
	DWORD regType	= expandStr?REG_EXPAND_SZ:REG_SZ;


	if (res == ERROR_SUCCESS)
	{
		DWORD err = RegSetValueExW(hk, parts[2].c_str(), NULL, regType, (LPBYTE)v.c_str(), dwSize);
		RegCloseKey(hk);
	}
}

void setRegBinaryValue(const std::string &regIndex, const char* blob, size_t size, bool use64bit)
{
	std::vector<std::wstring> parts;
	HKEY rootKey;

	if (!getRegSections(regIndex, parts, rootKey))
		return;

	HKEY hk;
	DWORD flags		= KEY_WRITE|(use64bit?KEY_WOW64_64KEY:0);
	DWORD res		= RegCreateKeyEx(rootKey, parts[1].c_str(), 0, NULL, REG_OPTION_NON_VOLATILE, flags, NULL, &hk, NULL) ;

	if (res == ERROR_SUCCESS)
	{
		DWORD err = RegSetValueExW(hk, parts[2].c_str(), NULL, REG_BINARY, (PBYTE)blob, size);
		RegCloseKey(hk);
	}
}

void setRegValue(const std::string &regIndex, uint32 value, bool use64bit)
{
	std::vector<std::wstring> parts;
	HKEY rootKey;

	if (!getRegSections(regIndex, parts, rootKey))
		return;

	HKEY hk;
	DWORD flags		= KEY_WRITE|(use64bit?KEY_WOW64_64KEY:0);
	DWORD res		= RegCreateKeyEx(rootKey, parts[1].c_str(), 0, NULL, REG_OPTION_NON_VOLATILE, flags, NULL, &hk, NULL) ;
	DWORD dwValue	= value;

	if (res == ERROR_SUCCESS)
	{
		DWORD err = RegSetValueExW(hk, parts[2].c_str(), NULL, REG_DWORD, (PBYTE)&dwValue, sizeof(PDWORD));
		RegCloseKey(hk);
	}
}

bool delRegValue(const std::string &regIndex, bool use64bit)
{
	std::vector<std::wstring> parts;
	HKEY rootKey;

	if (!getRegSections(regIndex, parts, rootKey))
		return false;

	HKEY hk;
	DWORD flags	= KEY_ALL_ACCESS|(use64bit?KEY_WOW64_64KEY:0);
	DWORD res	= RegOpenKeyEx(rootKey, parts[1].c_str(), 0, flags, &hk);

	if (res == ERROR_SUCCESS)
	{
		LONG lResult = RegDeleteValue(hk, parts[2].c_str());
		RegCloseKey(hk);

		return lResult == ERROR_SUCCESS;
	}

	return false;
}

bool delRegKey(const std::string &regIndex, bool use64bit)
{
	std::vector<std::wstring> parts;
	HKEY rootKey;

	if (!getRegSections(regIndex, parts, rootKey))
		return false;

	HKEY hk;
	DWORD flags	= KEY_ALL_ACCESS|(use64bit?KEY_WOW64_64KEY:0);
	DWORD res	= RegOpenKeyEx(rootKey, parts[1].c_str(), 0, flags, &hk);

	if (res == ERROR_SUCCESS)
	{
		LONG lResult = RegDeleteKey(hk, parts[2].c_str());
		RegCloseKey(hk);

		return lResult == ERROR_SUCCESS;
	}

	return false;
} 


bool delRegTree(const std::string &regIndex, bool use64bit)
{
	std::vector<std::wstring> parts;
	HKEY rootKey;

	if (!getRegSections(regIndex, parts, rootKey))
		return false;

	std::vector<std::string> list;
	getAllRegKeys(regIndex, list, use64bit);

	for (size_t x=0; x<list.size(); x++)
	{
		std::string newReg = regIndex + "\\" + list[x];
		
		if (!delRegKey(newReg, use64bit))
			return false;
	}

	return delRegKey(regIndex, use64bit);
}

void getAllRegKeys(const std::string &regIndex, std::vector<std::string> &regKeys, bool use64bit)
{
	std::vector<std::wstring> parts;
	HKEY rootKey;

	if (!getRegSections(regIndex, parts, rootKey))
		return;

	DWORD x=0;
	DWORD nameSize;
	wchar_t name[255];

	std::wstring keyName = parts[1] + L"\\" + parts[2];

	HKEY hk;

	DWORD flags = KEY_ENUMERATE_SUB_KEYS|(use64bit?KEY_WOW64_64KEY:0);
	DWORD res = RegOpenKeyEx(rootKey, keyName.c_str(), 0, flags, &hk);

	if (res == ERROR_SUCCESS)
	{
		do
		{
			nameSize = 255;
			res = RegEnumKeyEx(hk, x, name, &nameSize, NULL, NULL, NULL, NULL);

			if (res == ERROR_SUCCESS)
				regKeys.push_back(gcString(std::wstring(name, nameSize)));

			x++;
		}
		while (res != ERROR_NO_MORE_ITEMS);

		RegCloseKey(hk);
	}
}

void getAllRegValues(const std::string &regIndex, std::vector<std::string> &regKeys, bool use64bit)
{
	std::vector<std::wstring> parts;
	HKEY rootKey;

	if (!getRegSections(regIndex, parts, rootKey))
		return;

	DWORD x=0;
	DWORD nameSize;
	wchar_t name[255];

	std::wstring keyName = parts[1] + L"\\" + parts[2];

	HKEY hk;
	DWORD flags = KEY_ENUMERATE_SUB_KEYS|(use64bit?KEY_WOW64_64KEY:0);
	DWORD res = RegOpenKeyEx(rootKey, keyName.c_str(), 0, flags, &hk);

	if (res == ERROR_SUCCESS)
	{
		do
		{
			nameSize = 255;
			res = RegEnumValue(hk, x, name, &nameSize, NULL, NULL, NULL, NULL);

			if (res == ERROR_SUCCESS)
				regKeys.push_back(gcString(std::wstring(name, nameSize)));

			x++;
		}
		while (res != ERROR_NO_MORE_ITEMS);

		RegCloseKey(hk);
	}
}


uint64 getFreeSpace(const char* path)
{
	unsigned __int64 i64FreeBytes = 0;

	if (strlen(path) < 3)

		return 0;

	char drive[4];
	Safe::strncpy(drive, 4, path, 3);
	drive[3] = '\0';

	GetDiskFreeSpaceExA(drive, NULL, NULL, (PULARGE_INTEGER)&i64FreeBytes);
	return (uint64)i64FreeBytes;
}
  
				  

uint32 getHDDSerial()
{
	char volName[MAX_PATH+1];
	DWORD serial;

	GetVolumeInformationA( NULL, volName, MAX_PATH, &serial, NULL, NULL, NULL, 0);

	return serial;
}

gcWString getWindowsPath(const wchar_t *extra)
{
	wchar_t path[MAX_PATH];
	getSystemPath(CSIDL_WINDOWS, path);

	gcWString out(path);

	out += DIRS_WSTR;
	out += COMMONAPP_PATH_W;

	if (extra)
	{
		out += DIRS_WSTR;
		out += extra;
	}

	return out;
}

uint32 validateCert(const wchar_t* pwszSourceFile, char* message, size_t size)
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




bool isPointOnScreen(int32 x, int32 y)
{
	POINT p;
	p.x = x;
	p.y = y;

	HMONITOR hMonitor = MonitorFromPoint(p, MONITOR_DEFAULTTONEAREST);

	// get the work area or entire monitor rect.
	MONITORINFO mi;
	mi.cbSize = sizeof(mi);

	GetMonitorInfo(hMonitor, &mi);

	return PtInRect(&mi.rcWork, p)?true:false;
}


bool isMsiInstalled(const char* productCode)
{
	INSTALLSTATE res = MsiQueryProductStateA(productCode);
	return (res == INSTALLSTATE_LOCAL || res == INSTALLSTATE_DEFAULT);
}

void createShortCut(const wchar_t *path, const char* exe, const char* workingDir, const char* args, bool flagAsNonPinned, const char* icon)
{
	IShellLinkA* out = NULL;
	CoInitialize(NULL);

	if (CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLinkA, (void**)&out) == S_OK)
	{
		IPersistFile* pIPF = NULL;
		IPropertyStore* pIPS = NULL;

		bool isPersistFile = (out->QueryInterface(IID_IPersistFile, (void**)&pIPF) == S_OK);
		bool isPropStore = (out->QueryInterface(IID_IPropertyStore, (void**)&pIPS) == S_OK);

		if (isPersistFile)
		{
			if (isPropStore && flagAsNonPinned)
			{
				PROPVARIANT pvar;
				pvar.vt = VT_BOOL;
				pvar.boolVal = VARIANT_TRUE;

				pIPS->SetValue(PKEY_AppUserModel_PreventPinning, pvar);
				pIPS->SetValue(PKEY_AppUserModel_ExcludeFromShowInNewInstall, pvar);
			}

			out->SetArguments(args);

			if (icon)
				out->SetIconLocation(icon, 0);
			else
				out->SetIconLocation(exe, 0);

			out->SetPath(exe);
			out->SetWorkingDirectory(workingDir);

			pIPF->Save(path, TRUE);
			pIPF->SaveCompleted(path);
		}

		if (pIPF)
			pIPF->Release();

		if (pIPS)
			pIPS->Release();

		out->Release();
	}

	CoUninitialize();
}

bool launchExe(const char* exe, const char* args, bool elevateIfNeeded, HWND elevationHandle)
{
	DWORD res = 0;
	DWORD lastError = 0;

	gcString cmd("\"{0}\"", exe);
	gcString a(args);

	if (a.size() > 0)
		cmd += " " + a;

	std::string folderPath = UTIL::FS::PathWithFile(exe).getFolderPath();

	if (!elevateIfNeeded)
	{
		STARTUPINFOA si = {0};
		PROCESS_INFORMATION pi = {0};

		si.cb = sizeof(si);

		//turn on error boxes for launching exe's
		UINT oldErrMode = SetErrorMode(0);

		res = CreateProcessA(NULL, const_cast<char*>(cmd.c_str()), NULL, NULL, FALSE, 0, NULL, folderPath.c_str(), &si, &pi);
		lastError = GetLastError();

		SetErrorMode(oldErrMode);
		
		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);
	}

	//! did we fail elevation
	if (elevateIfNeeded || (!res && lastError == 740))
	{
		gcString param(args);

		SHELLEXECUTEINFOA info;
		memset(&info, 0, sizeof(SHELLEXECUTEINFO));

		info.hwnd = elevationHandle;
		info.cbSize = sizeof(SHELLEXECUTEINFO);

		uint32 os = UTIL::WIN::getOSId();

		if (os == WINDOWS_XP || os == WINDOWS_XP64)
			info.lpVerb = "open";
		else
			info.lpVerb = "runas";

		info.lpFile = exe;
		info.nShow = SW_SHOW;
		info.fMask = SEE_MASK_DEFAULT;
		info.lpParameters = param.c_str();
		info.lpDirectory = folderPath.c_str();

		res = ShellExecuteExA(&info);
	}

	return res?true:false;
}



void changeFolderPermissions(const std::wstring& dir)
{
	wchar_t* localDir = NULL;
	PACL pDacl = NULL;
	PACL pNewDACL = NULL;
	PSID psid = NULL;
	PSECURITY_DESCRIPTOR ppSecurityDescriptor = NULL;

	wchar_t everyone[255] = {0};
	wchar_t domain[255] = {0};
	DWORD eSize = 255;
	DWORD dSize = 255;
	SID_NAME_USE rSidNameUse;

	DWORD res = ERROR_SUCCESS;
	DWORD SidSize = SECURITY_MAX_SID_SIZE;

	EXPLICIT_ACCESS ExplicitAccess;
	memset(&ExplicitAccess, 0, sizeof(EXPLICIT_ACCESS));

	Safe::wcscpy(&localDir, dir.c_str(), dir.size());

	AutoDelete<wchar_t> ad(localDir);
	AutoDeleteLocal<PACL> adNewDACL(&pNewDACL);
	AutoDeleteLocal<PSID> adPsid(&psid);

	// Get the current Security Info for the path
	res = GetNamedSecurityInfo(localDir, SE_FILE_OBJECT, DACL_SECURITY_INFORMATION, NULL, NULL, &pDacl, NULL, &ppSecurityDescriptor);

	if (res != ERROR_SUCCESS)
		throw gcException(ERR_WIN, res, gcString("Failed to GetNamedSecurityInfo on folder [{0}].", localDir));

	if(!(psid = LocalAlloc(LMEM_FIXED, SidSize)))
		throw gcException(ERR_SERVICE, GetLastError(), gcString("Failed to LocalAlloc on folder [{0}].", localDir));

	if(!CreateWellKnownSid(WinWorldSid, NULL, psid, &SidSize))
		throw gcException(ERR_SERVICE, GetLastError(), gcString("Failed to CreateWellKnownSid on folder [{0}].", localDir));

	if (!LookupAccountSid(NULL, psid, everyone, &eSize, domain, &dSize, &rSidNameUse))
		throw gcException(ERR_SERVICE, GetLastError(), gcString("Failed to LookupAccountSid on folder [{0}].", localDir));

	BuildExplicitAccessWithName(&ExplicitAccess, everyone, GENERIC_ALL|DELETE, SET_ACCESS, CONTAINER_INHERIT_ACE|OBJECT_INHERIT_ACE);

	if (SetEntriesInAcl(1, &ExplicitAccess, pDacl, &pNewDACL))
		throw gcException(ERR_WIN, GetLastError(), gcString("Failed to SetEntriesInAcl on folder [{0}].", localDir));

	res = SetNamedSecurityInfo(localDir, SE_FILE_OBJECT, DACL_SECURITY_INFORMATION, NULL, NULL, pNewDACL, NULL);

	if (res != ERROR_SUCCESS)
		throw gcException(ERR_WIN, GetLastError(), gcString("Failed to SetNamedSecurityInfo on folder [{0}].", localDir));
}


void extractIconCB(const char* exe, UTIL::CB::CallbackI* callback)
{ 
	if (!exe)
		throw gcException(ERR_INVALID, gcString("Failed to locate file {0} for icon extract.", exe));

	SHFILEINFOA shFileINfo = {0};

	DWORD_PTR res = SHGetFileInfoA(exe, 0, &shFileINfo, sizeof(SHFILEINFOA), SHGFI_ICON|SHGFI_LARGEICON);

	if (!res)
		throw gcException(ERR_WIN, GetLastError(), gcString("Failed to get info of {0} for icon extract", exe));


	PICTDESC desc = { 0 };

	desc.cbSizeofstruct = sizeof(PICTDESC);
	desc.picType = PICTYPE_ICON;
	desc.icon.hicon = shFileINfo.hIcon;

	IPicture* pPicture = 0;
	HRESULT hr = OleCreatePictureIndirect(&desc, IID_IPicture, FALSE, (void**)&pPicture);
	DestroyIcon(shFileINfo.hIcon);

	if (FAILED(hr)) 
		throw gcException(ERR_WIN, GetLastError(), gcString("Failed to create picture of {0} for icon extract", exe));

	// Create a stream and save the image
	IStream* pStream = 0;
	CreateStreamOnHGlobal(0, TRUE, &pStream);

	LONG cbSize = 0;
	hr = pPicture->SaveAsFile(pStream, TRUE, &cbSize);
	pPicture->Release();

	if (FAILED(hr))
	{
		pStream->Release();
		throw gcException(ERR_WIN, GetLastError(), gcString("Failed to SaveAsFile of {0} for icon extract", exe));
	}

	try
	{
		HGLOBAL hBuf = 0;
		GetHGlobalFromStream(pStream, &hBuf);

		void* buffer = GlobalLock(hBuf);

		if (callback)
			callback->onData((unsigned char*)buffer, cbSize);

		GlobalUnlock(buffer);
	}
	catch (...)
	{
		pStream->Release();
		throw;
	}

	// Cleanup
	pStream->Release();
}

const char g_cBadChars[] = {
	'\\',
	'/',
	':',
	'*',
	'?',
	'"',
	'<',
	'>',
	'|',
	NULL
};

std::string sanitiseFileName(const char* name)
{
	if (!name)
		return "";

	std::string out;
	size_t size = strlen(name);
	out.reserve(size);

	for (size_t x=0; x<size; x++)
	{
		size_t y=0;
		bool bad = false;

		while (g_cBadChars[y])
		{
			if (g_cBadChars[y] == name[x])
			{
				bad = true;
				break;
			}

			y++;
		}

		if (!bad)
			out.push_back(name[x]);
	}

	//remove any spaces on the end
	while (out.size() > 0 && out[out.size()-1] == ' ')
		out.pop_back();

	return out;
}

bool runAs(const char* command, const char* area)
{
	SHELLEXECUTEINFOA info;
	memset(&info, 0, sizeof(SHELLEXECUTEINFO));

	info.cbSize = sizeof(SHELLEXECUTEINFO);
	info.lpVerb = area;
	info.lpFile = command;
	info.nShow = SW_SHOW;

	BOOL res = ShellExecuteExA(&info);
	return (res == TRUE);
}

}
}

#endif
