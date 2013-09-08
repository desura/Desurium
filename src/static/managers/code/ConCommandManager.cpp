/*
Desura is the leading indie game distribution platform
Copyright (C) Desura Ltd. <support@desura.com>
          (C) Karol Herbst <git@karolherbst.de>

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
#include "ConCommandManager.h"

ConsoleCommandManager* g_pConComMang = NULL;


void InitConComManger()
{
	if (!g_pConComMang)
		g_pConComMang = new ConsoleCommandManager();
}

void DestroyConComManager()
{
	safe_delete(g_pConComMang);
}


ConsoleCommandManager::ConsoleCommandManager() : BaseManager<ConCommand>()
{

}

ConsoleCommandManager::~ConsoleCommandManager()
{
}

bool ConsoleCommandManager::RegCCom(ConCommand* var)
{
	ConCommand* temp = findItem(var->getName());
	
	if (temp)
		return false;

	addItem(var);
	return true;
}

//if this screws up its too late any way
void  ConsoleCommandManager::UnRegCCom(ConCommand* var)
{
	removeItem(var->getName());
}

ConCommand* ConsoleCommandManager::findCCommand(const char* name)
{
	return findItem(name);
}

void ConsoleCommandManager::getConCommandList(std::vector<ConCommand*> &vList)
{
	for (uint32 x=0; x<getCount(); x++)
		vList.push_back(getItem(x));
}
