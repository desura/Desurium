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

#ifndef DESURA_VSBASETASK_H
#define DESURA_VSBASETASK_H
#ifdef _WIN32
#pragma once
#endif

#include "ItemHandle.h"
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


class VSBaseTask
{
public:
	VSBaseTask();

	enum RESULT
	{
		RES_NONE,
		RES_COMPLETE,
		RES_DOWNLOAD,
		RES_INSTALL,
		RES_DOWNLOADMISSING,
		RES_STOPPED,
	};

	RESULT getResult();
	void stop();

	Event<gcException> onErrorEvent;

	void setWebCore(WebCore::WebCoreI *wc);
	void setUserCore(UserCore::UserI *uc);

	void setItemHandle(UserCore::Item::ItemHandle* handle);
	void setMcfBuild(MCFBuild build);
	void setMcfBranch(MCFBranch branch);

	void setMcfHandle(MCFCore::MCFI* handle);

	virtual void onProgress(MCFCore::Misc::ProgressInfo& prog){}

protected:
	DesuraId getItemId();

	UserCore::Item::ItemHandle* getItemHandle();
	UserCore::Item::ItemInfo* getItemInfo();
	UserCore::Item::ItemInfo* getParentItemInfo();

	WebCore::WebCoreI* getWebCore();
	UserCore::UserI* getUserCore();

	MCFBuild getMcfBuild();
	MCFBranch getMcfBranch();

	bool isStopped();

	virtual void onStop(){;}

	void setResult(RESULT res);
	bool loadMcf(gcString mcfPath);

	MCFCore::MCFI* m_hMcf;

private:
	volatile bool m_bIsStopped;

	UserCore::Item::ItemHandle* m_pHandle;
	WebCore::WebCoreI* m_pWebCore;
	UserCore::UserI* m_pUserCore;

	MCFBranch m_uiMcfBranch;
	MCFBuild m_uiMcfBuild;

	RESULT m_Result;
};

}
}

#endif //DESURA_VSBASETASK_H
