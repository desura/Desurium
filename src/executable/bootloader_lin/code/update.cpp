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

#include "update.h"

#include "main.h"
#include "files.h"
#include "umcf/UMcf.h"
#include "UpdateForm.h"

#define UPDATE_LOCK ".update_lock.txt"

const char* g_UpdateReasons[] =
{
	"None",
	"MCF Update",
	"Files",
	"XML",
	"Service",
	"Cert",
	"Forced",
	"Service path needs updating",
	"Service needs change of location",
	"User data path is incorrect",
	"Service hash is bad",
	NULL
};

#ifdef DESURA_NONGPL_BUILD
bool CheckForUpdates()
{
	ERROR_OUTPUT(__func__);
	int nu = NeedUpdate();

	if (nu != UPDATE_NONE)
	{
		if (nu == UPDATE_MCF)
		{
			ERROR_OUTPUT("Updating from MCF.");
			int ret = McfUpdate();	
			
			// desura_update.mcf
			if (FileExists(UPDATEFILE))
				DeleteFile(UPDATEFILE);
				
			if(ret != ERR_USERCANCELED)
				RestartBootloader();
				
			return false;
		}
		else
		{
			ERROR_OUTPUT("Doing FULL update.");
			int ret = FullUpdate();
			
			// desura_update.mcf
			if (FileExists(UPDATEFILE))
				DeleteFile(UPDATEFILE);

			if(ret != ERR_USERCANCELED) // if they didn't cancel
				RestartBootloader();
			return false;
		}
	} 
	else 
	{
		ERROR_OUTPUT("Not doing update.");
	}
	
	return true;
}

int NeedUpdate()
{
	ERROR_OUTPUT(__func__);

	if (!FileExists(UPDATEXML))
	{
		ERROR_OUTPUT("UPDATE_XML");
		return UPDATE_XML;
	}
	else
	{
		if (!CheckInstall())
		{
			ERROR_OUTPUT("UPDATE_FILES");
			return UPDATE_FILES;
		}
		
		if (FileExists(UPDATEFILE) && CheckUpdate(UPDATEFILE))
		{
			ERROR_OUTPUT("UPDATE_MCF");
			return UPDATE_MCF;
		}
	}

	return UPDATE_NONE;
}

bool CheckUpdate(const char* path)
{
	ERROR_OUTPUT(__func__);
	wchar_t wPath[PATH_MAX];

	if (mbstowcs(wPath, path, PATH_MAX) == UINT_MAX)
	{
		ERROR_OUTPUT("Couldn't convert multibyte string to wide-character array!");
		return false;
	}

	UMcf updateMcf;
	updateMcf.setFile(wPath);

	uint8 res = updateMcf.parseMCF();

	if (res != UMCF_OK)
	{
		ShowHelpDialog(gcString("Failed to parse MCF! [{0}]. Deleting file to allow a fresh update.", res).c_str());
		// desura_update.mcf
		if (FileExists(UPDATEFILE))
			DeleteFile(UPDATEFILE);	
		
		return false;		
	}

	if (!updateMcf.isValidInstaller())
	{
		ShowHelpDialog("Error: Current MCF not a valid installer! Deleting file to allow a fresh update.");
		// desura_update.mcf
		if (FileExists(UPDATEFILE))
			DeleteFile(UPDATEFILE);
		return false;		
	}
	
	return true;
}

bool CheckInstall()
{
	ERROR_OUTPUT(__func__);
	UMcf updateMcf;
	updateMcf.loadFromFile(UPDATEXML_W);
	
	return updateMcf.checkFiles();
}

#else

bool CheckForUpdates()
{
	return false;
}

int NeedUpdate()
{
	return UPDATE_NONE;
}

bool CheckUpdate(const char* path)
{
	return false;
}

bool CheckInstall()
{
	return true;
}

#endif


int FullUpdate()
{
	ERROR_OUTPUT(__func__);

	// desura_update.mcf
	if (FileExists(UPDATEFILE))
		DeleteFile(UPDATEFILE);
		
	if (FileExists(UPDATE_LOCK))
	{
		ERROR_OUTPUT("It looks like Desura tried to update and failed.");
		DeleteFile(UPDATE_LOCK);

		return 2;
	}

	FILE* fh = Safe::fopen(UPDATE_LOCK, "wb");

	if (fh)
		fclose(fh);

	int res = DownloadAndInstallMCF();
	
	// .desura_lock
	if (FileExists(UPDATE_LOCK))
		DeleteFile(UPDATE_LOCK);
	// desura_update.mcf
	if (FileExists(UPDATEFILE))
		DeleteFile(UPDATEFILE);
	
	//UpdateIcons(true);

	return res;
}

int McfUpdate()
{
	ERROR_OUTPUT(__func__);
	UpdateForm up;
	up.Show();
	
	try
	{
		up.installOnly();
	}
	catch (gcException &e)
	{
		if (up.isCanceled())
		{
			ShowHelpDialog("Update was cancelling while installing. Desura is now in an inconsistent state and will need to finish updating before it can be used");
		}
		else
		{
			ShowHelpDialog(gcString("Install Desura update failed: {0}", e).c_str());
			DeleteFile(UPDATEFILE);
		}
		
		// .desura_lock
		if (FileExists(UPDATE_LOCK))
			DeleteFile(UPDATE_LOCK);
			
		return e.getErrId();
	}
	
	// Make sure the files are gone after success
	// .desura_lock
	if (FileExists(UPDATE_LOCK))
		DeleteFile(UPDATE_LOCK);
	// desura_update.mcf
	if (FileExists(UPDATEFILE))
		DeleteFile(UPDATEFILE);
	
	return 0;
}

int DownloadAndInstallMCF()
{	
	ERROR_OUTPUT(__func__);
	UpdateForm up;
	up.Show();
	
	try
	{
		up.downloadAndInstall();
	}
	catch (gcException &e)
	{
		if (!up.isCanceled())
			ShowHelpDialog(gcString("Download and install Desura failed: {0}", e).c_str());
		else
			DeleteFile(UPDATEFILE);

		if (up.wasInstalling())
		{
			ShowHelpDialog("Update was cancelling while installing. Desura is now in an inconsistent state and will need to finish updating before it can be used");
			DeleteFile(UPDATEXML);
		}
		
		return e.getErrId();
	}
	
	// Make sure the files are gone after success
	// .desura_lock
	if (FileExists(UPDATE_LOCK))
		DeleteFile(UPDATE_LOCK);
	// desura_update.mcf
	if (FileExists(UPDATEFILE))
		DeleteFile(UPDATEFILE);
	
	return 0;
}

int InstallFilesForTest()
{
	UMcf updateMcf;
	UpdateForm up;

	try
	{
		updateMcf.setFile(UPDATEFILE_W);
		
		if (updateMcf.parseMCF() != 0)
			return -1;

		updateMcf.isValidInstaller();

		up.installOnly();

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

int DownloadFilesForTest()
{
	UMcfEx updateMcf;
	DeleteFile(UPDATEXML);

	try
	{
		updateMcf.getUpdateInfo(true);
		updateMcf.checkMcf();
		updateMcf.setFile(UPDATEFILE_W);
		updateMcf.downloadMcf();
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

