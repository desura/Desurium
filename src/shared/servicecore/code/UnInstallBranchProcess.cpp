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
#include "UnInstallBranchProcess.h"
#include "McfInit.h"
#include "InstallScriptRunTime.h"

UninstallBranchProcess::UninstallBranchProcess(const char* oldBranchMcf, const char* newBranchMcf, const char* inspath, const char* installScript) : Thread::BaseThread("UninstallBranch Thread")
{
	m_szOldMcfPath = oldBranchMcf;
	m_szNewMcfPath = newBranchMcf;
	m_szInstallPath = inspath;
	m_szInstallScript = installScript;

	m_pOldBranchMcf = NULL;
	m_pNewBranchMcf = NULL;

	m_uiLastProg = 0;
}

UninstallBranchProcess::~UninstallBranchProcess()
{
	stop();

	if (m_pOldBranchMcf)
		mcfDelFactory(m_pOldBranchMcf);

	if (m_pNewBranchMcf)
		mcfDelFactory(m_pNewBranchMcf);
}


void UninstallBranchProcess::run()
{
	if (m_szOldMcfPath == "" || m_szNewMcfPath == "")
	{
		onFinishEvent();
		return;
	}

	MCFCore::MCFI *oldMcf = mcfFactory();
	m_pOldBranchMcf = oldMcf;

	MCFCore::MCFI *newMcf = mcfFactory();
	m_pNewBranchMcf = newMcf;

	oldMcf->setFile(m_szOldMcfPath.c_str());
	newMcf->setFile(m_szNewMcfPath.c_str());

	InstallScriptRunTime isrt(m_szInstallScript.c_str(), m_szInstallPath.c_str());

	try
	{
		oldMcf->parseMCF();
		newMcf->parseMCF();

		oldMcf->markChanged(newMcf);

		oldMcf->getErrorEvent() += delegate(this, &UninstallBranchProcess::onError);
		oldMcf->getProgEvent()  += delegate(this, &UninstallBranchProcess::onProgress);

		isrt.run("PreUninstall");
		oldMcf->removeFiles(m_szInstallPath.c_str(), false);
		isrt.run("PostUninstall");
	}
	catch (gcException &)
	{
	}

	m_pOldBranchMcf=NULL;
	m_pNewBranchMcf=NULL;

	mcfDelFactory(newMcf);
	mcfDelFactory(oldMcf);
	
	onCompleteEvent();
	onFinishEvent();
}

void UninstallBranchProcess::onStop()
{
	if (m_pNewBranchMcf)
		m_pNewBranchMcf->stop();
}

void UninstallBranchProcess::onPause()
{
	if (m_pNewBranchMcf)
		m_pNewBranchMcf->pause();
}

void UninstallBranchProcess::onUnpause()
{
	if (m_pNewBranchMcf)
		m_pNewBranchMcf->unpause();
}


void UninstallBranchProcess::onError(gcException& e)
{
	onErrorEvent(e);
}

void UninstallBranchProcess::onProgress(MCFCore::Misc::ProgressInfo& p)
{
	uint64 val = p.toInt64();

	if (val == m_uiLastProg)
		return;

	onProgressEvent(val);
	m_uiLastProg = val;
}
