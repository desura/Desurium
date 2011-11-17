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

#include "stdafx.h"
#define UTILWEB_INCLUDE

#include "Common.h"
#include "UMcfEx.h"

#include "UpdateMFCForm.h"
#include "AppUpdateInstall.h"

#include <process.h>
#include "updateFunctions.h"
#include <shlobj.h>



static int DispUpdateError(UpdateForm* temp, gcException &e)
{
	char msg[255];
	Safe::snprintf(msg, 255, "Failed to update Desura: %s [%d.%d]", e.getErrMsg(), e.getErrId(), e.getSecErrId());
	::MessageBox(temp->GetSafeHwnd(), msg, "Desura: ERROR", MB_OK);

	exit(-2);
	return e.getErrId();
}


UINT DownloadAndInstallMCF(UpdateForm* temp)
{
	UMcfEx *updateMcf = new UMcfEx();
	updateMcf->onProgressEvent += delegate(temp, &UpdateForm::onProgressN);
	updateMcf->onDownloadProgressEvent += delegate(temp, &UpdateForm::onProgressD);

	DeleteFile(UPDATEXML);

	std::wstring path = UTIL::OS::getAppDataPath(L"dumps\0\0");

	SHFILEOPSTRUCTW lpFileOp;
	lpFileOp.wFunc = FO_DELETE;
	lpFileOp.pFrom = path.c_str();
	lpFileOp.pTo = NULL;
	lpFileOp.hwnd = NULL;
	lpFileOp.fFlags = (FOF_SILENT | FOF_NOCONFIRMATION | FOF_NOERRORUI | FOF_NOCONFIRMMKDIR);
	SHFileOperationW(&lpFileOp);

#ifndef DEBUG
	temp->getProgressControl()->setMode(MODE_DOWNLOADING);

	try
	{
		updateMcf->getUpdateInfo(true);
	}
	catch (gcException &e)
	{
		return DispUpdateError(temp, e);
	}
	catch (...)
	{
		::MessageBox(temp->GetSafeHwnd(), "Failed to get update info.", "Desura: ERROR", MB_OK);
		return 1;
	}

	if (!updateMcf->checkMcf())
	{
		char msg[255];
		Safe::snprintf(msg, 255, "Failed to update Desura: %s", updateMcf->getLastError());
		::MessageBox(temp->GetSafeHwnd(), msg, "Desura: ERROR", MB_OK);

		exit(-3);
		return 1;
	}

	try
	{
		updateMcf->setFile(UPDATEFILE_W);
		updateMcf->downloadMcf();

		temp->getProgressControl()->setMode(MODE_INSTALLING);

		updateMcf->install(L".\\");
	}
	catch (gcException &e)
	{
		return DispUpdateError(temp, e);
	}

#ifndef DEBUG
	try
	{
		updateMcf->setRegValues();
	}
	catch (gcException &e)
	{
		return DispUpdateError(temp, e);
	}
#endif

	updateMcf->deleteMcf();
	safe_delete(updateMcf);

#else
	updateMcf->setFile(UPDATEFILE_W);
	updateMcf->parseMCF();
	updateMcf->install(L".\\test\\");
#endif

	return 0;
}



UINT DownloadFilesForTest()
{
	UMcfEx updateMcf;
	DeleteFile(UPDATEXML);

	try
	{
		updateMcf.getUpdateInfo(true);

		if (!updateMcf.checkMcf())
			return -2;

		updateMcf.setFile(UPDATEFILE_W);
		updateMcf.downloadMcf();
		updateMcf.install(L".\\");
	}
	catch (gcException &e)
	{
		return e.getErrId();
	}
	catch (...)
	{
		return -1;
	}

	return 0;
}

UINT InstallFilesForTest()
{
	UMcf updateMcf;

	try
	{
		updateMcf.setFile(UPDATEFILE_W);
		
		if (updateMcf.parseMCF() != 0)
			return -1;

		updateMcf.isValidInstaller();

		AppUpdateInstall aui(NULL, true);
		int res = aui.run();

		if (res != 0)
			return res;

		if (!updateMcf.checkFiles())
			return -2;
	}
	catch (gcException &e)
	{
		return e.getErrId();
	}
	catch (...)
	{
		return -3;
	}

	return 0;
}