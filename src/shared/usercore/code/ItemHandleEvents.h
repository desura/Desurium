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

#ifndef DESURA_ITEMHANDLEEVENTS_H
#define DESURA_ITEMHANDLEEVENTS_H
#ifdef _WIN32
#pragma once
#endif

#include "ItemHandle.h"
#include "usercore/ItemHelpersI.h"

namespace UserCore
{
namespace Item
{

class EventItemI;

class ItemHandleEvents
{
public:
	ItemHandleEvents(std::vector<Helper::ItemHandleHelperI*> &vHelperList);
	~ItemHandleEvents();

	void registerTask(UserCore::ItemTask::BaseItemTask* task);
	void deregisterTask(UserCore::ItemTask::BaseItemTask* task);

	void postAll(Helper::ItemHandleHelperI* helper);

	void reset();
	void onPause(bool state);

protected:
	void onComplete(uint32& status);
	void onProgressUpdate(uint32& progress);
	void onError(gcException& e);
	void onNeedWildCard(WCSpecialInfo& info);

	void onMcfProgress(MCFCore::Misc::ProgressInfo& info);
	void onComplete(gcString& str);

	void onDownloadProvider(UserCore::Misc::GuiDownloadProvider& provider);
	void onVerifyComplete(UserCore::Misc::VerifyComplete& info);

private:
	MCFCore::Misc::ProgressInfo m_LastProg;

	std::vector<EventItemI*> m_EventHistory;
	std::vector<Helper::ItemHandleHelperI*> &m_vHelperList;
};

}
}

#endif //DESURA_ITEMHANDLEEVENTS_H
