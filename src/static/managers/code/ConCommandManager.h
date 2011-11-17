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

#ifndef DESURA_CONVAR_MANAGER_H
#define DESURA_CONVAR_MANAGER_H
#ifdef _WIN32
#pragma once
#endif

#include "managers/ConCommand.h"
#include "managers/Managers.h"

void InitConComManger();
void DestroyConComManager();

class ConsoleCommandManager : public BaseManager<ConCommand>, public CCommandManagerI
{
public:
	ConsoleCommandManager();
	~ConsoleCommandManager();

	bool RegCCom(ConCommand* var);
	void UnRegCCom(ConCommand* var);

	virtual ConCommand* findCCommand(const char* name);
	virtual void getConCommandList(std::vector<ConCommand*> &vList);
};

extern ConsoleCommandManager* g_pConComMang;

#endif //DESURA_CONVAR_MANAGER_H
