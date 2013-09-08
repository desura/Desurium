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
#include "VSCheckMcf.h"
#include "McfManager.h"

#include "ItemHandle.h"
#include "ItemInfo.h"
#include "usercore/UserCoreI.h"
#include "webcore/WebCoreI.h"

#include "mcfcore/UserCookies.h"

namespace UserCore
{
namespace ItemTask
{

bool VSCheckMcf::doTask(bool &goodMcf)
{
	UserCore::MCFManager *mm = UserCore::GetMCFManager();
	gcString filePath = mm->getMcfPath(getItemId(), getMcfBranch(), getMcfBuild());



	if (!checkMcf(filePath))
	{
		UTIL::FS::delFile(filePath);
		filePath = downloadMCFHeader();
	}

	if (!checkMcf(filePath))
		return false;

	goodMcf = false;


	m_hMcf->setFile(filePath.c_str());

	if (isStopped())
		return false;

	try
	{
		m_hMcf->parseMCF();

		bool passedVerify = m_hMcf->verifyMCF();

		if (m_hMcf->isComplete())
			goodMcf = passedVerify;

		m_hMcf->saveMCFHeader();
	}
	catch (gcException &)
	{
	}

	return true;
}

void VSCheckMcf::onStop()
{
	if (m_hTempMcf.handle())
		m_hTempMcf->stop();
}

gcString VSCheckMcf::downloadMCFHeader()
{
	if (isStopped())
		return "";

	UserCore::MCFManager *mm = UserCore::GetMCFManager();
	gcString path = mm->getMcfPath(getItemId(), getMcfBranch(), getMcfBuild());

	if (path == "")
		path = mm->newMcfPath(getItemId(), getMcfBranch(), getMcfBuild());

	m_hTempMcf->setHeader(getItemId(), getMcfBranch(), getMcfBuild());
	

	try
	{
		MCFCore::Misc::UserCookies uc;
		getWebCore()->setMCFCookies(&uc); 

		m_hTempMcf->getDownloadProviders(getWebCore()->getMCFDownloadUrl(), &uc);
		m_hTempMcf->dlHeaderFromWeb();
	}
	catch (gcException &except)
	{
		onErrorEvent(except);
		return "";
	}

	m_hTempMcf->setFile(path.c_str());
	m_hTempMcf->markFiles(m_hTempMcf.handle(), false, false, false, false);
	m_hTempMcf->saveMCFHeader();

	if (isStopped())
		return "";

	m_hTempMcf = McfHandle();

	return path;
}

bool VSCheckMcf::checkMcf(gcString path)
{
	if (isStopped())
		return false;

	if (!UTIL::FS::isValidFile(UTIL::FS::PathWithFile(path)))
		return false;

	//read file to make sure it is what it says it is
	m_hTempMcf->setFile(path.c_str());
		
	try
	{
		m_hTempMcf->parseMCF();
	}
	catch (gcException &except)
	{
		Warning(gcString("Verify - MCF Error: {0}\n", except));
		return false;
	}

	if (isStopped())
		return false;

	MCFCore::MCFHeaderI *mcfHead = m_hTempMcf->getHeader();

	bool res = (mcfHead && mcfHead->getBuild() == getMcfBuild());

	m_hTempMcf = McfHandle();
	return res;
}

}
}
