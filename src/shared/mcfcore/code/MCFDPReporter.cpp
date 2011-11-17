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
#include "MCFDPReporter.h"

namespace MCFCore
{


DPReproter g_DPReporter;

DPReproter* GetDPReporter()
{
	return &g_DPReporter;
}




DPReproter::DPReproter() : BaseManager<DPProvider>(true)
{
	m_uiLastId = 0;
	m_uiTotal = 0;
}

uint32 DPReproter::getProviderCount()
{
	return getCount();
}

uint32 DPReproter::getProviderId(uint32 index)
{
	m_MapLock.lock();
	DPProvider* item = getItem(index);
	m_MapLock.unlock();

	if (item)
		return item->getId();

	return -1;
}

void DPReproter::getName(uint32 id, char* buff, uint32 size)
{
	m_MapLock.lock();
	DPProvider* item = findItem(id);
	m_MapLock.unlock();

	if (item)
	{
		const char* name = item->getName();
		Safe::strncpy(buff, size, name, size);
	}
}

uint32 DPReproter::getLastRate(uint32 id)
{
	m_MapLock.lock();
	DPProvider* item = findItem(id);
	m_MapLock.unlock();

	if (item)
		return item->getLastRate();

	return -1;
}

void DPReproter::reportProgress(uint32 id, uint32 prog)
{
	m_uiTotal += prog;

	m_MapLock.lock();
	DPProvider* item = findItem(id);
	m_MapLock.unlock();

	if (item)
		item->reportProgress(prog);
}

uint32 DPReproter::newProvider(const char* name)
{
	uint32 id = m_uiLastId;
	m_uiLastId++;

	m_MapLock.lock();
	addItem(new DPProvider(name, id));
	m_MapLock.unlock();

	return id;
}

void DPReproter::delProvider(uint32 id)
{
	m_MapLock.lock();
	removeItem(id, true);
	m_MapLock.unlock();
}

uint64 DPReproter::getTotalSinceStart()
{
	return m_uiTotal;
}

void DPReproter::resetStart()
{
	m_uiTotal = 0;
}























DPProvider::DPProvider(const char* name, uint32 id) : BaseItem()
{
	m_uiId = id;
	m_uiHash = id;
	m_szName = gcString(name);
	
	m_tStart = bpt::ptime(bpt::microsec_clock::universal_time());

	clear();
}

DPProvider::~DPProvider()
{
}

void DPProvider::clear()
{
	m_fAmmount = 0;
	
}

uint32 DPProvider::getId()
{
	return m_uiId;
}

const char* DPProvider::getName()
{
	return m_szName.c_str();
}


void DPProvider::reportProgress(uint32 prog)
{
	m_fAmmount += prog;
	m_tLastUpdate = bpt::ptime(bpt::microsec_clock::universal_time());
}

uint32 DPProvider::getLastRate()
{
	if (m_fAmmount < 1)
		return 0;

	bpt::ptime curTime(bpt::microsec_clock::universal_time());
	bpt::time_duration total = curTime - m_tStart;

	if (total.total_seconds() == 0)
		return 0;

	double avgRate = m_fAmmount / ((double)total.total_seconds());
	return (uint32)(avgRate/1024);
}




}
