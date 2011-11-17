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
#include "InstallCheckTask.h"
#include "managers/WildcardManager.h"

namespace UserCore
{
namespace ItemTask
{

InstallCheckTask::InstallCheckTask(UserCore::Item::ItemHandle *handle) : BaseItemTask(UserCore::Item::ItemHandleI::STAGE_INSTALL_CHECK, "InstallCheck", handle)
{
}

void InstallCheckTask::doRun()
{
	WildcardManager wildc = WildcardManager();
	wildc.onNeedSpecialEvent += delegate(&onNeedWCEvent);
	wildc.onNeedSpecialEvent += delegate(getUserCore()->getNeedWildCardEvent());

	getUserCore()->getItemManager()->retrieveItemInfo(getItemId(), 0, &wildc);

	uint32 prog = 0;
	onCompleteEvent(prog);

	getItemHandle()->completeStage(false);
}

}
}