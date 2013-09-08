/*
Desura is the leading indie game distribution platform
Copyright (C) Desura Ltd. <support@desura.com>

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

#ifndef DESURA_BASEITEMTASK_H
#define DESURA_BASEITEMTASK_H
#ifdef _WIN32
#pragma once
#endif

#include "usercore/ItemHelpersI.h"
#include "mcfcore/MCFMain.h"
#include "mcfcore/MCFI.h"

namespace WebCore
{
	class WebCoreI;
}

namespace UserCore
{
	class UserI;

	namespace Item
	{
		class ItemHandle;
		class ItemInfo;
	}

namespace ItemTask
{

class BaseItemTask
{
public:
	BaseItemTask(uint32 type, const char* name, UserCore::Item::ItemHandle* handle, MCFBranch branch = MCFBranch(), MCFBuild build = MCFBuild());
	virtual ~BaseItemTask();

	void setWebCore(WebCore::WebCoreI *wc);
	void setUserCore(UserCore::UserI *uc);

	virtual void onStop();
	virtual void onPause();
	virtual void onUnpause();
	virtual void doTask();

	virtual void cancel();

	Event<uint32> onCompleteEvent;
	Event<uint32> onProgUpdateEvent;
	Event<gcException> onErrorEvent;
	Event<WCSpecialInfo> onNeedWCEvent;

	//download mcf
	Event<UserCore::Misc::GuiDownloadProvider> onNewProviderEvent;
	Event<UserCore::Misc::VerifyComplete> onVerifyCompleteEvent;
	Event<MCFCore::Misc::ProgressInfo> onMcfProgressEvent;
	Event<gcString> onCompleteStrEvent;

	const char* getTaskName();
	uint32 getTaskType();

	UserCore::Item::ItemHandle* getItemHandle();

protected:
	virtual void doRun()=0;

	UserCore::Item::ItemInfo* getItemInfo();
	UserCore::Item::ItemInfo* getParentItemInfo();

	DesuraId getItemId();

	WebCore::WebCoreI* getWebCore();
	UserCore::UserI* getUserCore();

	MCFBuild getMcfBuild();
	MCFBranch getMcfBranch();

	bool isStopped();
	bool isPaused();

	McfHandle m_hMCFile;

	MCFBranch m_uiMcfBranch;
	MCFBuild m_uiMcfBuild;

private:
	volatile bool m_bIsStopped;
	volatile bool m_bIsPaused;

	UserCore::Item::ItemHandle* m_pHandle;
	WebCore::WebCoreI* m_pWebCore;
	UserCore::UserI* m_pUserCore;

	uint32 m_uiType;
	gcString m_szName;
};

}
}


#endif //DESURA_BASEITEMTASK_H
