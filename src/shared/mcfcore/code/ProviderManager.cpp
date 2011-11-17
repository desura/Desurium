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
#include "ProviderManager.h"

namespace MCFCore
{
namespace Misc
{

const uint32 g_uiTimeOutDelay = 15;

class ErrorInfo
{
public:
	ErrorInfo()
	{
		m_uiErrCount = 0;
		m_iOwner = -1;
		m_tExpTime = ptime(second_clock::universal_time());
	}

	void setTimeOut()
	{
		m_iOwner = -1;
		m_tExpTime = ptime(second_clock::universal_time()) + seconds( getTimeOut() );
		m_uiErrCount++;
	}

	bool isInTimeOut()
	{
		return (m_tExpTime > ptime(second_clock::universal_time()));
	}

	void setOwner(uint32 id)
	{
		m_iOwner = id;
	}

	uint32 getOwner()
	{
		return m_iOwner;
	}

protected:
	uint32 getTimeOut()
	{
		return (g_uiTimeOutDelay * m_uiErrCount);
	}

private:
	uint32 m_uiErrCount;
	uint32 m_iOwner;
	ptime m_tExpTime;
};


ProviderManager::ProviderManager(std::vector<MCFCore::Misc::DownloadProvider*> &source) : m_vSourceList(source)
{
	for (size_t x=0; x<m_vSourceList.size(); x++)
	{
		m_vErrorList.push_back(new ErrorInfo());
	}

}

ProviderManager::~ProviderManager()
{
	safe_delete(m_vErrorList);
}

gcString ProviderManager::getUrl(uint32 id)
{
	gcString url("NULL");
	std::vector<uint32> validList;

	m_WaitMutex.lock();

	for (size_t x=0; x<m_vErrorList.size(); x++)
	{
		if (!m_vErrorList[x]->isInTimeOut() && m_vErrorList[x]->getOwner() == UINT_MAX)
			validList.push_back(x);
	}

	//always take from the top of the list as they are prefered servers
	if (validList.size() > 0)
	{
		m_vErrorList[validList[0]]->setOwner(id);
		url = gcString(m_vSourceList[validList[0]]->getUrl());

		MCFCore::Misc::DP_s dp;
		dp.action = MCFCore::Misc::DownloadProvider::ADD;
		dp.provider = m_vSourceList[validList[0]];
		onProviderEvent(dp);
	}

	m_WaitMutex.unlock();

	return url;
}

gcString ProviderManager::getName(uint32 id)
{
	gcString name;

	for (size_t x=0; x<m_vSourceList.size(); x++)
	{
		if (!m_vSourceList[x])
			continue;

		if (id == m_vErrorList[x]->getOwner())
		{
			name = gcString(m_vSourceList[x]->getName());
		}
	}

	return name;
}

gcString ProviderManager::requestNewUrl(uint32 id, uint32 errCode, const char* errMsg)
{
	Warning(gcString("Mcf download thread errored out. Id: {0}, Error: {2} [{1}]\n", id, errCode, errMsg));

	removeAgent(id, true);
	return getUrl(id);
}

void ProviderManager::removeAgent(uint32 id, bool setTimeOut)
{
	m_WaitMutex.lock();

	for (size_t x=0; x<m_vSourceList.size(); x++)
	{
		if (!m_vSourceList[x])
			continue;

		if (id == m_vErrorList[x]->getOwner())
		{
			if (setTimeOut)
				m_vErrorList[x]->setTimeOut();
			else
				m_vErrorList[x]->setOwner(-1);

			MCFCore::Misc::DP_s dp;
			dp.action = MCFCore::Misc::DownloadProvider::REMOVE;
			dp.provider = m_vSourceList[x];
			onProviderEvent(dp);
		}
	}

	m_WaitMutex.unlock();
}

bool ProviderManager::hasValidAgents()
{
	bool res = false;

	m_WaitMutex.lock();

	for (size_t x=0; x<m_vSourceList.size(); x++)
	{
		if (!m_vSourceList[x])
			continue;

		if (!m_vErrorList[x]->isInTimeOut())
		{
			res = true;
			break;
		}
	}

	m_WaitMutex.unlock();

	return res;
}

}
}
