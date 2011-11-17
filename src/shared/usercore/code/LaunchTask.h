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

#ifndef DESURA_LAUNCHTASK_H
#define DESURA_LAUNCHTASK_H
#ifdef _WIN32
#pragma once
#endif


#include "util_thread/BaseThread.h"
#include "usercore/ItemInfoI.h"
#include "User.h"


#include "BaseItemTask.h"

namespace UserCore
{
namespace ItemTask
{

//! Install check thread sees if an item is installed on the local computer
//!
class LaunchTask : public BaseItemTask
{
public:
	LaunchTask(UserCore::Item::ItemHandle *handle)  : BaseItemTask(UserCore::Item::ItemHandleI::STAGE_LAUNCH, "Launch", handle)
	{
	}

protected:
	void doRun()
	{
		getItemHandle()->completeStage(true);
	}
};

}
}

#endif //DESURA_LAUNCHTASK_H
