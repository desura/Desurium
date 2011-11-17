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
#include "ComplexLaunchProcess.h"
#include "McfInit.h"

#include "mcfcore/MCFMain.h"
#include "InstallScriptRunTime.h"

enum
{
	MODE_UNKNOWN,
	MODE_INSTALL,
	MODE_REMOVE,
};

ComplexLaunchProcess::ComplexLaunchProcess() : Thread::BaseThread("Complex Launch Process Thread")
{
	m_iMode = MODE_UNKNOWN;
	m_uiLastProg = 0;
	m_bHashMissMatch=false;
}

ComplexLaunchProcess::~ComplexLaunchProcess()
{

}

void ComplexLaunchProcess::setRemove(const char* modMcfPath, const char* backupMcfPath, const char* installPath, const char* installScript)
{
	m_iMode = MODE_REMOVE;
	m_szIPath = gcString(installPath);
	m_szRemMCFPath = gcString(modMcfPath);
	m_szInsMCFPath = gcString(backupMcfPath);
	m_szInstallScript = installScript;
}

void ComplexLaunchProcess::setInstall(const char* mcfPath, const char* installPath, const char* installScript)
{
	m_iMode = MODE_INSTALL;
	m_szIPath = gcString(installPath);
	m_szInsMCFPath = gcString(mcfPath);
	m_szInstallScript = installScript;
}

void ComplexLaunchProcess::run()
{
	switch (m_iMode)
	{ 
		case MODE_REMOVE: 
			doRemove(); 
			break;

		case MODE_INSTALL: 
			doInstall(); 
			break;
	}
}


void ComplexLaunchProcess::doRemove()
{
	if (!UTIL::FS::isValidFile(m_szRemMCFPath.c_str()) || !UTIL::FS::isValidFolder(m_szIPath.c_str()))
	{
		gcException errInvPath(ERR_BADPATH, gcString("One of the paths for the complex remove was invalid [IP: {0} RMCF: {1}].", m_szIPath, m_szRemMCFPath));
		onErrorEvent(errInvPath);
		return;
	}

	m_iFirstStage = true;

	McfHandle mcfHandle;
	mcfHandle->getErrorEvent() += delegate(&onErrorEvent);
	mcfHandle->getProgEvent() += delegate(this, &ComplexLaunchProcess::onProgress);

	InstallScriptRunTime isrt(m_szInstallScript.c_str(), m_szIPath.c_str());

	try
	{
		mcfHandle->setFile(m_szRemMCFPath.c_str());
		mcfHandle->parseMCF();

		isrt.run("PreUninstall");
		mcfHandle->removeFiles(m_szIPath.c_str(), true);
		isrt.run("PostUninstall");
	}
	catch (gcException except)
	{
		onErrorEvent(except);
		return;
	}

	m_iFirstStage = false;

	mcfHandle = McfHandle();
	mcfHandle->getErrorEvent() += delegate(&onErrorEvent);
	mcfHandle->getProgEvent() += delegate(this, &ComplexLaunchProcess::onProgress);

	if (UTIL::FS::isValidFile(m_szInsMCFPath.c_str()))
	{
		try
		{
			mcfHandle->setFile(m_szInsMCFPath.c_str());
			mcfHandle->parseMCF();

			isrt.run("PreInstall");
			mcfHandle->saveFiles(m_szIPath.c_str());
			isrt.run("PostInstall");
		}
		catch (gcException &except)
		{
			onErrorEvent(except);
			return;
		}
	}

	onCompleteEvent();
}

void ComplexLaunchProcess::doInstall()
{
	UTIL::FS::makeFolder(m_szIPath);

	if (!UTIL::FS::isValidFile(m_szInsMCFPath))
	{
		gcException errInvPath(ERR_BADPATH, gcString("One of the paths for the complex install was invalid [IMCF: {0}].", m_szInsMCFPath));
		onErrorEvent(errInvPath);
		return;
	}

	McfHandle mcfHandle;
	mcfHandle->getErrorEvent() += delegate(this, &ComplexLaunchProcess::onError);
	mcfHandle->getProgEvent() += delegate(this, &ComplexLaunchProcess::onProgress);

	InstallScriptRunTime isrt(m_szInstallScript.c_str(), m_szIPath.c_str());

	try
	{
		mcfHandle->setFile(m_szInsMCFPath.c_str());
		mcfHandle->parseMCF();

		isrt.run("PreInstall");
		mcfHandle->saveFiles(m_szIPath.c_str());
		isrt.run("PostInstall");
	}
	catch (gcException &except)
	{
		onErrorEvent(except);
		return;
	}

	gcException errMismatch(ERR_HASHMISSMATCH);

	if (m_bHashMissMatch)
		onErrorEvent(errMismatch);

	onCompleteEvent();
}	

void ComplexLaunchProcess::onStop()
{

}

void ComplexLaunchProcess::onProgress(MCFCore::Misc::ProgressInfo& p)
{
	if (m_iMode == MODE_REMOVE)
	{
		if (m_iFirstStage)
			p.percent = p.percent/2;
		else
			p.percent = 50+p.percent/2;
	}

	uint64 val = p.toInt64();

	if (val == m_uiLastProg)
		return;

	onProgressEvent(val);
	m_uiLastProg = val;
}

void ComplexLaunchProcess::onError(gcException& e)
{
	if (e.getErrId() == ERR_HASHMISSMATCH)
		m_bHashMissMatch = true;
	else
		onErrorEvent(e);
}
