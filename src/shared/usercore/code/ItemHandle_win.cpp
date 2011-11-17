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
#include "ItemHandle.h"

#include "User.h"

DesuraId g_ElevatedGames [] = 
{
	DesuraId(196, DesuraId::TYPE_GAME),
	DesuraId(),
};




namespace UserCore
{
namespace Item
{


void ItemHandle::doLaunch(Helper::ItemLaunchHelperI* helper)
{
	preLaunchCheck();
	
	bool needElevation = false;

	size_t x=0;
	while (g_ElevatedGames[x].isOk())
	{
		if (getItemInfo()->getId() == g_ElevatedGames[x] || getItemInfo()->getParentId() == g_ElevatedGames[x])
		{
			needElevation = true;
			break;
		}

		x++;
	}
	
	UserCore::Item::Misc::ExeInfoI* ei = getItemInfo()->getActiveExe();

	gcString args;
	gcString ea(ei->getExeArgs());
	gcString ua(ei->getUserArgs());
	
	if (ea.size() > 0)
		args += " " + ea;

	if (ua.size() > 0)
		args += " " + ua;

	m_pUserCore->getItemManager()->setRecent(getItemInfo()->getId());

	bool res = UTIL::WIN::launchExe(ei->getExe(), args.c_str(), needElevation, m_pUserCore->getMainWindowHandle());

	if (!res)
		throw gcException(ERR_LAUNCH, GetLastError(), gcString("Failed to create {0} process. [{1}: {2}].\n", getItemInfo()->getName(), GetLastError(), ei->getExe()));
}

}
}
