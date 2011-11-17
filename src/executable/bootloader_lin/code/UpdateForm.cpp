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
#include "UpdateForm.h"
#include "files.h"

UpdateForm::UpdateForm()
{
	ERROR_OUTPUT(__func__);
	m_pUmcf = NULL;
	m_bCanceled = false;
}

UpdateForm::~UpdateForm()
{
	ERROR_OUTPUT(__func__);
	Destroy(false);
}

void UpdateForm::downloadAndInstall()
{
	ERROR_OUTPUT(__func__);
	
	UMcfEx updateMcf;
	m_pUmcf = &updateMcf;

	updateMcf.onProgressEvent += delegate(this, &UpdateForm::onProgressN);
	updateMcf.onDownloadProgressEvent += delegate(this, &UpdateForm::onProgressD);

	if (FileExists(UPDATEXML))
		DeleteFile(UPDATEXML);

	updateMcf.getUpdateInfo(true);
	updateMcf.checkMcf();

	SetStatus("Downloading");
	
	updateMcf.setFile(UPDATEFILE_W);
	m_bInstalling = false;
	updateMcf.downloadMcf();
	
	SetStatus("Installing");
	m_bInstalling = true;
	updateMcf.install(L"./");
	updateMcf.setRegValues();
	m_bInstalling = false;
	
	updateMcf.deleteMcf();
	
	m_pUmcf = NULL;
}

void UpdateForm::installOnly()
{
	ERROR_OUTPUT(__func__);
	
	SetStatus("Receiving Info");
		
	UMcf installMcf;
	m_pUmcf = &installMcf;
	
	installMcf.setFile(UPDATEFILE_W);
	installMcf.parseMCF();
	installMcf.onProgressEvent += delegate(this, &UpdateForm::onProgressN);

	//make sure patch is newer than currently installed version
	if (!installMcf.isUpdateNewer())
	{
		printf("Update is not newer than whats currently installed. Skipping...\n");
		return;
	}
	
	if (!installMcf.isValidInstaller())
		throw gcException(ERR_INVALIDFILE, gcString("The MCF file '{0}' is not a valid installer.", UPDATEFILE_W));

	std::wstring xmlPath(UPDATEXML_W);
	
	UMcf oldMcf;
	oldMcf.loadFromFile(xmlPath.c_str());

	DeleteFile(xmlPath.c_str());
	SetStatus("Installing");
	
	installMcf.install(L"./");
	installMcf.dumpXml(UPDATEXML_W);
	installMcf.setRegValues();
	
	installMcf.removeOldFiles(&oldMcf, L"./");

	m_pUmcf = NULL;
}

void UpdateForm::onProgressN(unsigned int& prog)
{
	if (!IsRunning())
		return;
	
	SetPercentage(prog);
}

void UpdateForm::onCancel()
{
	if (m_pUmcf)
		m_pUmcf->cancel();
		
	m_bCanceled = true;
}

void UpdateForm::onProgressD(Prog_s& info)
{
	info.abort = m_bCanceled;
	
	if(!IsRunning())
		return;

	uint64 done = (uint32)info.dlnow; //+= (uint64)size;
	uint32 per = 0;

	if (done > 0)
		per = (uint32)(done*100/info.dltotal);

	SetPercentage(per);
	SetCurrent((float)info.dlnow);
	SetTotal((float)info.dltotal);
}

