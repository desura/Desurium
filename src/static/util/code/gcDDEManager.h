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

#ifndef DESURA_GCDDEMANAGER_H
#define DESURA_GCDDEMANAGER_H
#ifdef _WIN32
#pragma once
#endif

#include "Common.h"
#include "util/gcDDE.h"

#include <set>

class gcDDEManager
{
public:
	gcDDEManager();
	~gcDDEManager();
		
	void init();
	void cleanUp();

	gcDDEServer* findServer(const char* name);
	gcDDEConnection* findConnection(HCONV conv);

	void addServer(gcDDEServer* server);
	void addClient(gcDDEClient* client);

	void removeServer(gcDDEServer* server);
	void removeClient(gcDDEClient* client);

	DWORD getDDEInst(){return m_hDDEIdInst;}

	HDDEDATA processCallBack(WORD wType, WORD wFmt, HCONV hConv, HSZ hsz1, HSZ hsz2, HDDEDATA hData, DWORD lData1, DWORD lData2);

	HSZ atomFromString(const char* string);
	void stringFromAtom(HSZ atom, char* str, size_t size);

	gcDDEConnection *m_pCurConnecting;

private:
	bool m_bDDEInit;
	DWORD m_hDDEIdInst;

	std::vector<gcDDEClient*> m_vClient;
	std::vector<gcDDEServer*> m_vServer;
	std::set<HSZ> m_vAtomList;
};

extern gcDDEManager* GetDDEManager();

#endif //DESURA_GCDDEMANAGER_H
