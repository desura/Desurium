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
#include <aclapi.h>

namespace UTIL
{
namespace WIN
{





void changeServiceAccess(const char* szName)
{
	assert(szName);

	SC_HANDLE scm = NULL;
	SC_HANDLE Service = NULL;

	BOOL                 bDaclPresent   = FALSE;
	BOOL                 bDaclDefaulted = FALSE;
	DWORD                dwSize         = 0;
	EXPLICIT_ACCESS      ea;
	PACL                 pacl           = NULL;
	PACL                 pNewAcl        = NULL;
	PSECURITY_DESCRIPTOR psd            = NULL;
	SECURITY_DESCRIPTOR  sd;


	//open connection to SCM
	scm = OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT);

	if (!scm)
		throw gcException(ERR_NULLSCMANAGER, GetLastError(), "Failed to open the Service Control Manager");

	gcWString wName(szName);

	try
	{
		//open service
		Service = OpenService(scm, wName.c_str(), READ_CONTROL|WRITE_DAC);
		if (!Service)
			throw gcException(ERR_NULLSERVICE, GetLastError(), gcString("Failed to open service: {0}", szName));


		// Get the current security descriptor.
		if (!QueryServiceObjectSecurity(Service, DACL_SECURITY_INFORMATION, psd, 0, &dwSize))
		{
			if (GetLastError() == ERROR_INSUFFICIENT_BUFFER)
			{
				psd = (PSECURITY_DESCRIPTOR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, dwSize);

				if (!psd)
					throw gcException(ERR_SERVICE, gcString("Failed heap allocation for service service: {0}", szName));

				//get securtty info
				if (!QueryServiceObjectSecurity(Service, DACL_SECURITY_INFORMATION, psd, dwSize, &dwSize))
					throw gcException(ERR_SERVICE, GetLastError(), gcString("QueryServiceObjectSecurity failed for service: {0}", szName));
			}
		}

		if (!psd)
			throw gcException(ERR_SERVICE, gcString("Failed heap allocation for service service: {0}", szName));

		// Get the DACL.
		if (!GetSecurityDescriptorDacl(psd, &bDaclPresent, &pacl, &bDaclDefaulted))		
			throw gcException(ERR_SERVICE, GetLastError(), gcString("GetSecurityDescriptorDacl failed for service: {0}", szName));


		DWORD SidSize;
		PSID TheSID;

		SidSize = SECURITY_MAX_SID_SIZE;
		// Allocate enough memory for the largest possible SID.
		if(!(TheSID = LocalAlloc(LMEM_FIXED, SidSize)))
		{    
			LocalFree(TheSID);
			throw gcException(ERR_SERVICE, GetLastError(), gcString("LocalAlloc failed for  service: {0}", szName));
		}

		if(!CreateWellKnownSid(WinWorldSid, NULL, TheSID, &SidSize))
		{
			LocalFree(TheSID);
			throw gcException(ERR_SERVICE, GetLastError(), gcString("CreateWellKnownSid failed for  service: {0}", szName));
		}

		wchar_t everyone[255];
		wchar_t domain[255];
		DWORD eSize = 255;
		DWORD dSize = 255;
		SID_NAME_USE rSidNameUse;

		if (!LookupAccountSid(NULL, TheSID, everyone, &eSize, domain, &dSize, &rSidNameUse))
		{
			LocalFree(TheSID);
			throw gcException(ERR_SERVICE, GetLastError(), gcString("LookupAccountSid failed for  service: {0}", szName));
		}

		LocalFree(TheSID);

		// Build the ACE.
		BuildExplicitAccessWithName(&ea, everyone, SERVICE_START|SERVICE_STOP|READ_CONTROL|SERVICE_QUERY_STATUS|PROCESS_QUERY_INFORMATION, SET_ACCESS, NO_INHERITANCE);

		if (SetEntriesInAcl(1, &ea, pacl, &pNewAcl) != ERROR_SUCCESS)
		{
			throw gcException(ERR_SERVICE, GetLastError(), gcString("SetEntriesInAcl failed for  service: {0}", szName));
		}

		// Initialize a NEW Security Descriptor.
		if (!InitializeSecurityDescriptor(&sd, SECURITY_DESCRIPTOR_REVISION))			
			throw gcException(ERR_SERVICE, GetLastError(), gcString("InitializeSecurityDescriptor failed for  service: {0}", szName));

		// Set the new DACL in the Security Descriptor.
		if (!SetSecurityDescriptorDacl(&sd, TRUE, pNewAcl, FALSE))						
			throw gcException(ERR_SERVICE, GetLastError(), gcString("SetSecurityDescriptorDacl failed for  service: {0}", szName));

		// Set the new DACL for the service object.
		if (!SetServiceObjectSecurity(Service, DACL_SECURITY_INFORMATION, &sd))		
			throw gcException(ERR_SERVICE, GetLastError(), gcString("SetServiceObjectSecurity failed for  service: {0}", szName));

	}
	catch (gcException &e)
	{
		if (Service)
			CloseServiceHandle(Service);

		if (scm)
			CloseServiceHandle(scm);

		// Free buffers.
		LocalFree((HLOCAL)pNewAcl);
		HeapFree(GetProcessHeap(), 0, (LPVOID)psd);

		throw e;
	}

	CloseServiceHandle(scm);
	CloseServiceHandle(Service);

	// Free buffers.
	LocalFree((HLOCAL)pNewAcl);
	HeapFree(GetProcessHeap(), 0, (LPVOID)psd);
}

void enableService(const char* szName)
{
	gcWString wName(szName);
	SC_HANDLE scm, Service;
	
	//open connection to SCM
	scm = OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT);

	if (!scm)
		throw gcException(ERR_NULLSCMANAGER, GetLastError(), "Failed to open the Service Control Manager");

	//open service
	Service = OpenService(scm, wName.c_str(), SERVICE_CHANGE_CONFIG|SERVICE_QUERY_STATUS);
	if (!Service)
	{
		CloseServiceHandle(scm);
		throw gcException(ERR_NULLSERVICE, GetLastError(), gcString("Failed to open service: {0}", szName));
	}

	BOOL res = ChangeServiceConfig(Service, SERVICE_NO_CHANGE, SERVICE_DEMAND_START, SERVICE_NO_CHANGE, NULL, NULL, NULL, NULL, NULL, NULL, NULL);

	CloseServiceHandle(scm);
	CloseServiceHandle(Service); 

	if (!res)
		throw gcException(ERR_SERVICE, GetLastError(), gcString("Failed to to renable service: {0}", szName)); 
}

BOOL doStartService(SC_HANDLE Service, const char* szName, std::vector<std::string> &args)
{
	BOOL res;

	if (args.size() == 0)
	{
		res = StartService(Service, 0, NULL);
	}
	else
	{
		const char** a = new const char*[args.size()+1];

		a[0] = szName;

		for (size_t x=0; x<args.size(); x++)
			a[x+1] = args[x].c_str();

		res = StartServiceA(Service, args.size()+1, a);
		delete [] a;
	}

	return res;
}

void startService(const char* szName, std::vector<std::string> &args)
{
	assert(szName);
	gcWString wName(szName);

	SC_HANDLE scm, Service;
	SERVICE_STATUS ssStatus; 
	DWORD dwWaitTime;
	
	//open connection to SCM
	scm = OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT);

	if (!scm)
		throw gcException(ERR_NULLSCMANAGER, GetLastError(), "Failed to open the Service Control Manager");

	//open service
	Service = OpenService(scm, wName.c_str(), SERVICE_START|SERVICE_QUERY_STATUS);
	if (!Service)
	{
		CloseServiceHandle(scm);
		throw gcException(ERR_NULLSERVICE, GetLastError(), gcString("Failed to open service: {0}", szName));
	}

	// Check the status until the service is no longer start pending. 
	if (!QueryServiceStatus( Service, &ssStatus) )
	{
		CloseServiceHandle(Service);
		CloseServiceHandle(scm);
		throw gcException(ERR_SERVICE, GetLastError(), gcString("Failed to Query service: {0}", szName));
	}

	while (ssStatus.dwCurrentState == SERVICE_STOP_PENDING)
	{
		gcSleep(1000);
	}

	if (ssStatus.dwCurrentState != SERVICE_STOPPED) 
	{
		CloseServiceHandle(Service);
		CloseServiceHandle(scm);
		throw gcException(ERR_SERVICE, GetLastError(), gcString("Failed to start service: Service {0} is not stopped [{1}]", szName, ssStatus.dwCurrentState));
	}

	BOOL res = doStartService(Service, szName, args);

	if (res == 0)
	{
		if (GetLastError() == 1058)
			throw gcException(ERR_SERVICE, GetLastError(), gcString("Failed to start service {0} as the service is disabled. Please use msconfig and renable service.", szName));

		throw gcException(ERR_SERVICE, GetLastError(), gcString("Failed to start service: {0}", szName));
	}

	// Check the status until the service is no longer start pending. 
	if (!QueryServiceStatus( Service, &ssStatus) )
	{
		CloseServiceHandle(Service);
		CloseServiceHandle(scm);
		throw gcException(ERR_SERVICE, GetLastError(), gcString("Failed to Query service: {0}", szName));
	}
		
	DWORD totalTime = 0;

	while (ssStatus.dwCurrentState != SERVICE_RUNNING) 
	{ 
		if (ssStatus.dwCurrentState == SERVICE_STOPPED)
		{
			//start service
			BOOL res = doStartService(Service, szName, args);

			if (res == 0)
				throw gcException(ERR_SERVICE, GetLastError(), gcString("Failed to start service: {0}", szName));
		}

		dwWaitTime = 1000;
		totalTime += dwWaitTime;

		gcSleep( dwWaitTime );

		// Check the status again. 
		if (!QueryServiceStatus( Service, &ssStatus) )
		{
			CloseServiceHandle(Service);
			CloseServiceHandle(scm);
			throw gcException(ERR_SERVICE, GetLastError(), gcString("Failed to Query service: {0}", szName));
		}

		if (totalTime > 30000)
		{
			CloseServiceHandle(Service);
			CloseServiceHandle(scm);
			throw gcException(ERR_SERVICE, gcString("Service {0} Startup timed out after 30 seconds.", szName));
		}
	}
		
	// Check the status again. 
	if (!QueryServiceStatus( Service, &ssStatus) )
	{
		CloseServiceHandle(Service);
		CloseServiceHandle(scm);
		throw gcException(ERR_SERVICE, GetLastError(), gcString("Failed to Query service: {0}", szName));
	}

	CloseServiceHandle(scm);
	CloseServiceHandle(Service); 

	if (ssStatus.dwCurrentState != SERVICE_RUNNING) 
		throw gcException(ERR_SERVICE, gcString("Failed to start service: {0} [{1}]", szName, ssStatus.dwCurrentState));
}

void stopService(const char* szName)
{
	assert(szName);
	gcWString wName(szName);

	SC_HANDLE scm, Service;
	SERVICE_STATUS ssStatus; 
	
	//open connection to SCM
	scm = OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT);

	if (!scm)
		throw gcException(ERR_NULLSCMANAGER, GetLastError(), "Failed to open the Service Control Manager");

	//open service
	Service = OpenService(scm, wName.c_str(), SERVICE_STOP|SERVICE_QUERY_STATUS);
	if (!Service)
	{
		CloseServiceHandle(scm);
		throw gcException(ERR_NULLSERVICE, GetLastError(), gcString("Failed to open service: {0}", szName));
	}

	// Check the status until the service is no longer start pending. 
	if (!QueryServiceStatus( Service, &ssStatus) )
	{
		CloseServiceHandle(Service);
		CloseServiceHandle(scm);
		throw gcException(ERR_SERVICE, GetLastError(), gcString("Failed to Query service: {0}", szName));
	}

	if (ssStatus.dwCurrentState & (SERVICE_STOP_PENDING|SERVICE_STOPPED))
		return;

	BOOL SUCCES = ControlService(Service, SERVICE_CONTROL_STOP, &ssStatus);

	CloseServiceHandle(Service);
	CloseServiceHandle(scm);

	if (!SUCCES)
		throw gcException(ERR_SERVICE, GetLastError(), gcString("Failed to stop service: {0}", szName));
}


void installService(const char* szName, const char* szPath, const char* szDispName)
{
	assert(szName);
	assert(szPath);

	gcWString wName(szName);
	gcWString wPath(szPath);
	gcWString wDispName(szDispName?szDispName:szName);

	SC_HANDLE newService;
	SC_HANDLE scm; 

	//open connection to SCM
	scm = OpenSCManager(NULL, NULL, SC_MANAGER_CREATE_SERVICE);

	if (!scm)
		throw gcException(ERR_NULLSCMANAGER, GetLastError(), "Failed to open the Service Control Manager");

	//install service
	newService = CreateService(
		scm,						//scm database
		wName.c_str(),					//service name
		wDispName.c_str(),				//display name
		GENERIC_READ|GENERIC_EXECUTE,	//access rights to the service
		SERVICE_WIN32_OWN_PROCESS,	//service type
		SERVICE_DEMAND_START,		//service start type
		SERVICE_ERROR_NORMAL,		//error control type
		wPath.c_str(),				//service path
		NULL,						//no load ordering group 
		NULL,						//no tag identifier
		NULL,						//no dependencies	
		NULL,						//LocalSystem account
		NULL);						//no password

	if(!newService)
	{
		CloseServiceHandle(scm);
		throw gcException(ERR_NULLSERVICE, GetLastError(), gcString("Failed to create new service: {0}", szName));
	}

	//clean up
	CloseServiceHandle(newService);
	CloseServiceHandle(scm);
}


void uninstallService(const char* szName)
{
	SC_HANDLE service;
	SC_HANDLE scm;
	BOOL SUCCESS;
	SERVICE_STATUS status;
	gcWString wName(szName);

	//Open connection to SCM
	scm = OpenSCManager(NULL, NULL, SC_MANAGER_CREATE_SERVICE);
	if (!scm)
		throw gcException(ERR_NULLSCMANAGER, GetLastError(), gcString("Failed to open the Service Control Manager"));

	//Get service's handle
	service = OpenService(scm, wName.c_str(), SERVICE_ALL_ACCESS | DELETE);
	if (!service)
	{
		CloseServiceHandle(scm);
		throw gcException(ERR_NULLSERVICE, GetLastError(), gcString("Failed to open service: {0}", szName));
	}

	//Get service status
	SUCCESS	= QueryServiceStatus(service, &status);
	if (!SUCCESS)
	{
		CloseServiceHandle(service);
		CloseServiceHandle(scm);
		throw gcException(ERR_SERVICE, GetLastError(), gcString("Failed to Query service: {0}", szName));
	}
	
	//Stop service if necessary		
	if (status.dwCurrentState != SERVICE_STOPPED)
	{
		SUCCESS = ControlService(service, SERVICE_CONTROL_STOP, &status);

		if (!SUCCESS)
		{
			CloseServiceHandle(service);
			CloseServiceHandle(scm);
			throw gcException(ERR_SERVICE, GetLastError(), gcString("Failed to stop service: {0}", szName));
		}

		gcSleep(500);
	}

	//Delete service
	SUCCESS = DeleteService(service);

	//Clean up
	CloseServiceHandle(service);
	CloseServiceHandle(scm);

	if (!SUCCESS)
		throw gcException(ERR_SERVICE, GetLastError(), gcString("Failed to delete service: {0}", szName));
}






SERVICE_STATUS_E queryService(const char* szName)
{
	SC_HANDLE service;
	SC_HANDLE scm;
	BOOL SUCCESS;
	SERVICE_STATUS status;

	gcWString wName(szName);

	//Open connection to SCM
	scm = OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT);
	if (!scm)
		throw gcException(ERR_NULLSCMANAGER, GetLastError(), gcString("Failed to open the Service Control Manager"));

	//Get service's handle
	service = OpenService(scm, wName.c_str(), SERVICE_QUERY_STATUS);
	if (!service)
	{
		CloseServiceHandle(scm);
		return SERVICE_STATUS_NOTINSTALLED;
	}

	//Get service status
	SUCCESS	= QueryServiceStatus(service, &status);
	if (!SUCCESS)
	{
		CloseServiceHandle(service);
		CloseServiceHandle(scm);
		return SERVICE_STATUS_NOTINSTALLED;
	}

	switch (status.dwCurrentState)
	{
	case SERVICE_CONTINUE_PENDING: return SERVICE_STATUS_CONTINUE_PENDING;
	case SERVICE_PAUSE_PENDING	: return SERVICE_STATUS_PAUSE_PENDING;
	case SERVICE_PAUSED			: return SERVICE_STATUS_PAUSED;
	case SERVICE_START_PENDING	: return SERVICE_STATUS_START_PENDING;
	case SERVICE_RUNNING		: return SERVICE_STATUS_RUNNING;
	case SERVICE_STOP_PENDING	: return SERVICE_STATUS_STOP_PENDING;
	case SERVICE_STOPPED		: return SERVICE_STATUS_STOPPED;
	}

	return SERVICE_STATUS_UNKNOWN;
}




}
}