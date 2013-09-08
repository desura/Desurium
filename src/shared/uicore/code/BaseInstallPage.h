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

#ifndef DESURA_BASEINSTALLPAGE_H
#define DESURA_BASEINSTALLPAGE_H
#ifdef _WIN32
#pragma once
#endif

#include "BasePage.h"
#include "usercore/ItemHandleI.h"

class ItemHandleHelper;

namespace UI
{
namespace Forms
{
namespace ItemFormPage
{

class BaseInstallPage : public ::BasePage 
{
public:
	BaseInstallPage(wxWindow* parent);
	~BaseInstallPage();

	virtual void init()=0;

	virtual void onComplete(uint32& status);
	virtual void onProgressUpdate(uint32& progress);
	virtual void onError(gcException& e);

	virtual void onMcfProgress(MCFCore::Misc::ProgressInfo& info);
	virtual void onComplete(gcString& str);
	virtual void onNewItem(gcString& item);
	virtual void onItemFound(DesuraId& id);
	virtual void onDownloadProvider(UserCore::Misc::GuiDownloadProvider& provider);

	virtual void onItemUpdate(UserCore::Item::ItemInfoI::ItemInfo_s& info);
	virtual void onVerifyComplete(UserCore::Misc::VerifyComplete& info);
	virtual void onPause(bool &state);

	void setInfo(DesuraId id);

	void pause(bool state);
	void nonBlockStop();
	void stop();
	
	void registerHandle();
	void deregisterHandle();	

protected:
	UserCore::Item::ItemHandleI* getItemHandle()
	{
		return m_pItemHandle;
	};

	virtual void run()
	{
	}

	virtual void dispose()
	{
	}

private:
	ItemHandleHelper* m_pIHH;
	UserCore::Item::ItemHandleI* m_pItemHandle;
};

}
}
}


#endif //DESURA_BASEINSTALLPAGE_H
