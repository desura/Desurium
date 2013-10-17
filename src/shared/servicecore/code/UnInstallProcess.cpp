/*
Desura is the leading indie game distribution platform
Copyright (C) Desura Ltd. <support@desura.com>

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
#include "UnInstallProcess.h"
#include "McfInit.h"
#include "InstallScriptRunTime.h"


UninstallProcess::UninstallProcess(const char* mcfpath, const char* inspath, const char* installScript) : Thread::BaseThread("UninstallProcess Thread")
{
	m_szMCFPath = gcString(mcfpath);
	m_szIPath = gcString(inspath);
	m_szInstallScript = installScript;

	m_pMcfHandle = NULL;
	m_uiLastProg = 0;
}


UninstallProcess::~UninstallProcess()
{
	if (m_pMcfHandle)
		mcfDelFactory(m_pMcfHandle);
}

void UninstallProcess::run()
{
	if (m_szIPath == "" || m_szMCFPath == "")
	{
		gcException errPathNull(ERR_BADPATH, gcString("One of the paths for uninstall was NULL [IP: {0} MCF: {1}].", m_szIPath, m_szMCFPath));
		onErrorEvent(errPathNull);
		return;
	}

	MCFCore::MCFI *mcfHandle = mcfFactory();
	m_pMcfHandle = mcfHandle;
	mcfHandle->getErrorEvent() += delegate(&onErrorEvent);
	mcfHandle->getProgEvent() += delegate(this, &UninstallProcess::onProgress);

	InstallScriptRunTime isrt(m_szInstallScript.c_str(), m_szIPath.c_str());

	try
	{
		mcfHandle->setFile(m_szMCFPath.c_str());
		mcfHandle->parseMCF();

		isrt.run("PreUninstall");
		mcfHandle->removeFiles(m_szIPath.c_str());
		isrt.run("PostUninstall");
	}
	catch (gcException &except)
	{
		onErrorEvent(except);
	}

	m_pMcfHandle=NULL;
	mcfDelFactory(mcfHandle);

	onCompleteEvent();
}

void UninstallProcess::onStop()
{
	if (m_pMcfHandle)
		m_pMcfHandle->stop();
}

void UninstallProcess::onProgress(MCFCore::Misc::ProgressInfo& p)
{
	uint64 val = p.toInt64();

	if (val == m_uiLastProg)
		return;

	onProgressEvent(val);
	m_uiLastProg = val;
}
