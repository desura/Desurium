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
#include "GatherInfoThread.h"
#include "managers/WildcardManager.h"

#include "ItemInfo.h"
#include "ItemHandle.h"

namespace UserCore
{
namespace Thread
{

GatherInfoThread::GatherInfoThread(DesuraId id, MCFBranch branch, MCFBuild build) : MCFThread( "GatherInfo Thread", id, branch, build )
{
}


void GatherInfoThread::doRun()
{
	UserCore::Item::ItemInfo* item;

	WildcardManager wildc = WildcardManager();
	wildc.onNeedSpecialEvent += delegate(&onNeedWCEvent);

	uint32 prog = 0;
	onProgUpdateEvent(prog);
	getUserCore()->getItemManager()->retrieveItemInfo(getItemId(), 0, &wildc, MCFBranch::BranchFromInt(getMcfBranch()), MCFBuild::BuildFromInt(getMcfBuild()));

	if (isStopped())
		return;

	item = getItemInfo();

	if (!item)
		throw gcException(ERR_INVALIDDATA, "The item handle was null (gather info failed)");

	uint32 itemId = item->getId().getItem();
	onCompleteEvent(itemId);
}


}
}
