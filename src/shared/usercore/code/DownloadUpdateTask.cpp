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
#include "DownloadUpdateTask.h"

#include "mcfcore/MCFI.h"
#include "mcfcore/MCFMain.h"

#include "XMLMacros.h"
#include "User.h"

#include "IPCServiceMain.h"

#include <branding/branding.h>

namespace UserCore
{
namespace Task
{

DownloadUpdateTask::DownloadUpdateTask(UserCore::User* user, uint32 appver, uint32 build) : UserTask(user)
{
	m_uiAppVer = appver;
	m_uiAppBuild = build;
	m_uiLastPercent = -1;

	m_bCompleted = false;
	m_bStopped = false;
}

void DownloadUpdateTask::doTask()
{
	try
	{
		downloadUpdate();
		m_bCompleted = true;
	}
	catch (gcException &except)
	{
		Warning(gcString("Failed to download " PRODUCT_NAME " Update: {0}.\n", except));
	}

	if (!m_bStopped)
	{
		uint32 prog = 0;
		onDownloadProgressEvent(prog);		
	}

	onDownloadCompleteEvent.reset();
	onDownloadStartEvent.reset();
}


void DownloadUpdateTask::onStop()
{
	m_bStopped = true;

	if (m_hMcfHandle.handle())
		m_hMcfHandle->stop();
	
	//if we didnt finish delete the incomplete file
	if (!m_bCompleted)
		UTIL::FS::delFile(UTIL::FS::PathWithFile(m_szPath));
}


//this is a feature of webcore but we need it here in case we need to shutdown during a download
void DownloadUpdateTask::downloadUpdate()
{
#ifdef DEBUG
	return;
#endif

	uint32 appver = m_uiAppVer;
	uint32 appbuild = m_uiAppBuild;


	HttpHandle wc(PRIMUPDATE);
	getWebCore()->setWCCookies(wc);
	
	wc->addPostText("appid", appver);
	wc->postWeb();


	if (wc->getDataSize() == 0)
		throw gcException(ERR_INVALIDDATA);


	TiXmlDocument doc;
	XML::loadBuffer(doc, const_cast<char*>(wc->getData()), wc->getDataSize());

	XML::processStatus(doc, "appupdate");

	TiXmlNode *uNode = doc.FirstChild("appupdate");

	if (!uNode)
		throw gcException(ERR_BADXML);

	TiXmlNode *mNode = uNode->FirstChild("mcf");

	if (!mNode)
		throw gcException(ERR_BADXML);

	uint32 version = 0;
	uint32 build = 0;
	gcString url;

	if (mNode->ToElement())
	{
		const char* szId = mNode->ToElement()->Attribute("appid");

		if (szId)
			version = atoi(szId);

		const char* szBuild = mNode->ToElement()->Attribute("build");
		
		if (szBuild)
			build = atoi(szBuild);
	}

	if (version == 0 || build == 0)
		throw gcException(ERR_BADXML);

	//check to see if its newer than last
	if (appbuild != 0 && build <= appbuild && appver == version)
	{
		return;
	}

	XML::GetChild("url", url, mNode);

	if (url.size() == 0)
		throw gcException(ERR_BADXML);

#ifdef WIN32
	const char *comAppPath = getUserCore()->getAppDataPath();
	m_szPath = gcString("{0}{1}{2}", comAppPath, DIRS_STR, UPDATEFILE);
#else
	m_szPath = gcString("{0}", UPDATEFILE);
#endif

	m_hMcfHandle->setFile(m_szPath.c_str());
	m_hMcfHandle->setWorkerCount(1);
	
	try
	{
		m_hMcfHandle->dlHeaderFromHttp(url.c_str());

#ifdef WIN32
		const char* dir = ".\\";
#else
		const char* dir = "./";
#endif

		//Dont use courgette for non admins for now
		bool useCourgette = false;

#ifdef WIN32
		if (getUserCore()->isAdmin())
			useCourgette = true;
#endif

		bool res = m_hMcfHandle->verifyInstall(dir, true, useCourgette);

		m_hMcfHandle->getProgEvent() += delegate(this, &DownloadUpdateTask::onDownloadProgress);

		UserCore::Misc::update_s info;
		info.build = build;

		if (!res)
		{
			Msg(gcString("Downloading " PRODUCT_NAME " update: Ver {0} build {1}\n", appver, build));
			info.alert = true;

			onDownloadStartEvent(info);

			if (useCourgette)
				m_hMcfHandle->dlFilesFromHttp(url.c_str(), dir);
			else
				m_hMcfHandle->dlFilesFromHttp(url.c_str());

			if (!isStopped())
				onDownloadCompleteEvent(info);
		}
		else
		{
			//sometimes this gets called after shutdown and causes major problems
			if (!getUserCore() || !getUserCore()->getServiceMain())
				return;

			gcString av("{0}", appver);
			gcString ab("{0}", build);
			info.alert = false;

			try
			{
				getUserCore()->getServiceMain()->updateRegKey(APPID, av.c_str());
				getUserCore()->getServiceMain()->updateRegKey(APPBUILD, ab.c_str());
			}
			catch (gcException &e)
			{
				Warning(gcString("Failed to update reg key: {0}\n", e));
			}

			onDownloadCompleteEvent(info);
		}
	}
	catch (gcException &e)
	{
		Warning(gcString("Download update failed: {0}\n", e));
	}
}

void DownloadUpdateTask::onDownloadProgress(MCFCore::Misc::ProgressInfo& p)
{
	uint32 prog = p.percent;
	
	if (m_uiLastPercent == prog)
		return;
		
	m_uiLastPercent = prog;
	onDownloadProgressEvent(prog);
}

}
}
