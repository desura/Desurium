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
#include "DownloadToolTask.h"

#include "ToolInfo.h"
#include "User.h"

namespace UserCore
{
namespace Task
{


DownloadToolTask::DownloadToolTask(UserCore::User* user, UserCore::ToolInfo* tool) : UserTask(user)
{
	m_pTool = tool;
	
	if (tool)
	{
		m_Path = tool->getPathFromUrl(user->getAppDataPath());
		UTIL::FS::recMakeFolder(m_Path);
	}

	m_uiRefCount = 1;
	m_uiPercent = 0;
	m_bStopped = false;

	m_pHttpHandle = NULL;
}

DownloadToolTask::~DownloadToolTask()
{
}

void DownloadToolTask::finish()
{
	if (!m_bStopped)
	{
		m_pTool->setExePath(m_Path.getFullPath().c_str());
		onCompleteEvent();
	}
	else
	{
		gcException e(ERR_USERCANCELED);
		onErrorEvent(e);
		UTIL::FS::delFile(m_Path);
	}
}

void DownloadToolTask::doTask()
{
	try
	{
		downloadTool();
		finish();
	}
	catch (gcException &except)
	{
		onErrorEvent(except);
	}
}

void DownloadToolTask::onStop()
{
	m_bStopped = true;
	
	if (m_pHttpHandle)
		m_pHttpHandle->abortTransfer();
}

void DownloadToolTask::downloadTool()
{
	m_fhFile.open(m_Path, UTIL::FS::FILE_WRITE);

	HttpHandle hh(m_pTool->getUrl());

	m_pHttpHandle = hh.operator->();

	hh->getProgressEvent() += delegate(this, &DownloadToolTask::onProgress);
	hh->getWriteEvent() += delegate(this, &DownloadToolTask::onWrite);
	
	hh->setUserAgent(getUserCore()->getWebCore()->getUserAgent());
	hh->getWeb();

	m_pHttpHandle = NULL;
	m_fhFile.close();
}

void DownloadToolTask::onProgress(Prog_s& prog)
{
	UserCore::Misc::ToolProgress p;

	p.done = (uint32)prog.dlnow;
	p.total = (uint32)prog.dltotal;
	p.percent = (uint32)(prog.dlnow * 100.0 / prog.dltotal);

	onProgressEvent(p);
	m_uiPercent = p.percent;
}

void DownloadToolTask::onWrite(WriteMem_s& mem)
{
	try
	{
		m_fhFile.write((char*)mem.data, mem.size);
		mem.wrote = mem.size;
	}
	catch (gcException& e)
	{
		onErrorEvent(e);
		mem.stop = true;
	}

	mem.handled = true;
}


uint32 DownloadToolTask::getRefCount()
{
	return m_uiRefCount;
}

void DownloadToolTask::increseRefCount()
{
	m_uiRefCount++;
}

void DownloadToolTask::decreaseRefCount(bool forced)
{
	m_uiRefCount--;

	if (m_uiRefCount == 0)
	{
		if (m_uiPercent < 75 || forced)
			onStop();
	}
}

}
}
