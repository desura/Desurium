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
#include "ToolTransaction.h"

#if defined NIX || defined MACOS
#include "ToolInstallThread.h"
#endif

namespace UserCore
{
namespace Misc
{

ToolTransInfo::ToolTransInfo(bool download, Misc::ToolTransaction* transaction, ToolManager* pToolManager)
{
	m_bIsDownload = download;
	m_pTransaction = transaction;
	m_uiCompleteCount = 0;

	m_pToolManager = pToolManager;

	if (m_bIsDownload)
	{
		for (size_t x=0; x<transaction->toolsList.size(); x++)
		{
			UserCore::Misc::ToolProgress tp;

			tp.done= 0;
			tp.percent = 0;
			tp.total = 0;
				
			ToolInfo* info = pToolManager->findItem(transaction->toolsList[x].toInt64());

			if (info)
				tp.total = info->getDownloadSize();

			m_vProgress.push_back(tp);
		}
	}
}

ToolTransInfo::~ToolTransInfo()
{
	safe_delete(m_pTransaction);
}

void ToolTransInfo::removeItem(DesuraId id)
{
	size_t i = getIndex(id);

	if (i == UINT_MAX)
		return;


	if (m_vProgress.size() >= i+1)
		m_vProgress.erase(m_vProgress.begin()+i);

	m_pTransaction->toolsList.erase(m_pTransaction->toolsList.begin()+i);

	if (m_uiCompleteCount == m_pTransaction->toolsList.size())
		m_pTransaction->onCompleteEvent();
}

void ToolTransInfo::onDLComplete(DesuraId id)
{
	size_t i = getIndex(id);

	if (i == UINT_MAX)
		return;

	m_vProgress[i].done = m_vProgress[i].total;
	m_vProgress[i].percent = 100;

	m_uiCompleteCount++;

	if (m_uiCompleteCount == m_pTransaction->toolsList.size())
		m_pTransaction->onCompleteEvent();
}

void ToolTransInfo::onDLError(DesuraId id, gcException e)
{
	size_t i = getIndex(id);

	if (i == UINT_MAX)
		return;

	m_pTransaction->onErrorEvent(e);
	m_uiCompleteCount++;

	if (m_uiCompleteCount == m_pTransaction->toolsList.size())
		m_pTransaction->onCompleteEvent();
}

void ToolTransInfo::onDLProgress(DesuraId id, UserCore::Misc::ToolProgress &prog)
{
	size_t i = getIndex(id);

	if (i == UINT_MAX)
		return;

	m_vProgress[i] = prog;

	UserCore::Misc::ToolProgress tp;

	for (size_t x=0; x<m_vProgress.size(); x++)
	{
		tp.done += m_vProgress[x].done;
		tp.percent += m_vProgress[x].percent;
		tp.total += m_vProgress[x].total;
	}

	tp.percent /= m_pTransaction->toolsList.size();
	m_pTransaction->onProgressEvent(tp);
}

bool ToolTransInfo::isDownload()
{
	return m_bIsDownload;
}

void ToolTransInfo::getIds(std::vector<DesuraId> &idList)
{
	if (m_pTransaction)
		idList = m_pTransaction->toolsList;
}

bool ToolTransInfo::startNextInstall(IPCToolMain* pToolMain, DesuraId &toolId)
{
	if (m_uiCompleteCount == m_pTransaction->toolsList.size())
		return false;

	toolId = m_pTransaction->toolsList[m_uiCompleteCount];
	ToolInfo* info = m_pToolManager->findItem(toolId.toInt64());

	if (!info)
		return true;

	if (info->isInstalled())
	{
		m_uiCompleteCount++;
		return startNextInstall(pToolMain, toolId);
	}

	DesuraId id = info->getId();
	m_pTransaction->onStartInstallEvent(id);

	gcException e = pToolMain->installTool(info);

#ifdef NIX
	if (e.getErrId() == ERR_COMPLETED)
	{
		onINComplete();
		return startNextInstall(pToolMain, toolId);
	}
#endif

	if (e.getErrId() != WARN_OK && e.getErrId() != ERR_UNKNOWNERROR)
	{
		onINError(e);
		return false;
	}
	else
	{
		UserCore::Misc::ToolProgress prog;

		prog.done = m_uiCompleteCount;
		prog.total = m_pTransaction->toolsList.size();
		prog.percent = prog.done*100/prog.total;

		m_pTransaction->onProgressEvent(prog);
	}
	
	return true;
}

void ToolTransInfo::startingIPC()
{
	m_pTransaction->onStartIPCEvent();
}

void ToolTransInfo::onINComplete()
{
	m_uiCompleteCount++;

	UserCore::Misc::ToolProgress prog;

	prog.done = m_uiCompleteCount;
	prog.total = m_pTransaction->toolsList.size();
	prog.percent = prog.done*100/prog.total;

	m_pTransaction->onProgressEvent(prog);

	if (m_uiCompleteCount == m_pTransaction->toolsList.size())
		m_pTransaction->onCompleteEvent();		
}

void ToolTransInfo::onINError(gcException &e)
{
	Warning(gcString("Tool install error: {0}\n",e));

	m_uiCompleteCount = m_pTransaction->toolsList.size();
	m_pTransaction->onErrorEvent(e);
}

void ToolTransInfo::updateTransaction(Misc::ToolTransaction* pTransaction)
{
	if (!m_pTransaction)
		return;

	m_pTransaction->onStartInstallEvent.reset();
	m_pTransaction->onCompleteEvent.reset();
	m_pTransaction->onErrorEvent.reset();
	m_pTransaction->onProgressEvent.reset();
	m_pTransaction->onStartIPCEvent.reset();

	if (!pTransaction)
		return;

	m_pTransaction->onStartInstallEvent = pTransaction->onStartInstallEvent;
	m_pTransaction->onCompleteEvent = pTransaction->onCompleteEvent;
	m_pTransaction->onErrorEvent = pTransaction->onErrorEvent;
	m_pTransaction->onProgressEvent = pTransaction->onProgressEvent;
	m_pTransaction->onStartIPCEvent = pTransaction->onStartIPCEvent;
}


size_t ToolTransInfo::getIndex(DesuraId id)
{
	for (size_t x=0; x<m_pTransaction->toolsList.size(); x++)
	{
		if (m_pTransaction->toolsList[x] == id)
		{
			return x;
		}
	}

	return -1;
}

}
}
