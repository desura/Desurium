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

#include <limits.h>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#ifdef BUILD_WITH_CMAKE
  #include <google_breakpad/exception_handler.h>
#else
  #include "client/linux/handler/exception_handler.h"
#endif

#include "../../../branding/branding.h"

google_breakpad::ExceptionHandler* MiniDumpGenerator::s_pExceptionHandler = NULL;

bool CompleteCallback(const char* dump_path, const char* minidump_id, void* context, bool succeeded)
{
	MiniDumpGenerator* mdg = (MiniDumpGenerator*)context;
	return mdg->complete(dump_path, minidump_id, succeeded);	
}

MiniDumpGenerator::MiniDumpGenerator()
{
	m_szUser = 0;
	m_bNoUpload = false;
	m_bCreatedHandle = false;

	if (s_pExceptionHandler)
		return;

	m_bCreatedHandle = true;

	s_pExceptionHandler = new google_breakpad::ExceptionHandler("/tmp", NULL, &CompleteCallback, this, true);
	m_pCrashCallback = NULL;
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

void MiniDumpGenerator::setUser(const char* user)
{
	if (m_szUser)
		delete [] m_szUser;

	if (!user)
	{
		m_szUser = NULL;
	}
	else
	{
		m_szUser = new char[255];
		strncpy(m_szUser, user, 255);
	}
}

bool MiniDumpGenerator::complete(const char* dump_path, const char* minidump_id, bool succeeded)
{
	char fullPath[PATH_MAX] = {0};
	snprintf(fullPath, PATH_MAX, "/tmp/%s.dmp", minidump_id);

	if (dumpreport(fullPath) == false && m_bShowMsgBox && succeeded)
	{
		char msg[1024];

		snprintf(msg, 1024, "An unrecoverable error has occurred.\n"
											"\n"
											"For some reason the normal crashdump tool could not be used to upload the\n"
											"report. Please use the dumpgen utility (in the 'bin' directory) to upload\n"
											"the dump and then relaunch " PRODUCT_NAME ".\n"
											"\n"
											"The information in the dump will help the developers fix the issue in \n"
											"future releases. If this issue is recurring or severe, please email\n"
											"support@desura.com and let us know.");
		fprintf(stderr, "%s\n", msg);
	}

	return succeeded;
}

bool MiniDumpGenerator::dumpreport(const char* file)
{
	if (m_pCrashCallback)
		return m_pCrashCallback(file);
		
	return false;
}

void MiniDumpGenerator::setCrashCallback(CrashCallbackFn callback)
{
	m_pCrashCallback = callback;
}
